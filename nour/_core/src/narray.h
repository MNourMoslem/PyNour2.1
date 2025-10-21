#ifndef NOUR__CORE_SRC_NOUR_NARRAY_H
#define NOUR__CORE_SRC_NOUR_NARRAY_H

#include "nour/nour.h"

/*
    NArray - Lightweight Array Structure
    ====================================
    
    NArray is a lightweight array structure designed for use in indexing operations
    and other scenarios where a full Node structure would be too heavyweight.
    
    Key differences from Node:
    - No reference counting
    - No operation graph tracking
    - No gradient tracking
    - Simpler memory management
    - Suitable for temporary array-like objects (e.g., Python lists, tuples)
    
    Use cases:
    - Integer array indexing
    - Boolean array indexing
    - Converting Python lists/tuples to array form
    - Temporary array operations that don't need full Node features
*/

typedef struct NArray
{
    void* data;              // Pointer to array data
    int ndim;                // Number of dimensions
    nr_intp* shape;          // Array shape
    nr_intp* strides;        // Array strides
    NR_DTYPE dtype;          // Data type
    nr_intp size;            // Total number of elements
    int owns_data;           // Flag: does this NArray own its data?
    int owns_shape;          // Flag: does this NArray own its shape array?
    int owns_strides;        // Flag: does this NArray own its strides array?
} NArray;

/* NArray access macros */
#define NARRAY_DATA(arr)      (arr)->data
#define NARRAY_DTYPE(arr)     (arr)->dtype
#define NARRAY_SHAPE(arr)     (arr)->shape
#define NARRAY_NDIM(arr)      (arr)->ndim
#define NARRAY_STRIDES(arr)   (arr)->strides
#define NARRAY_SIZE(arr)      (arr)->size

/*
    Create a new NArray with specified data and properties.
    
    Parameters:
        data: Pointer to data buffer
        ndim: Number of dimensions
        shape: Array shape
        strides: Array strides (can be NULL for default C-order strides)
        dtype: Data type
        copy_data: If 1, copy the data; if 0, reference it
    
    Returns:
        Pointer to newly created NArray, or NULL on error
*/
NR_PUBLIC NArray*
NArray_New(void* data, int ndim, const nr_intp* shape, const nr_intp* strides, 
           NR_DTYPE dtype, int copy_data);

/*
    Create a new NArray from a Node.
    
    Parameters:
        node: Source Node to create NArray from
        copy_data: If 1, copy the data; if 0, reference it
    
    Returns:
        Pointer to newly created NArray, or NULL on error
*/
NR_PUBLIC NArray*
NArray_FromNode(const Node* node, int copy_data);

/*
    Create an NArray from a 1D array of integers (for indexing).
    
    Parameters:
        indices: Pointer to integer array
        size: Number of elements
        copy_data: If 1, copy the data; if 0, reference it
    
    Returns:
        Pointer to newly created NArray, or NULL on error
*/
NR_PUBLIC NArray*
NArray_FromIntArray(const nr_intp* indices, nr_intp size, int copy_data);

/*
    Create an NArray from a 1D boolean array (for boolean masking).
    
    Parameters:
        mask: Pointer to boolean array
        size: Number of elements
        copy_data: If 1, copy the data; if 0, reference it
    
    Returns:
        Pointer to newly created NArray, or NULL on error
*/
NR_PUBLIC NArray*
NArray_FromBoolArray(const nr_bool* mask, nr_intp size, int copy_data);

/*
    Create a new empty NArray with specified shape and dtype.
    
    Parameters:
        ndim: Number of dimensions
        shape: Array shape
        dtype: Data type
    
    Returns:
        Pointer to newly created NArray, or NULL on error
*/
NR_PUBLIC NArray*
NArray_NewEmpty(int ndim, const nr_intp* shape, NR_DTYPE dtype);

/*
    Free an NArray and its associated memory.
    
    Parameters:
        arr: Pointer to NArray to free
*/
NR_PUBLIC void
NArray_Free(NArray* arr);

/*
    Convert an NArray to a Node.
    
    Parameters:
        arr: Source NArray
        copy_data: If 1, copy the data; if 0, reference it
    
    Returns:
        Pointer to newly created Node, or NULL on error
*/
NR_PUBLIC Node*
NArray_ToNode(const NArray* arr, int copy_data);

/*
    Check if NArray is contiguous in memory (C-order).
    
    Parameters:
        arr: NArray to check
    
    Returns:
        1 if contiguous, 0 otherwise
*/
NR_PUBLIC int
NArray_IsContiguous(const NArray* arr);

/*
    Calculate default C-order strides for given shape.
    
    Parameters:
        ndim: Number of dimensions
        shape: Array shape
        itemsize: Size of each element in bytes
        strides: Output buffer for strides
*/
NR_PUBLIC void
NArray_CalcStrides(int ndim, const nr_intp* shape, nr_intp itemsize, nr_intp* strides);

/*
    Get a pointer to a specific element in the NArray.
    
    Parameters:
        arr: NArray
        indices: Array of indices for each dimension
    
    Returns:
        Pointer to the element, or NULL if indices are out of bounds
*/
NR_PUBLIC void*
NArray_GetItem(const NArray* arr, const nr_intp* indices);

/*
    Copy data from one NArray to another.
    
    Parameters:
        dst: Destination NArray
        src: Source NArray
    
    Returns:
        0 on success, -1 on error
*/
NR_PUBLIC int
NArray_Copy(NArray* dst, const NArray* src);

#endif // NOUR__CORE_SRC_NOUR_NARRAY_H
