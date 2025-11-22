#include "nour/nour.h"
#include "cumulative.h"
#include "../niter.h"
#include "../nerror.h"
#include "../node_core.h"
#include "../tc_methods.h"
#include "../nfunc.h"
#include "../free.h"
#include "loops.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================================
 * NaN Checking Utilities
 * ============================================================================ */

#define ISNAN_F32(x) isnan((double)(x))
#define ISNAN_F64(x) isnan(x)
#define ISNAN_INT(x) (0)

/* ============================================================================
 * Cumulative Operations
 * ============================================================================ */

#define OP_SUM(acc, val)  ((acc) + (val))
#define OP_PROD(acc, val) ((acc) * (val))
#define OP_MIN(acc, val)  (((val) < (acc)) ? (val) : (acc))
#define OP_MAX(acc, val)  (((val) > (acc)) ? (val) : (acc))

/* ============================================================================
 * Helper: Cumulative Args Constructor
 * ============================================================================ */

NR_PUBLIC NFunc_CumArgs
NFunc_CumArgs_New(int axis)
{
    NFunc_CumArgs args = {0};
    args.axis = axis;
    return args;
}

/* ============================================================================
 * Helper: Normalize axis (handle negative)
 * ============================================================================ */

NR_PRIVATE int
normalize_axis(int axis, int ndim)
{
    if (axis < 0) axis += ndim;
    if (axis < 0 || axis >= ndim) {
        return -1;
    }
    return axis;
}

/* ============================================================================
 * Index Conversion Helpers
 * ============================================================================ */

NR_STATIC_INLINE void
linear_to_coords(nr_intp linear, const nr_intp* shape, int ndim, nr_intp* coords)
{
    for (int d = ndim - 1; d >= 0; d--) {
        coords[d] = linear % shape[d];
        linear /= shape[d];
    }
}

NR_STATIC_INLINE nr_intp
coords_to_linear(const nr_intp* coords, const nr_intp* shape, int ndim)
{
    nr_intp linear = 0, mult = 1;
    for (int d = ndim - 1; d >= 0; d--) {
        linear += coords[d] * mult;
        mult *= shape[d];
    }
    return linear;
}

NR_STATIC_INLINE nr_intp
coords_to_offset(const nr_intp* coords, const nr_intp* strides, int ndim)
{
    nr_intp offset = 0;
    for (int d = 0; d < ndim; d++) {
        offset += coords[d] * strides[d];
    }
    return offset;
}

/* ============================================================================
 * Input/Output Value Access Macros
 * ============================================================================ */

#define READ_VAL(NT, data, coords, node, is_contig, linear) \
    (is_contig ? ((NT*)data)[linear] : \
     *(NT*)((char*)data + coords_to_offset(coords, node->strides, node->ndim)))

#define WRITE_VAL(NT, data, coords, node, is_contig, linear, val) \
    do { \
        if (is_contig) { \
            ((NT*)data)[linear] = val; \
        } else { \
            *(NT*)((char*)data + coords_to_offset(coords, node->strides, node->ndim)) = val; \
        } \
    } while(0)

/* ============================================================================
 * Output Node Setup (common to all cumulative operations)
 * ============================================================================ */

#define SETUP_CUM_OUTPUT(O_NT, PROM_O_DT) \
    Node* n1 = args->in_nodes[0]; \
    Node* caller_out = args->out_nodes[0]; \
    NFunc_CumArgs* cargs = (NFunc_CumArgs*)args->extra; \
    \
    int axis = cargs ? cargs->axis : -1; \
    if (axis == -1) axis = n1->ndim - 1; \
    axis = normalize_axis(axis, n1->ndim); \
    if (axis < 0) { \
        NError_RaiseError(NError_ValueError, \
            "axis %d out of bounds for array of dimension %d", \
            cargs ? cargs->axis : -1, n1->ndim); \
        return -1; \
    } \
    \
    Node* out = NULL; \
    if (!caller_out || NODE_DTYPE(caller_out) != PROM_O_DT) { \
        out = Node_NewEmpty(n1->ndim, n1->shape, PROM_O_DT); \
        if (!out) return -1; \
    } else { \
        out = caller_out; \
        if (out->ndim != n1->ndim) { \
            NError_RaiseError(NError_ValueError, "output array has wrong ndim"); \
            return -1; \
        } \
        for (int i = 0; i < n1->ndim; ++i) { \
            if (out->shape[i] != n1->shape[i]) { \
                NError_RaiseError(NError_ValueError, "output array has wrong shape"); \
                return -1; \
            } \
        } \
    } \
    \
    O_NT* out_data = (O_NT*)NODE_DATA(out); \
    nr_intp n_items = Node_NItems(n1); \
    int in_contig = NODE_IS_CONTIGUOUS(n1); \
    int out_contig = NODE_IS_CONTIGUOUS(out); \
    (void)n_items; (void)in_contig; (void)out_contig;

