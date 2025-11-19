#ifndef NOUR__CORE_SRC_NMATH_MATH_BASIC_TEMPLATES_H
#define NOUR__CORE_SRC_NMATH_MATH_BASIC_TEMPLATES_H

#include "nour/nour.h"
#include "_nmath_func_core.h"
#include "../niter.h"
#include "../node_core.h"
#include "../operations.h"
#include "loops.h"

#define NMATH_2I_FUNCTION_TEMPLATE(OPC, OP, O_NT, I_NT)                             \
/**                                                                                 \
 * Performs `OPC` on two input nodes with type `I_NT` and stores the result         \
 * in the output node of type `O_NT`.                                               \
 *                                                                                  \
 * Parameters:                                                                      \
 *   - args: A pointer to NFuncArgs, containing:                                    \
 *       - nodes[0]: The first input node (`n1`) of type `I_NT`.                    \
 *       - nodes[1]: The second input node (`n2`) of type `I_NT`.                   \
 *       - out: Optional output node (`out`) of type `O_NT`. If NULL, a new node    \
 *              is allocated.                                                       \ 
 * Behavior:                                                                        \
 *   - Supports same-shape operation, broadcasting with scalar, and general         \
 *     broadcasting via NMultiIter.                                                 \
 *   - Handles contiguous and strided memory layouts.                               \
 * Returns:                                                                         \
 *   - 0 on success.                                                                \
 *   - -1 if memory allocation or iteration fails.                                  \
 */                                                                                 \
NR_PUBLIC int NMath_##OPC##_##I_NT(NFuncArgs* args) {                               \
    Node* n1 = args->nodes[0];                                                      \
    Node* n2 = args->nodes[1];                                                      \
    Node* out = args->out;                                                          \
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
                NMATH_LOOP_CCC(OP, O_NT, I_NT);                                   \
            } else if (n1c | n2c) {                                                 \
                NMATH_LOOP_CSC(OP, O_NT, I_NT);                                   \
            } else {                                                                \
                NMATH_LOOP_CSS(OP, O_NT, I_NT);                                   \
            }                                                                       \
        } else {                                                                    \
            if (n1c & n2c) {                                                        \
                NMATH_LOOP_SCC(OP, O_NT, I_NT);                                   \
            } else if (n1c | n2c) {                                                 \
                NMATH_LOOP_SSC(OP, O_NT, I_NT);                                   \
            } else {                                                                \
                NMATH_LOOP_SSS(OP, O_NT, I_NT);                                   \
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
                    NMATH_LOOP_CC_S(OP, O_NT, I_NT);                             \
                } else {                                                            \
                    NMATH_LOOP_CS_S(OP, O_NT, I_NT);                             \
                }                                                                   \
            } else {                                                                \
                if (nc) {                                                           \
                    NMATH_LOOP_SC_S(OP, O_NT, I_NT);                             \
                } else {                                                            \
                    NMATH_LOOP_SS_S(OP, O_NT, I_NT);                             \
                }                                                                   \
            }                                                                       \
        } else {                                                                    \
            NMultiIter mit;                                                         \
            if (NMultiIter_FromNodes(args->nodes, 2, &mit) != 0) {                        \
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
                *((O_NT*)out->data + i) = OP(                                     \
                    *((I_NT*)NMultiIter_ITEM(&mit, 0)),                             \
                    *((I_NT*)NMultiIter_ITEM(&mit, 1))                              \
                );                                                                  \
                i++;                                                                \
                NMultiIter_NEXT(&mit);                                              \
            }                                                                       \
        }                                                                           \
    }                                                                               \
    args->out = out;                                                                \
    return 0;                                                                       \
}


// Integer types (bool included)
#define NMATH_2I_FUNCTION_INT_TYPES(OP, OP_NAME) \
    NMATH_2I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_bool,    nr_bool)       \
    NMATH_2I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_int8,    nr_int8)       \
    NMATH_2I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_uint8,   nr_uint8)      \
    NMATH_2I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_int16,   nr_int16)      \
    NMATH_2I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_uint16,  nr_uint16)     \
    NMATH_2I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_int32,   nr_int32)      \
    NMATH_2I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_uint32,  nr_uint32)     \
    NMATH_2I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_int64,   nr_int64)      \
    NMATH_2I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_uint64,  nr_uint64)

// Floating-point types
#define NMATH_2I_FUNCTION_FLOAT_TYPES(OP, OP_NAME)                                 \
    NMATH_2I_FUNCTION_TEMPLATE(OP_NAME, OP, nr_float32, nr_float32)    \
    NMATH_2I_FUNCTION_TEMPLATE(OP_NAME, OP, nr_float64, nr_float64)

