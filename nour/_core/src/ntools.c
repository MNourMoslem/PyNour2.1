#include "ntools.h"

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