#define FINALIZE_CUM_OUTPUT() \
    if (!caller_out) { \
        args->out_nodes[0] = out; \
    } else if (out != caller_out) { \
        Node_ToType(caller_out, out, NODE_DTYPE(caller_out)); \
        NODE_DECREF(out); \
    } \
    return 0;

/* ============================================================================
 * BASIC CUMULATIVE KERNEL (cumsum, cumprod, cummin, cummax)
 * ============================================================================ */

#define DEFINE_CUM_KERNEL(OP_NAME, OP_FUNC, O_NT, I_NT, INIT_VAL, NEEDS_FIRST, PROM_O_DT) \
NR_PRIVATE int OP_NAME##_kernel_##I_NT(NFuncArgs* args) { \
    SETUP_CUM_OUTPUT(O_NT, PROM_O_DT) \
    I_NT* in_data = (I_NT*)NODE_DATA(n1); \
    \
    /* Calculate strides for iteration */ \
    nr_intp axis_len = n1->shape[axis]; \
    nr_intp n_slices = n_items / axis_len; \
    \
    /* Pre-compute stride multipliers for axis traversal */ \
    nr_intp inner_size = 1; \
    for (int d = axis + 1; d < n1->ndim; d++) { \
        inner_size *= n1->shape[d]; \
    } \
    \
    /* Iterate over all slices perpendicular to axis */ \
    for (nr_intp slice = 0; slice < n_slices; slice++) { \
        /* Compute base coordinates for this slice */ \
        nr_intp outer_idx = slice / inner_size; \
        nr_intp inner_idx = slice % inner_size; \
        \
        nr_intp base_coords[NR_NODE_MAX_NDIM]; \
        nr_intp temp = outer_idx; \
        for (int d = axis - 1; d >= 0; d--) { \
            base_coords[d] = temp % n1->shape[d]; \
            temp /= n1->shape[d]; \
        } \
        temp = inner_idx; \
        for (int d = n1->ndim - 1; d > axis; d--) { \
            base_coords[d] = temp % n1->shape[d]; \
            temp /= n1->shape[d]; \
        } \
        \
        /* Cumulative operation along axis */ \
        O_NT acc = (O_NT)(INIT_VAL); \
        int first = (NEEDS_FIRST); \
        \
        for (nr_intp i = 0; i < axis_len; i++) { \
            base_coords[axis] = i; \
            nr_intp lin = coords_to_linear(base_coords, n1->shape, n1->ndim); \
            \
            I_NT in_val = READ_VAL(I_NT, in_data, base_coords, n1, in_contig, lin); \
            \
            if (first) { \
                acc = (O_NT)in_val; \
                first = 0; \
            } else { \
                acc = OP_FUNC(acc, (O_NT)in_val); \
            } \
            \
            WRITE_VAL(O_NT, out_data, base_coords, out, out_contig, lin, acc); \
        } \
    } \
    \
    FINALIZE_CUM_OUTPUT() \
}

/* ============================================================================
 * NAN-IGNORING CUMULATIVE KERNEL (nancumsum, nancumprod)
 * ============================================================================ */

