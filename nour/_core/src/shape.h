#ifndef NOUR__CORE_SRC_NOUR_shape_H
#define NOUR__CORE_SRC_NOUR_shape_H

#include "nour/nour.h"

/* Expanded shape transformation API.
 * All functions return a Node* (view or modified in-place) on success, NULL on error.
 * Argument 'copy': if non-zero and the node refcount == 1, operation is performed in-place.
 * Otherwise a new view (or new allocation for resize) is returned.
 */
NR_PUBLIC Node* Node_Reshape(Node* node, nr_intp* new_shape, int new_ndim, int copy);
NR_PUBLIC Node* Node_Ravel(Node* node, int copy);
NR_PUBLIC Node* Node_Flatten(Node* node, int copy);
NR_PUBLIC Node* Node_MoveAxis(Node* node, int src_axis, int dst_axis, int copy);
NR_PUBLIC Node* Node_RollAxis(Node* node, int axis, int start, int copy);
NR_PUBLIC Node* Node_SwapAxes(Node* node, int axis1, int axis2, int copy);
NR_PUBLIC Node* Node_Transpose(Node* node, int copy);
NR_PUBLIC Node* Node_PermuteDims(Node* node, const int* order, int copy);
NR_PUBLIC Node* Node_MatrixTranspose(Node* node, int copy);
NR_PUBLIC Node* Node_ExpandDims(Node* node, int axis, int copy);
NR_PUBLIC Node* Node_Squeeze(Node* node, int copy);
NR_PUBLIC Node* Node_Resize(Node* node, nr_intp* new_shape, int new_ndim, int copy);

#endif