#include "ntools.h"
#include "nerror.h"

NR_PUBLIC int
NTools_BroadcastStrides(nr_intp* a_shape, int a_ndim,
                        nr_intp* a_strides, nr_intp* src_shape,
                        int src_ndim, nr_intp* out_strides)
{
    if (src_ndim < a_ndim){
        return -1;
    }

    for (int i = 0; i < src_ndim; i++){
        if (src_shape[i] == a_shape[i]){
            out_strides[i] = a_strides[i];
        }
        else if (a_shape[i] == 1){
            out_strides[i] = 0;
        }
        else{
            return -1;
        }
    }

    return 0;
}

NR_PUBLIC NR_DTYPE
NTools_BroadcastDtypes(NR_DTYPE a, NR_DTYPE b){
    if (a == b){
        return a;
    }

    NR_DTYPE c = a > b ? a : b;
    if (c <= NR_UINT64){
        if ((c & 1) == 0){
            return NR_FLOAT64;
        }
        return c;
    }
    return c;
}

NR_PUBLIC int
NTools_CalculateStrides(int nd, const nr_intp* shape,
                        nr_intp itemsize, nr_intp* _des_strides)
{
    nr_intp nitems = 1;
    if (!shape){
        return -1;
    }

    for (int i = nd-1; i > -1; i--){
        _des_strides[i] = nitems * itemsize;
        nitems *= shape[i];
    }

    return 0;
}

NR_PUBLIC void
NTools_ShapeAsString(nr_intp* shape, int ndim, char str[]) {
    str[0] = '(';

    int current = 1;

    for (int i = 0; i < ndim; i++) {
        int len = snprintf(str + current, 20, "%llu", shape[i]);
        current += len;

        if (i < ndim - 1) {
            str[current++] = ',';
            str[current++] = ' ';
        }
    }

    str[current++] = ')';
    str[current] = '\0';
}

NR_PUBLIC int
NTools_BroadcastShapes(Node** nodes, int n_nodes, nr_intp* out_shape, int* out_ndim) {
    if (!nodes || n_nodes <= 0 || !out_shape || !out_ndim) {
        return -1;
    }

    // Find the maximum number of dimensions
    int nd = -1;
    for (int i = 0; i < n_nodes; i++) {
        if (!nodes[i]) {
            return -1; // Null node
        }
        nd = NR_MAX(nodes[i]->ndim, nd);
    }
    *out_ndim = nd;

    int src_node;
    int tmp;
    nr_intp dim;
    Node* node;

    // Compute the broadcast shape
    for (int i = 0; i < nd; i++) {
        out_shape[i] = 1;
        for (int j = 0; j < n_nodes; j++) {
            node = nodes[j];
            tmp = i + node->ndim - nd;
            if (tmp >= 0) {
                dim = node->shape[tmp];
                if (dim == 1) {
                    continue;
                }
                else if (out_shape[i] == 1) {   
                    out_shape[i] = dim;
                    src_node = j;
                }
                else if (out_shape[i] != dim) {
                    char shape1[100];
                    char shape2[100];

                    NTools_ShapeAsString(node->shape, node->ndim, shape1);
                    NTools_ShapeAsString(nodes[src_node]->shape, nodes[src_node]->ndim, shape2);

                    NError_RaiseError(NError_ValueError,
                        "objects cannot be broadcast due to mismatch at arg %d " 
                        "with shape %s and arg %d with shape %s",
                        j, shape1, src_node, shape2);
                    return -1;
                }
            }
        }
    }

    return 0;
}