#define DEFINE_NANCUM_KERNEL(OP_NAME, OP_FUNC, O_NT, I_NT, INIT_VAL, NEEDS_FIRST, PROM_O_DT, ISNAN_MACRO) \
NR_PRIVATE int OP_NAME##_kernel_##I_NT(NFuncArgs* args) { \
    SETUP_CUM_OUTPUT(O_NT, PROM_O_DT) \
    I_NT* in_data = (I_NT*)NODE_DATA(n1); \
    \
    nr_intp axis_len = n1->shape[axis]; \
    nr_intp n_slices = n_items / axis_len; \
    \
    nr_intp inner_size = 1; \
    for (int d = axis + 1; d < n1->ndim; d++) { \
        inner_size *= n1->shape[d]; \
    } \
    \
    for (nr_intp slice = 0; slice < n_slices; slice++) { \
        nr_intp outer_idx = slice / inner_size; \
        nr_intp inner_idx = slice % inner_size; \
        \
        nr_intp base_coords[NR_NODE_MAX_NDIM]; \
        nr_intp temp = outer_idx; \
        for (int d = axis - 1; d >= 0; d--) { \
            base_coords[d] = temp % n1->shape[d]; \
            temp /= n1->shape[d]; \
        } \
        temp = inner_idx; \
        for (int d = n1->ndim - 1; d > axis; d--) { \
            base_coords[d] = temp % n1->shape[d]; \
            temp /= n1->shape[d]; \
        } \
        \
        O_NT acc = (O_NT)(INIT_VAL); \
        int first = (NEEDS_FIRST); \
        \
        for (nr_intp i = 0; i < axis_len; i++) { \
            base_coords[axis] = i; \
            nr_intp lin = coords_to_linear(base_coords, n1->shape, n1->ndim); \
            \
            I_NT in_val = READ_VAL(I_NT, in_data, base_coords, n1, in_contig, lin); \
            \
            /* Skip NaN values - treat as identity */ \
            if (ISNAN_MACRO(in_val)) { \
                WRITE_VAL(O_NT, out_data, base_coords, out, out_contig, lin, acc); \
                continue; \
            } \
            \
            if (first) { \
                acc = (O_NT)in_val; \
                first = 0; \
            } else { \
                acc = OP_FUNC(acc, (O_NT)in_val); \
            } \
            \
            WRITE_VAL(O_NT, out_data, base_coords, out, out_contig, lin, acc); \
        } \
    } \
    \
    FINALIZE_CUM_OUTPUT() \
}

/* ============================================================================
 * DIFF KERNEL (numpy.diff - differences between consecutive elements)
 * ============================================================================ */

#define DEFINE_DIFF_KERNEL(OP_NAME, O_NT, I_NT, PROM_O_DT) \
NR_PRIVATE int OP_NAME##_kernel_##I_NT(NFuncArgs* args) { \
    Node* n1 = args->in_nodes[0]; \
    Node* caller_out = args->out_nodes[0]; \
    NFunc_CumArgs* cargs = (NFunc_CumArgs*)args->extra; \
    \
    int axis = cargs ? cargs->axis : -1; \
    if (axis == -1) axis = n1->ndim - 1; \
    axis = normalize_axis(axis, n1->ndim); \
    if (axis < 0) { \
        NError_RaiseError(NError_ValueError, \
            "axis %d out of bounds for array of dimension %d", \
            cargs ? cargs->axis : -1, n1->ndim); \
        return -1; \
    } \
    \
    /* Output shape: same except axis dimension is reduced by 1 */ \
    nr_intp out_shape[NR_NODE_MAX_NDIM]; \
    for (int i = 0; i < n1->ndim; i++) { \
        out_shape[i] = n1->shape[i]; \
    } \
    if (n1->shape[axis] <= 1) { \
        NError_RaiseError(NError_ValueError, "diff requires axis length > 1"); \
        return -1; \
    } \
    out_shape[axis] = n1->shape[axis] - 1; \
    \
    Node* out = NULL; \
    if (!caller_out || NODE_DTYPE(caller_out) != PROM_O_DT) { \
        out = Node_NewEmpty(n1->ndim, out_shape, PROM_O_DT); \
        if (!out) return -1; \
    } else { \
        out = caller_out; \
        for (int i = 0; i < n1->ndim; ++i) { \
            if (out->shape[i] != out_shape[i]) { \
                NError_RaiseError(NError_ValueError, "output array has wrong shape"); \
                return -1; \
            } \
        } \
    } \
    \
    O_NT* out_data = (O_NT*)NODE_DATA(out); \
    I_NT* in_data = (I_NT*)NODE_DATA(n1); \
    int in_contig = NODE_IS_CONTIGUOUS(n1); \
    int out_contig = NODE_IS_CONTIGUOUS(out); \
    \
    nr_intp axis_len = n1->shape[axis]; \
    nr_intp out_axis_len = out_shape[axis]; \
    nr_intp n_items = Node_NItems(n1); \
    nr_intp n_slices = n_items / axis_len; \
    \
    nr_intp inner_size = 1; \
    for (int d = axis + 1; d < n1->ndim; d++) { \
        inner_size *= n1->shape[d]; \
    } \
    \
    for (nr_intp slice = 0; slice < n_slices; slice++) { \
        nr_intp outer_idx = slice / inner_size; \
        nr_intp inner_idx = slice % inner_size; \
        \
        nr_intp in_coords[NR_NODE_MAX_NDIM]; \
        nr_intp out_coords[NR_NODE_MAX_NDIM]; \
        nr_intp temp = outer_idx; \
        for (int d = axis - 1; d >= 0; d--) { \
            in_coords[d] = out_coords[d] = temp % n1->shape[d]; \
            temp /= n1->shape[d]; \
        } \
        temp = inner_idx; \
        for (int d = n1->ndim - 1; d > axis; d--) { \
            in_coords[d] = out_coords[d] = temp % n1->shape[d]; \
            temp /= n1->shape[d]; \
        } \
        \
        /* Compute differences */ \
        in_coords[axis] = 0; \
        nr_intp lin_prev = coords_to_linear(in_coords, n1->shape, n1->ndim); \
        I_NT prev_val = READ_VAL(I_NT, in_data, in_coords, n1, in_contig, lin_prev); \
        \
        for (nr_intp i = 0; i < out_axis_len; i++) { \
            in_coords[axis] = i + 1; \
            out_coords[axis] = i; \
            \
            nr_intp lin_in = coords_to_linear(in_coords, n1->shape, n1->ndim); \
            nr_intp lin_out = coords_to_linear(out_coords, out_shape, n1->ndim); \
            \
            I_NT curr_val = READ_VAL(I_NT, in_data, in_coords, n1, in_contig, lin_in); \
            O_NT diff = (O_NT)curr_val - (O_NT)prev_val; \
            \
            WRITE_VAL(O_NT, out_data, out_coords, out, out_contig, lin_out, diff); \
            prev_val = curr_val; \
        } \
    } \
    \
    FINALIZE_CUM_OUTPUT() \
}

