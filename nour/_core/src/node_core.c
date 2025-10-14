#include <string.h>

#include "node_core.h"
#include "nerror.h"
#include "niter.h"
#include "free.h"

typedef struct
{
    Node* node;
    nr_intp nitems;
} NodeCopyHelper;


char* NR_NODE_NAME = "node";

NR_PRIVATE Node*
_Node_NewInit(int ndim, NR_DTYPE dtype){
    Node* node = malloc(sizeof(Node));
    if (!node){
        return NError_RaiseMemoryError();
    }

    node->name = NR_NODE_NAME;

    if (ndim > NR_NODE_MAX_NDIM){
        NError_RaiseError(
            NError_ValueError,
            "%s object could not have more than %i dimensions. got %i\n",
            node->name, NR_NODE_MAX_NDIM, ndim
        );
        free(node);
        return NULL;
    }

    node->ndim = ndim;
    node->dtype = NDtype_New(dtype);
    node->base = NULL;
    node->flags = 0;

    return node;
}

NR_PRIVATE nr_intp
_Node_NewInitShapeAndStrides(Node* node, nr_intp* shape){
    nr_intp len = sizeof(nr_intp) * node->ndim;
    node->shape = malloc(len);
    if (!node->shape){
        NError_RaiseMemoryError();
        return -1;
    }
    memcpy(node->shape, shape, len);

    node->strides = malloc(len);
    if (!node->strides){
        free(node->shape);
        NError_RaiseMemoryError();
        return -1;
    }

    nr_intp nitems = 1;
    for (int i = node->ndim-1; i > -1; i--){
        node->strides[i] = nitems * node->dtype.size;
        nitems *= node->shape[i];
    }

    return nitems;
}

NR_PRIVATE int
_Node_NewInitAndCopyData(Node* node, void* data, nr_intp nitems){
    node->data = malloc(nitems * node->dtype.size);
    if (!node->data){
        NError_RaiseMemoryError();
        return -1;
    }

    memcpy(node->data, data, nitems * node->dtype.size);
    return 0;
}

NR_PRIVATE NodeCopyHelper
_Node_NewEmpty(int ndim, nr_intp* shape, NR_DTYPE dtype){
    NodeCopyHelper nch;
    nch.node = _Node_NewInit(ndim, dtype);
    if (!nch.node){
        nch.nitems = -1;
        return nch;
    }

    nch.nitems = _Node_NewInitShapeAndStrides(nch.node, shape);
    if (nch.nitems < 0){
        free(nch.node);
        nch.node = NULL;
        return nch;
    }
    
    return nch;
}

NR_PUBLIC Node*
Node_New(void* data_block, int copy_data, int ndim, nr_intp* shape, NR_DTYPE dtype){
    NodeCopyHelper nch = _Node_NewEmpty(ndim, shape, dtype);
    if (!nch.node || nch.nitems < 0){
        NError_RaiseMemoryError();
        return NULL;
    }

    if (copy_data){
        nr_size_t data_size = nch.nitems * nch.node->dtype.size;
        nch.node->data = malloc(data_size);
        if (!nch.node->data) {
            free(nch.node->shape);
            free(nch.node->strides);
            free(nch.node);
            NError_RaiseMemoryError();
            return NULL;
        }
        memcpy(nch.node->data, data_block, data_size);
        NR_SETFLG(nch.node->flags, NR_NODE_OWNDATA);
    }
    else{
        nch.node->data = data_block;
    }

    NR_SETFLG(nch.node->flags, (NR_NODE_C_ORDER | NR_NODE_CONTIGUOUS | NR_NODE_WRITABLE));

    return nch.node;
}

NR_PUBLIC Node*
Node_NewEmpty(int ndim, nr_intp* shape, NR_DTYPE dtype){
    NodeCopyHelper nch = _Node_NewEmpty(ndim, shape, dtype);
    if (!nch.node || nch.nitems < 0){
        NError_RaiseMemoryError();
        return NULL;
    }

    nch.node->data = malloc(nch.node->dtype.size * nch.nitems);
    if (!nch.node->data){
        free(nch.node->shape);
        free(nch.node->strides);
        free(nch.node);
        NError_RaiseMemoryError();
        return NULL;
    }

    NR_SETFLG(nch.node->flags, (NR_NODE_C_ORDER | NR_NODE_OWNDATA | NR_NODE_CONTIGUOUS | NR_NODE_WRITABLE));

    return nch.node;
}

NR_PUBLIC Node*
Node_Copy(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, src->dtype.dtype);
    }
    else if (!Node_SameShape(dst, src)){
        char dshp[30];
        char sshp[30];

        NTools_ShapeAsString(dst->shape, dst->ndim, dshp);
        NTools_ShapeAsString(src->shape, src->ndim, sshp);

        NError_RaiseError(
            NError_ValueError,
            "%s object has to have the same shape as src %s. got %s and %s",
            dst->name, src->name, dshp, sshp
        );

        return NULL;
    }
    else if (dst->dtype.dtype != src->dtype.dtype){
        char dt[30];
        char st[30];

        NDtype_AsStringOnlyType(dst->dtype.dtype, dt);
        NDtype_AsStringOnlyType(src->dtype.dtype, st);
     
        NError_RaiseError(
            NError_ValueError,
            "%s object and src %s object must have the same dtype. got %s and %s",
            dst->name, src->name, dt, st
        );

        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_intp nitems = Node_NItems(dst);
        memcpy(dst->data, src->data, nitems * dst->dtype.size);
    }
    else if (dcon | scon){
        if (dcon){
            nr_intp bsize = dst->dtype.size;

            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            nr_intp i = 0;
            while (NIter_NOTDONE(&it))
            {
                memcpy(dst->data + i, NIter_ITEM(&it), bsize);
                i+=bsize;
                NIter_NEXT(&it);
            }
        }
        else{
            nr_intp bsize = dst->dtype.size;

            NIter it;
            NIter_FromNode(&it, dst, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            nr_intp i = 0;
            while (NIter_NOTDONE(&it))
            {
                memcpy(NIter_ITEM(&it), src->data + i, bsize);
                i+=bsize;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        nr_intp bsize = dst->dtype.size;

        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, dst, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            memcpy(NIter_ITEM(&dit), NIter_ITEM(&sit), bsize);
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}