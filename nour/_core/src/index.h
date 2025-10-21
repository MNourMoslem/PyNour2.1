#ifndef NOUR__CORE_SRC_NOUR_INDEX_H
#define NOUR__CORE_SRC_NOUR_INDEX_H

#include "nour/nour.h"
#include "narray.h"

typedef struct {
    nr_intp start;
    nr_intp stop;
    nr_intp step;
} Slice;

/* Slice-based indexing functions */

NR_PUBLIC Node*
Node_Slice(Node* nout, const Node* node, const Slice slice, int dim);

NR_PUBLIC Node*
Node_MultiSlice(Node* nout, const Node* node, const Slice* slices, int num_slices);

NR_PUBLIC Node*
Node_BooleanMask(const Node* node, const Node* bool_mask);

/* NArray-based indexing functions */

/*
    Index a node using an integer array (fancy indexing).
    Similar to NumPy's arr[indices] where indices is an array of integers.
    
    Parameters:
        node: Source node to index
        indices: NArray containing integer indices
        axis: Axis along which to index (use -1 for flattened indexing)
    
    Returns:
        New node with indexed elements, or NULL on error
*/
NR_PUBLIC Node*
Node_IndexWithIntArray(const Node* node, const NArray* indices, int axis);

/*
    Index a node using a boolean array (boolean masking with NArray).
    Similar to NumPy's arr[mask] where mask is a boolean array.
    
    Parameters:
        node: Source node to index
        bool_mask: NArray containing boolean mask
    
    Returns:
        New 1D node with selected elements, or NULL on error
*/
NR_PUBLIC Node*
Node_IndexWithBooleanArray(const Node* node, const NArray* bool_mask);

/*
    Advanced indexing with multiple NArrays.
    Supports indexing along multiple axes with integer arrays.
    
    Parameters:
        node: Source node to index
        indices: Array of NArray pointers (one per axis to index)
        num_indices: Number of NArray indices
        axes: Array of axes to index (NULL means use first num_indices axes)
    
    Returns:
        New node with indexed elements, or NULL on error
*/
NR_PUBLIC Node*
Node_AdvancedIndex(const Node* node, const NArray** indices, int num_indices, const int* axes);

/*
    Take elements from a node along an axis using an integer array.
    Similar to NumPy's np.take(arr, indices, axis=axis).
    
    Parameters:
        node: Source node
        indices: NArray of integer indices
        axis: Axis along which to take elements
        mode: How to handle out-of-bounds indices:
              0 = error (default)
              1 = wrap (negative modulo)
              2 = clip (clip to valid range)
    
    Returns:
        New node with taken elements, or NULL on error
*/
NR_PUBLIC Node*
Node_Take(const Node* node, const NArray* indices, int axis, int mode);

/*
    Put values into a node at positions specified by an integer array.
    Similar to NumPy's np.put(arr, indices, values).
    
    Parameters:
        node: Destination node (modified in-place)
        indices: NArray of integer indices (flattened positions)
        values: Node containing values to put
        mode: How to handle out-of-bounds indices (0=error, 1=wrap, 2=clip)
    
    Returns:
        0 on success, -1 on error
*/
NR_PUBLIC int
Node_Put(Node* node, const NArray* indices, const Node* values, int mode);

#endif // NOUR__CORE_SRC_NOUR_INDEX_H