/* ============================================================================
 * GRADIENT KERNEL (numpy.gradient - central differences)
 * Output same shape as input, uses forward/backward diff at edges
 * ============================================================================ */

#define DEFINE_GRADIENT_KERNEL(OP_NAME, O_NT, I_NT, PROM_O_DT) \
NR_PRIVATE int OP_NAME##_kernel_##I_NT(NFuncArgs* args) { \
    SETUP_CUM_OUTPUT(O_NT, PROM_O_DT) \
    I_NT* in_data = (I_NT*)NODE_DATA(n1); \
    \
    nr_intp axis_len = n1->shape[axis]; \
    nr_intp n_slices = n_items / axis_len; \
    \
    nr_intp inner_size = 1; \
    for (int d = axis + 1; d < n1->ndim; d++) { \
        inner_size *= n1->shape[d]; \
    } \
    \
    for (nr_intp slice = 0; slice < n_slices; slice++) { \
        nr_intp outer_idx = slice / inner_size; \
        nr_intp inner_idx = slice % inner_size; \
        \
        nr_intp coords[NR_NODE_MAX_NDIM]; \
        nr_intp temp = outer_idx; \
        for (int d = axis - 1; d >= 0; d--) { \
            coords[d] = temp % n1->shape[d]; \
            temp /= n1->shape[d]; \
        } \
        temp = inner_idx; \
        for (int d = n1->ndim - 1; d > axis; d--) { \
            coords[d] = temp % n1->shape[d]; \
            temp /= n1->shape[d]; \
        } \
        \
        for (nr_intp i = 0; i < axis_len; i++) { \
            coords[axis] = i; \
            nr_intp lin = coords_to_linear(coords, n1->shape, n1->ndim); \
            \
            O_NT grad; \
            if (i == 0) { \
                /* Forward difference at start */ \
                coords[axis] = 0; \
                nr_intp lin0 = coords_to_linear(coords, n1->shape, n1->ndim); \
                coords[axis] = 1; \
                nr_intp lin1 = coords_to_linear(coords, n1->shape, n1->ndim); \
                I_NT v0 = READ_VAL(I_NT, in_data, coords, n1, in_contig, lin0); \
                coords[axis] = 1; \
                I_NT v1 = READ_VAL(I_NT, in_data, coords, n1, in_contig, lin1); \
                grad = (O_NT)v1 - (O_NT)v0; \
            } else if (i == axis_len - 1) { \
                /* Backward difference at end */ \
                coords[axis] = axis_len - 2; \
                nr_intp lin0 = coords_to_linear(coords, n1->shape, n1->ndim); \
                coords[axis] = axis_len - 1; \
                nr_intp lin1 = coords_to_linear(coords, n1->shape, n1->ndim); \
                I_NT v0 = READ_VAL(I_NT, in_data, coords, n1, in_contig, lin0); \
                coords[axis] = axis_len - 1; \
                I_NT v1 = READ_VAL(I_NT, in_data, coords, n1, in_contig, lin1); \
                grad = (O_NT)v1 - (O_NT)v0; \
            } else { \
                /* Central difference */ \
                coords[axis] = i - 1; \
                nr_intp lin0 = coords_to_linear(coords, n1->shape, n1->ndim); \
                coords[axis] = i + 1; \
                nr_intp lin1 = coords_to_linear(coords, n1->shape, n1->ndim); \
                coords[axis] = i - 1; \
                I_NT v0 = READ_VAL(I_NT, in_data, coords, n1, in_contig, lin0); \
                coords[axis] = i + 1; \
                I_NT v1 = READ_VAL(I_NT, in_data, coords, n1, in_contig, lin1); \
                grad = ((O_NT)v1 - (O_NT)v0) / (O_NT)2.0; \
            } \
            \
            coords[axis] = i; \
            WRITE_VAL(O_NT, out_data, coords, out, out_contig, lin, grad); \
        } \
    } \
    \
    FINALIZE_CUM_OUTPUT() \
}

