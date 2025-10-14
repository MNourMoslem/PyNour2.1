#include "shape.h"
#include "node_core.h"
#include "ntools.h"
#include "nerror.h"

/**
 * Reshapes a node to new dimensions while preserving the total number of elements.
 * 
 * Parameters:
 *   node: The node to reshape
 *   new_shp: Array containing the new shape dimensions
 *   new_ndim: Number of dimensions in the new shape
 * 
 * The function will:
 *   - Validate the new dimensions
 *   - Check that total elements remain the same
 *   - Update shape and strides arrays
 *   - Handle memory reallocation if needed
 * 
 * Returns:
 *   0 on success, -1 on error
 */
NR_PUBLIC int
Node_Reshape(Node* node, nr_long* new_shp, int new_ndim) {
    if (new_ndim > NR_NODE_MAX_NDIM) {
        NError_RaiseError(
            NError_ValueError,
            "maximum number of dimensions is %d, got %d for %s",
            NR_NODE_MAX_NDIM, new_ndim, node->name
        );
        return -1;
    }

    if (NODE_IS_SCALAR(node)) {
        NError_RaiseError(
            NError_ValueError,
            "cannot reshape a scalar %s",
            node->name
        );
        return -1;
    }

    nr_long old_nitems = Node_NItems(node);
    nr_long new_nitems = NR_NItems(new_ndim, new_shp);
    
    if (new_nitems != old_nitems) {
        NError_RaiseError(
            NError_ValueError,
            "cannot reshape %s of size %llu into shape of size %llu",
            node->name, old_nitems, new_nitems
        );
        return -1;
    }

    if (node->ndim <= new_ndim) {
        memcpy(node->shape, new_shp, new_ndim * sizeof(nr_long));
        NTools_CalculateStrides(new_ndim, new_shp, node->dtype.size, node->strides);
    } else {
        nr_long* new_shp_block = malloc(sizeof(nr_long) * new_ndim);
        if (!new_shp_block) {
            NError_RaiseMemoryError();
            return -1;
        }
        memcpy(new_shp_block, new_shp, new_ndim * sizeof(nr_long));

        nr_long* new_strides = malloc(sizeof(nr_long) * new_ndim);
        if (!new_strides) {
            free(new_shp_block);
            NError_RaiseMemoryError();
            return -1;
        }
        NTools_CalculateStrides(new_ndim, new_shp, node->dtype.size, new_strides);
        
        free(node->shape);
        free(node->strides);
        node->shape = new_shp_block;
        node->strides = new_strides;
    }

    node->ndim = new_ndim;
    return 0;
}

/**
 * Removes all dimensions of size 1 from the node's shape.
 * For example: shape (2, 1, 3, 1, 4) becomes (2, 3, 4)
 * 
 * Parameters:
 *   node: The node to squeeze
 * 
 * The function will:
 *   - Remove all dimensions of size 1
 *   - Convert to scalar if all dimensions are 1
 *   - Reallocate memory for shape and strides if needed
 *   - Recalculate strides for the new shape
 * 
 * Returns:
 *   0 on success, -1 on error
 */
NR_PUBLIC int
Node_Squeeze(Node* node) {
    if (NODE_IS_SCALAR(node)) {
        NError_RaiseError(
            NError_ValueError,
            "cannot squeeze a scalar %s",
            node->name
        );
        return -1;
    }

    // Count new dimensions and create temporary arrays
    int new_ndim = 0;
    nr_long new_shape[NR_NODE_MAX_NDIM];
    
    for (int i = 0; i < node->ndim; i++) {
        if (node->shape[i] != 1) {
            new_shape[new_ndim++] = node->shape[i];
        }
    }

    // If all dimensions were 1, result should be a scalar
    if (new_ndim == 0) {
        free(node->shape);
        free(node->strides);
        node->shape = NULL;
        node->strides = NULL;
        node->ndim = 0;
        return 0;
    }

    // Allocate new arrays if needed
    if (new_ndim != node->ndim) {
        memcpy(node->shape, new_shape, new_ndim * sizeof(nr_long));
        NTools_CalculateStrides(new_ndim, node->shape, node->dtype.size, node->strides);
    }

    node->ndim = new_ndim;
    return 0;
}

/**
 * Reverses the order of dimensions in the node.
 * For example: shape (2, 3, 4) becomes (4, 3, 2)
 * 
 * Parameters:
 *   node: The node to transpose
 * 
 * The function will:
 *   - Reverse the order of dimensions
 *   - Update both shape and strides arrays
 *   - Handle 0-D and 1-D cases appropriately
 *   - No memory reallocation needed
 * 
 * Returns:
 *   0 on success, -1 on error
 */
NR_PUBLIC int
Node_Transpose(Node* node) {
    if (NODE_IS_SCALAR(node)) {
        NError_RaiseError(
            NError_ValueError,
            "cannot transpose a scalar %s",
            node->name
        );
        return -1;
    }

    if (node->ndim <= 1) {
        return 0;  // No change needed for 0-D or 1-D
    }

    nr_long temp;
    // Reverse shape and strides
    for (int i = 0; i < node->ndim / 2; i++) {
        int j = node->ndim - 1 - i;
        
        // Swap shapes
        temp = node->shape[i];
        node->shape[i] = node->shape[j];
        node->shape[j] = temp;

        // Swap strides
        temp = node->strides[i];
        node->strides[i] = node->strides[j];
        node->strides[j] = temp;
    }

    return 0;
}

