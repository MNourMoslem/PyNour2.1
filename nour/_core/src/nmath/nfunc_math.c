#include "nfunc_math.h"
#include "../node_core.h"
#include "../niter.h"
#include "../ntools.h"
#include "../nerror.h"
#include "../node2str.h"
#include "../free.h"
#include "../tc_methods.h"
#include "loops.h"
#include "nour/nr_math.h"


#define FUNC_NAME(OP_NAME, I_NT) OP_NAME##_kernel_##I_NT

/*
 * 2-Input / 1-Output Elementwise NFunc Kernel Template
 * ----------------------------------------------------
 * Use DEFINE_BIN_EWISE_KERNEL to declare a per-dtype kernel implementing
 * an elementwise binary operation. Each kernel:
 *  - Accepts already validated/promotion-adjusted inputs (same dtype or broadcastable).
 *  - Allocates output node if NULL (using broadcast shape when needed).
 *  - Handles fast path for same-shape contiguous memory.
 *  - Falls back to NMultiIter for broadcasting or strided iteration.
 *
 * Parameters:
 *  - OP_NAME: Operation name (e.g., add, sub, mul)
 *  - OP_MACRO: Function-style macro that takes two args (e.g., NMATH_ADD, NMATH_SUB)
 *  - I_NT: Input C type (e.g., nr_int32, nr_float64)
 *  - O_NT: Output C type (typically same as I_NT)
 *
 * Returns 0 on success, -1 on error.
 */
#define DEFINE_BIN_EWISE_KERNEL(OP_NAME, OP_MACRO, I_NT, O_NT)                      \
NR_STATIC int FUNC_NAME(OP_NAME, I_NT)(NFuncArgs* args){                            \
    Node* n1 = args->in_nodes[0];                                                   \
    Node* n2 = args->in_nodes[1];                                                   \
    Node* out = args->out_nodes[0];                                                 \
                                                                                    \
    int ss = Node_SameShape(n1, n2);                                                \
    if (!out && ss) {                                                               \
        out = Node_NewEmpty(n1->ndim, n1->shape, args->outtype);                    \
        if (!out) {                                                                 \
            return -1;                                                              \
        }                                                                           \
    }                                                                               \
                                                                                    \
    int n1c = NODE_IS_CONTIGUOUS(n1);                                               \
    int n2c = NODE_IS_CONTIGUOUS(n2);                                               \
    int outc = out ? NODE_IS_CONTIGUOUS(out) : 0;                                   \
                                                                                    \
    if (ss) {                                                                       \
        if (outc) {                                                                 \
            if (n1c & n2c) {                                                        \
                NMATH_LOOP_CCC(OP_MACRO, O_NT, I_NT);                               \
            } else if (n1c | n2c) {                                                 \
                NMATH_LOOP_CSC(OP_MACRO, O_NT, I_NT);                               \
            } else {                                                                \
                NMATH_LOOP_CSS(OP_MACRO, O_NT, I_NT);                               \
            }                                                                       \
        } else {                                                                    \
            if (n1c & n2c) {                                                        \
                NMATH_LOOP_SCC(OP_MACRO, O_NT, I_NT);                               \
            } else if (n1c | n2c) {                                                 \
                NMATH_LOOP_SSC(OP_MACRO, O_NT, I_NT);                               \
            } else {                                                                \
                NMATH_LOOP_SSS(OP_MACRO, O_NT, I_NT);                               \
            }                                                                       \
        }                                                                           \
    } else {                                                                        \
        int issclr = NODE_IS_SCALAR(n1) | NODE_IS_SCALAR(n2);                       \
        if (issclr) {                                                               \
            I_NT sclr = *(I_NT*)(NODE_IS_SCALAR(n1) ? n1->data : n2->data);         \
            Node* n = NODE_IS_SCALAR(n1) ? n2 : n1;                                 \
            int nc = NODE_IS_CONTIGUOUS(n);                                         \
                                                                                    \
            if (outc) {                                                             \
                if (nc) {                                                           \
                    NMATH_LOOP_CC_S(OP_MACRO, O_NT, I_NT);                          \
                } else {                                                            \
                    NMATH_LOOP_CS_S(OP_MACRO, O_NT, I_NT);                          \
                }                                                                   \
            } else {                                                                \
                if (nc) {                                                           \
                    NMATH_LOOP_SC_S(OP_MACRO, O_NT, I_NT);                          \
                } else {                                                            \
                    NMATH_LOOP_SS_S(OP_MACRO, O_NT, I_NT);                          \
                }                                                                   \
            }                                                                       \
        } else {                                                                    \
            NMultiIter mit;                                                         \
            if (NMultiIter_FromNodes(args->in_nodes, 2, &mit) != 0) {               \
                return -1;                                                          \
            }                                                                       \
                                                                                    \
            if (!out) {                                                             \
                out = Node_NewEmpty(mit.out_ndim, mit.out_shape, args->outtype);    \
                if (!out) {                                                         \
                    return -1;                                                      \
                }                                                                   \
            }                                                                       \
                                                                                    \
            nr_size_t i = 0;                                                        \
            NMultiIter_ITER(&mit);                                                  \
            while (NMultiIter_NOTDONE(&mit)) {                                      \
                *((O_NT*)out->data + i) = OP_MACRO(                                 \
                    *((I_NT*)NMultiIter_ITEM(&mit, 0)),                             \
                    *((I_NT*)NMultiIter_ITEM(&mit, 1))                              \
                );                                                                  \
                i++;                                                                \
                NMultiIter_NEXT2(&mit);                                             \
            }                                                                       \
        }                                                                           \
    }                                                                               \
    args->out_nodes[0] = out;                                                       \
    return 0;                                                                       \
}

/* -------- TYPE GROUPS -------- */

#define BOOL_KERNEL(OP, MACRO) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_bool, nr_bool)

#define INT_KERNELS(OP, MACRO) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int8,    nr_int8) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint8,   nr_uint8) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int16,   nr_int16) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint16,  nr_uint16) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int32,   nr_int32) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint32,  nr_uint32) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int64,   nr_int64) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint64,  nr_uint64)

#define FLOAT_KERNELS(OP, MACRO) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_float32, nr_float32) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_float64, nr_float64)
/* -------- NEW VERSION: TYPE FILTERING -------- */

#define DEFINE_BIN_EWISE_ALL_TYPES(OP_NAME, OP_MACRO, ALLOW_BOOL, ALLOW_INT, ALLOW_FLOAT) \
    ENABLE_IF_##ALLOW_BOOL  ( BOOL_KERNEL (OP_NAME, OP_MACRO) ) \
    ENABLE_IF_##ALLOW_INT   ( INT_KERNELS(OP_NAME, OP_MACRO) ) \
    ENABLE_IF_##ALLOW_FLOAT ( FLOAT_KERNELS(OP_NAME, OP_MACRO) )


/* ----------------------------------------------
   Type blocks (expanded depending on flags)
   ---------------------------------------------- */
#define BOOL_BLOCK(OP, FUNC) \
    case NR_BOOL: FUNC = FUNC_NAME(OP, nr_bool); break;

