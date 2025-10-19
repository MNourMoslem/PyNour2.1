#ifndef NOUR__CORE__SRC__OPERATIONS_H
#define NOUR__CORE__SRC__OPERATIONS_H

#include "nour/nr_node.h"
#include "nour/nr_func.h"
#include "nour/nr_op.h"
#include "free.h"
#include <stdlib.h>

NR_PUBLIC NodeOperation*
NodeOperation_New(NFunc* func, GradFunc* diff_func, struct Node** input_nodes, int n_input_nodes, int dim, void* extra, size_t extra_size);

NR_PUBLIC void 
NodeOperation_Free(NodeOperation* op);

NR_PUBLIC int
NodeOperation_GenerateForNode(Node* target_node, NFunc* func, GradFunc* diff_func, struct Node** input_nodes, int n_input_nodes, int dim, void* extra, size_t extra_size);

NR_PUBLIC int
NodeOperation_GenerateForNodeIfTrackable(Node* target_node, NFunc* func, GradFunc* diff_func, struct Node** input_nodes, int n_input_nodes, int dim, void* extra, size_t extra_size);


#endif // NOUR__CORE__SRC__OPERATIONS_H