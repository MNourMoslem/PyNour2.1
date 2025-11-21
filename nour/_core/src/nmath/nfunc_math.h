#ifndef NOUR__CORE_SRC_NMATH_NFUNC_MATH_H
#define NOUR__CORE_SRC_NMATH_NFUNC_MATH_H

#include "nour/nour.h"
#include "../nfunc.h"

extern const NFunc add_nfunc;       // Addition
extern const NFunc sub_nfunc;       // Subtraction
extern const NFunc mul_nfunc;       // Multiplication
extern const NFunc div_nfunc;       // Division
extern const NFunc truediv_nfunc;   // True Division
extern const NFunc mod_nfunc;       // Modulus
extern const NFunc pow_nfunc;

extern const NFunc bg_nfunc;        // Greater than
extern const NFunc bge_nfunc;       // Greater than or equal
extern const NFunc ls_nfunc;        // Less than
extern const NFunc lse_nfunc;       // Less than or equal
extern const NFunc eq_nfunc;        // Equal to
extern const NFunc neq_nfunc;       // Not equal to

extern const NFunc bit_and_nfunc;   // Bitwise AND
extern const NFunc bit_or_nfunc;    // Bitwise OR
extern const NFunc bit_xor_nfunc;   // Bitwise XOR
extern const NFunc bit_not_nfunc;   // Bitwise NOT
extern const NFunc bit_lsh_nfunc;   // Bitwise left shift
extern const NFunc bit_rsh_nfunc;   // Bitwise right shift

extern const NFunc neg_nfunc;       // Negation
extern const NFunc sin_nfunc;       // Sine
extern const NFunc cos_nfunc;       // Cosine
extern const NFunc tan_nfunc;       // Tangent
extern const NFunc cot_nfunc;       // Cotangent
extern const NFunc sinh_nfunc;      // Hyperbolic sine
extern const NFunc cosh_nfunc;      // Hyperbolic cosine
extern const NFunc tanh_nfunc;      // Hyperbolic tangent
extern const NFunc coth_nfunc;      // Hyperbolic cotangent
extern const NFunc asin_nfunc;      // Arc sine
extern const NFunc acos_nfunc;      // Arc cosine
extern const NFunc atan_nfunc;      // Arc tangent
extern const NFunc acot_nfunc;      // Arc cotangent
extern const NFunc asinh_nfunc;     // Inverse hyperbolic sine
extern const NFunc acosh_nfunc;     // Inverse hyperbolic cosine
extern const NFunc atanh_nfunc;     // Inverse hyperbolic tangent
extern const NFunc acoth_nfunc;     // Inverse hyperbolic cotangent
extern const NFunc exp_nfunc;       // Exponential (e^x)
extern const NFunc exp2_nfunc;      // Base-2 exponential (2^x)
extern const NFunc expm1_nfunc;     // Exponential minus 1 (e^x - 1)
extern const NFunc log_nfunc;       // Natural logarithm
extern const NFunc log10_nfunc;     // Base-10 logarithm
extern const NFunc log1p_nfunc;     // Logarithm of (1 + x)
extern const NFunc sqrt_nfunc;      // Square root
extern const NFunc cbrt_nfunc;      // Cube root
extern const NFunc abs_nfunc;       // Absolute value
extern const NFunc ceil_nfunc;      // Ceiling
extern const NFunc floor_nfunc;     // Floor
extern const NFunc trunc_nfunc;     // Truncate
extern const NFunc rint_nfunc;      // Round to nearest integer
extern const NFunc fmod_nfunc;      // Floating-point modulus
extern const NFunc copysign_nfunc;  // Copy sign
extern const NFunc nextafter_nfunc; // Next representable value
extern const NFunc frexp_nfunc;     // Extract mantissa and exponent
extern const NFunc ldexp_nfunc;     // Load exponent
extern const NFunc modf_nfunc;      // Extract integer and fractional parts

#endif // NOUR__CORE_SRC_NMATH_NFUNC_MATH_H