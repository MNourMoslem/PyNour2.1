#ifndef NOUR__CORE_SRC_NMATH_CUMULATIVE_H
#define NOUR__CORE_SRC_NMATH_CUMULATIVE_H

#include "nour/nour.h"

// Cumulative args structure
typedef struct {
    int axis;
} NFunc_CumArgs;


// Public API declarations
NR_PUBLIC NFunc_CumArgs NFunc_CumArgs_New(int axis);

NR_PUBLIC Node* NMath_Cumsum(Node* c, Node* a, int axis);
NR_PUBLIC Node* NMath_Cumprod(Node* c, Node* a, int axis);
NR_PUBLIC Node* NMath_Cummin(Node* c, Node* a, int axis);
NR_PUBLIC Node* NMath_Cummax(Node* c, Node* a, int axis);
NR_PUBLIC Node* NMath_Diff(Node* c, Node* a, int axis);
NR_PUBLIC Node* NMath_Gradient(Node* c, Node* a, int axis);
NR_PUBLIC Node* NMath_NanCumsum(Node* c, Node* a, int axis);
NR_PUBLIC Node* NMath_NanCumprod(Node* c, Node* a, int axis);
NR_PUBLIC Node* NMath_NanCummin(Node* c, Node* a, int axis);
NR_PUBLIC Node* NMath_NanCummax(Node* c, Node* a, int axis);


#endif // NOUR__CORE_SRC_NMATH_CUMULATIVE_H