#define INT_BLOCK(OP, FUNC) \
    case NR_INT8:    FUNC = FUNC_NAME(OP, nr_int8);  break; \
    case NR_UINT8:   FUNC = FUNC_NAME(OP, nr_uint8); break; \
    case NR_INT16:   FUNC = FUNC_NAME(OP, nr_int16); break; \
    case NR_UINT16:  FUNC = FUNC_NAME(OP, nr_uint16); break; \
    case NR_INT32:   FUNC = FUNC_NAME(OP, nr_int32); break; \
    case NR_UINT32:  FUNC = FUNC_NAME(OP, nr_uint32); break; \
    case NR_INT64:   FUNC = FUNC_NAME(OP, nr_int64); break; \
    case NR_UINT64:  FUNC = FUNC_NAME(OP, nr_uint64); break;

#define FLOAT_BLOCK(OP, FUNC) \
    case NR_FLOAT32: FUNC = FUNC_NAME(OP, nr_float32); break; \
    case NR_FLOAT64: FUNC = FUNC_NAME(OP, nr_float64); break;

/* ----------------------------------------------
   ENABLE / DISABLE blocks at compile time
   ---------------------------------------------- */
#define ENABLE_IF_1(block) block
#define ENABLE_IF_0(block) /* nothing */

/* ----------------------------------------------
   MAIN MACRO — this is what you call.
   Example:
     DEFINE_DTYPE_TO_FUNC(add, dtype, func, 1, 1, 1)
     DEFINE_DTYPE_TO_FUNC(div, dtype, func, 0, 0, 1)
   ---------------------------------------------- */
#define DEFINE_DTYPE_TO_FUNC(OP, DTYPE, func, ALLOW_BOOL, ALLOW_INT, ALLOW_FLOAT) \
do { \
    switch (DTYPE) { \
        ENABLE_IF_##ALLOW_BOOL ( BOOL_BLOCK(OP, func) ) \
        ENABLE_IF_##ALLOW_INT  ( INT_BLOCK(OP, func) ) \
        ENABLE_IF_##ALLOW_FLOAT( FLOAT_BLOCK(OP, func) ) \
        default: func = NULL; break; \
    } \
} while (0)


/* Main dispatch function generator.
   This now uses the `func` pointer set by DEFINE_DTYPE_TO_FUNC
   and invokes it, or raises an error if unsupported.
*/
#define DEFINE_BIN_EWISE_MAIN_FUNC(OP_NAME, OP_STR, ALLOW_BOOL, ALLOW_INT, ALLOW_FLOAT) \
NR_PRIVATE int OP_NAME##_function(NFuncArgs* args){            \
    Node* a = args->in_nodes[0];                                  \
    NR_DTYPE adt = NODE_DTYPE(a);                              \
    NFuncFunc func = NULL;                                     \
    DEFINE_DTYPE_TO_FUNC(OP_NAME, adt, func, ALLOW_BOOL, ALLOW_INT, ALLOW_FLOAT); \
    if (!func) {                                                \
        NError_RaiseError(NError_TypeError, OP_STR " unsupported dtype %d", adt); \
        return -1;                                              \
    }                                                           \
    return func(args);                                          \
}


/* -------- Function Implementations -------- */
// Addition
DEFINE_BIN_EWISE_ALL_TYPES(Add, NMATH_ADD, 1, 1, 1)
DEFINE_BIN_EWISE_MAIN_FUNC(Add, "add", 1, 1, 1)
const NFunc add_nfunc = {
    .name = "add",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_NONE,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Add_function,
    .grad_func = NULL
};

// Subtraction
DEFINE_BIN_EWISE_ALL_TYPES(Sub, NMATH_SUB, 1, 1, 1)
DEFINE_BIN_EWISE_MAIN_FUNC(Sub, "sub", 1, 1, 1)
const NFunc sub_nfunc = {
    .name = "sub",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_NONE,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Sub_function,
    .grad_func = NULL
};

// Multiplication
DEFINE_BIN_EWISE_ALL_TYPES(Mul, NMATH_MUL, 1, 1, 1)
DEFINE_BIN_EWISE_MAIN_FUNC(Mul, "mul", 1, 1, 1)
const NFunc mul_nfunc = {
    .name = "mul",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_NONE,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Mul_function,
    .grad_func = NULL
};

// Division (float only)
DEFINE_BIN_EWISE_ALL_TYPES(Div, NMATH_DIV, 0, 0, 1)
DEFINE_BIN_EWISE_MAIN_FUNC(Div, "div", 0, 0, 1)
const NFunc div_nfunc = {
    .name = "div",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Div_function,
    .grad_func = NULL
};


// True division (int only)
DEFINE_BIN_EWISE_ALL_TYPES(TrueDiv, NMATH_TRUEDIV, 1, 1, 0)
DEFINE_BIN_EWISE_MAIN_FUNC(TrueDiv, "true div", 1, 1, 0)
const NFunc truediv_nfunc = {
    .name = "truediv",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_INT,
    .out_type = NDTYPE_INT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = TrueDiv_function,
    .grad_func = NULL
};


//Modulus (int only)
DEFINE_BIN_EWISE_ALL_TYPES(Mod, NMATH_MOD, 1, 1, 0)
DEFINE_BIN_EWISE_MAIN_FUNC(Mod, "mod", 1, 1, 0)
const NFunc mod_nfunc = {
    .name = "mod",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_INT,
    .out_type = NDTYPE_INT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Mod_function,
    .grad_func = NULL
};


// Power
DEFINE_BIN_EWISE_ALL_TYPES(Pow, NMATH_POW, 1, 1, 1)
DEFINE_BIN_EWISE_MAIN_FUNC(Pow, "pow", 1, 1, 1)
const NFunc pow_nfunc = {
    .name = "pow",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_NONE,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Pow_function,
    .grad_func = NULL
};

/*
    Boolean Comparison Operations
*/

#define BOOL_OPERATIONS_KERNEL(OP, MACRO) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_bool,    nr_bool) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int8,    nr_bool) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint8,   nr_bool) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int16,   nr_bool) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint16,  nr_bool) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int32,   nr_bool) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint32,  nr_bool) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int64,   nr_bool) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint64,  nr_bool) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_float32, nr_bool) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_float64, nr_bool)


// Bigger Than
BOOL_OPERATIONS_KERNEL(Bg, NMATH_BG)
DEFINE_BIN_EWISE_MAIN_FUNC(Bg, "bigger than", 1, 1, 1)
const NFunc bg_nfunc = {
    .name = "bg",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_BOOL,
    .in_dtype = NR_NONE,
    .out_dtype = NR_BOOL,
    .func = Bg_function,
    .grad_func = NULL
};

// Bigger Equal Than
BOOL_OPERATIONS_KERNEL(Bge, NMATH_BGE)
DEFINE_BIN_EWISE_MAIN_FUNC(Bge, "bigger equal than", 1, 1, 1)
const NFunc bge_nfunc = {
    .name = "bge",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_BOOL,
    .in_dtype = NR_NONE,
    .out_dtype = NR_BOOL,
    .func = Bge_function,
    .grad_func = NULL
};

