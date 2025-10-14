#ifndef NR__CORE__INCLUDE__NR_MATH_H
#define NR__CORE__INCLUDE__NR_MATH_H

#include <math.h>
#include <float.h>
#include <stdint.h>

/* Infinity definitions */
#define NR_INFF (float)INFINITY
#define NR_INF INFINITY

/* Mathematical constants (double precision) */
#define NR_E         2.718281828459045235360287471352662498  /* e */
#define NR_LOG2E     1.442695040888963407359924681001892137  /* log_2 e */
#define NR_LOG10E    0.434294481903251827651128918916605082  /* log_10 e */
#define NR_LOGE2     0.693147180559945309417232121458176568  /* log_e 2 */
#define NR_LOGE10    2.302585092994045684017991454684364208  /* log_e 10 */
#define NR_PI        3.141592653589793238462643383279502884  /* pi */
#define NR_PI_2      1.570796326794896619231321691639751442  /* pi/2 */
#define NR_PI_4      0.785398163397448309615660845819875721  /* pi/4 */
#define NR_1_PI      0.318309886183790671537767526745028724  /* 1/pi */
#define NR_2_PI      0.636619772367581343075535053490057448  /* 2/pi */
#define NR_EULER     0.577215664901532860606512090082402431  /* Euler constant */
#define NR_SQRT2     1.414213562373095048801688724209698079  /* sqrt(2) */
#define NR_SQRT1_2   0.707106781186547524400844362104849039  /* 1/sqrt(2) */

/* Mathematical constants (single precision) */
#define NR_Ef        2.718281828459045235360287471352662498F /* e */
#define NR_LOG2Ef    1.442695040888963407359924681001892137F /* log_2 e */
#define NR_LOG10Ef   0.434294481903251827651128918916605082F /* log_10 e */
#define NR_LOGE2f    0.693147180559945309417232121458176568F /* log_e 2 */
#define NR_LOGE10f   2.302585092994045684017991454684364208F /* log_e 10 */
#define NR_PIf       3.141592653589793238462643383279502884F /* pi */
#define NR_PI_2f     1.570796326794896619231321691639751442F /* pi/2 */
#define NR_PI_4f     0.785398163397448309615660845819875721F /* pi/4 */
#define NR_1_PIf     0.318309886183790671537767526745028724F /* 1/pi */
#define NR_2_PIf     0.636619772367581343075535053490057448F /* 2/pi */
#define NR_EULERf    0.577215664901532860606512090082402431F /* Euler constant */
#define NR_SQRT2f    1.414213562373095048801688724209698079F /* sqrt(2) */
#define NR_SQRT1_2f  0.707106781186547524400844362104849039F /* 1/sqrt(2) */

/* Double precision mathematical functions */
#define nr_sinh sinh
#define nr_cosh cosh
#define nr_tanh tanh
#define nr_asin asin
#define nr_acos acos
#define nr_atan atan
#define nr_log log
#define nr_log10 log10
#define nr_cbrt cbrt
#define nr_fabs fabs
#define nr_ceil ceil
#define nr_fmod fmod
#define nr_floor floor
#define nr_expm1 expm1
#define nr_log1p log1p
#define nr_acosh acosh
#define nr_asinh asinh
#define nr_atanh atanh
#define nr_rint rint
#define nr_trunc trunc
#define nr_exp2 exp2
#define nr_frexp frexp
#define nr_ldexp ldexp
#define nr_copysign copysign
#define nr_exp exp
#define nr_sqrt sqrt
#define nr_pow pow
#define nr_modf modf
#define nr_nextafter nextafter

/* Single precision mathematical functions */
#define nr_sinhf sinhf
#define nr_coshf coshf
#define nr_tanhf tanhf
#define nr_asinf asinf
#define nr_acosf acosf
#define nr_atanf atanf
#define nr_logf logf
#define nr_log10f log10f
#define nr_cbrtf cbrtf
#define nr_fabsf fabsf
#define nr_ceilf ceilf
#define nr_fmodf fmodf
#define nr_floorf floorf
#define nr_expm1f expm1f
#define nr_log1pf log1pf
#define nr_asinhf asinhf
#define nr_acoshf acoshf
#define nr_atanhf atanhf
#define nr_rintf rintf
#define nr_truncf truncf
#define nr_exp2f exp2f
#define nr_frexpf frexpf
#define nr_ldexpf ldexpf
#define nr_copysignf copysignf
#define nr_nextafterf nextafterf

/* Basic mathematical operations */
#define NMATH_ADD(a, b) (a + b)
#define NMATH_MUL(a, b) (a * b)
#define NMATH_SUB(a, b) (a - b)
#define NMATH_BG(a, b) (a > b)
#define NMATH_BGE(a, b) (a >= b)
#define NMATH_LS(a, b) (a < b)
#define NMATH_LSE(a, b) (a <= b)
#define NMATH_EQ(a, b) (a == b)
#define NMATH_NEQ(a, b) (a != b)

/* Integer-specific operations */
#define NMATH_MOD(a, b) (a % b)
#define NMATH_TDV(a, b) (a / b)

/* Floating-point operations */
#define NMATH_DIV(a, b) (a / b)
#define NMATH_POW(a, b) (nr_pow(a, b))

#endif // NR__CORE__INCLUDE__NR_MATH_H