#ifndef NR__CORE__INCLUDE__NR_OP_H
#define NR__CORE__INCLUDE__NR_OP_H


/* Forward declare Node and NFunc to avoid circular dependencies with nr_node.h */
struct Node;
#include "nr_func.h"

typedef struct NodeOperation
{
    NFunc* func;          // Function pointer to the operation
    GradFunc* diff_func;     // Function pointer to the differentiation operation

    struct Node** input_nodes;  // Array of input nodes
    int n_input_nodes;          // Number of input nodes
    int dim;                    // Dimension parameter. required for certain operations
    void* extra;         // Extra parameters for the operation
} NodeOperation;

#define NODE_OP_FUNC(op_struct) ((op_struct).func)
#define NODE_OP_DIFF_FUNC(op_struct) ((op_struct).diff_func)
#define NODE_OP_INPUT_NODES(op_struct) ((op_struct).input_nodes)
#define NODE_OP_N_INPUT_NODES(op_struct) ((op_struct).n_input_nodes)
#define NODE_OP_DIM(op_struct) ((op_struct).dim)
#define NODE_OP_EXTRA(op_struct) ((op_struct).extra)
#define NODE_OP_EXTRA_AS_TYPE(op_struct, type) ((type)((op_struct).extra))

#endif // NR__CORE__INCLUDE__NR_OP_H