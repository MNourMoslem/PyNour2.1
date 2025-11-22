#include "nour/nour.h"
#include "reduce.h"
#include "../niter.h"
#include "../nerror.h"
#include "../node_core.h"
#include "../tc_methods.h"
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
 * Basic Reduction Operations
 * ============================================================================ */

#define OP_SUM(acc, val)  ((acc) + (val))
#define OP_PROD(acc, val) ((acc) * (val))
#define OP_MIN(acc, val)  (((val) < (acc)) ? (val) : (acc))
#define OP_MAX(acc, val)  (((val) > (acc)) ? (val) : (acc))

/* ============================================================================
 * Helper: Reduce Args Constructor
 * ============================================================================ */

NR_PUBLIC NFunc_ReduceArgs
NFunc_ReduceArgs_New(const int* axes, const int n_axes)
{
    NFunc_ReduceArgs args = {0};
    if (axes && n_axes > 0) {
        args.n_axis = n_axes;
        for (int i = 0; i < n_axes; ++i) {
            args.axis[i] = axes[i];
        }
    }
    return args;
}

/* ============================================================================
 * Helper: Compute output shape from reduction args
 * ============================================================================ */

NR_PRIVATE int
compute_out_shape(NFunc_ReduceArgs* rargs, Node* input, nr_intp* out_shape)
{
    if (!rargs || rargs->n_axis == 0) {
        return 0;
    }

    int is_reduced[NR_NODE_MAX_NDIM] = {0};
    
    for (int i = 0; i < rargs->n_axis; ++i) {
        int axis = rargs->axis[i];
        if (axis < 0) axis += input->ndim;
        if (axis < 0 || axis >= input->ndim) {
            NError_RaiseError(NError_ValueError,
                "reduce axis %d out of bounds for array of dimension %d",
                rargs->axis[i], input->ndim);
            return -1;
        }
        is_reduced[axis] = 1;
    }

    int out_ndim = 0;
    for (int i = 0; i < input->ndim; ++i) {
        if (!is_reduced[i]) {
            out_shape[out_ndim++] = input->shape[i];
        }
    }

    return out_ndim;
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
coords_to_offset(const nr_intp* coords, const nr_intp* strides, int ndim)
{
    nr_intp offset = 0;
    for (int d = 0; d < ndim; d++) {
        offset += coords[d] * strides[d];
    }
    return offset;
}

NR_STATIC_INLINE nr_intp
coords_to_out_idx(const nr_intp* coords, const nr_intp* out_shape, 
                  const int* is_reduced, int in_ndim, int out_ndim)
{
    nr_intp out_idx = 0, mult = 1;
    for (int d = in_ndim - 1, o = out_ndim - 1; d >= 0; d--) {
        if (!is_reduced[d]) {
            out_idx += coords[d] * mult;
            mult *= out_shape[o--];
        }
    }
    return out_idx;
}

/* ============================================================================
 * Input Value Reader (handles contiguous vs strided)
 * ============================================================================ */

#define READ_INPUT(I_NT, in_data, lin, coords, n1, in_contig) \
    (in_contig ? ((I_NT*)in_data)[lin] : \
     *(I_NT*)((char*)in_data + coords_to_offset(coords, n1->strides, n1->ndim)))

/* ============================================================================
 * Output Node Setup (common to all reduce operations)
 * ============================================================================ */

#define SETUP_REDUCE_OUTPUT(O_NT, PROM_O_DT) \
    Node* n1 = args->in_nodes[0]; \
    Node* caller_out = args->out_nodes[0]; \
    NFunc_ReduceArgs* rargs = (NFunc_ReduceArgs*)args->extra; \
    \
    nr_intp out_shape[NR_NODE_MAX_NDIM]; \
    int out_ndim = compute_out_shape(rargs, n1, out_shape); \
    if (out_ndim < 0) return -1; \
    \
    Node* out = NULL; \
    if (!caller_out || NODE_DTYPE(caller_out) != PROM_O_DT) { \
        out = Node_NewEmpty(out_ndim, out_shape, PROM_O_DT); \
        if (!out) return -1; \
    } else { \
        out = caller_out; \
        if (out->ndim != out_ndim) { \
            NError_RaiseError(NError_ValueError, "output array has wrong ndim"); \
            return -1; \
        } \
        for (int i = 0; i < out_ndim; ++i) { \
            if (out->shape[i] != out_shape[i]) { \
                NError_RaiseError(NError_ValueError, "output array has wrong shape"); \
                return -1; \
            } \
        } \
    } \
    \
    O_NT* out_data = (O_NT*)NODE_DATA(out); \
    nr_intp n_in = Node_NItems(n1); \
    nr_intp n_out = Node_NItems(out); \
    int in_contig = NODE_IS_CONTIGUOUS(n1); \
    (void)n_out;

#define FINALIZE_REDUCE_OUTPUT() \
    if (!caller_out) { \
        args->out_nodes[0] = out; \
    } else if (out != caller_out) { \
        /* Manual element-wise cast to avoid incorrect Node_ToType behavior for reductions */ \
        nr_intp n_el__ = Node_NItems(out); \
        void* dst_data__ = NODE_DATA(caller_out); \
        switch (NODE_DTYPE(caller_out)) { \
            case NR_BOOL:    for (nr_intp i=0;i<n_el__;++i)((nr_bool*)dst_data__)[i]    =(nr_bool)out_data[i]; break; \
            case NR_INT8:    for (nr_intp i=0;i<n_el__;++i)((nr_int8*)dst_data__)[i]    =(nr_int8)out_data[i]; break; \
            case NR_INT16:   for (nr_intp i=0;i<n_el__;++i)((nr_int16*)dst_data__)[i]   =(nr_int16)out_data[i]; break; \
            case NR_INT32:   for (nr_intp i=0;i<n_el__;++i)((nr_int32*)dst_data__)[i]   =(nr_int32)out_data[i]; break; \
            case NR_INT64:   for (nr_intp i=0;i<n_el__;++i)((nr_int64*)dst_data__)[i]   =(nr_int64)out_data[i]; break; \
            case NR_UINT8:   for (nr_intp i=0;i<n_el__;++i)((nr_uint8*)dst_data__)[i]   =(nr_uint8)out_data[i]; break; \
            case NR_UINT16:  for (nr_intp i=0;i<n_el__;++i)((nr_uint16*)dst_data__)[i]  =(nr_uint16)out_data[i]; break; \
            case NR_UINT32:  for (nr_intp i=0;i<n_el__;++i)((nr_uint32*)dst_data__)[i]  =(nr_uint32)out_data[i]; break; \
            case NR_UINT64:  for (nr_intp i=0;i<n_el__;++i)((nr_uint64*)dst_data__)[i]  =(nr_uint64)out_data[i]; break; \
            case NR_FLOAT32: for (nr_intp i=0;i<n_el__;++i)((nr_float32*)dst_data__)[i]=(nr_float32)out_data[i]; break; \
            case NR_FLOAT64: for (nr_intp i=0;i<n_el__;++i)((nr_float64*)dst_data__)[i]=(nr_float64)out_data[i]; break; \
            default: NError_RaiseError(NError_TypeError, "Unsupported output dtype in reduce cast"); break; \
        } \
        args->out_nodes[0] = caller_out; \
        NODE_DECREF(out); \
    } else { \
        args->out_nodes[0] = out; \
    } \
    return 0;

/* ============================================================================
 * BASIC REDUCE KERNEL (sum, prod, min, max)
 * ============================================================================ */

/* Generic reduce kernel for sum/prod/min/max. For min/max NEEDS_FIRST selects first-element initialization. */
#define DEFINE_REDUCE_KERNEL(OP_NAME, OP_FUNC, O_NT, I_NT, INIT_VAL, NEEDS_FIRST, PROM_O_DT) \
NR_PRIVATE int OP_NAME##_kernel_##I_NT(NFuncArgs* args) { \
    SETUP_REDUCE_OUTPUT(O_NT, PROM_O_DT) \
    I_NT* in_data = (I_NT*)NODE_DATA(n1); \
    if (!rargs || rargs->n_axis == 0 ) { \
        O_NT acc = (O_NT)(INIT_VAL); \
        int first = (NEEDS_FIRST); \
        if (in_contig) { \
            for (nr_intp i = 0; i < n_in; i++) { \
                O_NT val = (O_NT)in_data[i]; \
                if (first) { acc = val; first = 0; } else { acc = OP_FUNC(acc, val); } \
            } \
        } else { \
            NIter it; NIter_FromNode(&it, n1, NITER_MODE_STRIDED); \
            NIter_ITER(&it); \
            while (NIter_NOTDONE(&it)) { \
                O_NT val = (O_NT)(*(I_NT*)NIter_ITEM(&it)); \
                if (first) { acc = val; first = 0; } else { acc = OP_FUNC(acc, val); } \
                NIter_NEXT(&it); \
            } \
        } \
        out_data[0] = acc; \
    } else { \
        int is_reduced[NR_NODE_MAX_NDIM] = {0}; \
        for (int i = 0; i < rargs->n_axis; i++) { \
            int ax = rargs->axis[i]; \
            if (ax < 0) ax += n1->ndim; \
            is_reduced[ax] = 1; \
        } \
        nr_bool* first_flag = NULL; \
        if (NEEDS_FIRST) { \
            first_flag = (nr_bool*)calloc(n_out, sizeof(nr_bool)); \
            if (!first_flag) { NError_RaiseMemoryError(); return -1; } \
        } else { \
            for (nr_intp i = 0; i < n_out; i++) out_data[i] = (O_NT)(INIT_VAL); \
        } \
        nr_intp coords[NR_NODE_MAX_NDIM]; \
        for (nr_intp lin = 0; lin < n_in; lin++) { \
            linear_to_coords(lin, n1->shape, n1->ndim, coords); \
            nr_intp oi = coords_to_out_idx(coords, out_shape, is_reduced, n1->ndim, out_ndim); \
            I_NT vtmp = READ_INPUT(I_NT, in_data, lin, coords, n1, in_contig); \
            O_NT val = (O_NT)vtmp; \
            if (NEEDS_FIRST) { \
                if (!first_flag[oi]) { out_data[oi] = val; first_flag[oi] = 1; } \
                else { out_data[oi] = OP_FUNC(out_data[oi], val); } \
            } else { \
                out_data[oi] = OP_FUNC(out_data[oi], val); \
            } \
        } \
        if (first_flag) free(first_flag); \
    } \
    FINALIZE_REDUCE_OUTPUT() \
}

