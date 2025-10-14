#include "niter.h"
#include "nerror.h"

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
NMultiIter_New(Node** nodes, int n_nodes, NMultiIter* mit){
    int nd = -1;
    for (int i = 0; i < n_nodes; i++){
        nd = NR_MAX(nodes[i]->ndim, nd);
    }
    mit->out_ndim = nd;

    int src_node, tmp;
    nr_intp dim;
    Node* node;
    for (int i = 0; i < nd; i++){
        mit->out_shape[i] = 1;
        for (int j = 0; j < n_nodes; j++){
            node = nodes[j];
            tmp = i + node->ndim - nd;
            if (tmp >= 0){
                dim = node->shape[tmp];
                if (dim == 1){
                    continue;
                }
                else if (mit->out_shape[i] == 1)
                {   
                    mit->out_shape[i] = dim;
                    src_node = j;
                }
                else if (mit->out_shape[i] != dim){
                    char shape1[100];
                    char shape2[100];

                    NTools_ShapeAsString(node->shape, node->ndim, shape1);
                    NTools_ShapeAsString(nodes[src_node]->shape, nodes[src_node]->ndim, shape2);

                    NError_RaiseError(NError_ValueError,
                        "objects cannot be broadcast due mismatch at arg %d " 
                        "with shape %s and arg %d with shape %s",
                        j, shape1, src_node, shape2
                    );
                }
            }
        }
    }

    nr_intp tmp_str[NR_NODE_MAX_NDIM];
    for (int i = 0; i < n_nodes; i++){
        node = nodes[i];
        tmp = NTools_BroadcastStrides(node->shape, node->ndim, node->strides, mit->out_shape, mit->out_ndim, tmp_str);
    
        if (tmp != 0){
            return -1;
        }

        if (node->ndim == mit->out_ndim 
            && memcmp(node->shape, mit->out_shape, node->ndim * sizeof(nr_intp)) == 0 
            && NODE_IS_CONTIGUOUS(node))
        {
            tmp = NITER_MODE_CONTIGUOUS;
        }
        else
        {
            tmp = NITER_MODE_NONE;
        }

        NIter_New(mit->iters + i, node->data, node->ndim, mit->out_shape, tmp_str, tmp);
    }

    mit->end = (int)NR_NItems(mit->out_ndim, mit->out_shape);
    mit->n_iter = n_nodes;

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