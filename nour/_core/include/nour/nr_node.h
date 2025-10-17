#ifndef NR__CORE__INCLUDE__NR_NODE_H
#define NR__CORE__INCLUDE__NR_NODE_H

#include "nr_types.h"
#include "nr_dtypes.h"
/* Forward declare dependent types to avoid circular includes */
struct NodeOperation;
#include <string.h>

/* Array memory layout types */
typedef enum
{
    NR_CONTIGUOUS_TYPE = 0,  // Contiguous memory layout
    NR_STRIDED_TYPE,         // Strided memory layout
    NR_SCALAR_TYPE,          // Scalar value type
}nr_narray_type;

extern char* NR_NODE_NAME;

/* Node flags for various array properties */
#define NR_NODE_C_ORDER 0x1      // C-style memory order
#define NR_NODE_F_ORDER 0x2      // Fortran-style memory order
#define NR_NODE_CONTIGUOUS 0x4   // Contiguous memory layout
#define NR_NODE_STRIDED 0x8      // Strided memory layout
#define NR_NODE_SCALAR 0x10      // Scalar value
#define NR_NODE_WRITABLE 0x20    // Writable array
#define NR_NODE_SORTED 0x40      // Sorted array
#define NR_NODE_OWNDATA 0x80     // Owns its data
#define NR_NODE_TRACK 0x100      // Memory tracking enabled


/* Core array node structure */
typedef struct Node
{
    void* data;              // Pointer to array data

    int ndim;                // Number of dimensions
    nr_intp* shape;          // Array shape
    nr_intp* strides;        // Array strides
    
    NDtype dtype;            // Data type information

    struct Node* base;       // Base array reference (pointer to parent Node)
    int ref_count;           // Reference count for memory management

    int flags;               // Array flags

    // Node name for identification.
    // This is useful for inhertance and debugging.
    const char* name;        
    
    /* Optional attached operation (computation graph). Forward-declared to avoid circular include. */
    struct NodeOperation* op;
    struct Node* grad;  // Gradient node reference
} Node;

/* Node access macros */
#define NODE_DATA(node) node->data
#define NODE_DTYPE(node) node->dtype.dtype
#define NODE_ITEMSIZE(node) node->dtype.size
#define NODE_SHAPE(node) node->shape
#define NODE_NDIM(node) node->ndim
#define NODE_STRIDES(node) node->strides

/* Node property check macros */
#define NODE_IS_C_ORDER(node) NR_CHKFLG(node->flags, NR_NODE_C_ORDER)
#define NODE_IS_F_ORDER(node) NR_CHKFLG(node->flags, NR_NODE_F_ORDER)
#define NODE_IS_CONTIGUOUS(node) NR_CHKFLG(node->flags, NR_NODE_CONTIGUOUS)
#define NODE_IS_STRIDED(node) NR_CHKFLG(node->flags, NR_NODE_STRIDED)
// #define NODE_IS_SCALAR(node) NR_CHKFLG(node->flags, NR_NODE_SCALAR)
#define NODE_IS_WRITABLE(node) NR_CHKFLG(node->flags, NR_NODE_WRITABLE)
#define NODE_IS_SORTED(node) NR_CHKFLG(node->flags, NR_NODE_SORTED)
#define NODE_IS_OWNDATA(node) NR_CHKFLG(node->flags, NR_NODE_OWNDATA)
#define NODE_IS_TRACK(node) NR_CHKFLG(node->flags, NR_NODE_TRACK)

#define NODE_IS_SCALAR(node) (node->ndim == 0)

/*
    Calculates total number of items in array.
    
    Parameters:
        node: Target array node
    
    Returns:
        Total number of items
*/
NR_STATIC_INLINE nr_intp
Node_NItems(const Node* node){
    nr_intp nitems = 1;
    for (int i = 0; i < node->ndim; i++){
        nitems *= node->shape[i];
    }
    return nitems;
}

/*
    Checks if two nodes have identical shapes.
    
    Parameters:
        a: First node
        b: Second node
    
    Returns:
        1 if shapes match, 0 otherwise
*/
NR_STATIC_INLINE int
Node_SameShape(const Node* a, const Node* b){
    return a->ndim == b->ndim 
            && memcmp(a->shape, b->shape, sizeof(nr_intp) * a->ndim) == 0;
}

/* Function type for node-to-node operations */
typedef Node* (*Node2NodeFunc) (Node* , const Node*);

#endif // NR__CORE__INCLUDE__NR_NODE_H