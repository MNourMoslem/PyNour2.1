#ifndef NOUR__CORE_SRC_NMATH_REDUCE_H
#define NOUR__CORE_SRC_NMATH_REDUCE_H


#include "nour/nour.h"
#include "../nfunc.h"


typedef struct
{
    int axis[NR_NODE_MAX_NDIM];
    int n_axis;
}NFunc_ReduceArgs;

NR_PUBLIC NFunc_ReduceArgs
NFunc_ReduceArgs_New(const int* axes, const int n_axes);

// Public API declarations
NR_PUBLIC Node* NMath_Sum(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_Prod(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_Min(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_Max(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_Mean(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_Var(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_Std(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_Argmin(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_Argmax(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_All(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_Any(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_CountNonzero(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_NanSum(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_NanProd(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_NanMin(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_NanMax(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_NanMean(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_NanVar(Node* c, Node* a, int* axis, int na);
NR_PUBLIC Node* NMath_NanStd(Node* c, Node* a, int* axis, int na);

#endif // NOUR__CORE_SRC_NMATH_REDUCE_H