// Less Than
BOOL_OPERATIONS_KERNEL(Ls, NMATH_LS)
DEFINE_BIN_EWISE_MAIN_FUNC(Ls, "less than", 1, 1, 1)
const NFunc ls_nfunc = {
    .name = "ls",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_BOOL,
    .in_dtype = NR_NONE,
    .out_dtype = NR_BOOL,
    .func = Ls_function,
    .grad_func = NULL
};

// Less Equal Than
BOOL_OPERATIONS_KERNEL(Lse, NMATH_LSE)
DEFINE_BIN_EWISE_MAIN_FUNC(Lse, "less equal than", 1, 1, 1)
const NFunc lse_nfunc = {
    .name = "lse",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_BOOL,
    .in_dtype = NR_NONE,
    .out_dtype = NR_BOOL,
    .func = Lse_function,
    .grad_func = NULL
};

// Equal To
BOOL_OPERATIONS_KERNEL(Eq, NMATH_EQ)
DEFINE_BIN_EWISE_MAIN_FUNC(Eq, "equal to", 1, 1, 1)
const NFunc eq_nfunc = {
    .name = "eq",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_BOOL,
    .in_dtype = NR_NONE,
    .out_dtype = NR_BOOL,
    .func = Eq_function,
    .grad_func = NULL
};

// Not Equal To
BOOL_OPERATIONS_KERNEL(Neq, NMATH_NEQ)
DEFINE_BIN_EWISE_MAIN_FUNC(Neq, "not equal to", 1, 1, 1)
const NFunc neq_nfunc = {
    .name = "neq",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_BOOL,
    .in_dtype = NR_NONE,
    .out_dtype = NR_BOOL,
    .func = Neq_function,
    .grad_func = NULL
};


/*
    Bitwise Operations
*/

#define BITWISE_OPERATIONS_KERNEL(OP, MACRO) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_bool,    nr_bool) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int8,    nr_int8) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint8,   nr_uint8) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int16,   nr_int16) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint16,  nr_uint16) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int32,   nr_int32) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint32,  nr_uint32) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_int64,   nr_int64) \
    DEFINE_BIN_EWISE_KERNEL(OP, MACRO, nr_uint64,  nr_uint64)

// Bitwise AND
BITWISE_OPERATIONS_KERNEL(BitAnd, NMATH_BIT_AND)
DEFINE_BIN_EWISE_MAIN_FUNC(BitAnd, "bitwise and", 1, 1, 0)
const NFunc bit_and_nfunc = {
    .name = "and",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_INT,
    .out_type = NDTYPE_INT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = BitAnd_function,
    .grad_func = NULL
};

// Bitwise OR
BITWISE_OPERATIONS_KERNEL(BitOr, NMATH_BIT_OR)
DEFINE_BIN_EWISE_MAIN_FUNC(BitOr, "bitwise or", 1, 1, 0)
const NFunc bit_or_nfunc = {
    .name = "or",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_INT,
    .out_type = NDTYPE_INT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = BitOr_function,
    .grad_func = NULL
};

// Bitwise XOR
BITWISE_OPERATIONS_KERNEL(BitXor, NMATH_BIT_XOR)
DEFINE_BIN_EWISE_MAIN_FUNC(BitXor, "bitwise xor", 1, 1, 0)
const NFunc bit_xor_nfunc = {
    .name = "xor",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_INT,
    .out_type = NDTYPE_INT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = BitXor_function,
    .grad_func = NULL
};

// Bitwise Left Shift
BITWISE_OPERATIONS_KERNEL(BitLSH, NMATH_BIT_LSH)
DEFINE_BIN_EWISE_MAIN_FUNC(BitLSH, "bitwise left shift", 1, 1, 0)
const NFunc bit_lsh_nfunc = {
    .name = "lshift",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_INT,
    .out_type = NDTYPE_INT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = BitLSH_function,
    .grad_func = NULL
};

// Bitwise Right Shift
BITWISE_OPERATIONS_KERNEL(BitRSH, NMATH_BIT_RSH)
DEFINE_BIN_EWISE_MAIN_FUNC(BitRSH, "bitwise right shift", 1, 1, 0)
const NFunc bit_rsh_nfunc = {
    .name = "rshift",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_INT,
    .out_type = NDTYPE_INT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = BitRSH_function,
    .grad_func = NULL
};


/*
    Unary Mathematical Operations
*/

/*
 * 1-Input / 1-Output Elementwise NFunc Kernel Template
 * ----------------------------------------------------
 * Use DEFINE_UN_EWISE_KERNEL to declare a per-dtype kernel implementing
 * an elementwise unary operation. Each kernel:
 *  - Accepts already validated input.
 *  - Allocates output node if NULL (using same shape as input).
 *  - Handles fast path for contiguous memory.
 *  - Falls back to NIter for strided iteration.
 *
 * Parameters:
 *  - OP_NAME: Operation name (e.g., neg, abs, sin)
 *  - OP_MACRO: Function-style macro that takes one arg (e.g., NMATH_NEG, NMATH_ABS)
 *  - I_NT: Input C type (e.g., nr_int32, nr_float64)
 *  - O_NT: Output C type (typically same as I_NT)
 *
 * Returns 0 on success, -1 on error.
 */
#define DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, I_NT, O_NT)                                          \
NR_STATIC int FUNC_NAME(OP_NAME, I_NT)(NFuncArgs* args){                            \
    Node* n1 = args->in_nodes[0];                                                   \
    Node* out = args->out_nodes[0];                                                 \
                                                                                    \
    if (!out) {                                                                     \
        out = Node_NewEmpty(n1->ndim, n1->shape, args->outtype);                    \
        if (!out) {                                                                 \
            return -1;                                                              \
        }                                                                           \
    }                                                                               \
                                                                                    \
    int n1c = NODE_IS_CONTIGUOUS(n1);                                               \
    int outc = NODE_IS_CONTIGUOUS(out);                                             \
                                                                                    \
    if (outc) {                                                                     \
        if (n1c) {                                                                  \
            NMATH_LOOP_CC_1I(OP_MACRO, O_NT, I_NT);                                \
        } else {                                                                    \
            NMATH_LOOP_CS_1I(OP_MACRO, O_NT, I_NT);                                \
        }                                                                           \
    } else {                                                                        \
        if (n1c) {                                                                  \
            NMATH_LOOP_SC_1I(OP_MACRO, O_NT, I_NT);                                \
        } else {                                                                    \
            NMATH_LOOP_SS_1I(OP_MACRO, O_NT, I_NT);                                \
        }                                                                           \
    }                                                                               \
                                                                                    \
    args->out_nodes[0] = out;                                                                \
    return 0;                                                                       \
}

