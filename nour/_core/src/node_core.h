#ifndef NR__CORE__SRC__NODE_CORE_H
#define NR__CORE__SRC__NODE_CORE_H

#include "nour/nour.h"

NR_PUBLIC Node*
Node_New(void* data, int copy_data, int ndim, nr_intp* shape, NR_DTYPE dtype);

NR_PUBLIC Node*
Node_NewEmpty(int ndim, nr_intp* shape, NR_DTYPE dtype);

NR_PUBLIC Node*
Node_Copy(Node* dst, const Node* src);

NR_PUBLIC Node*
Node_CopyWithReference(const Node* src);

#endif // NR__CORE__SRC__NODE_CORE_H