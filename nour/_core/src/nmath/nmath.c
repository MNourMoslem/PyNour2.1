#include "nmath.h"


// Addition
NMATH_2I_FUNCTION_AND_METHODS_ALL_TYPES(NMATH_ADD, Add)
NMATH_2I_MAIN_FUNCTION(Add, "addition", NONE, NONE, 1)


// Subtraction
NMATH_2I_FUNCTION_AND_METHODS_ALL_TYPES(NMATH_SUB, Sub)
NMATH_2I_MAIN_FUNCTION(Sub, "subtraction", NONE, NONE, 1)


// Multiplication
NMATH_2I_FUNCTION_AND_METHODS_ALL_TYPES(NMATH_MUL, Mul)
NMATH_2I_MAIN_FUNCTION(Mul, "multiplication", NONE, NONE, 1)


// Division (float only)
NMATH_2I_FUNCTION_AND_METHODS_ALL_TYPES(NMATH_DIV, Div)
NMATH_2I_MAIN_FUNCTION(Div, "division", FLOAT, FLOAT, 1)


// Modulus (int only)
NMATH_2I_FUNCTION_AND_METHODS_INT_TYPES(NMATH_MOD, Mod)
NMATH_2I_MAIN_FUNCTION(Mod, "modulus", INT, INT, 1)


// True division (int only)
NMATH_2I_FUNCTION_AND_METHODS_ALL_TYPES(NMATH_TRUEDIV, TrueDiv)
NMATH_2I_MAIN_FUNCTION(TrueDiv, "true division", INT, INT, 1)


// Power
NMATH_2I_FUNCTION_AND_METHODS_ALL_TYPES(NMATH_POW, Pow)
NMATH_2I_MAIN_FUNCTION(Pow, "power", NONE, NONE, 1)

// Bigger Than
NMATH_2I_FUNCTION_TEMPLATE(Bg, NMATH_BG, nr_bool, nr_bool)
NMATH_2I_FUNCTION_TEMPLATE(Bg, NMATH_BG, nr_bool, nr_int8)
NMATH_2I_FUNCTION_TEMPLATE(Bg, NMATH_BG, nr_bool, nr_uint8)
NMATH_2I_FUNCTION_TEMPLATE(Bg, NMATH_BG, nr_bool, nr_int16)
NMATH_2I_FUNCTION_TEMPLATE(Bg, NMATH_BG, nr_bool, nr_uint16)
NMATH_2I_FUNCTION_TEMPLATE(Bg, NMATH_BG, nr_bool, nr_int32)
NMATH_2I_FUNCTION_TEMPLATE(Bg, NMATH_BG, nr_bool, nr_uint32)
NMATH_2I_FUNCTION_TEMPLATE(Bg, NMATH_BG, nr_bool, nr_int64)
NMATH_2I_FUNCTION_TEMPLATE(Bg, NMATH_BG, nr_bool, nr_uint64)
NMATH_2I_FUNCTION_TEMPLATE(Bg, NMATH_BG, nr_bool, nr_float32)
NMATH_2I_FUNCTION_TEMPLATE(Bg, NMATH_BG, nr_bool, nr_float64)

NMATH_METHOD_ARRAY(Bg, NMATH_METHODS_ALL(Bg))

NMATH_2I_MAIN_FUNCTION(Bg, "greater than operation", NONE, BOOL, 1)