#define UN_EWISE_ALL_TYPES(OP_NAME, OP_MACRO, ALLOW_BOOL, ALLOW_INT, ALLOW_FLOAT) \
    ENABLE_IF_##ALLOW_BOOL  ( DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, nr_bool, nr_bool) ) \
    ENABLE_IF_##ALLOW_INT   ( DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, nr_int8, nr_int8) \
                             DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, nr_uint8, nr_uint8) \
                             DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, nr_int16, nr_int16) \
                             DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, nr_uint16, nr_uint16) \
                             DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, nr_int32, nr_int32) \
                             DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, nr_uint32, nr_uint32) \
                             DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, nr_int64, nr_int64) \
                             DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, nr_uint64, nr_uint64) ) \
    ENABLE_IF_##ALLOW_FLOAT ( DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, nr_float32, nr_float32) \
                             DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO, nr_float64, nr_float64) )

#define DEFINE_UN_EWISE_MAIN_FUNC(OP_NAME, OP_STR, ALLOW_BOOL, ALLOW_INT, ALLOW_FLOAT) \
NR_PRIVATE int OP_NAME##_function(NFuncArgs* args){            \
    Node* a = args->in_nodes[0];                                  \
    NR_DTYPE adt = NODE_DTYPE(a);                              \
    NFuncFunc func = NULL;                                     \
    DEFINE_DTYPE_TO_FUNC(OP_NAME, adt, func, ALLOW_BOOL, ALLOW_INT, ALLOW_FLOAT); \
    if (!func) {                                                \
        NError_RaiseError(NError_TypeError, OP_STR " unsupported dtype %d", adt); \
        return -1;                                              \
    }                                                           \
    return func(args);                                          \
}

// Negation (all numeric types)
UN_EWISE_ALL_TYPES(Neg, NMATH_NEG, 0, 1, 1)
DEFINE_UN_EWISE_MAIN_FUNC(Neg, "negation", 0, 1, 1)
const NFunc neg_nfunc = {
    .name = "neg",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_NONE,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Neg_function,
    .grad_func = NULL
};

// Bitwise NOT (integers only)
UN_EWISE_ALL_TYPES(BitNot, NMATH_BIT_NOT, 1, 1, 0)
DEFINE_UN_EWISE_MAIN_FUNC(BitNot, "bitwise not", 1, 1, 0)
const NFunc bit_not_nfunc = {
    .name = "bitnot",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_INT,
    .out_type = NDTYPE_INT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = BitNot_function,
    .grad_func = NULL
};

#define UN_EWISE_FLOAT_TYPES(OP_NAME, OP_MACRO_FLOAT32, OP_MACRO_FLOAT64) \
    DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO_FLOAT32, nr_float32, nr_float32) \
    DEFINE_UN_EWISE_KERNEL(OP_NAME, OP_MACRO_FLOAT64, nr_float64, nr_float64)

// Sine (floats only)
UN_EWISE_FLOAT_TYPES(Sin, nr_sinf, nr_sin)
DEFINE_UN_EWISE_MAIN_FUNC(Sin, "sine", 0, 0, 1)
const NFunc sin_nfunc = {
    .name = "sin",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Sin_function,
    .grad_func = NULL
};

// Cosine (floats only)
UN_EWISE_FLOAT_TYPES(Cos, nr_cosf, nr_cos)
DEFINE_UN_EWISE_MAIN_FUNC(Cos, "cosine", 0, 0, 1)
const NFunc cos_nfunc = {
    .name = "cos",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Cos_function,
    .grad_func = NULL
};

// Tangent (floats only)
UN_EWISE_FLOAT_TYPES(Tan, nr_tanf, nr_tan)
DEFINE_UN_EWISE_MAIN_FUNC(Tan, "tangent", 0, 0, 1)
const NFunc tan_nfunc = {
    .name = "tan",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Tan_function,
    .grad_func = NULL
};

// Cotangent (floats only)
UN_EWISE_FLOAT_TYPES(Cot, nr_cotf, nr_cot)
DEFINE_UN_EWISE_MAIN_FUNC(Cot, "cotangent", 0, 0, 1)
const NFunc cot_nfunc = {
    .name = "cot",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Cot_function,
    .grad_func = NULL
};

// Exponential (floats only)
UN_EWISE_FLOAT_TYPES(Exp, nr_expf, nr_exp)
DEFINE_UN_EWISE_MAIN_FUNC(Exp, "exponential", 0, 0, 1)
const NFunc exp_nfunc = {
    .name = "exp",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Exp_function,
    .grad_func = NULL
};

// Natural Logarithm (floats only)
UN_EWISE_FLOAT_TYPES(Log, nr_logf, nr_log)
DEFINE_UN_EWISE_MAIN_FUNC(Log, "natural logarithm", 0, 0, 1)
const NFunc log_nfunc = {
    .name = "log",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_TYPE_BROADCASTABLE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Log_function,
    .grad_func = NULL
};

// Hyperbolic Sine (floats only)
UN_EWISE_FLOAT_TYPES(Sinh, nr_sinhf, nr_sinh)
DEFINE_UN_EWISE_MAIN_FUNC(Sinh, "hyperbolic sine", 0, 0, 1)
const NFunc sinh_nfunc = {
    .name = "sinh",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Sinh_function,
    .grad_func = NULL
};

// Hyperbolic Cosine (floats only)
UN_EWISE_FLOAT_TYPES(Cosh, nr_coshf, nr_cosh)
DEFINE_UN_EWISE_MAIN_FUNC(Cosh, "hyperbolic cosine", 0, 0, 1)
const NFunc cosh_nfunc = {
    .name = "cosh",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Cosh_function,
    .grad_func = NULL
};

// Hyperbolic Tangent (floats only)
UN_EWISE_FLOAT_TYPES(Tanh, nr_tanhf, nr_tanh)
DEFINE_UN_EWISE_MAIN_FUNC(Tanh, "hyperbolic tangent", 0, 0, 1)
const NFunc tanh_nfunc = {
    .name = "tanh",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Tanh_function,
    .grad_func = NULL
};

// Hyperbolic Cotangent (floats only)
UN_EWISE_FLOAT_TYPES(Coth, nr_cothf, nr_coth)
DEFINE_UN_EWISE_MAIN_FUNC(Coth, "hyperbolic cotangent", 0, 0, 1)
const NFunc coth_nfunc = {
    .name = "coth",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Coth_function,
    .grad_func = NULL
};

// Arc Sine (floats only)
UN_EWISE_FLOAT_TYPES(Asin, nr_asinf, nr_asin)
DEFINE_UN_EWISE_MAIN_FUNC(Asin, "arc sine", 0, 0, 1)
const NFunc asin_nfunc = {
    .name = "asin",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Asin_function,
    .grad_func = NULL
};

// Arc Cosine (floats only)
UN_EWISE_FLOAT_TYPES(Acos, nr_acosf, nr_acos)
DEFINE_UN_EWISE_MAIN_FUNC(Acos, "arc cosine", 0, 0, 1)
const NFunc acos_nfunc = {
    .name = "acos",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Acos_function,
    .grad_func = NULL
};

// Arc Tangent (floats only)
UN_EWISE_FLOAT_TYPES(Atan, nr_atanf, nr_atan)
DEFINE_UN_EWISE_MAIN_FUNC(Atan, "arc tangent", 0, 0, 1)
const NFunc atan_nfunc = {
    .name = "atan",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Atan_function,
    .grad_func = NULL
};

