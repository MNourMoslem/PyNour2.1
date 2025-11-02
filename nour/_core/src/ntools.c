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
    nr_intp* shapes[NR_MULTIITER_MAX_NITER];
    int ndims[NR_MULTIITER_MAX_NITER];

    for (int i = 0; i < n_nodes; i++) {
        if (!nodes[i]) {
            return -1; // Null node
        }
        shapes[i] = nodes[i]->shape;
        ndims[i] = nodes[i]->ndim;
    }

    return NTools_BroadcastShapesFromArrays(shapes, ndims, n_nodes, out_shape, out_ndim);
}

NR_PUBLIC int
NTools_BroadcastShapesFromArrays(nr_intp** shapes, int* ndims, int n_shapes, nr_intp* out_shape, int* out_ndim) {
    if (!shapes || !ndims || n_shapes <= 0 || !out_shape || !out_ndim) {
        return -1;
    }

    // Find the maximum number of dimensions
    int nd = -1;
    for (int i = 0; i < n_shapes; i++) {
        if (!shapes[i]) {
            return -1; // Null shape
        }
        nd = NR_MAX(ndims[i], nd);
    }
    *out_ndim = nd;

    int src_shape_idx;
    int tmp;
    nr_intp dim;

    // Compute the broadcast shape
    for (int i = 0; i < nd; i++) {
        out_shape[i] = 1;
        for (int j = 0; j < n_shapes; j++) {
            tmp = i + ndims[j] - nd;
            if (tmp >= 0) {
                dim = shapes[j][tmp];
                if (dim == 1) {
                    continue;
                }
                else if (out_shape[i] == 1) {   
                    out_shape[i] = dim;
                    src_shape_idx = j;
                }
                else if (out_shape[i] != dim) {
                    char shape1[100];
                    char shape2[100];

                    NTools_ShapeAsString(shapes[j], ndims[j], shape1);
                    NTools_ShapeAsString(shapes[src_shape_idx], ndims[src_shape_idx], shape2);

                    NError_RaiseError(NError_ValueError,
                        "objects cannot be broadcast due to mismatch at arg %d " 
                        "with shape %s and arg %d with shape %s",
                        j, shape1, src_shape_idx, shape2);
                    return -1;
                }
            }
        }
    }

    return 0;
}