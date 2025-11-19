/*
    PyNour - High Performance Scientific Computing Library
    Copyright (c) 2023-present PyNour Contributors
    Licensed under MIT License

    This file contains the iterator interface for PyNour arrays. The iterator system
    is a crucial component that enables efficient traversal of n-dimensional arrays
    with different memory layouts and access patterns.

    Key Components:
    --------------
    1. NIter Structure:
       - Core iterator structure for traversing array data
       - Supports both contiguous and strided memory access
       - Maintains coordinates, strides and shape information
       - Optimized for both C and Fortran order arrays

    2. Iterator Modes:
       NITER_MODE_NONE (0): No iteration mode set
       NITER_MODE_CONTIGUOUS (1): For contiguous memory blocks
       NITER_MODE_STRIDED (2): For strided memory access

    3. Constants:
       NR_MULTIITER_MAX_NITER: Maximum number of simultaneous iterators (32)
       NR_NODE_MAX_NDIM: Maximum number of dimensions supported

    The iterator system provides:
    - Cache-friendly memory access patterns
    - Support for broadcasting
    - Efficient multi-array operations
    - Coordinate tracking during iteration
    - Flexible stride handling

    This implementation is inspired by NumPy's iterator system but optimized
    for PyNour's specific needs and performance requirements.

    Usage:
    ------
    The iterator is typically used internally by PyNour operations but can
    also be used directly for custom array operations requiring fine-grained
    control over array traversal.

    Related Files:
    -------------
    - nr_node.h: Array structure definitions
    - nr_types.h: Basic type definitions
    - nr_dtypes.h: Data type handling
*/


#ifndef NOUR__CORE_INCLUDE__NOUR_NR_ITER_H
#define NOUR__CORE_INCLUDE__NOUR_NR_ITER_H

#include "nr_error.h"
#include "nr_types.h"
#include "nr_dtypes.h"
#include "nr_node.h"

// maximum number of iterators
#define NR_MULTIITER_MAX_NITER 32

// iterator modes
#define NITER_MODE_NONE 0
#define NITER_MODE_CONTIGUOUS 1
#define NITER_MODE_STRIDED 2

/*
    Thanks to NumPy - this iterator system was inspired by their implementation
*/


/*
========================================
            NIter structure
========================================
*/


// NIter is the main iterator structure for iterating over arrays.
typedef struct NIter
{
    void* data;                                  // pointer to the data
    int nd_m1;                                   // number of dimensions - 1
    nr_intp shape_m1[NR_NODE_MAX_NDIM];          // shape - 1
    nr_intp strides[NR_NODE_MAX_NDIM];           // strides
    nr_intp backstrides[NR_NODE_MAX_NDIM];       // backstrides
    void* current;                               // current position
    nr_intp coords[NR_NODE_MAX_NDIM];            // coordinates
    int idx;                                     // current index
    int end;                                     // end index
    nr_intp step;                                // step size
    int iter_mode;                               // iterator mode
}NIter;

// Initialize the iterator
#define NIter_ITER(iter_ptr) do { \
        (iter_ptr)->idx = 0; \
        (iter_ptr)->current = (iter_ptr)->data; \
        if ((iter_ptr)->iter_mode == NITER_MODE_CONTIGUOUS){\
            break;\
        }\
        memset((iter_ptr)->coords, 0, \
               ((iter_ptr)->nd_m1 + 1)*sizeof(nr_intp)); \
} while (0)

// Next iterator for contiguous data
#define NIter_NEXT_CONTIGUOUS(iter_ptr) do { \
        (iter_ptr)->idx++;\
    (iter_ptr)->current = (void*)(((char*)(iter_ptr)->current) + (iter_ptr)->step);\
} while (0)

// Next iterator for strided data
#define NIter_NEXT_STRIDED(iter_ptr) do { \
    (iter_ptr)->idx++;\
    for (int i = (iter_ptr)->nd_m1; i > -1; i--){ \
        if ((iter_ptr)->coords[i] < (iter_ptr)->shape_m1[i]){\
            (iter_ptr)->coords[i]++;\
            (iter_ptr)->current = (void*)(((char*)(iter_ptr)->current) + (iter_ptr)->strides[i]);\
            break;\
        }\
        (iter_ptr)->coords[i] = 0;\
        (iter_ptr)->current = (void*)(((char*)(iter_ptr)->current) - (iter_ptr)->backstrides[i]);\
    }\
} while (0)

// Next iterator
#define NIter_NEXT(iter_ptr) do { \
        if ((iter_ptr)->iter_mode == NITER_MODE_CONTIGUOUS){\
            NIter_NEXT_CONTIGUOUS((iter_ptr));\
        }\
        else{\
            NIter_NEXT_STRIDED((iter_ptr));\
        }\
} while(0)

