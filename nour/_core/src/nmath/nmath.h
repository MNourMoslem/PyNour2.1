#ifndef NOUR__CORE_SRC_NMATH_NMATH_H
#define NOUR__CORE_SRC_NMATH_NMATH_H

#include "nour/nr_node.h"

NR_PUBLIC Node* NMath_Add(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Sub(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Mul(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Div(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Mod(Node* c, Node* b, Node* a);

NR_PUBLIC Node* NMath_TrueDiv(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Pow(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Bg(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Bge(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Bl(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Ble(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Be(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_Bne(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_BitAnd(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_BitOr(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_BitXor(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_BitNot(Node* c, Node* a);
NR_PUBLIC Node* NMath_BitLsh(Node* c, Node* b, Node* a);
NR_PUBLIC Node* NMath_BitRsh(Node* c, Node* b, Node* a);

// Reductions (scalar outputs)
NR_PUBLIC Node* NMath_Sum(Node* c, Node* a);
NR_PUBLIC Node* NMath_Prod(Node* c, Node* a);
NR_PUBLIC Node* NMath_Min(Node* c, Node* a);
NR_PUBLIC Node* NMath_Max(Node* c, Node* a);


#endif // NOUR__CORE_SRC_NMATH_NMATH_H