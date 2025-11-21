/*
    PyNour Function Interface
    ========================

    Defines the function interface for PyNour operations.
    Provides structures and types for handling function
    arguments and execution.

    Key Components:
    --------------
    1. Function argument structure (NFuncArgs)
    2. Function pointer type definition (NFuncFunc)
*/

#ifndef NOUR__CORE_INCLUDE__NOUR_NR_FUNC_H
#define NOUR__CORE_INCLUDE__NOUR_NR_FUNC_H

/* Forward declare Node to avoid circular include with nr_node.h */
struct Node;

/* Structure for passing arguments to PyNour functions */
typedef struct
{
    struct Node** in_nodes;      // Array of input nodes
    struct Node** out_nodes;     // Array of output nodes
    int nin;              // Number of input nodes
    int nout;             // Number of output nodes
    NR_DTYPE outtype;         // Desired output data type (not used and would be deleted)
    void* extra;              // Additional parameters

    int __ref_count;        // Internal reference count for memory management
                            // it is more efficient instead of makeing copies
                            // for each output node.
}NFuncArgs;

/* Function pointer type for PyNour operations */
typedef int (*NFuncFunc) (NFuncArgs*);

typedef int (*GradFuncFunc) (NFuncArgs*);

typedef struct
{
    GradFuncFunc grad_func_float32;
    GradFuncFunc grad_func_float64;
}GradFunc;


typedef struct
{
    const char* name;       // Name of the function
    int flags;              // Function flags

    int nin;                // Number of input nodes
    int nout;               // Number of output nodes

    int in_type;            // Input data type if set to NONE, any type is allowed
    int out_type;           // Output data type

    NR_DTYPE in_dtype;      // Only Allowed dtype when NFUNC_FLAG_ONE_DTYPE is set
    NR_DTYPE out_dtype;     // The output dtype of the resulting node. if set to -1, it will be resolved at runtime.

    NFuncFunc func;          // Pointer to the function implementation
    GradFunc* grad_func;      // Pointer to the gradient function (if applicable)    
} NFunc;

typedef struct
{
    const NFunc* nfunc;
    NFuncArgs* args;
    int out_idx;
} NFuncFuncInfo;

#define NFUNC_FLAG_GRADIENT 0x1             // Function supports gradient computation
#define NFUNC_FLAG_INPLACE 0x2              // Function supports in-place operation
#define NFUNC_FLAG_REDUCE 0x4               // Function is a reduction operation
#define NFUNC_FLAG_ELEMENTWISE 0x8          // Function is element-wise operation
#define NFUNC_FLAG_ONE_DTYPE 0x10           // Function requires all inputs to be of the same data type
#define NFUNC_FLAG_TYPE_BROADCASTABLE 0x20  // Function supports type broadcasting
#define NFUNC_FLAG_OUT_DTYPES_NOT_SAME 0x40 // If there are multiple output nodes, their dtypes may differ
#define NFUNC_FLAG_NO_USER_OUT_NODES 0x80   // Function does not allow user-provided output nodes

#endif