/* Mean reduction kernel (optionally ignore NaNs). */
#define DEFINE_MEAN_KERNEL(OP_NAME, O_NT, I_NT, PROM_O_DT, IGNORE_NAN, ISNAN_CHECK) \
NR_PRIVATE int OP_NAME##_kernel_##I_NT(NFuncArgs* args) { \
    SETUP_REDUCE_OUTPUT(O_NT, PROM_O_DT) \
    I_NT* in_data = (I_NT*)NODE_DATA(n1); \
    if (!rargs || rargs->n_axis == 0) { \
        long long count = 0; \
        O_NT sum = 0; \
        if (in_contig) { \
            for (nr_intp i = 0; i < n_in; i++) { \
                I_NT v = in_data[i]; \
                if (IGNORE_NAN && ISNAN_CHECK(v)) continue; \
                sum += (O_NT)v; count++; \
            } \
        } else { \
            NIter it; NIter_FromNode(&it, n1, NITER_MODE_STRIDED); \
            NIter_ITER(&it); \
            while (NIter_NOTDONE(&it)) { \
                I_NT v = *(I_NT*)NIter_ITEM(&it); \
                if (IGNORE_NAN && ISNAN_CHECK(v)) { NIter_NEXT(&it); continue; } \
                sum += (O_NT)v; count++; \
                NIter_NEXT(&it); \
            } \
        } \
        out_data[0] = count == 0 ? NAN : (sum / (O_NT)count); \
    } else { \
        int is_reduced[NR_NODE_MAX_NDIM] = {0}; \
        for (int i = 0; i < rargs->n_axis; i++) { int ax = rargs->axis[i]; if (ax < 0) ax += n1->ndim; is_reduced[ax] = 1; } \
        O_NT* sums = out_data; \
        for (nr_intp i = 0; i < n_out; i++) sums[i] = 0; \
        long long* counts = (long long*)calloc(n_out, sizeof(long long)); \
        if (!counts) { NError_RaiseMemoryError(); return -1; } \
        nr_intp coords[NR_NODE_MAX_NDIM]; \
        for (nr_intp lin = 0; lin < n_in; lin++) { \
            linear_to_coords(lin, n1->shape, n1->ndim, coords); \
            I_NT vtmp = READ_INPUT(I_NT, in_data, lin, coords, n1, in_contig); \
            if (IGNORE_NAN && ISNAN_CHECK(vtmp)) continue; \
            O_NT v = (O_NT)vtmp; \
            nr_intp oi = coords_to_out_idx(coords, out_shape, is_reduced, n1->ndim, out_ndim); \
            sums[oi] += v; counts[oi]++; \
        } \
        for (nr_intp i = 0; i < n_out; i++) { out_data[i] = counts[i] == 0 ? NAN : (sums[i] / (O_NT)counts[i]); } \
        free(counts); \
    } \
    FINALIZE_REDUCE_OUTPUT() \
}