// Check if the iterator is done used with a while loop
#define NIter_NOTDONE(iter_ptr) ((iter_ptr)->idx < (iter_ptr)->end)

// Get the current item
#define NIter_ITEM(iter_ptr) ((iter_ptr)->current)

/*
========================================
            NMultiIter structure
========================================
*/

// NMultiIter is a structure for iterating over multiple arrays simultaneously
typedef struct
{
    NIter iters[NR_MULTIITER_MAX_NITER];            // array of iterators
    int n_iter;                                     // number of iterators    

    nr_intp out_shape[NR_NODE_MAX_NDIM];          // output shape
    int out_ndim;                                   // output number of dimensions

    int idx;                                        // current index
    int end;                                        // end index
    void* current[NR_NODE_MAX_NDIM];                // current position
}NMultiIter;

// Initialize the multi-iterator
#define NMultiIter_ITER(mit_ptr) do {\
    (mit_ptr)->idx = 0;\
    if ((mit_ptr)->n_iter == 2){\
        NIter_ITER((mit_ptr)->iters);\
        NIter_ITER((mit_ptr)->iters + 1);\
    }\
    for (int i = 0; i < (mit_ptr)->n_iter; i++){\
        NIter_ITER((mit_ptr)->iters + i);\
    }\
} while(0)

// Next iterator
#define NMultiIter_NEXT(mit_ptr) do {\
    (mit_ptr)->idx++;\
    if ((mit_ptr)->n_iter == 1){\
        NIter_NEXT((mit_ptr)->iters);\
        break;\
    }\
    else if ((mit_ptr)->n_iter == 2){\
        NIter_NEXT((mit_ptr)->iters);\
        NIter_NEXT((mit_ptr)->iters + 1);\
    }\
    else{\
        for (int i = 0; i < (mit_ptr)->n_iter; i++){\
            NIter_NEXT((mit_ptr)->iters + i);\
        }\
    }\
} while(0)

// Next iterator for 2 iterators
#define NMultiIter_NEXT2(mit_ptr) do {\
    (mit_ptr)->idx++;\
    NIter_NEXT((mit_ptr)->iters);\
    NIter_NEXT((mit_ptr)->iters + 1);\
} while(0)

// Next iterator for 3 iterators 
#define NMultiIter_NEXT3(mit_ptr) do {\
    (mit_ptr)->idx++;\
    NIter_NEXT((mit_ptr)->iters);\
    NIter_NEXT((mit_ptr)->iters + 1);\
    NIter_NEXT((mit_ptr)->iters + 2);\
} while(0)

// Next iterator for 4 iterators
#define NMultiIter_NEXT4(mit_ptr) do {\
    (mit_ptr)->idx++;\
    NIter_NEXT((mit_ptr)->iters);\
    NIter_NEXT((mit_ptr)->iters + 1);\
    NIter_NEXT((mit_ptr)->iters + 2);\
    NIter_NEXT((mit_ptr)->iters + 3);\
} while(0)

// Check if the multi-iterator is done used with a while loop
#define NMultiIter_NOTDONE(mit_ptr) ((mit_ptr)->idx < (mit_ptr)->end)

// Get the current item
#define NMultiIter_ITEM(mit_ptr, i) (NIter_ITEM((mit_ptr)->iters + i))

/*
========================================
            NWindowIter structure
========================================
*/

// NWindowIter is a structure for iterating over a window of an array
typedef struct
{
    void* data;                                  // pointer to the data 
    int nd_m1;                                   // number of dimensions - 1
    nr_intp shape_m1[NR_NODE_MAX_NDIM];        // shape - 1
    nr_intp strides[NR_NODE_MAX_NDIM];         // strides
    nr_intp backstrides[NR_NODE_MAX_NDIM];     // backstrides
    nr_intp coords[NR_NODE_MAX_NDIM];          // coordinates
    void* current;                               // current position
    int idx;                                     // current index
    int end;                                     // end index
    int iter_mode;                               // iterator mode
    nr_intp step;                              // step size
    nr_intp bounds[NR_NODE_MAX_NDIM];          // bounds

    int widx;                                     // current index
    int wend;                                     // end index
    nr_intp wstep;                              // step size
    void *wcurrent;                               // current position
    nr_intp wshape_m1[NR_NODE_MAX_NDIM];        // window shape - 1
    nr_intp wstrides[NR_NODE_MAX_NDIM];         // window strides
    nr_intp wbackstrides[NR_NODE_MAX_NDIM];     // window backstrides
    nr_intp wcoords[NR_NODE_MAX_NDIM];          // window coordinates
    int wmode;                                    // window mode
}NWindowIter;

