#ifndef NOUR__CORE_SRC_NOUR___TC_METHODS_H
#define NOUR__CORE_SRC_NOUR___TC_METHODS_H

#include "nour/nour.h"

extern Node2NodeFunc __NODE_TC_METHODS_ARRAY__[];

NR_PUBLIC Node*
Node_ToType(Node* dst, const Node* src, NR_DTYPE dtype);
#endif