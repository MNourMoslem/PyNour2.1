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

NR_PUBLIC Node*
Node_NewAdvanced(void* data_block, int copy_data, int ndim, nr_intp* shape, 
                 nr_intp* strides, NR_DTYPE dtype, int flags, const char* name);

NR_PUBLIC Node*
Node_NewEmptyAdvanced(int ndim, nr_intp* shape, nr_intp* strides, 
                      NR_DTYPE dtype, int flags, const char* name);

NR_PUBLIC void
Node_SetName(Node* node, const char* name);

NR_PUBLIC int
_Node_Inherit(Node* dst, Node* src);

#endif // NR__CORE__SRC__NODE_CORE_H