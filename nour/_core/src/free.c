#include "free.h"
#include <stdlib.h>
#include <stdio.h>

NR_PUBLIC void
Node_Free(Node* node){
    if (!node) return;
    
    // Decrement reference count
    node->ref_count--;
    
    // If still referenced, don't free anything
    if (node->ref_count > 0) return;
    
    // Free shape and strides
    if (node->shape) {
        free(node->shape);
    }
    if (node->strides) {
        free(node->strides);
    }
    
    // Free data if this node owns it
    if (NODE_IS_OWNDATA(node) && node->data) {
        free(node->data);
    }
    
    // If this node references a base, decrement base's ref count
    // and potentially free it
    if (node->base) {
        Node_Free(node->base);  // Recursive call handles ref_count properly
    }
    
    // Finally free the node structure itself
    free(node);
}