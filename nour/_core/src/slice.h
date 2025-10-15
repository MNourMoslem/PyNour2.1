#ifndef NOUR__CORE_SRC_NOUR_SLICE_H
#define NOUR__CORE_SRC_NOUR_SLICE_H

#include "nour/nour.h"

typedef struct {
    nr_intp start;
    nr_intp stop;
    nr_intp step;
} Slice;

NR_PUBLIC int 
Node_Slice(Node* node, const Slice slice, int dim);

NR_PUBLIC int 
Node_MultiSlice(Node* node, const Slice* slices, int num_slices);

#endif // NOUR__CORE_SRC_NOUR_SLICE_H