/* Variance / Std kernel (DO_SQRT selects std). */
#define DEFINE_VAR_KERNEL(OP_NAME, O_NT, I_NT, PROM_O_DT, IGNORE_NAN, ISNAN_CHECK, DO_SQRT) \
NR_PRIVATE int OP_NAME##_kernel_##I_NT(NFuncArgs* args) { \
    SETUP_REDUCE_OUTPUT(O_NT, PROM_O_DT) \
    I_NT* in_data = (I_NT*)NODE_DATA(n1); \
    if (!rargs || rargs->n_axis == 0) { \
        long long count = 0; \
        O_NT sum = 0, sumsq = 0; \
        for (nr_intp i = 0; i < n_in; i++) { \
            I_NT v = in_data[i]; \
            if (IGNORE_NAN && ISNAN_CHECK(v)) continue; \
            O_NT fv = (O_NT)v; \
            sum += fv; sumsq += fv*fv; count++; \
        } \
        if (count == 0) { out_data[0] = NAN; } else { \
            O_NT mean = sum / (O_NT)count; \
            O_NT var = (sumsq / (O_NT)count) - mean*mean; \
            out_data[0] = DO_SQRT ? (O_NT)sqrt(var) : var; \
        } \
    } else { \
        int is_reduced[NR_NODE_MAX_NDIM] = {0}; \
        for (int i = 0; i < rargs->n_axis; i++) { int ax = rargs->axis[i]; if (ax < 0) ax += n1->ndim; is_reduced[ax] = 1; } \
        O_NT* sums = (O_NT*)calloc(n_out, sizeof(O_NT)); \
        O_NT* sumsq = (O_NT*)calloc(n_out, sizeof(O_NT)); \
        long long* counts = (long long*)calloc(n_out, sizeof(long long)); \
        if (!sums || !sumsq || !counts) { NError_RaiseMemoryError(); free(sums); free(sumsq); free(counts); return -1; } \
        nr_intp coords[NR_NODE_MAX_NDIM]; \
        for (nr_intp lin = 0; lin < n_in; lin++) { \
            linear_to_coords(lin, n1->shape, n1->ndim, coords); \
            I_NT vtmp = READ_INPUT(I_NT, in_data, lin, coords, n1, in_contig); \
            if (IGNORE_NAN && ISNAN_CHECK(vtmp)) continue; \
            O_NT v = (O_NT)vtmp; \
            nr_intp oi = coords_to_out_idx(coords, out_shape, is_reduced, n1->ndim, out_ndim); \
            sums[oi] += v; sumsq[oi] += v*v; counts[oi]++; \
        } \
        for (nr_intp i = 0; i < n_out; i++) { \
            if (counts[i] == 0) { out_data[i] = NAN; } else { \
                O_NT mean = sums[i] / (O_NT)counts[i]; \
                O_NT var = (sumsq[i] / (O_NT)counts[i]) - mean*mean; \
                out_data[i] = DO_SQRT ? (O_NT)sqrt(var) : var; \
            } \
        } \
        free(sums); free(sumsq); free(counts); \
    } \
    FINALIZE_REDUCE_OUTPUT() \
}