// Bigger Equal Than
NMATH_2I_FUNCTION_TEMPLATE(Be, NMATH_BGE, nr_bool, nr_bool)
NMATH_2I_FUNCTION_TEMPLATE(Be, NMATH_BGE, nr_bool, nr_int8)
NMATH_2I_FUNCTION_TEMPLATE(Be, NMATH_BGE, nr_bool, nr_uint8)
NMATH_2I_FUNCTION_TEMPLATE(Be, NMATH_BGE, nr_bool, nr_int16)
NMATH_2I_FUNCTION_TEMPLATE(Be, NMATH_BGE, nr_bool, nr_uint16)
NMATH_2I_FUNCTION_TEMPLATE(Be, NMATH_BGE, nr_bool, nr_int32)
NMATH_2I_FUNCTION_TEMPLATE(Be, NMATH_BGE, nr_bool, nr_uint32)
NMATH_2I_FUNCTION_TEMPLATE(Be, NMATH_BGE, nr_bool, nr_int64)
NMATH_2I_FUNCTION_TEMPLATE(Be, NMATH_BGE, nr_bool, nr_uint64)
NMATH_2I_FUNCTION_TEMPLATE(Be, NMATH_BGE, nr_bool, nr_float32)
NMATH_2I_FUNCTION_TEMPLATE(Be, NMATH_BGE, nr_bool, nr_float64)

NMATH_METHOD_ARRAY(Be, NMATH_METHODS_ALL(Be))

NMATH_2I_MAIN_FUNCTION(Be, "greater than or equal operation", NONE, BOOL, 1)


// Less Than
NMATH_2I_FUNCTION_TEMPLATE(Ls, NMATH_LS, nr_bool, nr_bool)
NMATH_2I_FUNCTION_TEMPLATE(Ls, NMATH_LS, nr_bool, nr_int8)
NMATH_2I_FUNCTION_TEMPLATE(Ls, NMATH_LS, nr_bool, nr_uint8)
NMATH_2I_FUNCTION_TEMPLATE(Ls, NMATH_LS, nr_bool, nr_int16)
NMATH_2I_FUNCTION_TEMPLATE(Ls, NMATH_LS, nr_bool, nr_uint16)
NMATH_2I_FUNCTION_TEMPLATE(Ls, NMATH_LS, nr_bool, nr_int32)
NMATH_2I_FUNCTION_TEMPLATE(Ls, NMATH_LS, nr_bool, nr_uint32)
NMATH_2I_FUNCTION_TEMPLATE(Ls, NMATH_LS, nr_bool, nr_int64)
NMATH_2I_FUNCTION_TEMPLATE(Ls, NMATH_LS, nr_bool, nr_uint64)
NMATH_2I_FUNCTION_TEMPLATE(Ls, NMATH_LS, nr_bool, nr_float32)
NMATH_2I_FUNCTION_TEMPLATE(Ls, NMATH_LS, nr_bool, nr_float64)

NMATH_METHOD_ARRAY(Ls, NMATH_METHODS_ALL(Ls))
NMATH_2I_MAIN_FUNCTION(Ls, "less than operation", NONE, BOOL, 1)


// Less Equal Than
NMATH_2I_FUNCTION_TEMPLATE(Lse, NMATH_LSE, nr_bool, nr_bool)
NMATH_2I_FUNCTION_TEMPLATE(Lse, NMATH_LSE, nr_bool, nr_int8)
NMATH_2I_FUNCTION_TEMPLATE(Lse, NMATH_LSE, nr_bool, nr_uint8)
NMATH_2I_FUNCTION_TEMPLATE(Lse, NMATH_LSE, nr_bool, nr_int16)
NMATH_2I_FUNCTION_TEMPLATE(Lse, NMATH_LSE, nr_bool, nr_uint16)
NMATH_2I_FUNCTION_TEMPLATE(Lse, NMATH_LSE, nr_bool, nr_int32)
NMATH_2I_FUNCTION_TEMPLATE(Lse, NMATH_LSE, nr_bool, nr_uint32)
NMATH_2I_FUNCTION_TEMPLATE(Lse, NMATH_LSE, nr_bool, nr_int64)
NMATH_2I_FUNCTION_TEMPLATE(Lse, NMATH_LSE, nr_bool, nr_uint64)
NMATH_2I_FUNCTION_TEMPLATE(Lse, NMATH_LSE, nr_bool, nr_float32)
NMATH_2I_FUNCTION_TEMPLATE(Lse, NMATH_LSE, nr_bool, nr_float64)

