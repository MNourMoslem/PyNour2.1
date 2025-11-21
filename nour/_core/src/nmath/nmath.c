#include "nmath.h"
#include "../nfunc.h"
#include "../node2str.h"
#include "nfunc_math.h"


#define TWO_IN_OPERATIONS(name, nfunc_name) \
NR_PUBLIC Node* NMath_##name(Node* c, Node* a, Node* b){ \
    NFuncArgs* args = NFuncArgs_New(2, 1);               \
    args->in_nodes[0] = a;                               \
    args->in_nodes[1] = b;                               \
    args->out_nodes[0] = c;                              \
    int result = NFunc_Call(&nfunc_name, args);          \
    Node* out_node = args->out_nodes[0];                 \
    NFuncArgs_DECREF(args);                              \
    return result != 0 ? NULL : out_node;                \
}

#define ONE_IN_OPERATIONS(name, nfunc_name)              \
NR_PUBLIC Node* NMath_##name(Node* c, Node* a){          \
    NFuncArgs* args = NFuncArgs_New(1, 1);               \
    args->in_nodes[0] = a;                               \
    args->out_nodes[0] = c;                              \
    int result = NFunc_Call(&nfunc_name, args);          \
    Node* out_node = args->out_nodes[0];                 \
    NFuncArgs_DECREF(args);                              \
    return result != 0 ? NULL : out_node;                \
}


TWO_IN_OPERATIONS(Add, add_nfunc) // Addition
TWO_IN_OPERATIONS(Sub, sub_nfunc) // Subtraction
TWO_IN_OPERATIONS(Mul, mul_nfunc) // Multiplication
TWO_IN_OPERATIONS(Div, div_nfunc) // Division
TWO_IN_OPERATIONS(Truediv, truediv_nfunc) // True Division
TWO_IN_OPERATIONS(Mod, mod_nfunc) // Modulus
TWO_IN_OPERATIONS(Pow, pow_nfunc) // Power
TWO_IN_OPERATIONS(Bg, bg_nfunc) // Bigger Than
TWO_IN_OPERATIONS(Bge, bge_nfunc) // Bigger Equal Than
TWO_IN_OPERATIONS(Ls, ls_nfunc) // Less Than
TWO_IN_OPERATIONS(Lse, lse_nfunc) // Less Equal Than
TWO_IN_OPERATIONS(Eq, eq_nfunc) // Equal
TWO_IN_OPERATIONS(Neq, neq_nfunc) // Not Equal
TWO_IN_OPERATIONS(BitAnd, bit_and_nfunc) // Bitwise AND
TWO_IN_OPERATIONS(BitOr, bit_or_nfunc) // Bitwise OR
TWO_IN_OPERATIONS(BitXor, bit_xor_nfunc) // Bitwise XOR
TWO_IN_OPERATIONS(BitLsh, bit_lsh_nfunc) // Bitwise Left Shift
TWO_IN_OPERATIONS(BitRsh, bit_rsh_nfunc) // Bitwise Right Shift


ONE_IN_OPERATIONS(Neg, neg_nfunc) // Negation
ONE_IN_OPERATIONS(BitNot, bit_not_nfunc) // Bitwise NOT
ONE_IN_OPERATIONS(Sin, sin_nfunc) // Sine
ONE_IN_OPERATIONS(Cos, cos_nfunc) // Cosine
ONE_IN_OPERATIONS(Tan, tan_nfunc) // Tangent
ONE_IN_OPERATIONS(Cot, cot_nfunc) // Cotangent
ONE_IN_OPERATIONS(Exp, exp_nfunc) // Exponential
ONE_IN_OPERATIONS(Log, log_nfunc) // Natural Logarithm
ONE_IN_OPERATIONS(Log10, log10_nfunc) // Base-10 Logarithm
ONE_IN_OPERATIONS(Sinh, sinh_nfunc) // Hyperbolic Sine
ONE_IN_OPERATIONS(Cosh, cosh_nfunc) // Hyperbolic Cosine
ONE_IN_OPERATIONS(Tanh, tanh_nfunc) // Hyperbolic Tangent
ONE_IN_OPERATIONS(Coth, coth_nfunc) // Hyperbolic Cotangent
ONE_IN_OPERATIONS(Asin, asin_nfunc) // Arc Sine
ONE_IN_OPERATIONS(Acos, acos_nfunc) // Arc Cosine
ONE_IN_OPERATIONS(Atan, atan_nfunc) // Arc Tangent
ONE_IN_OPERATIONS(Asinh, asinh_nfunc) // Inverse Hyperbolic Sine
ONE_IN_OPERATIONS(Acosh, acosh_nfunc) // Inverse Hyperbolic Cosine
ONE_IN_OPERATIONS(Atanh, atanh_nfunc) // Inverse Hyperbolic Tangent
ONE_IN_OPERATIONS(Exp2, exp2_nfunc) // Base-2 Exponential
ONE_IN_OPERATIONS(Expm1, expm1_nfunc) // Exponential minus 1
ONE_IN_OPERATIONS(Log1p, log1p_nfunc) // Logarithm of (1 + x)
ONE_IN_OPERATIONS(Sqrt, sqrt_nfunc) // Square Root
ONE_IN_OPERATIONS(Cbrt, cbrt_nfunc) // Cube Root
ONE_IN_OPERATIONS(Abs, abs_nfunc) // Absolute Value
ONE_IN_OPERATIONS(Ceil, ceil_nfunc) // Ceiling
ONE_IN_OPERATIONS(Floor, floor_nfunc) // Floor
ONE_IN_OPERATIONS(Trunc, trunc_nfunc) // Truncate
ONE_IN_OPERATIONS(Rint, rint_nfunc) // Round to Nearest Integer

NR_PUBLIC int NMath_Frexp(Node** mantissa, Node** exponent, Node* a) {
    NFuncArgs* args = NFuncArgs_New(1, 2);
    args->in_nodes[0] = a;
    int result = NFunc_Call(&frexp_nfunc, args);
    *mantissa = args->out_nodes[0];
    *exponent = args->out_nodes[1];
    NFuncArgs_DECREF(args);
    return result;
}

NR_PUBLIC Node* NMath_Ldexp(Node* c, Node* mantissa, Node* exponent) {
    NFuncArgs* args = NFuncArgs_New(2, 1);
    args->in_nodes[0] = mantissa;
    args->in_nodes[1] = exponent;
    args->out_nodes[0] = c;
    int result = NFunc_Call(&ldexp_nfunc, args);
    Node* out_node = args->out_nodes[0];
    NFuncArgs_DECREF(args);
    return result != 0 ? NULL : out_node;
}

NR_PUBLIC int NMath_Modf(Node** fractional, Node** integer, Node* a) {
    NFuncArgs* args = NFuncArgs_New(1, 2);
    args->in_nodes[0] = a;
    int result = NFunc_Call(&modf_nfunc, args);
    *fractional = args->out_nodes[0];
    *integer = args->out_nodes[1];
    NFuncArgs_DECREF(args);
    return result;
}