/* ============================================================================
 * TYPE INSTANTIATION MACROS
 * ============================================================================ */

/* Cumsum/Cumprod: integers -> int64/uint64, floats -> float64 */
#define DEFINE_CUM_PROMOTED(OP, FUNC, INIT, NEED_FIRST) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_int64,   nr_bool,    INIT, NEED_FIRST, NR_INT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_int64,   nr_int8,    INIT, NEED_FIRST, NR_INT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_int64,   nr_int16,   INIT, NEED_FIRST, NR_INT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_int64,   nr_int32,   INIT, NEED_FIRST, NR_INT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_int64,   nr_int64,   INIT, NEED_FIRST, NR_INT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_uint64,  nr_uint8,   INIT, NEED_FIRST, NR_UINT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_uint64,  nr_uint16,  INIT, NEED_FIRST, NR_UINT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_uint64,  nr_uint32,  INIT, NEED_FIRST, NR_UINT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_uint64,  nr_uint64,  INIT, NEED_FIRST, NR_UINT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_float64, nr_float32, INIT, NEED_FIRST, NR_FLOAT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_float64, nr_float64, INIT, NEED_FIRST, NR_FLOAT64)

/* Cummin/Cummax: same type in/out */
#define DEFINE_CUM_SAME_TYPE(OP, FUNC, INIT, NEED_FIRST) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_bool,    nr_bool,    INIT, NEED_FIRST, NR_BOOL) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_int8,    nr_int8,    INIT, NEED_FIRST, NR_INT8) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_int16,   nr_int16,   INIT, NEED_FIRST, NR_INT16) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_int32,   nr_int32,   INIT, NEED_FIRST, NR_INT32) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_int64,   nr_int64,   INIT, NEED_FIRST, NR_INT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_uint8,   nr_uint8,   INIT, NEED_FIRST, NR_UINT8) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_uint16,  nr_uint16,  INIT, NEED_FIRST, NR_UINT16) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_uint32,  nr_uint32,  INIT, NEED_FIRST, NR_UINT32) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_uint64,  nr_uint64,  INIT, NEED_FIRST, NR_UINT64) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_float32, nr_float32, INIT, NEED_FIRST, NR_FLOAT32) \
    DEFINE_CUM_KERNEL(OP, FUNC, nr_float64, nr_float64, INIT, NEED_FIRST, NR_FLOAT64)

/* NaN variants: float only */
#define DEFINE_NANCUM_FLOATS(OP, FUNC, INIT, NEED_FIRST) \
    DEFINE_NANCUM_KERNEL(OP, FUNC, nr_float64, nr_float32, INIT, NEED_FIRST, NR_FLOAT64, ISNAN_F32) \
    DEFINE_NANCUM_KERNEL(OP, FUNC, nr_float64, nr_float64, INIT, NEED_FIRST, NR_FLOAT64, ISNAN_F64)