/* Argmin/Argmax kernel: returns int64 indices (linear indices). */
#define DEFINE_ARG_KERNEL(OP_NAME, I_NT, COMPARE_OP, PROM_O_DT) \
NR_PRIVATE int OP_NAME##_kernel_##I_NT(NFuncArgs* args) { \
    SETUP_REDUCE_OUTPUT(nr_int64, PROM_O_DT) \
    I_NT* in_data = (I_NT*)NODE_DATA(n1); \
    if (!rargs || rargs->n_axis == 0) { \
        if (n_in == 0) { NError_RaiseError(NError_ValueError, #OP_NAME ": empty input"); return -1; } \
        I_NT best = in_data[0]; nr_int64 best_idx = 0; \
        for (nr_intp i = 1; i < n_in; i++) { I_NT v = in_data[i]; if (v COMPARE_OP best) { best = v; best_idx = (nr_int64)i; } } \
        out_data[0] = best_idx; \
    } else { \
        int is_reduced[NR_NODE_MAX_NDIM] = {0}; \
        int reduced_axes[NR_NODE_MAX_NDIM]; int nred = 0; \
        for (int i = 0; i < rargs->n_axis; i++) { int ax = rargs->axis[i]; if (ax < 0) ax += n1->ndim; if (!is_reduced[ax]) { is_reduced[ax] = 1; reduced_axes[nred++] = ax; } } \
        I_NT* best_vals = (I_NT*)calloc(n_out, sizeof(I_NT)); \
        nr_int64* best_indices = (nr_int64*)calloc(n_out, sizeof(nr_int64)); \
        nr_bool* first_flag = (nr_bool*)calloc(n_out, sizeof(nr_bool)); \
        if (!best_vals || !best_indices || !first_flag) { NError_RaiseMemoryError(); free(best_vals); free(best_indices); free(first_flag); return -1; } \
        nr_intp coords[NR_NODE_MAX_NDIM]; \
        for (nr_intp lin = 0; lin < n_in; lin++) { \
            linear_to_coords(lin, n1->shape, n1->ndim, coords); \
            nr_intp oi = coords_to_out_idx(coords, out_shape, is_reduced, n1->ndim, out_ndim); \
            I_NT v = READ_INPUT(I_NT, in_data, lin, coords, n1, in_contig); \
            if (!first_flag[oi]) { \
                best_vals[oi] = v; \
                /* compute local index across reduced axes fused */ \
                nr_int64 local_idx = 0; nr_int64 mult = 1; \
                for (int k = nred - 1; k >= 0; --k) { local_idx += coords[reduced_axes[k]] * mult; mult *= n1->shape[reduced_axes[k]]; } \
                best_indices[oi] = local_idx; first_flag[oi] = 1; \
            } else if (v COMPARE_OP best_vals[oi]) { \
                best_vals[oi] = v; \
                nr_int64 local_idx = 0; nr_int64 mult = 1; \
                for (int k = nred - 1; k >= 0; --k) { local_idx += coords[reduced_axes[k]] * mult; mult *= n1->shape[reduced_axes[k]]; } \
                best_indices[oi] = local_idx; \
            } \
        } \
        for (nr_intp i = 0; i < n_out; i++) out_data[i] = best_indices[i]; \
        free(best_vals); free(best_indices); free(first_flag); \
    } \
    FINALIZE_REDUCE_OUTPUT() \
}

/* Boolean reductions (All/Any). SHORT flag indicates short-circuit target value (0 for All, 1 for Any). */
/* Boolean reduction (All/Any). INIT_VAL: starting accumulator (1 for All, 0 for Any). SHORT_TARGET: value that enables early termination (0 for All when accumulator becomes 0, 1 for Any when accumulator becomes 1). */
#define DEFINE_BOOL_REDUCE_KERNEL(OP_NAME, I_NT, INIT_VAL, SHORT_TARGET, PROM_O_DT) \
NR_PRIVATE int OP_NAME##_kernel_##I_NT(NFuncArgs* args) { \
    SETUP_REDUCE_OUTPUT(nr_bool, PROM_O_DT) \
    I_NT* in_data = (I_NT*)NODE_DATA(n1); \
    nr_bool init = (nr_bool)(INIT_VAL); \
    nr_bool short_target = (nr_bool)(SHORT_TARGET); \
    if (!rargs || rargs->n_axis == 0) { \
        nr_bool acc = init; \
        if (in_contig) { \
            for (nr_intp i = 0; i < n_in; i++) { \
                nr_bool val = (nr_bool)(in_data[i] != 0); \
                acc = (short_target ? (acc || val) : (acc && val)); \
                if (acc == short_target) break; \
            } \
        } else { \
            NIter it; NIter_FromNode(&it, n1, NITER_MODE_STRIDED); NIter_ITER(&it); \
            while (NIter_NOTDONE(&it)) { \
                nr_bool val = (nr_bool)(*(I_NT*)NIter_ITEM(&it) != 0); \
                acc = (short_target ? (acc || val) : (acc && val)); \
                if (acc == short_target) break; \
                NIter_NEXT(&it); \
            } \
        } \
        out_data[0] = acc; \
    } else { \
        int is_reduced[NR_NODE_MAX_NDIM] = {0}; \
        for (int i = 0; i < rargs->n_axis; i++) { int ax = rargs->axis[i]; if (ax < 0) ax += n1->ndim; is_reduced[ax] = 1; } \
        for (nr_intp i = 0; i < n_out; i++) out_data[i] = init; \
        nr_bool* done = (nr_bool*)calloc(n_out, sizeof(nr_bool)); \
        if (!done) { NError_RaiseMemoryError(); return -1; } \
        nr_intp coords[NR_NODE_MAX_NDIM]; \
        for (nr_intp lin = 0; lin < n_in; lin++) { \
            linear_to_coords(lin, n1->shape, n1->ndim, coords); \
            nr_intp oi = coords_to_out_idx(coords, out_shape, is_reduced, n1->ndim, out_ndim); \
            if (done[oi]) continue; \
            nr_bool val = (nr_bool)(READ_INPUT(I_NT, in_data, lin, coords, n1, in_contig) != 0); \
            out_data[oi] = (short_target ? (out_data[oi] || val) : (out_data[oi] && val)); \
            if (out_data[oi] == short_target) done[oi] = 1; \
        } \
        free(done); \
    } \
    FINALIZE_REDUCE_OUTPUT() \
}

