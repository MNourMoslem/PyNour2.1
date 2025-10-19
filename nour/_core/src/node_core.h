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

NR_PUBLIC Node*
Node_NewScalar(void* data, NR_DTYPE dtype);

NR_PUBLIC void
Node_SetName(Node* node, const char* name);

#endif // NR__CORE__SRC__NODE_CORE_H