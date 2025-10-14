#include "free.h"
#include <stdlib.h>
#include <stdio.h>

NR_PUBLIC void
Node_Free(Node* node){
    if (node){
        if (node->shape) {free(node->shape);}
        if (node->strides) {free(node->strides);}
        if (NODE_IS_OWNDATA(node)){
            free(node->data);
        }
        free(node);
    }
}