/* NaN ignoring reduce (sum/prod/min/max variants for floats only). */
#define DEFINE_NANREDUCE_KERNEL(OP_NAME, OP_FUNC, O_NT, I_NT, INIT_VAL, NEEDS_FIRST, PROM_O_DT, ISNAN_CHECK) \
NR_PRIVATE int OP_NAME##_kernel_##I_NT(NFuncArgs* args) { \
    SETUP_REDUCE_OUTPUT(O_NT, PROM_O_DT) \
    I_NT* in_data = (I_NT*)NODE_DATA(n1); \
    if (!rargs || rargs->n_axis == 0) { \
        O_NT acc = (O_NT)(INIT_VAL); int first = (NEEDS_FIRST); \
        for (nr_intp i = 0; i < n_in; i++) { \
            I_NT raw = in_data[i]; if (ISNAN_CHECK(raw)) continue; O_NT val = (O_NT)raw; \
            if (first) { acc = val; first = 0; } else { acc = OP_FUNC(acc, val); } \
        } \
        out_data[0] = (NEEDS_FIRST && first) ? NAN : acc; \
    } else { \
        int is_reduced[NR_NODE_MAX_NDIM]={0}; for(int i=0;i<rargs->n_axis;i++){int ax=rargs->axis[i]; if(ax<0) ax+=n1->ndim; is_reduced[ax]=1;} \
        nr_bool* first_flag = NULL; if(NEEDS_FIRST){ first_flag = (nr_bool*)calloc(n_out,sizeof(nr_bool)); if(!first_flag){NError_RaiseMemoryError(); return -1;} } \
        for(nr_intp i=0;i<n_out;i++) if(!NEEDS_FIRST) out_data[i]=(O_NT)(INIT_VAL); \
        nr_intp coords[NR_NODE_MAX_NDIM]; \
        for(nr_intp lin=0; lin<n_in; lin++){ linear_to_coords(lin,n1->shape,n1->ndim,coords); I_NT raw=READ_INPUT(I_NT,in_data,lin,coords,n1,in_contig); if(ISNAN_CHECK(raw)) continue; O_NT val=(O_NT)raw; nr_intp oi=coords_to_out_idx(coords,out_shape,is_reduced,n1->ndim,out_ndim); if(NEEDS_FIRST){ if(!first_flag[oi]){ out_data[oi]=val; first_flag[oi]=1;} else { out_data[oi]=OP_FUNC(out_data[oi],val);} } else { out_data[oi]=OP_FUNC(out_data[oi],val);} } \
        if(first_flag){ for(nr_intp i=0;i<n_out;i++) if(!first_flag[i]) out_data[i]=NAN; free(first_flag);} \
    } \
    FINALIZE_REDUCE_OUTPUT() \
}

/* ============================================================================
 * COUNT NONZERO KERNEL
 * ============================================================================ */

#define DEFINE_COUNT_KERNEL(OP_NAME, I_NT, PROM_O_DT) \
NR_PRIVATE int OP_NAME##_kernel_##I_NT(NFuncArgs* args) { \
    SETUP_REDUCE_OUTPUT(nr_intp, PROM_O_DT) \
    I_NT* in_data = (I_NT*)NODE_DATA(n1); \
    \
    if (!rargs || rargs->n_axis == 0) { \
        nr_intp count = 0; \
        if (in_contig) { \
            for (nr_intp i = 0; i < n_in; i++) { \
                if (in_data[i] != 0) count++; \
            } \
        } else { \
            NIter it; NIter_FromNode(&it, n1, NITER_MODE_STRIDED); \
            NIter_ITER(&it); \
            while (NIter_NOTDONE(&it)) { \
                if (*(I_NT*)NIter_ITEM(&it) != 0) count++; \
                NIter_NEXT(&it); \
            } \
        } \
        out_data[0] = count; \
    } else { \
        int is_reduced[NR_NODE_MAX_NDIM] = {0}; \
        for (int i = 0; i < rargs->n_axis; i++) { \
            int ax = rargs->axis[i]; \
            is_reduced[ax < 0 ? ax + n1->ndim : ax] = 1; \
        } \
        for (nr_intp i = 0; i < n_out; i++) out_data[i] = 0; \
        \
        nr_intp coords[NR_NODE_MAX_NDIM]; \
        for (nr_intp lin = 0; lin < n_in; lin++) { \
            linear_to_coords(lin, n1->shape, n1->ndim, coords); \
            I_NT val = READ_INPUT(I_NT, in_data, lin, coords, n1, in_contig); \
            if (val != 0) { \
                nr_intp oi = coords_to_out_idx(coords, out_shape, is_reduced, n1->ndim, out_ndim); \
                out_data[oi]++; \
            } \
        } \
    } \
    FINALIZE_REDUCE_OUTPUT() \
}

/* ============================================================================
 * TYPE INSTANTIATION MACROS
 * ============================================================================ */