// Inverse Hyperbolic Sine (floats only)
UN_EWISE_FLOAT_TYPES(Asinh, nr_asinhf, nr_asinh)
DEFINE_UN_EWISE_MAIN_FUNC(Asinh, "inverse hyperbolic sine", 0, 0, 1)
const NFunc asinh_nfunc = {
    .name = "asinh",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Asinh_function,
    .grad_func = NULL
};

// Inverse Hyperbolic Cosine (floats only)
UN_EWISE_FLOAT_TYPES(Acosh, nr_acoshf, nr_acosh)
DEFINE_UN_EWISE_MAIN_FUNC(Acosh, "inverse hyperbolic cosine", 0, 0, 1)
const NFunc acosh_nfunc = {
    .name = "acosh",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Acosh_function,
    .grad_func = NULL
};

// Inverse Hyperbolic Tangent (floats only)
UN_EWISE_FLOAT_TYPES(Atanh, nr_atanhf, nr_atanh)
DEFINE_UN_EWISE_MAIN_FUNC(Atanh, "inverse hyperbolic tangent", 0, 0, 1)
const NFunc atanh_nfunc = {
    .name = "atanh",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Atanh_function,
    .grad_func = NULL
};

// Base-2 Exponential (floats only)
UN_EWISE_FLOAT_TYPES(Exp2, nr_exp2f, nr_exp2)
DEFINE_UN_EWISE_MAIN_FUNC(Exp2, "base-2 exponential", 0, 0, 1)
const NFunc exp2_nfunc = {
    .name = "exp2",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Exp2_function,
    .grad_func = NULL
};

// Exponential minus 1 (floats only)
UN_EWISE_FLOAT_TYPES(Expm1, nr_expm1f, nr_expm1)
DEFINE_UN_EWISE_MAIN_FUNC(Expm1, "exponential minus 1", 0, 0, 1)
const NFunc expm1_nfunc = {
    .name = "expm1",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Expm1_function,
    .grad_func = NULL
};

// Base-10 Logarithm (floats only)
UN_EWISE_FLOAT_TYPES(Log10, nr_log10f, nr_log10)
DEFINE_UN_EWISE_MAIN_FUNC(Log10, "base-10 logarithm", 0, 0, 1)
const NFunc log10_nfunc = {
    .name = "log10",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Log10_function,
    .grad_func = NULL
};

// Logarithm plus 1 (floats only)
UN_EWISE_FLOAT_TYPES(Log1p, nr_log1pf, nr_log1p)
DEFINE_UN_EWISE_MAIN_FUNC(Log1p, "logarithm plus 1", 0, 0, 1)
const NFunc log1p_nfunc = {
    .name = "log1p",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Log1p_function,
    .grad_func = NULL
};

// Square Root (floats only)
UN_EWISE_FLOAT_TYPES(Sqrt, nr_sqrtf, nr_sqrt)
DEFINE_UN_EWISE_MAIN_FUNC(Sqrt, "square root", 0, 0, 1)
const NFunc sqrt_nfunc = {
    .name = "sqrt",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Sqrt_function,
    .grad_func = NULL
};

// Cube Root (floats only)
UN_EWISE_FLOAT_TYPES(Cbrt, nr_cbrtf, nr_cbrt)
DEFINE_UN_EWISE_MAIN_FUNC(Cbrt, "cube root", 0, 0, 1)
const NFunc cbrt_nfunc = {
    .name = "cbrt",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Cbrt_function,
    .grad_func = NULL
};

// Absolute Value (all numeric types)
// Special case. for signed integers we use NMATH_ABS.
// For unsigned integers and bools, we just do noting
// For floats we use nr_fabsf/nr_fabs.

#define nothing_op(x) (x)

DEFINE_UN_EWISE_KERNEL(Abs, nothing_op, nr_bool, nr_bool)
DEFINE_UN_EWISE_KERNEL(Abs, NMATH_ABS, nr_int8, nr_int8)
DEFINE_UN_EWISE_KERNEL(Abs, nothing_op, nr_uint8, nr_uint8)
DEFINE_UN_EWISE_KERNEL(Abs, NMATH_ABS, nr_int16, nr_int16)
DEFINE_UN_EWISE_KERNEL(Abs, nothing_op, nr_uint16, nr_uint16)
DEFINE_UN_EWISE_KERNEL(Abs, NMATH_ABS, nr_int32, nr_int32)
DEFINE_UN_EWISE_KERNEL(Abs, nothing_op, nr_uint32, nr_uint32)
DEFINE_UN_EWISE_KERNEL(Abs, NMATH_ABS, nr_int64, nr_int64)
DEFINE_UN_EWISE_KERNEL(Abs, nothing_op, nr_uint64, nr_uint64)
DEFINE_UN_EWISE_KERNEL(Abs, nr_fabsf, nr_float32, nr_float32)
DEFINE_UN_EWISE_KERNEL(Abs, nr_fabs, nr_float64, nr_float64)

DEFINE_UN_EWISE_MAIN_FUNC(Abs, "absolute value", 1, 1, 1)
const NFunc abs_nfunc = {
    .name = "abs",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_NONE,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Abs_function,
    .grad_func = NULL
};

// Ceiling (floats only)
UN_EWISE_FLOAT_TYPES(Ceil, nr_ceilf, nr_ceil)
DEFINE_UN_EWISE_MAIN_FUNC(Ceil, "ceiling", 0, 0, 1)
const NFunc ceil_nfunc = {
    .name = "ceil",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Ceil_function,
    .grad_func = NULL
};

// Floor (floats only)
UN_EWISE_FLOAT_TYPES(Floor, nr_floorf, nr_floor)
DEFINE_UN_EWISE_MAIN_FUNC(Floor, "floor", 0, 0, 1)
const NFunc floor_nfunc = {
    .name = "floor",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Floor_function,
    .grad_func = NULL
};

// Truncate (floats only)
UN_EWISE_FLOAT_TYPES(Trunc, nr_truncf, nr_trunc)
DEFINE_UN_EWISE_MAIN_FUNC(Trunc, "truncate", 0, 0, 1)
const NFunc trunc_nfunc = {
    .name = "trunc",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Trunc_function,
    .grad_func = NULL
};

// Round to nearest integer (floats only)
UN_EWISE_FLOAT_TYPES(Rint, nr_rintf, nr_rint)
DEFINE_UN_EWISE_MAIN_FUNC(Rint, "round to nearest integer", 0, 0, 1)
const NFunc rint_nfunc = {
    .name = "rint",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Rint_function,
    .grad_func = NULL
};


/*
    Special cases for: frexp, ldexp, modf
    These functions have multiple outputs and need special handling.
*/

