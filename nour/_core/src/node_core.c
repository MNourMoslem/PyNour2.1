#include <string.h>

#include "node_core.h"
#include "nerror.h"
#include "niter.h"
#include "free.h"

char* NR_NODE_NAME = "node";


NR_PUBLIC Node*
Node_NewAdvanced(void* data, int own_data, int ndim, nr_intp* shape, 
                 nr_intp* strides, NR_DTYPE dtype, int flags, const char* name) 
{
    if (ndim < 0 || ndim > NR_NODE_MAX_NDIM){
        NError_RaiseError(NError_ValueError,
            "Invalid number of dimensions: %d. Must be between 0 and %d.",
            ndim, NR_NODE_MAX_NDIM);
        return NULL;
    }

    int is_contiguous = 1;

    Node* node = (Node*)malloc(sizeof(Node));
    if (!node){
        NError_RaiseMemoryError();
        return NULL;
    }

    NDtype dt;
    dt.dtype = dtype;
    dt.size = NDtype_Size(dtype);

    node->ndim = ndim;
    node->dtype = dt;
    node->flags = flags;
    node->ref_count = 1;
    node->base = NULL;
    node->name = name ? name : NR_NODE_NAME;
    node->op = NULL;
    node->grad = NULL;

    nr_int s = sizeof(nr_intp) * ndim;

    // Allocate and set shape
    node->shape = (nr_intp*)malloc(s);
    if (!node->shape){
        free(node);
        NError_RaiseMemoryError();
        return NULL;
    }
    memcpy(node->shape, shape, s);

    // Allocate and set strides
    node->strides = (nr_intp*)malloc(s);
    if (!node->strides){
        free(node->shape);
        free(node);
        NError_RaiseMemoryError();
        return NULL;
    }

    if (strides){
        memcpy(node->strides, strides, s);
        for (int i = 0; i < ndim - 1; i++){
            if (node->strides[i] < 0){
                is_contiguous = 0;
                break;
            }
        }
    
    } else {
        nr_intp itemsize = NDtype_Size(dtype);
        NTools_CalculateStrides(ndim, shape, itemsize, node->strides);
    }

    node->data = data;
    node->flags = NR_NODE_C_ORDER;
    if (is_contiguous){
        node->flags |= NR_NODE_CONTIGUOUS;
    }
    else{
        node->flags |= NR_NODE_STRIDED;
    }
    
    if (own_data == 1){
        node->flags |= NR_NODE_OWNDATA;
    }
    if (ndim == 0){
        node->flags = NR_NODE_SCALAR;
    }

    return node;
}

NR_PUBLIC Node*
Node_New(void* data, int own_data, int ndim, nr_intp* shape, NR_DTYPE dtype) {
    return Node_NewAdvanced(data, own_data, ndim, shape, NULL, dtype, 0, NR_NODE_NAME);
}

NR_PUBLIC Node*
Node_NewEmpty(int ndim, nr_intp* shape, NR_DTYPE dtype) {
    nr_intp dtype_size = NDtype_Size(dtype);
    nr_intp nitems = NR_NItems(ndim, shape);
    void* data = malloc(nitems * dtype_size);
    if (!data){
        NError_RaiseMemoryError();
        return NULL;
    }
    return Node_New(data, 1, ndim, shape, dtype);
}

NR_PUBLIC Node*
Node_NewScalar(void* data, NR_DTYPE dtype) {
    return Node_New(data, 0, 0, NULL, dtype);
}

NR_PUBLIC void
Node_SetName(Node* node, const char* name) {
    if (node && name) {
        node->name = strdup(name);
        if (!node->name) {
            NError_RaiseMemoryError();
            node->name = NR_NODE_NAME;
        }
    }
}

NR_PUBLIC Node*
Node_Copy(Node* dst, const Node* src) {
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, src->dtype.dtype);
    }

    return Node_CopyFromNonNode(dst, NODE_DATA(src), src->ndim,
                                src->shape, src->strides, src->dtype.dtype,
                                NODE_IS_CONTIGUOUS(src));
}

NR_PUBLIC Node*
Node_NewChild(Node* src, int ndim, nr_intp* shape, nr_intp* strides, nr_intp offset) {
    char* data = (char*)NODE_DATA(src) + offset;
    NR_DTYPE dtype = NODE_DTYPE(src);
    Node* dst = Node_NewAdvanced(data, 0, ndim, shape, strides, dtype, 0, NULL);
    if (!dst){
        return NULL;
    }

    dst->base = src;
    src->ref_count += 1;

    return dst;
}

NR_PUBLIC Node*
Node_CopyFromNonNode(Node* dst, const void* src_data, int ndim, 
                     nr_intp* shape, nr_intp* strides, NR_DTYPE dtype, int src_is_contiguous) {
    if (!dst){
        dst = Node_NewEmpty(ndim, shape, dtype);
        if (!dst){
            return NULL;
        }
    }

    int isConDst = NODE_IS_CONTIGUOUS(dst);

    if (src_is_contiguous && isConDst){
        nr_intp nitems = NR_NItems(ndim, shape);
        nr_intp dtype_size = NDtype_Size(dtype);
        memcpy(dst->data, src_data, nitems * dtype_size);
    }
    else{
        NIter src_iter;
        NIter dst_iter;

        NIter_New(&src_iter, (void*)src_data, ndim,
                         shape, strides, NITER_MODE_STRIDED);
        NIter_New(&dst_iter, dst->data, dst->ndim,
                         dst->shape, dst->strides, NITER_MODE_STRIDED);

        NIter_ITER(&src_iter);
        NIter_ITER(&dst_iter);

        nr_intp dtype_size = NDtype_Size(dtype);
        while (NIter_NOTDONE(&src_iter) && NIter_NOTDONE(&dst_iter)){
            memcpy(NIter_ITEM(&dst_iter), NIter_ITEM(&src_iter), dtype_size);
            NIter_NEXT(&src_iter);
            NIter_NEXT(&dst_iter);
        }
    }
    return dst;
}