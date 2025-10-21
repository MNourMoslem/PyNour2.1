#ifndef NOUR__CORE_SRC_NOUR_INDEX_H
#define NOUR__CORE_SRC_NOUR_INDEX_H

#include "nour/nour.h"

typedef struct {
    nr_intp start;
    nr_intp stop;
    nr_intp step;
} Slice;

NR_PUBLIC Node*
Node_Slice(Node* nout, const Node* node, const Slice slice, int dim);

NR_PUBLIC Node*
Node_MultiSlice(Node* nout, const Node* node, const Slice* slices, int num_slices);

NR_PUBLIC Node*
Node_BooleanMask(const Node* node, const Node* bool_mask);

#endif // NOUR__CORE_SRC_NOUR_INDEX_H