/* Diff: signed output for proper differences */
#define DEFINE_DIFF_ALL(OP) \
    DEFINE_DIFF_KERNEL(OP, nr_int64,   nr_bool,    NR_INT64) \
    DEFINE_DIFF_KERNEL(OP, nr_int64,   nr_int8,    NR_INT64) \
    DEFINE_DIFF_KERNEL(OP, nr_int64,   nr_int16,   NR_INT64) \
    DEFINE_DIFF_KERNEL(OP, nr_int64,   nr_int32,   NR_INT64) \
    DEFINE_DIFF_KERNEL(OP, nr_int64,   nr_int64,   NR_INT64) \
    DEFINE_DIFF_KERNEL(OP, nr_int64,   nr_uint8,   NR_INT64) \
    DEFINE_DIFF_KERNEL(OP, nr_int64,   nr_uint16,  NR_INT64) \
    DEFINE_DIFF_KERNEL(OP, nr_int64,   nr_uint32,  NR_INT64) \
    DEFINE_DIFF_KERNEL(OP, nr_int64,   nr_uint64,  NR_INT64) \
    DEFINE_DIFF_KERNEL(OP, nr_float64, nr_float32, NR_FLOAT64) \
    DEFINE_DIFF_KERNEL(OP, nr_float64, nr_float64, NR_FLOAT64)

/* Gradient: always float output */
#define DEFINE_GRADIENT_ALL(OP) \
    DEFINE_GRADIENT_KERNEL(OP, nr_float64, nr_bool,    NR_FLOAT64) \
    DEFINE_GRADIENT_KERNEL(OP, nr_float64, nr_int8,    NR_FLOAT64) \
    DEFINE_GRADIENT_KERNEL(OP, nr_float64, nr_int16,   NR_FLOAT64) \
    DEFINE_GRADIENT_KERNEL(OP, nr_float64, nr_int32,   NR_FLOAT64) \
    DEFINE_GRADIENT_KERNEL(OP, nr_float64, nr_int64,   NR_FLOAT64) \
    DEFINE_GRADIENT_KERNEL(OP, nr_float64, nr_uint8,   NR_FLOAT64) \
    DEFINE_GRADIENT_KERNEL(OP, nr_float64, nr_uint16,  NR_FLOAT64) \
    DEFINE_GRADIENT_KERNEL(OP, nr_float64, nr_uint32,  NR_FLOAT64) \
    DEFINE_GRADIENT_KERNEL(OP, nr_float64, nr_uint64,  NR_FLOAT64) \
    DEFINE_GRADIENT_KERNEL(OP, nr_float64, nr_float32, NR_FLOAT64) \
    DEFINE_GRADIENT_KERNEL(OP, nr_float64, nr_float64, NR_FLOAT64)

/* ============================================================================
 * DISPATCHER MACROS
 * ============================================================================ */

#define DEFINE_DISPATCHER(OP_NAME) \
NR_PRIVATE int OP_NAME##_dispatch(NFuncArgs* args) { \
    switch (NODE_DTYPE(args->in_nodes[0])) { \
        case NR_BOOL:    return OP_NAME##_kernel_nr_bool(args); \
        case NR_INT8:    return OP_NAME##_kernel_nr_int8(args); \
        case NR_INT16:   return OP_NAME##_kernel_nr_int16(args); \
        case NR_INT32:   return OP_NAME##_kernel_nr_int32(args); \
        case NR_INT64:   return OP_NAME##_kernel_nr_int64(args); \
        case NR_UINT8:   return OP_NAME##_kernel_nr_uint8(args); \
        case NR_UINT16:  return OP_NAME##_kernel_nr_uint16(args); \
        case NR_UINT32:  return OP_NAME##_kernel_nr_uint32(args); \
        case NR_UINT64:  return OP_NAME##_kernel_nr_uint64(args); \
        case NR_FLOAT32: return OP_NAME##_kernel_nr_float32(args); \
        case NR_FLOAT64: return OP_NAME##_kernel_nr_float64(args); \
        default: \
            NError_RaiseError(NError_TypeError, #OP_NAME ": unsupported dtype"); \
            return -1; \
    } \
}

#define DEFINE_DISPATCHER_FLOAT_ONLY(OP_NAME) \
NR_PRIVATE int OP_NAME##_dispatch(NFuncArgs* args) { \
    switch (NODE_DTYPE(args->in_nodes[0])) { \
        case NR_FLOAT32: return OP_NAME##_kernel_nr_float32(args); \
        case NR_FLOAT64: return OP_NAME##_kernel_nr_float64(args); \
        default: \
            NError_RaiseError(NError_TypeError, #OP_NAME ": only float types supported"); \
            return -1; \
    } \
}