/**
 * Swaps two axes in the node's shape.
 * For example: shape (2, 3, 4) with axes (0,2) becomes (4, 3, 2)
 * 
 * Parameters:
 *   node: The node to modify
 *   axis1: First axis to swap
 *   axis2: Second axis to swap
 * 
 * The function will:
 *   - Validate axis indices
 *   - Swap dimensions at the specified axes
 *   - Update both shape and strides arrays
 *   - No memory reallocation needed
 * 
 * Returns:
 *   0 on success, -1 on error
 */
NR_PUBLIC int
Node_SwapAxes(Node* node, int axis1, int axis2) {
    if (!node->shape) {
        NError_RaiseError(
            NError_ValueError,
            "cannot swap axes of a scalar %s",
            node->name
        );
        return -1;
    }

    if (axis1 == axis2) {
        return 0;  // No swap needed
    }

    // Validate axes
    if (axis1 < 0 || axis1 >= node->ndim || axis2 < 0 || axis2 >= node->ndim) {
        NError_RaiseError(
            NError_ValueError,
            "invalid axes (%d, %d) for %s with %d dimensions",
            axis1, axis2, node->name, node->ndim
        );
        return -1;
    }

    nr_long temp;
    // Swap shapes
    temp = node->shape[axis1];
    node->shape[axis1] = node->shape[axis2];
    node->shape[axis2] = temp;

    // Swap strides
    temp = node->strides[axis1];
    node->strides[axis1] = node->strides[axis2];
    node->strides[axis2] = temp;

    return 0;
}

/**
 * Swaps multiple pairs of axes in the node's shape.
 * Allows performing multiple axis swaps in a single operation.
 * 
 * Parameters:
 *   node: The node to modify
 *   axes1: Array of first axes for each swap
 *   axes2: Array of second axes for each swap
 *   n_swaps: Number of axis pairs to swap
 * 
 * The function will:
 *   - Validate all axis pairs before performing any swaps
 *   - Perform all swaps sequentially
 *   - Skip same-axis swaps
 *   - Update both shape and strides arrays
 *   - No memory reallocation needed
 * 
 * Returns:
 *   0 on success, -1 on error
 */
NR_PUBLIC int
Node_SwapAxesMulti(Node* node, const int* axes1, const int* axes2, int n_swaps) {
    if (NODE_IS_SCALAR(node)) {
        NError_RaiseError(
            NError_ValueError,
            "cannot swap axes of a scalar %s",
            node->name
        );
        return -1;
    }

    if (n_swaps <= 0) {
        return 0;  // No swaps requested
    }

    // Validate all axes pairs before performing any swaps
    for (int i = 0; i < n_swaps; i++) {
        int axis1 = axes1[i];
        int axis2 = axes2[i];

        if (axis1 < 0 || axis1 >= node->ndim || axis2 < 0 || axis2 >= node->ndim) {
            NError_RaiseError(
                NError_ValueError,
                "invalid axes pair (%d, %d) at index %d for %s with %d dimensions",
                axis1, axis2, i, node->name, node->ndim
            );
            return -1;
        }
    }

    nr_long temp;
    // Perform all swaps
    for (int i = 0; i < n_swaps; i++) {
        int axis1 = axes1[i];
        int axis2 = axes2[i];

        if (axis1 == axis2) {
            continue;  // Skip same-axis swaps
        }

        // Swap shapes
        temp = node->shape[axis1];
        node->shape[axis1] = node->shape[axis2];
        node->shape[axis2] = temp;

        // Swap strides
        temp = node->strides[axis1];
        node->strides[axis1] = node->strides[axis2];
        node->strides[axis2] = temp;
    }

    return 0;
}

/**
 * Performs a matrix transpose operation on a 2D node.
 * For example: shape (M, N) becomes (N, M)
 * 
 * Parameters:
 *   node: The node to transpose (must be 2-dimensional)
 * 
 * The function will:
 *   - Verify the node is 2-dimensional
 *   - Swap dimensions and strides for the 2D matrix
 *   - No memory reallocation needed
 * 
 * Returns:
 *   0 on success, -1 on error
 */
NR_PUBLIC int
Node_MatrixTranspose(Node* node) {
    if (NODE_IS_SCALAR(node)) {
        NError_RaiseError(
            NError_ValueError,
            "cannot transpose a scalar %s",
            node->name
        );
        return -1;
    }

    if (node->ndim != 2) {
        NError_RaiseError(
            NError_ValueError,
            "matrix transpose requires 2 dimensions, got %d for %s",
            node->ndim, node->name
        );
        return -1;
    }

    nr_long temp;
    // Swap shapes
    temp = node->shape[0];
    node->shape[0] = node->shape[1];
    node->shape[1] = temp;

    // Swap strides
    temp = node->strides[0];
    node->strides[0] = node->strides[1];
    node->strides[1] = temp;

    return 0;
}