NMATH_METHOD_ARRAY(Lse, NMATH_METHODS_ALL(Lse))
NMATH_2I_MAIN_FUNCTION(Lse, "less than or equal operation", NONE, BOOL, 1)


// Equal
NMATH_2I_FUNCTION_TEMPLATE(Eq, NMATH_EQ, nr_bool, nr_bool)
NMATH_2I_FUNCTION_TEMPLATE(Eq, NMATH_EQ, nr_bool, nr_int8)
NMATH_2I_FUNCTION_TEMPLATE(Eq, NMATH_EQ, nr_bool, nr_uint8)
NMATH_2I_FUNCTION_TEMPLATE(Eq, NMATH_EQ, nr_bool, nr_int16)
NMATH_2I_FUNCTION_TEMPLATE(Eq, NMATH_EQ, nr_bool, nr_uint16)
NMATH_2I_FUNCTION_TEMPLATE(Eq, NMATH_EQ, nr_bool, nr_int32)
NMATH_2I_FUNCTION_TEMPLATE(Eq, NMATH_EQ, nr_bool, nr_uint32)
NMATH_2I_FUNCTION_TEMPLATE(Eq, NMATH_EQ, nr_bool, nr_int64)
NMATH_2I_FUNCTION_TEMPLATE(Eq, NMATH_EQ, nr_bool, nr_uint64)
NMATH_2I_FUNCTION_TEMPLATE(Eq, NMATH_EQ, nr_bool, nr_float32)
NMATH_2I_FUNCTION_TEMPLATE(Eq, NMATH_EQ, nr_bool, nr_float64)

NMATH_METHOD_ARRAY(Eq, NMATH_METHODS_ALL(Eq))
NMATH_2I_MAIN_FUNCTION(Eq, "equality operation", NONE, BOOL, 1)


// Not Equal
NMATH_2I_FUNCTION_TEMPLATE(Neq, NMATH_NEQ, nr_bool, nr_bool)
NMATH_2I_FUNCTION_TEMPLATE(Neq, NMATH_NEQ, nr_bool, nr_int8)
NMATH_2I_FUNCTION_TEMPLATE(Neq, NMATH_NEQ, nr_bool, nr_uint8)
NMATH_2I_FUNCTION_TEMPLATE(Neq, NMATH_NEQ, nr_bool, nr_int16)
NMATH_2I_FUNCTION_TEMPLATE(Neq, NMATH_NEQ, nr_bool, nr_uint16)
NMATH_2I_FUNCTION_TEMPLATE(Neq, NMATH_NEQ, nr_bool, nr_int32)
NMATH_2I_FUNCTION_TEMPLATE(Neq, NMATH_NEQ, nr_bool, nr_uint32)
NMATH_2I_FUNCTION_TEMPLATE(Neq, NMATH_NEQ, nr_bool, nr_int64)
NMATH_2I_FUNCTION_TEMPLATE(Neq, NMATH_NEQ, nr_bool, nr_uint64)
NMATH_2I_FUNCTION_TEMPLATE(Neq, NMATH_NEQ, nr_bool, nr_float32)
NMATH_2I_FUNCTION_TEMPLATE(Neq, NMATH_NEQ, nr_bool, nr_float64)

NMATH_METHOD_ARRAY(Neq, NMATH_METHODS_ALL(Neq))
NMATH_2I_MAIN_FUNCTION(Neq, "not equal operation", NONE, BOOL, 1)


/*
    Bitwise Operations (int only and not castable from float)
*/
// Bitwise AND
NMATH_2I_FUNCTION_AND_METHODS_INT_TYPES(NMATH_BIT_AND, BitAnd)
NMATH_2I_MAIN_FUNCTION(BitAnd, "bitwise and", INT, INT, 0)

