#include "tc_methods.h"
#include "./build/generated/__tc_methods_generated.c"
#include "nerror.h"
#include "node_core.h"

/* Internal helper: validate single-value node */
NR_STATIC_INLINE int _node_single_value(const Node* node){
    if (!node){
        NError_RaiseError(NError_ValueError, "Node_As*: NULL node");
        return 0;
    }
    nr_intp nitems = NODE_IS_SCALAR(node) ? 1 : Node_NItems(node);
    if (nitems != 1){
        NError_RaiseError(NError_ValueError, "Node_As*: expected scalar or size-1 node (got %lld items)", (long long)nitems);
        return 0;
    }
    return 1;
}

#define IMPLEMENT_NODE_AS(FUNCNAME, RETTYPE) \
    NR_PUBLIC RETTYPE Node_As##FUNCNAME(const Node* node){ \
        if(!_node_single_value(node)) return (RETTYPE)0; \
        switch (NODE_DTYPE(node)) { \
            case NR_BOOL:    return (RETTYPE)(*(nr_bool*)   NODE_DATA(node)); \
            case NR_INT8:    return (RETTYPE)(*(nr_int8*)   NODE_DATA(node)); \
            case NR_UINT8:   return (RETTYPE)(*(nr_uint8*)  NODE_DATA(node)); \
            case NR_INT16:   return (RETTYPE)(*(nr_int16*)  NODE_DATA(node)); \
            case NR_UINT16:  return (RETTYPE)(*(nr_uint16*) NODE_DATA(node)); \
            case NR_INT32:   return (RETTYPE)(*(nr_int32*)  NODE_DATA(node)); \
            case NR_UINT32:  return (RETTYPE)(*(nr_uint32*) NODE_DATA(node)); \
            case NR_INT64:   return (RETTYPE)(*(nr_int64*)  NODE_DATA(node)); \
            case NR_UINT64:  return (RETTYPE)(*(nr_uint64*) NODE_DATA(node)); \
            case NR_FLOAT32: return (RETTYPE)(*(nr_float32*)NODE_DATA(node)); \
            case NR_FLOAT64: return (RETTYPE)(*(nr_float64*)NODE_DATA(node)); \
            default: \
                NError_RaiseError(NError_TypeError, "Node_As*: unsupported dtype %d", (int)NODE_DTYPE(node)); \
                return (RETTYPE)0; \
        } \
    }

/* Generate all variants */
IMPLEMENT_NODE_AS(Bool,   nr_bool)
IMPLEMENT_NODE_AS(Byte,   nr_int8)
IMPLEMENT_NODE_AS(UByte,  nr_uint8)
IMPLEMENT_NODE_AS(Short,  nr_int16)
IMPLEMENT_NODE_AS(UShort, nr_uint16)
IMPLEMENT_NODE_AS(Int,    nr_int32)
IMPLEMENT_NODE_AS(UInt,   nr_uint32)
IMPLEMENT_NODE_AS(Long,   nr_int64)
IMPLEMENT_NODE_AS(ULong,  nr_uint64)
IMPLEMENT_NODE_AS(Float,  nr_float32)
IMPLEMENT_NODE_AS(Double, nr_float64)

NR_PUBLIC Node*
Node_ToType(Node* dst, const Node* src, NR_DTYPE dtype){
    int idx = dtype * NR_NUM_NUMIRC_DT + src->dtype.dtype;
    Node2NodeFunc func = __NODE_TC_METHODS_ARRAY__[idx];
    return func(dst, src);
}