// All numeric types (int + float)
#define NMATH_2I_FUNCTION_ALL_TYPES(OP, OP_NAME) \
    NMATH_2I_FUNCTION_INT_TYPES(OP, OP_NAME)     \
    NMATH_2I_FUNCTION_FLOAT_TYPES(OP, OP_NAME)


// Integer types (including bool)
#define NMATH_METHODS_INT(OP_NAME) \
    NMath_##OP_NAME##_nr_bool,     \
    NMath_##OP_NAME##_nr_int8,     \
    NMath_##OP_NAME##_nr_uint8,    \
    NMath_##OP_NAME##_nr_int16,    \
    NMath_##OP_NAME##_nr_uint16,   \
    NMath_##OP_NAME##_nr_int32,    \
    NMath_##OP_NAME##_nr_uint32,   \
    NMath_##OP_NAME##_nr_int64,    \
    NMath_##OP_NAME##_nr_uint64

// Floating-point types
#define NMATH_METHODS_FLOAT(OP_NAME) \
    NMath_##OP_NAME##_nr_float32, \
    NMath_##OP_NAME##_nr_float64

// All numeric types (int + float)
#define NMATH_METHODS_ALL(OP_NAME) \
    NMATH_METHODS_INT(OP_NAME), \
    NMATH_METHODS_FLOAT(OP_NAME)

// Full template for method array declaration
#define NMATH_METHOD_ARRAY(OP_NAME, METHODS) \
    NR_STATIC NFunc __NMATH_##OP_NAME##_METHODS__[] = { \
        METHODS \
    };

#define NMATH_2I_FUNCTION_AND_METHODS_ALL_TYPES(OP, OP_NAME) \
    NMATH_2I_FUNCTION_ALL_TYPES(OP, OP_NAME) \
    NMATH_METHOD_ARRAY(OP_NAME, NMATH_METHODS_ALL(OP_NAME))

#define NMATH_2I_FUNCTION_AND_METHODS_INT_TYPES(OP, OP_NAME) \
    NMATH_2I_FUNCTION_INT_TYPES(OP, OP_NAME) \
    NMATH_METHOD_ARRAY(OP_NAME, NMATH_METHODS_INT(OP_NAME))

#define NMATH_2I_FUNCTION_AND_METHODS_FLOAT_TYPES(OP, OP_NAME) \
    NMATH_2I_FUNCTION_FLOAT_TYPES(OP, OP_NAME) \
    NMATH_METHOD_ARRAY(OP_NAME, NMATH_METHODS_FLOAT(OP_NAME))

#define NMATH_2I_MAIN_FUNCTION(OP_NAME, OP_ADJ_NAME, IN_TYPE, OUT_TYPE, CASTABLE) \
NR_PUBLIC Node* NMath_##OP_NAME(Node* c, Node* b, Node* a) {  \
    return _NMath_Func_2in1out(                             \
        c, b, a,                                            \
        __NMATH_##OP_NAME##_METHODS__,                      \
        NDTYPE_##IN_TYPE,                                   \
        NDTYPE_##OUT_TYPE,                                  \
        CASTABLE,                                           \
        #OP_ADJ_NAME                                        \
    );                                                      \
}                                                           


#define NMATH_1I_FUNCTION_TEMPLATE(OPC, OP, O_NT, I_NT)                             \
/**                                                                                 \
 * Applies unary operation `OPC` on input node of type `I_NT` and stores            \
 * the result in output node of type `O_NT`.                                        \
 *                                                                                  \
 * Parameters:                                                                      \
 *   - args: A pointer to NFuncArgs, containing:                                    \
 *       - nodes[0]: The input node (`n1`) of type `I_NT`.                          \
 *       - out: Optional output node (`out`) of type `O_NT`. If NULL, a new node    \
 *              is allocated.                                                       \
 * Behavior:                                                                        \
 *   - Matches shapes or broadcasts scalar input.                                   \
 *   - Supports contiguous and strided iteration.                                   \
 * Returns:                                                                         \
 *   - 0 on success.                                                                \
 *   - -1 if memory allocation or iteration fails.                                  \
 */                                                                                 \