/* Sum/Prod: integers -> int64/uint64, floats -> float64 */
#define DEFINE_REDUCE_PROMOTED(OP, FUNC, INIT, NEED_FIRST) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_int64,   nr_bool,    INIT, NEED_FIRST, NR_INT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_int64,   nr_int8,    INIT, NEED_FIRST, NR_INT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_int64,   nr_int16,   INIT, NEED_FIRST, NR_INT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_int64,   nr_int32,   INIT, NEED_FIRST, NR_INT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_int64,   nr_int64,   INIT, NEED_FIRST, NR_INT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_uint64,  nr_uint8,   INIT, NEED_FIRST, NR_UINT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_uint64,  nr_uint16,  INIT, NEED_FIRST, NR_UINT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_uint64,  nr_uint32,  INIT, NEED_FIRST, NR_UINT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_uint64,  nr_uint64,  INIT, NEED_FIRST, NR_UINT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_float64, nr_float32, INIT, NEED_FIRST, NR_FLOAT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_float64, nr_float64, INIT, NEED_FIRST, NR_FLOAT64)

/* Min/Max: same type in/out */
#define DEFINE_REDUCE_SAME_TYPE(OP, FUNC, INIT, NEED_FIRST) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_bool,    nr_bool,    INIT, NEED_FIRST, NR_BOOL) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_int8,    nr_int8,    INIT, NEED_FIRST, NR_INT8) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_int16,   nr_int16,   INIT, NEED_FIRST, NR_INT16) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_int32,   nr_int32,   INIT, NEED_FIRST, NR_INT32) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_int64,   nr_int64,   INIT, NEED_FIRST, NR_INT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_uint8,   nr_uint8,   INIT, NEED_FIRST, NR_UINT8) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_uint16,  nr_uint16,  INIT, NEED_FIRST, NR_UINT16) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_uint32,  nr_uint32,  INIT, NEED_FIRST, NR_UINT32) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_uint64,  nr_uint64,  INIT, NEED_FIRST, NR_UINT64) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_float32, nr_float32, INIT, NEED_FIRST, NR_FLOAT32) \
    DEFINE_REDUCE_KERNEL(OP, FUNC, nr_float64, nr_float64, INIT, NEED_FIRST, NR_FLOAT64)

/* NaN variants: float only */
#define DEFINE_NANREDUCE_FLOATS(OP, FUNC, INIT, NEED_FIRST) \
    DEFINE_NANREDUCE_KERNEL(OP, FUNC, nr_float64, nr_float32, INIT, NEED_FIRST, NR_FLOAT64, ISNAN_F32) \
    DEFINE_NANREDUCE_KERNEL(OP, FUNC, nr_float64, nr_float64, INIT, NEED_FIRST, NR_FLOAT64, ISNAN_F64)

/* Mean: all types -> float64 */
#define DEFINE_MEAN_ALL(OP, ISNAN_CHECK) \
    DEFINE_MEAN_KERNEL(OP, nr_float64, nr_bool,    NR_FLOAT64, ISNAN_CHECK, ISNAN_INT) \
    DEFINE_MEAN_KERNEL(OP, nr_float64, nr_int8,    NR_FLOAT64, ISNAN_CHECK, ISNAN_INT) \
    DEFINE_MEAN_KERNEL(OP, nr_float64, nr_int16,   NR_FLOAT64, ISNAN_CHECK, ISNAN_INT) \
    DEFINE_MEAN_KERNEL(OP, nr_float64, nr_int32,   NR_FLOAT64, ISNAN_CHECK, ISNAN_INT) \
    DEFINE_MEAN_KERNEL(OP, nr_float64, nr_int64,   NR_FLOAT64, ISNAN_CHECK, ISNAN_INT) \
    DEFINE_MEAN_KERNEL(OP, nr_float64, nr_uint8,   NR_FLOAT64, ISNAN_CHECK, ISNAN_INT) \
    DEFINE_MEAN_KERNEL(OP, nr_float64, nr_uint16,  NR_FLOAT64, ISNAN_CHECK, ISNAN_INT) \
    DEFINE_MEAN_KERNEL(OP, nr_float64, nr_uint32,  NR_FLOAT64, ISNAN_CHECK, ISNAN_INT) \
    DEFINE_MEAN_KERNEL(OP, nr_float64, nr_uint64,  NR_FLOAT64, ISNAN_CHECK, ISNAN_INT) \
    DEFINE_MEAN_KERNEL(OP, nr_float64, nr_float32, NR_FLOAT64, ISNAN_CHECK, ISNAN_F32) \
    DEFINE_MEAN_KERNEL(OP, nr_float64, nr_float64, NR_FLOAT64, ISNAN_CHECK, ISNAN_F64)

/* Var/Std: all types -> float64 */
#define DEFINE_VAR_ALL(OP, ISNAN_CHECK, DO_SQRT) \
    DEFINE_VAR_KERNEL(OP, nr_float64, nr_bool,    NR_FLOAT64, ISNAN_CHECK, ISNAN_INT, DO_SQRT) \
    DEFINE_VAR_KERNEL(OP, nr_float64, nr_int8,    NR_FLOAT64, ISNAN_CHECK, ISNAN_INT, DO_SQRT) \
    DEFINE_VAR_KERNEL(OP, nr_float64, nr_int16,   NR_FLOAT64, ISNAN_CHECK, ISNAN_INT, DO_SQRT) \
    DEFINE_VAR_KERNEL(OP, nr_float64, nr_int32,   NR_FLOAT64, ISNAN_CHECK, ISNAN_INT, DO_SQRT) \
    DEFINE_VAR_KERNEL(OP, nr_float64, nr_int64,   NR_FLOAT64, ISNAN_CHECK, ISNAN_INT, DO_SQRT) \
    DEFINE_VAR_KERNEL(OP, nr_float64, nr_uint8,   NR_FLOAT64, ISNAN_CHECK, ISNAN_INT, DO_SQRT) \
    DEFINE_VAR_KERNEL(OP, nr_float64, nr_uint16,  NR_FLOAT64, ISNAN_CHECK, ISNAN_INT, DO_SQRT) \
    DEFINE_VAR_KERNEL(OP, nr_float64, nr_uint32,  NR_FLOAT64, ISNAN_CHECK, ISNAN_INT, DO_SQRT) \
    DEFINE_VAR_KERNEL(OP, nr_float64, nr_uint64,  NR_FLOAT64, ISNAN_CHECK, ISNAN_INT, DO_SQRT) \
    DEFINE_VAR_KERNEL(OP, nr_float64, nr_float32, NR_FLOAT64, ISNAN_CHECK, ISNAN_F32, DO_SQRT) \
    DEFINE_VAR_KERNEL(OP, nr_float64, nr_float64, NR_FLOAT64, ISNAN_CHECK, ISNAN_F64, DO_SQRT)

