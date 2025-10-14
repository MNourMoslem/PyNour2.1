#include "tc_methods.h"
#include "./build/generated/__tc_methods_generated.c"

NR_PUBLIC Node*
Node_ToType(Node* dst, const Node* src, NR_DTYPE dtype){
    int idx = dtype * NR_NUM_NUMIRC_DT + src->dtype.dtype;
    Node2NodeFunc func = __NODE_TC_METHODS_ARRAY__[idx];
    return func(dst, src);
}