// FREXP - Extract mantissa and exponent (1 input, 2 outputs)
NR_STATIC int Frexp_kernel_nr_float32(NFuncArgs* args){
    Node* n1 = args->in_nodes[0];
    Node* out_mantissa = args->out_nodes[0];
    Node* out_exponent = args->out_nodes[1];

    // Allocate output nodes (always NULL due to NFUNC_FLAG_NO_USER_OUT_NODES)
    out_mantissa = Node_NewEmpty(n1->ndim, n1->shape, NR_FLOAT32);
    if (!out_mantissa) {
        return -1;
    }

    out_exponent = Node_NewEmpty(n1->ndim, n1->shape, NR_INT32);
    if (!out_exponent) {
        NODE_DECREF(out_mantissa);
        return -1;
    }

    int n1c = NODE_IS_CONTIGUOUS(n1);
    int out1c = NODE_IS_CONTIGUOUS(out_mantissa);
    int out2c = NODE_IS_CONTIGUOUS(out_exponent);
    
    // Fast path: all contiguous
    if (n1c && out1c && out2c) {
        nr_float32* n1_data = (nr_float32*)n1->data;
        nr_float32* mant_data = (nr_float32*)out_mantissa->data;
        nr_int32* exp_data = (nr_int32*)out_exponent->data;
        
        nr_size_t size = Node_NItems(n1);
        for (nr_size_t i = 0; i < size; i++) {
            int exp;
            mant_data[i] = nr_frexpf(n1_data[i], &exp);
            exp_data[i] = exp;
        }
    }
    // Mixed contiguous/strided cases
    else if (n1c && (out1c || out2c)) {
        nr_float32* n1_data = (nr_float32*)n1->data;
        nr_size_t size = Node_NItems(n1);
        
        if (out1c && out2c) {
            // Input strided, outputs contiguous
            nr_float32* mant_data = (nr_float32*)out_mantissa->data;
            nr_int32* exp_data = (nr_int32*)out_exponent->data;
            
            NIter it;
            NIter_FromNode(&it, n1, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            nr_size_t i = 0;
            while (NIter_NOTDONE(&it)) {
                int exp;
                mant_data[i] = nr_frexpf(*(nr_float32*)NIter_ITEM(&it), &exp);
                exp_data[i] = exp;
                NIter_NEXT(&it);
                i++;
            }
        } else {
            // Input contiguous, some outputs strided
            NIter it1, it2;
            NIter_FromNode(&it1, out_mantissa, NITER_MODE_NONE);
            NIter_FromNode(&it2, out_exponent, NITER_MODE_NONE);
            NIter_ITER(&it1);
            NIter_ITER(&it2);
            
            for (nr_size_t i = 0; i < size; i++) {
                int exp;
                nr_float32 mant = nr_frexpf(n1_data[i], &exp);
                *(nr_float32*)NIter_ITEM(&it1) = mant;
                *(nr_int32*)NIter_ITEM(&it2) = exp;
                NIter_NEXT(&it1);
                NIter_NEXT(&it2);
            }
        }
    }
    // All strided
    else {
        NIter it_in, it_mant, it_exp;
        NIter_FromNode(&it_in, n1, NITER_MODE_NONE);
        NIter_FromNode(&it_mant, out_mantissa, NITER_MODE_NONE);
        NIter_FromNode(&it_exp, out_exponent, NITER_MODE_NONE);
        NIter_ITER(&it_in);
        NIter_ITER(&it_mant);
        NIter_ITER(&it_exp);
        
        while (NIter_NOTDONE(&it_in)) {
            int exp;
            nr_float32 mant = nr_frexpf(*(nr_float32*)NIter_ITEM(&it_in), &exp);
            *(nr_float32*)NIter_ITEM(&it_mant) = mant;
            *(nr_int32*)NIter_ITEM(&it_exp) = exp;
            NIter_NEXT(&it_in);
            NIter_NEXT(&it_mant);
            NIter_NEXT(&it_exp);
        }
    }

    args->out_nodes[0] = out_mantissa;
    args->out_nodes[1] = out_exponent;

    return 0;
}

NR_STATIC int Frexp_kernel_nr_float64(NFuncArgs* args){
    Node* n1 = args->in_nodes[0];
    Node* out_mantissa = args->out_nodes[0];
    Node* out_exponent = args->out_nodes[1];

    // Allocate output nodes (always NULL due to NFUNC_FLAG_NO_USER_OUT_NODES)
    out_mantissa = Node_NewEmpty(n1->ndim, n1->shape, NR_FLOAT64);
    if (!out_mantissa) {
        return -1;
    }

    out_exponent = Node_NewEmpty(n1->ndim, n1->shape, NR_INT32);
    if (!out_exponent) {
        NODE_DECREF(out_mantissa);
        return -1;
    }

    int n1c = NODE_IS_CONTIGUOUS(n1);
    int out1c = NODE_IS_CONTIGUOUS(out_mantissa);
    int out2c = NODE_IS_CONTIGUOUS(out_exponent);
    
    // Fast path: all contiguous
    if (n1c && out1c && out2c) {
        nr_float64* n1_data = (nr_float64*)n1->data;
        nr_float64* mant_data = (nr_float64*)out_mantissa->data;
        nr_int32* exp_data = (nr_int32*)out_exponent->data;
        
        nr_size_t size = Node_NItems(n1);
        for (nr_size_t i = 0; i < size; i++) {
            int exp;
            mant_data[i] = nr_frexp(n1_data[i], &exp);
            exp_data[i] = exp;
        }
    }
    // Mixed contiguous/strided cases
    else if (n1c && (out1c || out2c)) {
        nr_float64* n1_data = (nr_float64*)n1->data;
        nr_size_t size = Node_NItems(n1);
        
        if (out1c && out2c) {
            // Input strided, outputs contiguous
            nr_float64* mant_data = (nr_float64*)out_mantissa->data;
            nr_int32* exp_data = (nr_int32*)out_exponent->data;
            
            NIter it;
            NIter_FromNode(&it, n1, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            nr_size_t i = 0;
            while (NIter_NOTDONE(&it)) {
                int exp;
                mant_data[i] = nr_frexp(*(nr_float64*)NIter_ITEM(&it), &exp);
                exp_data[i] = exp;
                NIter_NEXT(&it);
                i++;
            }
        } else {
            // Input contiguous, some outputs strided
            NIter it1, it2;
            NIter_FromNode(&it1, out_mantissa, NITER_MODE_STRIDED);
            NIter_FromNode(&it2, out_exponent, NITER_MODE_STRIDED);
            NIter_ITER(&it1);
            NIter_ITER(&it2);
            
            for (nr_size_t i = 0; i < size; i++) {
                int exp;
                nr_float64 mant = nr_frexp(n1_data[i], &exp);
                *(nr_float64*)NIter_ITEM(&it1) = mant;
                *(nr_int32*)NIter_ITEM(&it2) = exp;
                NIter_NEXT(&it1);
                NIter_NEXT(&it2);
            }
        }
    }
    // All strided
    else {
        NIter it_in, it_mant, it_exp;
        NIter_FromNode(&it_in, n1, NITER_MODE_STRIDED);
        NIter_FromNode(&it_mant, out_mantissa, NITER_MODE_STRIDED);
        NIter_FromNode(&it_exp, out_exponent, NITER_MODE_STRIDED);
        NIter_ITER(&it_in);
        NIter_ITER(&it_mant);
        NIter_ITER(&it_exp);
        
        while (NIter_NOTDONE(&it_in)) {
            int exp;
            nr_float64 mant = nr_frexp(*(nr_float64*)NIter_ITEM(&it_in), &exp);
            *(nr_float64*)NIter_ITEM(&it_mant) = mant;
            *(nr_int32*)NIter_ITEM(&it_exp) = exp;
            NIter_NEXT(&it_in);
            NIter_NEXT(&it_mant);
            NIter_NEXT(&it_exp);
        }
    }

    args->out_nodes[0] = out_mantissa;
    args->out_nodes[1] = out_exponent;
    return 0;
}

NR_STATIC int Frexp_function(NFuncArgs* args){
    Node* input = args->in_nodes[0];
    NR_DTYPE dtype = NODE_DTYPE(input);
    
    NFuncFunc func = NULL;
    DEFINE_DTYPE_TO_FUNC(Frexp, dtype, func, 0, 0, 1);
    if (!func) {
        NError_RaiseError(NError_TypeError, "frexp unsupported dtype %d", dtype);
        return -1;
    }
    
    return func(args);
}

const NFunc frexp_nfunc = {
    .name = "frexp",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_OUT_DTYPES_NOT_SAME | NFUNC_FLAG_NO_USER_OUT_NODES,
    .nin = 1,
    .nout = 2,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Frexp_function,
    .grad_func = NULL
};

// LDEXP - Multiply by power of 2 (2 inputs, 1 output)
NR_STATIC int Ldexp_kernel_nr_float32(NFuncArgs* args){
    Node* n1 = args->in_nodes[0];  // mantissa
    Node* n2 = args->in_nodes[1];  // exponent
    Node* out = args->out_nodes[0];

    // Validate mantissa is float32
    if (NODE_DTYPE(n1) != NR_FLOAT32) {
        NError_RaiseError(NError_TypeError, "ldexp: mantissa must be float32, got dtype %d", NODE_DTYPE(n1));
        return -1;
    }
    
    // Validate exponent is integer type and convert to int32 if needed
    if (!NDtype_IsInteger(NODE_DTYPE(n2))) {
        NError_RaiseError(NError_TypeError, "ldexp: exponent must be integer type, got dtype %d", NODE_DTYPE(n2));
        return -1;
    }
    
    // Convert exponent to int32 if it's not already
    Node* exp_int32 = n2;
    if (NODE_DTYPE(n2) != NR_INT32) {
        exp_int32 = Node_ToType(NULL, n2, NR_INT32);
        if (!exp_int32) {
            return -1;
        }
    }

    // Allocate output node if NULL
    if (!out) {
        out = Node_NewEmpty(n1->ndim, n1->shape, NR_FLOAT32);
        if (!out) {
            if (exp_int32 != n2) Node_Free(exp_int32);
            return -1;
        }
    }

    int n1c = NODE_IS_CONTIGUOUS(n1);
    int n2c = NODE_IS_CONTIGUOUS(exp_int32);
    int outc = NODE_IS_CONTIGUOUS(out);
    
    int ss = Node_SameShape(n1, exp_int32);
    if (ss && n1c && n2c && outc) {
        // Fast path: all contiguous and same shape
        nr_float32* n1_data = (nr_float32*)n1->data;
        nr_int32* n2_data = (nr_int32*)exp_int32->data;
        nr_float32* out_data = (nr_float32*)out->data;
        
        nr_size_t size = Node_NItems(n1);
        for (nr_size_t i = 0; i < size; i++) {
            out_data[i] = nr_ldexpf(n1_data[i], n2_data[i]);
        }
    } else {
        // Use broadcasting multiiter
        NMultiIter multiiter;
        Node* nodes[3] = {n1, exp_int32, out};
        if (NMultiIter_FromNodes(nodes, 3, &multiiter) < 0) {
            if (exp_int32 != n2) Node_Free(exp_int32);
            return -1;
        }
        
        NMultiIter_ITER(&multiiter);
        while (NMultiIter_NOTDONE(&multiiter)) {
            nr_float32* val1 = (nr_float32*)NIter_ITEM(multiiter.iters + 0);
            nr_int32* val2 = (nr_int32*)NIter_ITEM(multiiter.iters + 1);
            nr_float32* out_val = (nr_float32*)NIter_ITEM(multiiter.iters + 2);
            *out_val = nr_ldexpf(*val1, *val2);
            NMultiIter_NEXT(&multiiter);
        }
    }
    
    // Clean up temporary node if we created one
    if (exp_int32 != n2) {
        Node_Free(exp_int32);
    }
    
    args->out_nodes[0] = out;
    return 0;
}

NR_STATIC int Ldexp_kernel_nr_float64(NFuncArgs* args){
    Node* n1 = args->in_nodes[0];  // mantissa
    Node* n2 = args->in_nodes[1];  // exponent
    Node* out = args->out_nodes[0];

    // Validate mantissa is float64
    if (NODE_DTYPE(n1) != NR_FLOAT64) {
        NError_RaiseError(NError_TypeError, "ldexp: mantissa must be float64, got dtype %d", NODE_DTYPE(n1));
        return -1;
    }
    
    // Validate exponent is integer type and convert to int32 if needed
    if (!NDtype_IsInteger(NODE_DTYPE(n2))) {
        NError_RaiseError(NError_TypeError, "ldexp: exponent must be integer type, got dtype %d", NODE_DTYPE(n2));
        return -1;
    }
    
    // Convert exponent to int32 if it's not already
    Node* exp_int32 = n2;
    if (NODE_DTYPE(n2) != NR_INT32) {
        exp_int32 = Node_ToType(NULL, n2, NR_INT32);
        if (!exp_int32) {
            return -1;
        }
    }

    // Allocate output node if NULL
    if (!out) {
        out = Node_NewEmpty(n1->ndim, n1->shape, NR_FLOAT64);
        if (!out) {
            if (exp_int32 != n2) Node_Free(exp_int32);
            return -1;
        }
    }

    int n1c = NODE_IS_CONTIGUOUS(n1);
    int n2c = NODE_IS_CONTIGUOUS(exp_int32);
    int outc = NODE_IS_CONTIGUOUS(out);
    
    int ss = Node_SameShape(n1, exp_int32);
    if (ss && n1c && n2c && outc) {
        // Fast path: all contiguous and same shape
        nr_float64* n1_data = (nr_float64*)n1->data;
        nr_int32* n2_data = (nr_int32*)exp_int32->data;
        nr_float64* out_data = (nr_float64*)out->data;
        
        nr_size_t size = Node_NItems(n1);
        for (nr_size_t i = 0; i < size; i++) {
            out_data[i] = nr_ldexp(n1_data[i], n2_data[i]);
        }
    } else {
        // Use broadcasting multiiter
        NMultiIter multiiter;
        Node* nodes[3] = {n1, exp_int32, out};
        if (NMultiIter_FromNodes(nodes, 3, &multiiter) < 0) {
            if (exp_int32 != n2) Node_Free(exp_int32);
            return -1;
        }
        
        NMultiIter_ITER(&multiiter);
        while (NMultiIter_NOTDONE(&multiiter)) {
            nr_float64* val1 = (nr_float64*)NIter_ITEM(multiiter.iters + 0);
            nr_int32* val2 = (nr_int32*)NIter_ITEM(multiiter.iters + 1);
            nr_float64* out_val = (nr_float64*)NIter_ITEM(multiiter.iters + 2);
            *out_val = nr_ldexp(*val1, *val2);
            NMultiIter_NEXT(&multiiter);
        }
    }
    
    // Clean up temporary node if we created one
    if (exp_int32 != n2) {
        Node_Free(exp_int32);
    }
    
    args->out_nodes[0] = out;
    return 0;
}

NR_STATIC int Ldexp_function(NFuncArgs* args){
    Node* mantissa = args->in_nodes[0];
    NR_DTYPE mantissa_dtype = NODE_DTYPE(mantissa);
    
    // Validate mantissa is float type
    if (!NDtype_IsFloat(mantissa_dtype)) {
        NError_RaiseError(NError_TypeError, "ldexp: mantissa must be float type, got dtype %d", mantissa_dtype);
        return -1;
    }
    
    // Dispatch based on mantissa dtype
    if (mantissa_dtype == NR_FLOAT32) {
        return Ldexp_kernel_nr_float32(args);
    } else if (mantissa_dtype == NR_FLOAT64) {
        return Ldexp_kernel_nr_float64(args);
    } else {
        NError_RaiseError(NError_TypeError, "ldexp unsupported float dtype %d", mantissa_dtype);
        return -1;
    }
}

const NFunc ldexp_nfunc = {
    .name = "ldexp",
    .flags = NFUNC_FLAG_ELEMENTWISE,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,  // First input float, second int
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Ldexp_function,
    .grad_func = NULL
};

// MODF - Extract integer and fractional parts (1 input, 2 outputs)
NR_STATIC int Modf_kernel_nr_float32(NFuncArgs* args){
    Node* n1 = args->in_nodes[0];
    Node* out_frac = args->out_nodes[0];
    Node* out_int = args->out_nodes[1];

    // Allocate output nodes (always NULL due to NFUNC_FLAG_NO_USER_OUT_NODES)
    out_frac = Node_NewEmpty(n1->ndim, n1->shape, NR_FLOAT32);
    if (!out_frac) {
        return -1;
    }

    out_int = Node_NewEmpty(n1->ndim, n1->shape, NR_FLOAT32);
    if (!out_int) {
        NODE_DECREF(out_frac);
        return -1;
    }

    // All contiguous - fast path
    if (NODE_IS_CONTIGUOUS(n1) && NODE_IS_CONTIGUOUS(out_frac) && NODE_IS_CONTIGUOUS(out_int)) {
        nr_float32* n1_data = (nr_float32*)n1->data;
        nr_float32* frac_data = (nr_float32*)out_frac->data;
        nr_float32* int_data = (nr_float32*)out_int->data;
        
        nr_size_t size = Node_NItems(n1);
        for (nr_size_t i = 0; i < size; i++) {
            frac_data[i] = nr_modff(n1_data[i], &int_data[i]);
        }
    } else {
        // Use multiiter for strided case
        NMultiIter multiiter;
        Node* nodes[3] = {n1, out_frac, out_int};
        if (NMultiIter_FromNodes(nodes, 3, &multiiter) < 0) {
            NODE_DECREF(out_frac);
            NODE_DECREF(out_int);
            return -1;
        }
        
        NMultiIter_ITER(&multiiter);
        while (NMultiIter_NOTDONE(&multiiter)) {
            nr_float32* val = (nr_float32*)NIter_ITEM(multiiter.iters + 0);
            nr_float32* frac_val = (nr_float32*)NIter_ITEM(multiiter.iters + 1);
            nr_float32* int_val = (nr_float32*)NIter_ITEM(multiiter.iters + 2);
            *frac_val = nr_modff(*val, int_val);
            NMultiIter_NEXT(&multiiter);
        }
    }
    
    args->out_nodes[0] = out_frac;
    args->out_nodes[1] = out_int;
    return 0;
}

NR_STATIC int Modf_kernel_nr_float64(NFuncArgs* args){
    Node* n1 = args->in_nodes[0];
    Node* out_frac = args->out_nodes[0];
    Node* out_int = args->out_nodes[1];

    // Allocate output nodes (always NULL due to NFUNC_FLAG_NO_USER_OUT_NODES)
    out_frac = Node_NewEmpty(n1->ndim, n1->shape, NR_FLOAT64);
    if (!out_frac) {
        return -1;
    }

    out_int = Node_NewEmpty(n1->ndim, n1->shape, NR_FLOAT64);
    if (!out_int) {
        NODE_DECREF(out_frac);
        return -1;
    }

    // All contiguous - fast path
    if (NODE_IS_CONTIGUOUS(n1) && NODE_IS_CONTIGUOUS(out_frac) && NODE_IS_CONTIGUOUS(out_int)) {
        nr_float64* n1_data = (nr_float64*)n1->data;
        nr_float64* frac_data = (nr_float64*)out_frac->data;
        nr_float64* int_data = (nr_float64*)out_int->data;
        
        nr_size_t size = Node_NItems(n1);
        for (nr_size_t i = 0; i < size; i++) {
            frac_data[i] = nr_modf(n1_data[i], &int_data[i]);
        }
    } else {
        // Use multiiter for strided case
        NMultiIter multiiter;
        Node* nodes[3] = {n1, out_frac, out_int};
        if (NMultiIter_FromNodes(nodes, 3, &multiiter) < 0) {
            NODE_DECREF(out_frac);
            NODE_DECREF(out_int);
            return -1;
        }
        
        NMultiIter_ITER(&multiiter);
        while (NMultiIter_NOTDONE(&multiiter)) {
            nr_float64* val = (nr_float64*)NIter_ITEM(multiiter.iters + 0);
            nr_float64* frac_val = (nr_float64*)NIter_ITEM(multiiter.iters + 1);
            nr_float64* int_val = (nr_float64*)NIter_ITEM(multiiter.iters + 2);
            *frac_val = nr_modf(*val, int_val);
            NMultiIter_NEXT(&multiiter);
        }
    }
    
    args->out_nodes[0] = out_frac;
    args->out_nodes[1] = out_int;
    return 0;
}

NR_STATIC int Modf_function(NFuncArgs* args){
    Node* input = args->in_nodes[0];
    NR_DTYPE dtype = NODE_DTYPE(input);
    
    NFuncFunc func = NULL;
    DEFINE_DTYPE_TO_FUNC(Modf, dtype, func, 0, 0, 1);
    if (!func) {
        NError_RaiseError(NError_TypeError, "modf unsupported dtype %d", dtype);
        return -1;
    }
    
    return func(args);
}

const NFunc modf_nfunc = {
    .name = "modf",
    .flags = NFUNC_FLAG_ELEMENTWISE | NFUNC_FLAG_NO_USER_OUT_NODES,
    .nin = 1,
    .nout = 2,
    .in_type = NDTYPE_FLOAT,
    .out_type = NDTYPE_FLOAT,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = Modf_function,
    .grad_func = NULL
};