// Bitwise OR
NMATH_2I_FUNCTION_AND_METHODS_INT_TYPES(NMATH_BIT_OR, BitOr)
NMATH_2I_MAIN_FUNCTION(BitOr, "bitwise or", INT, INT, 0)

// Bitwise XOR
NMATH_2I_FUNCTION_AND_METHODS_INT_TYPES(NMATH_BIT_XOR, BitXor)
NMATH_2I_MAIN_FUNCTION(BitXor, "bitwise xor", INT, INT, 0)

// Bitwise Left Shift
NMATH_2I_FUNCTION_AND_METHODS_INT_TYPES(NMATH_BIT_LSH, BitLsh)
NMATH_2I_MAIN_FUNCTION(BitLsh, "bitwise left shift", INT, INT, 0)

// Bitwise Right Shift
NMATH_2I_FUNCTION_AND_METHODS_INT_TYPES(NMATH_BIT_RSH, BitRsh)
NMATH_2I_MAIN_FUNCTION(BitRsh, "bitwise right shift", INT, INT, 0)

// Bitwise NOT (1 input)
NMATH_1I_FUNCTION_AND_METHODS_INT_TYPES(NMATH_BIT_NOT, BitNot)
NMATH_1I_MAIN_FUNCTION(BitNot, "bitwise not", INT, INT, 0)


// Trigonometric and other functions can be added similarly

// Sin
NMATH_1I_FUNCTION_TEMPLATE(Sin, nr_sinf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Sin, nr_sin, nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Sin, NMATH_METHODS_FLOAT(Sin))
NMATH_1I_MAIN_FUNCTION(Sin, "sine", FLOAT, FLOAT, 1)

// Cos
NMATH_1I_FUNCTION_TEMPLATE(Cos, nr_cosf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Cos, nr_cos,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Cos, NMATH_METHODS_FLOAT(Cos))
NMATH_1I_MAIN_FUNCTION(Cos, "cosine", FLOAT, FLOAT, 1)

// Tan
NMATH_1I_FUNCTION_TEMPLATE(Tan, nr_tanf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Tan, nr_tan,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Tan, NMATH_METHODS_FLOAT(Tan))
NMATH_1I_MAIN_FUNCTION(Tan, "tangent", FLOAT, FLOAT, 1)

// Cot
NMATH_1I_FUNCTION_TEMPLATE(Cot, nr_cotf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Cot, nr_cot,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Cot, NMATH_METHODS_FLOAT(Cot))
NMATH_1I_MAIN_FUNCTION(Cot, "cotangent", FLOAT, FLOAT, 1)


// Logarithm
NMATH_1I_FUNCTION_TEMPLATE(Log, nr_logf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Log, nr_log,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Log, NMATH_METHODS_FLOAT(Log))
NMATH_1I_MAIN_FUNCTION(Log, "natural logarithm", FLOAT, FLOAT, 1)

// Base-10 Logarithm
NMATH_1I_FUNCTION_TEMPLATE(Log10, nr_log10f, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Log10, nr_log10,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Log10, NMATH_METHODS_FLOAT(Log10))
NMATH_1I_MAIN_FUNCTION(Log10, "base-10 logarithm", FLOAT, FLOAT, 1)

// Hyperbolic Sine
NMATH_1I_FUNCTION_TEMPLATE(Sinh, nr_sinhf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Sinh, nr_sinh,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Sinh, NMATH_METHODS_FLOAT(Sinh))
NMATH_1I_MAIN_FUNCTION(Sinh, "hyperbolic sine", FLOAT, FLOAT, 1)

// Hyperbolic Cosine
NMATH_1I_FUNCTION_TEMPLATE(Cosh, nr_coshf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Cosh, nr_cosh,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Cosh, NMATH_METHODS_FLOAT(Cosh))
NMATH_1I_MAIN_FUNCTION(Cosh, "hyperbolic cosine", FLOAT, FLOAT, 1)

