#include "free.h"
#include <stdlib.h>
#include <stdio.h>

NR_PUBLIC void
Node_Free(Node* node){
    if (node){
        node->ref_count--;
        if (node->ref_count > 0) return; // Still referenced elsewhere

        if (node->shape) {free(node->shape);}
        if (node->strides) {free(node->strides);}
        if (NODE_IS_OWNDATA(node) && node->base_data){
            free(node->base_data);
        }
        free(node);
    }
}