/* ============================================================================
 * NFUNC AND API MACROS
 * ============================================================================ */

#define DEFINE_NFUNC(OP_NAME, name_str) \
const NFunc name_str##_nfunc = { \
    .name = #name_str, \
    .flags = NFUNC_FLAG_OUT_DTYPES_NOT_SAME, \
    .nin = 1, .nout = 1, \
    .in_type = NDTYPE_NONE, .out_type = NDTYPE_NONE, \
    .in_dtype = NR_NONE, .out_dtype = NR_NONE, \
    .func = OP_NAME##_dispatch, \
    .grad_func = NULL \
};

#define DEFINE_API(ApiName, name_str) \
NR_PUBLIC Node* NMath_##ApiName(Node* c, Node* a, int axis) { \
    NFuncArgs* args = NFuncArgs_New(1, 1); \
    args->in_nodes[0] = a; \
    args->out_nodes[0] = c; \
    NFunc_CumArgs cargs = NFunc_CumArgs_New(axis); \
    args->extra = &cargs; \
    int result = NFunc_Call(&name_str##_nfunc, args); \
    Node* out = args->out_nodes[0]; \
    NFuncArgs_DECREF(args); \
    return result != 0 ? NULL : out; \
}

/* ============================================================================
 * INSTANTIATE ALL OPERATIONS
 * ============================================================================ */

/* === Cumsum === */
DEFINE_CUM_PROMOTED(Cumsum, OP_SUM, 0, 0)
DEFINE_DISPATCHER(Cumsum)
DEFINE_NFUNC(Cumsum, cumsum)
DEFINE_API(Cumsum, cumsum)

/* === Cumprod === */
DEFINE_CUM_PROMOTED(Cumprod, OP_PROD, 1, 0)
DEFINE_DISPATCHER(Cumprod)
DEFINE_NFUNC(Cumprod, cumprod)
DEFINE_API(Cumprod, cumprod)

/* === Cummin === */
DEFINE_CUM_SAME_TYPE(Cummin, OP_MIN, 0, 1)
DEFINE_DISPATCHER(Cummin)
DEFINE_NFUNC(Cummin, cummin)
DEFINE_API(Cummin, cummin)

/* === Cummax === */
DEFINE_CUM_SAME_TYPE(Cummax, OP_MAX, 0, 1)
DEFINE_DISPATCHER(Cummax)
DEFINE_NFUNC(Cummax, cummax)
DEFINE_API(Cummax, cummax)

/* === Diff === */
DEFINE_DIFF_ALL(Diff)
DEFINE_DISPATCHER(Diff)
DEFINE_NFUNC(Diff, diff)
DEFINE_API(Diff, diff)

/* === Gradient === */
DEFINE_GRADIENT_ALL(Gradient)
DEFINE_DISPATCHER(Gradient)
DEFINE_NFUNC(Gradient, gradient)
DEFINE_API(Gradient, gradient)

/* ============================================================================
 * NAN-IGNORING VARIANTS (float types only)
 * ============================================================================ */

/* === NanCumsum === */
DEFINE_NANCUM_FLOATS(NanCumsum, OP_SUM, 0, 0)
DEFINE_DISPATCHER_FLOAT_ONLY(NanCumsum)
DEFINE_NFUNC(NanCumsum, nancumsum)
DEFINE_API(NanCumsum, nancumsum)

/* === NanCumprod === */
DEFINE_NANCUM_FLOATS(NanCumprod, OP_PROD, 1, 0)
DEFINE_DISPATCHER_FLOAT_ONLY(NanCumprod)
DEFINE_NFUNC(NanCumprod, nancumprod)
DEFINE_API(NanCumprod, nancumprod)

/* === NanCummin === */
DEFINE_NANCUM_FLOATS(NanCummin, OP_MIN, 0, 1)
DEFINE_DISPATCHER_FLOAT_ONLY(NanCummin)
DEFINE_NFUNC(NanCummin, nancummin)
DEFINE_API(NanCummin, nancummin)

/* === NanCummax === */
DEFINE_NANCUM_FLOATS(NanCummax, OP_MAX, 0, 1)
DEFINE_DISPATCHER_FLOAT_ONLY(NanCummax)
DEFINE_NFUNC(NanCummax, nancummax)
DEFINE_API(NanCummax, nancummax)