/* Argmin/Argmax: all types -> int64 */
#define DEFINE_ARG_ALL(OP, COMPARE_OP) \
    DEFINE_ARG_KERNEL(OP, nr_bool,    COMPARE_OP, NR_INT64) \
    DEFINE_ARG_KERNEL(OP, nr_int8,    COMPARE_OP, NR_INT64) \
    DEFINE_ARG_KERNEL(OP, nr_int16,   COMPARE_OP, NR_INT64) \
    DEFINE_ARG_KERNEL(OP, nr_int32,   COMPARE_OP, NR_INT64) \
    DEFINE_ARG_KERNEL(OP, nr_int64,   COMPARE_OP, NR_INT64) \
    DEFINE_ARG_KERNEL(OP, nr_uint8,   COMPARE_OP, NR_INT64) \
    DEFINE_ARG_KERNEL(OP, nr_uint16,  COMPARE_OP, NR_INT64) \
    DEFINE_ARG_KERNEL(OP, nr_uint32,  COMPARE_OP, NR_INT64) \
    DEFINE_ARG_KERNEL(OP, nr_uint64,  COMPARE_OP, NR_INT64) \
    DEFINE_ARG_KERNEL(OP, nr_float32, COMPARE_OP, NR_INT64) \
    DEFINE_ARG_KERNEL(OP, nr_float64, COMPARE_OP, NR_INT64)

/* All/Any: all types -> bool */
#define DEFINE_BOOL_ALL(OP, INIT, SHORT) \
    DEFINE_BOOL_REDUCE_KERNEL(OP, nr_bool,    INIT, SHORT, NR_BOOL) \
    DEFINE_BOOL_REDUCE_KERNEL(OP, nr_int8,    INIT, SHORT, NR_BOOL) \
    DEFINE_BOOL_REDUCE_KERNEL(OP, nr_int16,   INIT, SHORT, NR_BOOL) \
    DEFINE_BOOL_REDUCE_KERNEL(OP, nr_int32,   INIT, SHORT, NR_BOOL) \
    DEFINE_BOOL_REDUCE_KERNEL(OP, nr_int64,   INIT, SHORT, NR_BOOL) \
    DEFINE_BOOL_REDUCE_KERNEL(OP, nr_uint8,   INIT, SHORT, NR_BOOL) \
    DEFINE_BOOL_REDUCE_KERNEL(OP, nr_uint16,  INIT, SHORT, NR_BOOL) \
    DEFINE_BOOL_REDUCE_KERNEL(OP, nr_uint32,  INIT, SHORT, NR_BOOL) \
    DEFINE_BOOL_REDUCE_KERNEL(OP, nr_uint64,  INIT, SHORT, NR_BOOL) \
    DEFINE_BOOL_REDUCE_KERNEL(OP, nr_float32, INIT, SHORT, NR_BOOL) \
    DEFINE_BOOL_REDUCE_KERNEL(OP, nr_float64, INIT, SHORT, NR_BOOL)

/* CountNonzero: all types -> int64 */
#define DEFINE_COUNT_ALL(OP) \
    DEFINE_COUNT_KERNEL(OP, nr_bool,    NR_INT64) \
    DEFINE_COUNT_KERNEL(OP, nr_int8,    NR_INT64) \
    DEFINE_COUNT_KERNEL(OP, nr_int16,   NR_INT64) \
    DEFINE_COUNT_KERNEL(OP, nr_int32,   NR_INT64) \
    DEFINE_COUNT_KERNEL(OP, nr_int64,   NR_INT64) \
    DEFINE_COUNT_KERNEL(OP, nr_uint8,   NR_INT64) \
    DEFINE_COUNT_KERNEL(OP, nr_uint16,  NR_INT64) \
    DEFINE_COUNT_KERNEL(OP, nr_uint32,  NR_INT64) \
    DEFINE_COUNT_KERNEL(OP, nr_uint64,  NR_INT64) \
    DEFINE_COUNT_KERNEL(OP, nr_float32, NR_INT64) \
    DEFINE_COUNT_KERNEL(OP, nr_float64, NR_INT64)

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
NR_PUBLIC Node* NMath_##ApiName(Node* c, Node* a, int* axis, int na) { \
    NFuncArgs* args = NFuncArgs_New(1, 1); \
    args->in_nodes[0] = a; \
    args->out_nodes[0] = c; \
    NFunc_ReduceArgs rargs = NFunc_ReduceArgs_New(axis, na); \
    args->extra = &rargs; \
    int result = NFunc_Call(&name_str##_nfunc, args); \
    Node* out = args->out_nodes[0]; \
    NFuncArgs_DECREF(args); \
    return result != 0 ? NULL : out; \
}

