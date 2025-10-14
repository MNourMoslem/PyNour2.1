#ifndef NOUR__CORE_SRC_NOUR_NODE_NTOOLS_H
#define NOUR__CORE_SRC_NOUR_NODE_NTOOLS_H

#include "nour/nour.h"

NR_PUBLIC int
NTools_BroadcastStrides(nr_intp* a_shape, int a_ndim,
                        nr_intp* a_strides, nr_intp* src_shape,
                        int src_ndim, nr_intp* out_strides);

NR_PUBLIC NR_DTYPE
NTools_BroadcastDtypes(NR_DTYPE a, NR_DTYPE b);

NR_PUBLIC int
NTools_CalculateStrides(int nd, const nr_intp* shape,
                        nr_intp itemsize, nr_intp* _des_strides);

NR_PUBLIC void
NTools_ShapeAsString(nr_intp* shape, int ndim, char str[]);

#endif