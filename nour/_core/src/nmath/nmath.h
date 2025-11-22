#ifndef NOUR__CORE_SRC_NMATH_NMATH_H
#define NOUR__CORE_SRC_NMATH_NMATH_H

#include "nour/nr_node.h"
#include "reduce.h"

NR_PUBLIC Node* NMath_Add(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Sub(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Mul(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Div(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Mod(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_TrueDiv(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Pow(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Bg(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Bge(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Ls(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Lse(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Eq(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Neq(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_BitAnd(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_BitOr(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_BitXor(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_BitNot(Node* c, Node* a);
NR_PUBLIC Node* NMath_BitLsh(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_BitRsh(Node* c, Node* b, Node* a);

NR_PUBLIC Node* NMath_Neg(Node* c, Node* a);
NR_PUBLIC Node* NMath_Sin(Node* c, Node* a);
NR_PUBLIC Node* NMath_Cos(Node* c, Node* a);
NR_PUBLIC Node* NMath_Tan(Node* c, Node* a);
NR_PUBLIC Node* NMath_Cot(Node* c, Node* a);
NR_PUBLIC Node* NMath_Exp(Node* c, Node* a);
NR_PUBLIC Node* NMath_Log(Node* c, Node* a);
NR_PUBLIC Node* NMath_Log10(Node* c, Node* a);
NR_PUBLIC Node* NMath_Sinh(Node* c, Node* a);
NR_PUBLIC Node* NMath_Cosh(Node* c, Node* a);
NR_PUBLIC Node* NMath_Tanh(Node* c, Node* a);
NR_PUBLIC Node* NMath_Coth(Node* c, Node* a);
NR_PUBLIC Node* NMath_Asin(Node* c, Node* a);
NR_PUBLIC Node* NMath_Acos(Node* c, Node* a);
NR_PUBLIC Node* NMath_Atan(Node* c, Node* a);
NR_PUBLIC Node* NMath_Asinh(Node* c, Node* a);
NR_PUBLIC Node* NMath_Acosh(Node* c, Node* a);
NR_PUBLIC Node* NMath_Atanh(Node* c, Node* a);
NR_PUBLIC Node* NMath_Exp2(Node* c, Node* a);
NR_PUBLIC Node* NMath_Expm1(Node* c, Node* a);
NR_PUBLIC Node* NMath_Log1p(Node* c, Node* a);
NR_PUBLIC Node* NMath_Sqrt(Node* c, Node* a);
NR_PUBLIC Node* NMath_Cbrt(Node* c, Node* a);
NR_PUBLIC Node* NMath_Abs(Node* c, Node* a);
NR_PUBLIC Node* NMath_Ceil(Node* c, Node* a);
NR_PUBLIC Node* NMath_Floor(Node* c, Node* a);
NR_PUBLIC Node* NMath_Trunc(Node* c, Node* a);
NR_PUBLIC Node* NMath_Rint(Node* c, Node* a);
NR_PUBLIC int NMath_Frexp(Node** mantissa, Node** exponent, Node* a);
NR_PUBLIC Node* NMath_Ldexp(Node* c, Node* mantissa, Node* exponent);
NR_PUBLIC int NMath_Modf(Node** fractional, Node** integer, Node* a);


#endif // NOUR__CORE_SRC_NMATH_NMATH_H