#include "operations.h"
#include "nour/nour.h"
#include "nerror.h"

NR_PUBLIC NodeOperation*
NodeOperation_New(NFunc* func, GradFunc* diff_func, struct Node** input_nodes, int n_input_nodes, int dim, void* extra, size_t extra_size) {
    NodeOperation* op = (NodeOperation*)malloc(sizeof(NodeOperation));
    if (!op) {
        NError_RaiseMemoryError();
        return NULL;
    }

    op->func = func;
    op->diff_func = diff_func;
    op->dim = dim;

    op->input_nodes = (Node**)malloc(sizeof(Node*) * n_input_nodes);
    if (!op->input_nodes) {
        free(op);
        NError_RaiseMemoryError();
        return NULL;
    }

    for (int i = 0; i < n_input_nodes; i++) {
        op->input_nodes[i] = input_nodes[i];
        op->input_nodes[i]->ref_count++;  // Increment ref count for each input node
    }

    op->n_input_nodes = n_input_nodes;

    // it is important to own the intilizing of extra data
    // it is the responsibility of the caller to free any allocated extra data
    if (extra && extra_size != 0) {
        if (extra_size < 0){
            // use the pointer directly
            op->extra = extra;
        }
        else{
            op->extra = malloc(extra_size);
            if (!op->extra) {
                free(op->input_nodes);
                free(op);
                NError_RaiseMemoryError();
                return NULL;
            }
            memcpy(op->extra, extra, extra_size);
        }
    } else {
        op->extra = NULL;
    }

    return op;
}

NR_PUBLIC void
NodeOperation_Free(NodeOperation* op) {
    if (!op) return;

    if (op->input_nodes) {
        for (int i = 0; i < op->n_input_nodes; i++) {
            Node_Free(op->input_nodes[i]);  // Decrement ref count for each input node
        }
        free(op->input_nodes);
    }

    if (op->extra) {
        free(op->extra);
    }

    // Free the operation structure itself
    free(op);
}

NR_PUBLIC int
NodeOperation_GenerateForNode(Node* target_node, NFunc* func, GradFunc* diff_func, struct Node** input_nodes, int n_input_nodes, int dim, void* extra, size_t extra_size) {
    if (!target_node) {
        NError_RaiseError(NError_ValueError, "Target node cannot be NULL.");
        return -1;
    }

    NodeOperation* op = NodeOperation_New(func, diff_func, input_nodes, n_input_nodes, dim, extra, extra_size);
    if (!op) {
        return -1;  // Error already raised in NodeOperation_New
    }

    // Free existing operation if any
    if (target_node->op) {
        NodeOperation_Free(target_node->op);
    }

    target_node->op = op;
    return 0;
}


NR_PUBLIC int
NodeOperation_GenerateForNodeIfTrackable(Node* target_node, NFunc* func, GradFunc* diff_func, struct Node** input_nodes, int n_input_nodes, int dim, void* extra, size_t extra_size) {
    if (!target_node) {
        NError_RaiseError(NError_ValueError, "Target node cannot be NULL.");
        return -1;
    }

    if (NODE_IS_TRACK(target_node)) {
        // Target node is trackable; generate operation
        return NodeOperation_GenerateForNode(target_node, func, diff_func, input_nodes, n_input_nodes, dim, extra, extra_size);
    }

    return 0;  // No operation generated, but not an error
}