// Hyperbolic Tangent
NMATH_1I_FUNCTION_TEMPLATE(Tanh, nr_tanhf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Tanh, nr_tanh,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Tanh, NMATH_METHODS_FLOAT(Tanh))
NMATH_1I_MAIN_FUNCTION(Tanh, "hyperbolic tangent", FLOAT, FLOAT, 1)

// Arc Sine
NMATH_1I_FUNCTION_TEMPLATE(Asin, nr_asinf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Asin, nr_asin,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Asin, NMATH_METHODS_FLOAT(Asin))
NMATH_1I_MAIN_FUNCTION(Asin, "arc sine", FLOAT, FLOAT, 1)

// Arc Cosine
NMATH_1I_FUNCTION_TEMPLATE(Acos, nr_acosf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Acos, nr_acos,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Acos, NMATH_METHODS_FLOAT(Acos))
NMATH_1I_MAIN_FUNCTION(Acos, "arc cosine", FLOAT, FLOAT, 1)

// Arc Tangent
NMATH_1I_FUNCTION_TEMPLATE(Atan, nr_atanf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Atan, nr_atan,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Atan, NMATH_METHODS_FLOAT(Atan))
NMATH_1I_MAIN_FUNCTION(Atan, "arc tangent", FLOAT, FLOAT, 1)

// Cube Root
NMATH_1I_FUNCTION_TEMPLATE(Cbrt, nr_cbrtf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Cbrt, nr_cbrt,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Cbrt, NMATH_METHODS_FLOAT(Cbrt))
NMATH_1I_MAIN_FUNCTION(Cbrt, "cube root", FLOAT, FLOAT, 1)

// Absolute Value
NMATH_1I_FUNCTION_TEMPLATE(Fabs, nr_fabsf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Fabs, nr_fabs,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Fabs, NMATH_METHODS_FLOAT(Fabs))
NMATH_1I_MAIN_FUNCTION(Fabs, "absolute value", FLOAT, FLOAT, 1)

// Ceiling
NMATH_1I_FUNCTION_TEMPLATE(Ceil, nr_ceilf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Ceil, nr_ceil,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Ceil, NMATH_METHODS_FLOAT(Ceil))
NMATH_1I_MAIN_FUNCTION(Ceil, "ceiling", FLOAT, FLOAT, 1)

// Floor
NMATH_1I_FUNCTION_TEMPLATE(Floor, nr_floorf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Floor, nr_floor,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Floor, NMATH_METHODS_FLOAT(Floor))
NMATH_1I_MAIN_FUNCTION(Floor, "floor", FLOAT, FLOAT, 1)

// exp(x) - 1
NMATH_1I_FUNCTION_TEMPLATE(Expm1, nr_expm1f, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Expm1, nr_expm1,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Expm1, NMATH_METHODS_FLOAT(Expm1))
NMATH_1I_MAIN_FUNCTION(Expm1, "exp(x) - 1", FLOAT, FLOAT, 1)

// log(1 + x)
NMATH_1I_FUNCTION_TEMPLATE(Log1p, nr_log1pf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Log1p, nr_log1p,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Log1p, NMATH_METHODS_FLOAT(Log1p))
NMATH_1I_MAIN_FUNCTION(Log1p, "log(1 + x)", FLOAT, FLOAT, 1)

// Inverse Hyperbolic Cosine
NMATH_1I_FUNCTION_TEMPLATE(Acosh, nr_acoshf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Acosh, nr_acosh,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Acosh, NMATH_METHODS_FLOAT(Acosh))
NMATH_1I_MAIN_FUNCTION(Acosh, "inverse hyperbolic cosine", FLOAT, FLOAT, 1)

// Inverse Hyperbolic Sine
NMATH_1I_FUNCTION_TEMPLATE(Asinh, nr_asinhf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Asinh, nr_asinh,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Asinh, NMATH_METHODS_FLOAT(Asinh))
NMATH_1I_MAIN_FUNCTION(Asinh, "inverse hyperbolic sine", FLOAT, FLOAT, 1)