NR_PUBLIC int NMath_##OPC##_##I_NT(NFuncArgs* args) {                               \
    Node* n1 = args->nodes[0];                                                      \
    Node* out = args->out;                                                          \
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
    if (Node_SameShape(n1, out)) {                                                  \
        if (outc) {                                                                 \
            if (n1c) {                                                              \
                NMATH_LOOP_CC_1I(OP, O_NT, I_NT);                                   \
            } else {                                                                \
                NMATH_LOOP_CS_1I(OP, O_NT, I_NT);                                   \
            }                                                                       \
        } else {                                                                    \
            if (n1c) {                                                              \
                NMATH_LOOP_SC_1I(OP, O_NT, I_NT);                                   \
            } else {                                                                \
                NMATH_LOOP_SS_1I(OP, O_NT, I_NT);                                   \
            }                                                                       \
        }                                                                           \
    } else {                                                                        \
        int issclr = NODE_IS_SCALAR(n1);                                            \
        if (issclr) {                                                               \
            I_NT sclr = *(I_NT*)(n1->data);                                         \
            *((O_NT*)out->data) = OP(sclr);                                         \
        } else {                                                                    \
            NMultiIter mit;                                                         \
            if (NMultiIter_FromNodes(args->nodes, 1, &mit) != 0) {                        \
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
                *((O_NT*)out->data + i) = OP(                                       \
                    *((I_NT*)NMultiIter_ITEM(&mit, 0))                              \
                );                                                                  \
                i++;                                                                \
                NMultiIter_NEXT(&mit);                                              \
            }                                                                       \
        }                                                                           \
    }                                                                               \
                                                                                    \
    args->out = out;                                                                \
    return 0;                                                                       \
}


// Integer types (bool included)
#define NMATH_1I_FUNCTION_INT_TYPES(OP, OP_NAME) \
    NMATH_1I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_bool,    nr_bool)       \
    NMATH_1I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_int8,    nr_int8)       \
    NMATH_1I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_uint8,   nr_uint8)      \
    NMATH_1I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_int16,   nr_int16)      \
    NMATH_1I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_uint16,  nr_uint16)     \
    NMATH_1I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_int32,   nr_int32)      \
    NMATH_1I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_uint32,  nr_uint32)     \
    NMATH_1I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_int64,   nr_int64)      \
    NMATH_1I_FUNCTION_TEMPLATE(OP_NAME, OP,  nr_uint64,  nr_uint64)

// Floating-point types
#define NMATH_1I_FUNCTION_FLOAT_TYPES(OP, OP_NAME)                                 \
    NMATH_1I_FUNCTION_TEMPLATE(OP_NAME, OP, nr_float32, nr_float32)    \
    NMATH_1I_FUNCTION_TEMPLATE(OP_NAME, OP, nr_float64, nr_float64)

// All numeric types (int + float)
#define NMATH_1I_FUNCTION_ALL_TYPES(OP, OP_NAME) \
    NMATH_1I_FUNCTION_INT_TYPES(OP, OP_NAME)     \
    NMATH_1I_FUNCTION_FLOAT_TYPES(OP, OP_NAME)

// Full template of 1 input function and methods
#define NMATH_1I_FUNCTION_AND_METHODS_ALL_TYPES(OP, OP_NAME) \
    NMATH_1I_FUNCTION_ALL_TYPES(OP, OP_NAME) \
    NMATH_METHOD_ARRAY(OP_NAME, NMATH_METHODS_ALL(OP_NAME))

#define NMATH_1I_FUNCTION_AND_METHODS_INT_TYPES(OP, OP_NAME) \
    NMATH_1I_FUNCTION_INT_TYPES(OP, OP_NAME) \
    NMATH_METHOD_ARRAY(OP_NAME, NMATH_METHODS_INT(OP_NAME))

#define NMATH_1I_FUNCTION_AND_METHODS_FLOAT_TYPES(OP, OP_NAME) \
    NMATH_1I_FUNCTION_FLOAT_TYPES(OP, OP_NAME) \
    NMATH_METHOD_ARRAY(OP_NAME, NMATH_METHODS_FLOAT(OP_NAME))

#define NMATH_1I_MAIN_FUNCTION(OP_NAME, OP_ADJ_NAME, IN_TYPE, OUT_TYPE, CASTABLE) \
NR_PUBLIC Node* NMath_##OP_NAME(Node* c, Node* a) {  \
    return _NMath_Func_1in1out(                             \
        c, a,                                               \
        __NMATH_##OP_NAME##_METHODS__,                      \
        NDTYPE_##IN_TYPE,                                   \
        NDTYPE_##OUT_TYPE,                                  \
        CASTABLE,                                           \
        #OP_ADJ_NAME                                        \
    );                                                      \
}

#endif