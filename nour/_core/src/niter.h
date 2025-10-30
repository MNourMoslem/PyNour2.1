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

#define Node_LOOP_MODED_NAMED(node, iter_name, iter_mode, code)         \
do{                                                    \
                                                       \
    NIter iter_name;                                   \
    NIter_FromNode(&iter_name, (node), (iter_mode));   \
    NIter_ITER(&iter_name);                            \
    while (NIter_NOTDONE(&iter_name))                  \
    {                                                  \
        code                                           \
        NIter_NEXT(&iter_name);                        \
    }                                                  \
} while (0);


#define Node_LOOP_MODED(node, iter_mode, code) \
    Node_LOOP_MODED_NAMED(node, __iter, iter_mode, code)

#define Node_LOOP_NAMED(node, iter_name, code) \
    Node_LOOP_MODED_NAMED(node, iter_name, NITER_MODE_NONE, code)

#define Node_LOOP(node, code) \
    Node_LOOP_MODED(node, NITER_MODE_NONE, code)


#endif