// Inverse Hyperbolic Tangent
NMATH_1I_FUNCTION_TEMPLATE(Atanh, nr_atanhf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Atanh, nr_atanh,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Atanh, NMATH_METHODS_FLOAT(Atanh))
NMATH_1I_MAIN_FUNCTION(Atanh, "inverse hyperbolic tangent", FLOAT, FLOAT, 1)

// Round to Nearest Integer
NMATH_1I_FUNCTION_TEMPLATE(Rint, nr_rintf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Rint, nr_rint,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Rint, NMATH_METHODS_FLOAT(Rint))
NMATH_1I_MAIN_FUNCTION(Rint, "round to nearest integer", FLOAT, FLOAT, 1)

// Truncate
NMATH_1I_FUNCTION_TEMPLATE(Trunc, nr_truncf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Trunc, nr_trunc,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Trunc, NMATH_METHODS_FLOAT(Trunc))
NMATH_1I_MAIN_FUNCTION(Trunc, "truncate", FLOAT, FLOAT, 1)

// 2^x
NMATH_1I_FUNCTION_TEMPLATE(Exp2, nr_exp2f, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Exp2, nr_exp2,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Exp2, NMATH_METHODS_FLOAT(Exp2))
NMATH_1I_MAIN_FUNCTION(Exp2, "2^x", FLOAT, FLOAT, 1)

// Exponential
NMATH_1I_FUNCTION_TEMPLATE(Exp, nr_expf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Exp, nr_exp,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Exp, NMATH_METHODS_FLOAT(Exp))
NMATH_1I_MAIN_FUNCTION(Exp, "exponential", FLOAT, FLOAT, 1)

// Square Root
NMATH_1I_FUNCTION_TEMPLATE(Sqrt, nr_sqrtf, nr_float32, nr_float32)
NMATH_1I_FUNCTION_TEMPLATE(Sqrt, nr_sqrt,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Sqrt, NMATH_METHODS_FLOAT(Sqrt))
NMATH_1I_MAIN_FUNCTION(Sqrt, "square root", FLOAT, FLOAT, 1)

// Floating-point Modulo
NMATH_2I_FUNCTION_TEMPLATE(Fmod, nr_fmodf, nr_float32, nr_float32)
NMATH_2I_FUNCTION_TEMPLATE(Fmod, nr_fmod,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Fmod, NMATH_METHODS_FLOAT(Fmod))
NMATH_2I_MAIN_FUNCTION(Fmod, "floating-point modulo", FLOAT, FLOAT, 1)

// Load Exponent (2-input)
NMATH_2I_FUNCTION_TEMPLATE(Ldexp, nr_ldexpf, nr_float32, nr_float32)
NMATH_2I_FUNCTION_TEMPLATE(Ldexp, nr_ldexp,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Ldexp, NMATH_METHODS_FLOAT(Ldexp))
NMATH_2I_MAIN_FUNCTION(Ldexp, "load exponent", FLOAT, FLOAT, 1)

// Copy Sign (2-input)
NMATH_2I_FUNCTION_TEMPLATE(Copysign, nr_copysignf, nr_float32, nr_float32)
NMATH_2I_FUNCTION_TEMPLATE(Copysign, nr_copysign,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Copysign, NMATH_METHODS_FLOAT(Copysign))
NMATH_2I_MAIN_FUNCTION(Copysign, "copy sign", FLOAT, FLOAT, 1)

// Next Representable Value (2-input)
NMATH_2I_FUNCTION_TEMPLATE(Nextafter, nr_nextafterf, nr_float32, nr_float32)
NMATH_2I_FUNCTION_TEMPLATE(Nextafter, nr_nextafter,  nr_float64, nr_float64)
NMATH_METHOD_ARRAY(Nextafter, NMATH_METHODS_FLOAT(Nextafter))
NMATH_2I_MAIN_FUNCTION(Nextafter, "next representable value", FLOAT, FLOAT, 1)
