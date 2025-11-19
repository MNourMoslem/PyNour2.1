#ifndef NR__CORE__SRC__NODE_CORE_H
#define NR__CORE__SRC__NODE_CORE_H

#include "nour/nour.h"

NR_PUBLIC Node*
Node_New(void* data, int own_data, int ndim, nr_intp* shape, NR_DTYPE dtype);

NR_PUBLIC Node*
Node_NewEmpty(int ndim, nr_intp* shape, NR_DTYPE dtype);

NR_PUBLIC Node*
Node_Copy(Node* dst, const Node* src);

NR_PUBLIC Node*
Node_NewScalar(void* data, NR_DTYPE dtype);

NR_PUBLIC Node*
Node_NewAdvanced(void* data_block, int copy_data, int ndim, nr_intp* shape, 
                 nr_intp* strides, NR_DTYPE dtype, int flags, const char* name);

NR_PUBLIC void
Node_SetName(Node* node, const char* name);

NR_PUBLIC Node*
Node_NewChild(Node* src, int ndim, nr_intp* shape, nr_intp* strides, nr_intp offset);
    
NR_PUBLIC Node*
Node_CopyFromNonNode(Node* dst, const void* src_data, int ndim, 
                     nr_intp* shape, nr_intp* strides, NR_DTYPE dtype, int src_is_contiguous);

#endif // NR__CORE__SRC__NODE_CORE_H