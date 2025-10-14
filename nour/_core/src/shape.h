#ifndef NOUR__CORE_SRC_NOUR_shape_H
#define NOUR__CORE_SRC_NOUR_shape_H

#include "nour/nour.h"

/**
 * Reshapes a node to new dimensions while preserving total elements.
 * Returns 0 on success, -1 on error.
 */
NR_PUBLIC int
Node_Reshape(Node* node, nr_long* new_shp, int new_ndim);

/**
 * Removes all dimensions of size 1 from the node's shape.
 * Returns 0 on success, -1 on error.
 */
NR_PUBLIC int
Node_Squeeze(Node* node);

/**
 * Reverses the order of all dimensions in the node.
 * Returns 0 on success, -1 on error.
 */
NR_PUBLIC int
Node_Transpose(Node* node);

/**
 * Swaps two specified axes in the node's shape.
 * Returns 0 on success, -1 on error.
 */
NR_PUBLIC int
Node_SwapAxes(Node* node, int axis1, int axis2);

/**
 * Swaps multiple pairs of axes in the node's shape.
 * Returns 0 on success, -1 on error.
 */
NR_PUBLIC int
Node_SwapAxesMulti(Node* node, const int* axes1, const int* axes2, int n_swaps);

/**
 * Performs matrix transpose on a 2D node.
 * Returns 0 on success, -1 on error.
 */
NR_PUBLIC int
Node_MatrixTranspose(Node* node);

#endif