/* ============================================================================
 * INSTANTIATE ALL OPERATIONS
 * ============================================================================ */

/* === Sum === */
DEFINE_REDUCE_PROMOTED(Sum, OP_SUM, 0, 0)
DEFINE_DISPATCHER(Sum)
DEFINE_NFUNC(Sum, sum)
DEFINE_API(Sum, sum)

/* === Prod === */
DEFINE_REDUCE_PROMOTED(Prod, OP_PROD, 1, 0)
DEFINE_DISPATCHER(Prod)
DEFINE_NFUNC(Prod, prod)
DEFINE_API(Prod, prod)

/* === Min === */
DEFINE_REDUCE_SAME_TYPE(Min, OP_MIN, 0, 1)
DEFINE_DISPATCHER(Min)
DEFINE_NFUNC(Min, min)
DEFINE_API(Min, min)

/* === Max === */
DEFINE_REDUCE_SAME_TYPE(Max, OP_MAX, 0, 1)
DEFINE_DISPATCHER(Max)
DEFINE_NFUNC(Max, max)
DEFINE_API(Max, max)

/* === Mean === */
DEFINE_MEAN_ALL(Mean, 0)
DEFINE_DISPATCHER(Mean)
DEFINE_NFUNC(Mean, mean)
DEFINE_API(Mean, mean)

/* === Var === */
DEFINE_VAR_ALL(Var, 0, 0)
DEFINE_DISPATCHER(Var)
DEFINE_NFUNC(Var, var)
DEFINE_API(Var, var)

/* === Std === */
DEFINE_VAR_ALL(Std, 0, 1)
DEFINE_DISPATCHER(Std)
DEFINE_NFUNC(Std, std)
DEFINE_API(Std, std)

/* === Argmin === */
DEFINE_ARG_ALL(Argmin, <)
DEFINE_DISPATCHER(Argmin)
DEFINE_NFUNC(Argmin, argmin)
DEFINE_API(Argmin, argmin)

/* === Argmax === */
DEFINE_ARG_ALL(Argmax, >)
DEFINE_DISPATCHER(Argmax)
DEFINE_NFUNC(Argmax, argmax)
DEFINE_API(Argmax, argmax)

/* === All === */
DEFINE_BOOL_ALL(All, 1, 0)
DEFINE_DISPATCHER(All)
DEFINE_NFUNC(All, all)
DEFINE_API(All, all)

/* === Any === */
DEFINE_BOOL_ALL(Any, 0, 1)
DEFINE_DISPATCHER(Any)
DEFINE_NFUNC(Any, any)
DEFINE_API(Any, any)

/* === CountNonzero === */
DEFINE_COUNT_ALL(CountNonzero)
DEFINE_DISPATCHER(CountNonzero)
DEFINE_NFUNC(CountNonzero, count_nonzero)
DEFINE_API(CountNonzero, count_nonzero)

/* ============================================================================
 * NAN-IGNORING VARIANTS (float types only)
 * ============================================================================ */

/* === NanSum === */
DEFINE_NANREDUCE_FLOATS(NanSum, OP_SUM, 0, 0)
DEFINE_DISPATCHER_FLOAT_ONLY(NanSum)
DEFINE_NFUNC(NanSum, nansum)
DEFINE_API(NanSum, nansum)

/* === NanProd === */
DEFINE_NANREDUCE_FLOATS(NanProd, OP_PROD, 1, 0)
DEFINE_DISPATCHER_FLOAT_ONLY(NanProd)
DEFINE_NFUNC(NanProd, nanprod)
DEFINE_API(NanProd, nanprod)

/* === NanMin === */
DEFINE_NANREDUCE_FLOATS(NanMin, OP_MIN, 0, 1)
DEFINE_DISPATCHER_FLOAT_ONLY(NanMin)
DEFINE_NFUNC(NanMin, nanmin)
DEFINE_API(NanMin, nanmin)

/* === NanMax === */
DEFINE_NANREDUCE_FLOATS(NanMax, OP_MAX, 0, 1)
DEFINE_DISPATCHER_FLOAT_ONLY(NanMax)
DEFINE_NFUNC(NanMax, nanmax)
DEFINE_API(NanMax, nanmax)

/* === NanMean === */
DEFINE_MEAN_KERNEL(NanMean, nr_float64, nr_float32, NR_FLOAT64, 1, ISNAN_F32)
DEFINE_MEAN_KERNEL(NanMean, nr_float64, nr_float64, NR_FLOAT64, 1, ISNAN_F64)
DEFINE_DISPATCHER_FLOAT_ONLY(NanMean)
DEFINE_NFUNC(NanMean, nanmean)
DEFINE_API(NanMean, nanmean)

/* === NanVar === */
DEFINE_VAR_KERNEL(NanVar, nr_float64, nr_float32, NR_FLOAT64, 1, ISNAN_F32, 0)
DEFINE_VAR_KERNEL(NanVar, nr_float64, nr_float64, NR_FLOAT64, 1, ISNAN_F64, 0)
DEFINE_DISPATCHER_FLOAT_ONLY(NanVar)
DEFINE_NFUNC(NanVar, nanvar)
DEFINE_API(NanVar, nanvar)

/* === NanStd === */
DEFINE_VAR_KERNEL(NanStd, nr_float64, nr_float32, NR_FLOAT64, 1, ISNAN_F32, 1)
DEFINE_VAR_KERNEL(NanStd, nr_float64, nr_float64, NR_FLOAT64, 1, ISNAN_F64, 1)
DEFINE_DISPATCHER_FLOAT_ONLY(NanStd)
DEFINE_NFUNC(NanStd, nanstd)
DEFINE_API(NanStd, nanstd)