#ifndef NOUR__CORE_SRC_NOUR_NITER_H
#define NOUR__CORE_SRC_NOUR_NITER_H

#include "nour/nour.h"
#include "ntools.h"

#define NR_MULTIITER_MAX_NITER 32

NR_PUBLIC void
NIter_FromNode(NIter* niter, const Node* node, int iter_mode);

NR_PUBLIC void
NIter_New(NIter* iter ,void* data, int ndim, const nr_intp* shape, const nr_intp* strides, int iter_mode);

NR_PUBLIC int
NMultiIter_New(Node** nodes, int n_nodes, NMultiIter* mit);

NR_PUBLIC int
NWindowIter_New(const Node* node, NWindowIter* wit, const nr_intp* window_dims,
                const nr_intp* strides_factor, const nr_intp* dilation);

#endif