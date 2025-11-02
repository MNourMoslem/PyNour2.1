#include "niter.h"
#include "nerror.h"
#include "ntools.h"

NR_PUBLIC void
NIter_FromNode(NIter* niter, const Node* node, int iter_mode){
    if (iter_mode == NITER_MODE_NONE){
        iter_mode = NODE_IS_CONTIGUOUS(node) ? NITER_MODE_CONTIGUOUS : NITER_MODE_STRIDED;
    }

    NIter_New(niter, node->data, node->ndim,
                     node->shape, node->strides, iter_mode);
}

NR_PUBLIC void
NIter_New(NIter* niter ,void* data, int ndim, const nr_intp* shape,
          const nr_intp* strides, int iter_mode)
{
    niter->data = data;
    niter->nd_m1 = ndim - 1;
    memcpy(niter->strides, strides, sizeof(nr_intp) * ndim);
    for (int i = 0; i < ndim; i++){
        niter->shape_m1[i] = shape[i] - 1;
        niter->backstrides[i] = strides[i] * niter->shape_m1[i];
    }

    niter->end = NR_NItems(ndim, shape);
    niter->idx = niter->end;
    niter->step = niter->strides[niter->nd_m1];
    niter->iter_mode = iter_mode;
}

NR_PUBLIC int
NMultiIter_FromNodes(Node** nodes, int n_nodes, NMultiIter* mit){
    nr_intp* shapes[NR_MULTIITER_MAX_NITER];
    nr_intp* strides[NR_MULTIITER_MAX_NITER];
    int ndims[NR_MULTIITER_MAX_NITER];
    void* data_ptrs[NR_MULTIITER_MAX_NITER];

    for (int i = 0; i < n_nodes; i++){
        shapes[i] = nodes[i]->shape;
        strides[i] = nodes[i]->strides;
        ndims[i] = nodes[i]->ndim;
        data_ptrs[i] = nodes[i]->data;
    }

    return NMultiIter_New(data_ptrs, n_nodes, ndims, shapes, strides, mit);
}

NR_PUBLIC int
NMultiIter_New(void** data_ptr, int num, int* ndims, nr_intp** shapes, nr_intp** strides, NMultiIter* mit){
    // Use the new broadcast shapes function for arrays
    if (NTools_BroadcastShapesFromArrays(shapes, ndims, num, mit->out_shape, &mit->out_ndim) != 0) {
        return -1;
    }

    nr_intp tmp_str[NR_NODE_MAX_NDIM];
    int tmp;
    
    for (int i = 0; i < num; i++){
        tmp = NTools_BroadcastStrides(shapes[i], ndims[i], strides[i], mit->out_shape, mit->out_ndim, tmp_str);
    
        if (tmp != 0){
            return -1;
        }

        // Check if this array matches the output shape exactly and has contiguous strides
        if (ndims[i] == mit->out_ndim 
            && memcmp(shapes[i], mit->out_shape, ndims[i] * sizeof(nr_intp)) == 0)
        {
            // Check if strides are contiguous
            nr_intp expected_stride = 1;  // Assuming itemsize of 1, this could be parameterized
            int is_contiguous = 1;
            for (int j = ndims[i] - 1; j >= 0; j--) {
                if (strides[i][j] != expected_stride) {
                    is_contiguous = 0;
                    break;
                }
                expected_stride *= shapes[i][j];
            }
            tmp = is_contiguous ? NITER_MODE_CONTIGUOUS : NITER_MODE_NONE;
        }
        else
        {
            tmp = NITER_MODE_NONE;
        }

        NIter_New(mit->iters + i, data_ptr[i], ndims[i], mit->out_shape, tmp_str, tmp);
    }

    mit->end = (int)NR_NItems(mit->out_ndim, mit->out_shape);
    mit->n_iter = num;

    return 0;
}


#define _SET_POINTER_TO_ONES_BLOCK(ptr) do {\
    if (NR_NODE_MAX_NDIM == 32){\
        nr_intp ones[] = NR_32ONES;\
        ptr = ones;\
    }\
    else{\
        nr_intp ones[NR_NODE_MAX_NDIM];\
        for (int i = 0; i < NR_NODE_MAX_NDIM; i++){\
            ones[i] = 1;\
        }\
        ptr = ones;\
    }\
} while (0)


NR_PUBLIC int
NWindowIter_New(const Node* node, NWindowIter* wit, const nr_intp* window_dims,
                const nr_intp* strides_factor, const nr_intp* dilation)
{
    for (int i = 0; i < node->ndim; i++){
        if (node->shape[i] < window_dims[i]){
            NError_RaiseError(
                NError_ValueError,
                "dimension length of window at dim %i is bigger than node. got %llu and %llu",
                i, window_dims[i], node->shape[i]
            );
            return -1;
        }
    }

    if (!strides_factor){
        _SET_POINTER_TO_ONES_BLOCK(strides_factor);
    }
    if (!dilation){
        _SET_POINTER_TO_ONES_BLOCK(dilation);
    }

    wit->end = 1;
    wit->data = node->data;
    wit->nd_m1 = node->ndim - 1;
    memcpy(wit->strides, node->strides, sizeof(nr_intp) * node->ndim);


    nr_intp wdim_len;
    for (int i = 0; i < node->ndim; i++){
        wdim_len = dilation[i] * (window_dims[i] - 1) + 1;
        wit->shape_m1[i] = ((node->shape[i] - wdim_len) / strides_factor[i]);
        wit->strides[i] = node->strides[i] * strides_factor[i];
        wit->backstrides[i] = wit->strides[i] * wit->shape_m1[i];

        wit->wshape_m1[i] = window_dims[i] - 1;
        wit->wstrides[i] = node->strides[i] * dilation[i];
        wit->wbackstrides[i] = wit->wstrides[i] * wit->wshape_m1[i];

        wit->end *= (int)wit->shape_m1[i] + 1;
    }

    wit->idx = wit->end;
    wit->wend = NR_NItems(node->ndim, window_dims);
    wit->widx = wit->wend;
    wit->iter_mode =  NITER_MODE_STRIDED;

    if (wit->nd_m1 == 0){
        wit->wstep = wit->wstrides[wit->nd_m1];
        wit->wmode = NITER_MODE_CONTIGUOUS;
    }
    else{
        wit->wmode = NITER_MODE_STRIDED;
    }

    return 0;
}