// Initialize the window iterator
#define NWindowIter_ITER(witer_ptr) NIter_ITER(witer_ptr)

// Next iterator
#define NWindowIter_NEXT(witer_ptr) NIter_NEXT(witer_ptr)

// Check if the window iterator is done used with a while loop
#define NWindowIter_NOTDONE(witer_ptr) ((witer_ptr)->idx < (witer_ptr)->end)

// Get the current item
#define NWindowIter_ITEM(witer_ptr) ((witer_ptr)->current)

// Initialize the window iterator
#define NWindowIter_ITER_WINDOW(witer_ptr) do{\
    (witer_ptr)->widx = 0; \
    (witer_ptr)->wcurrent = (witer_ptr)->current; \
    if ((witer_ptr)->wmode == NITER_MODE_CONTIGUOUS){\
        break;\
    }\
    memset((witer_ptr)->wcoords, 0, \
            ((witer_ptr)->nd_m1 + 1)*sizeof(nr_intp)); \
} while (0)

// Next iterator for contiguous data
#define NWindowIter_NEXT_WINDOW_CONTIGUOUS(witer_ptr) do { \
    (witer_ptr)->wcurrent = (void*)(((char*)(witer_ptr)->wcurrent) + (witer_ptr)->wstep); \
} while (0)

// Next iterator for strided data
#define NWindowIter_NEXT_WINDOW_STRIDED(witer_ptr) do { \
    for (int i = (witer_ptr)->nd_m1; i > -1; i--){ \
        if ((witer_ptr)->wcoords[i] < (witer_ptr)->wshape_m1[i]){\
            (witer_ptr)->wcoords[i]++;\
            (witer_ptr)->wcurrent = (void*)(((char*)(witer_ptr)->wcurrent) + (witer_ptr)->wstrides[i]);\
            break;\
        }\
        (witer_ptr)->wcoords[i] = 0;\
        (witer_ptr)->wcurrent = (void*)(((char*)(witer_ptr)->wcurrent) - (witer_ptr)->wbackstrides[i]);\
    }\
} while (0)

// Next iterator
#define NWindowIter_NEXT_WINDOW(witer_ptr) do { \
        (witer_ptr)->widx++;\
        if ((witer_ptr)->wmode == NITER_MODE_CONTIGUOUS){\
            NWindowIter_NEXT_WINDOW_CONTIGUOUS((witer_ptr));\
        }\
        else{\
            NWindowIter_NEXT_WINDOW_STRIDED((witer_ptr));\
        }\
} while(0)

// Check if the window iterator is done used with a while loop
#define NWindowIter_NOTDONE_WINDOW(witer_ptr) ((witer_ptr)->widx < (witer_ptr)->wend)

// Get the current item
#define NWindowIter_ITEM_WINDOW(witer_ptr) ((witer_ptr)->wcurrent)

/*
========================================
            NCoordIter structure
========================================
*/

// NCoordIter is a structure for iterating over coordinates in nested loop fashion
// For shape (3, 2): (0,0), (0,1), (1,0), (1,1), (2,0), (2,1)
typedef struct
{
    int ndim;                                       // number of dimensions
    nr_intp shape[NR_NODE_MAX_NDIM];               // shape of the coordinate space
    nr_intp coords[NR_NODE_MAX_NDIM];              // current coordinates
    int idx;                                        // current flat index
    int end;                                        // total number of coordinates
} NCoordIter;

// Initialize the coordinate iterator
#define NCoordIter_ITER(citer_ptr) do { \
    (citer_ptr)->idx = 0; \
    memset((citer_ptr)->coords, 0, (citer_ptr)->ndim * sizeof(nr_intp)); \
} while (0)

// Next coordinate iterator
#define NCoordIter_NEXT(citer_ptr) do { \
    (citer_ptr)->idx++; \
    for (int i = (citer_ptr)->ndim - 1; i >= 0; i--) { \
        if ((citer_ptr)->coords[i] < (citer_ptr)->shape[i] - 1) { \
            (citer_ptr)->coords[i]++; \
            break; \
        } \
        (citer_ptr)->coords[i] = 0; \
    } \
} while (0)

// Check if the coordinate iterator is done
#define NCoordIter_NOTDONE(citer_ptr) ((citer_ptr)->idx < (citer_ptr)->end)

// Get the current coordinates
#define NCoordIter_COORDS(citer_ptr) ((citer_ptr)->coords)

// Get coordinate at specific dimension
#define NCoordIter_COORD(citer_ptr, dim) ((citer_ptr)->coords[dim])

#endif

