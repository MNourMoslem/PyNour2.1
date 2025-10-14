#include "nour/nour.h"
#include "../../niter.h"
#include "../../node_core.h"
#include "../../nerror.h"

/**
 * Raises an error if the source and destination nodes do not have the same shape.
 *
 * Parameters:
 *   - dst: The destination node.
 *   - src: The source node.
 *
 * Error Raised:
 *   - ValueError: Includes the shapes of both nodes in the error message.
 */
NR_STATIC void
_TC_RAISE_MISS_SHAPE(Node* dst, const Node* src){
    char dshp[30];
    char sshp[30];

    NTools_ShapeAsString(dst->shape, dst->ndim, dshp);
    NTools_ShapeAsString(src->shape, src->ndim, sshp);

    NError_RaiseError(
        NError_ValueError,
        "destenation node has to have the same shape as src node. got %s and %s",
        dshp, sshp
    );
}

/**
 * Raises an error if the source and destination nodes have different data types.
 *
 * Parameters:
 *   - dt: The destination node data type.
 *   - st: The source node data type.
 *
 * Error Raised:
 *   - ValueError: Includes the data types of both nodes in the error message.
 */
NR_STATIC void
_TC_RAISE_MISS_DTYPE(NR_DTYPE dt, NR_DTYPE st){
    char dtstr[30];
    char ststr[30];

    NDtype_AsStringOnlyType(dt, dtstr);
    NDtype_AsStringOnlyType(st, ststr);
    
    NError_RaiseError(
        NError_ValueError,
        "destenation and src nodes must have the same dtype. got %s and %s",
        dtstr, ststr
    );
}


/*
==================================================
         Type Convert Methods For NR_BOOL         
==================================================
*/
/**
 * Converts data from a source node with type NR_INT8 to a
 * destination node with type NR_BOOL and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_BOOL. If NULL, a new node is created.
 *   - src: The source node with type NR_INT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT8` to `NR_BOOL` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT8_to_NR_BOOL(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_BOOL);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_BOOL){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_BOOL);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_bool*)dst->data + i) = (nr_bool)*((nr_int8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)dst->data + i) = (nr_bool)*((nr_int8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)NIter_ITEM(&it)) = (nr_bool)*((nr_int8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_bool*)NIter_ITEM(&dit)) = (nr_bool)*((nr_int8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT8 to a
 * destination node with type NR_BOOL and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_BOOL. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT8` to `NR_BOOL` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT8_to_NR_BOOL(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_BOOL);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_BOOL){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_BOOL);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_bool*)dst->data + i) = (nr_bool)*((nr_uint8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)dst->data + i) = (nr_bool)*((nr_uint8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)NIter_ITEM(&it)) = (nr_bool)*((nr_uint8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_bool*)NIter_ITEM(&dit)) = (nr_bool)*((nr_uint8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT16 to a
 * destination node with type NR_BOOL and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_BOOL. If NULL, a new node is created.
 *   - src: The source node with type NR_INT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT16` to `NR_BOOL` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT16_to_NR_BOOL(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_BOOL);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_BOOL){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_BOOL);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_bool*)dst->data + i) = (nr_bool)*((nr_int16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)dst->data + i) = (nr_bool)*((nr_int16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)NIter_ITEM(&it)) = (nr_bool)*((nr_int16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_bool*)NIter_ITEM(&dit)) = (nr_bool)*((nr_int16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT16 to a
 * destination node with type NR_BOOL and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_BOOL. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT16` to `NR_BOOL` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT16_to_NR_BOOL(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_BOOL);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_BOOL){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_BOOL);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_bool*)dst->data + i) = (nr_bool)*((nr_uint16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)dst->data + i) = (nr_bool)*((nr_uint16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)NIter_ITEM(&it)) = (nr_bool)*((nr_uint16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_bool*)NIter_ITEM(&dit)) = (nr_bool)*((nr_uint16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT32 to a
 * destination node with type NR_BOOL and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_BOOL. If NULL, a new node is created.
 *   - src: The source node with type NR_INT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT32` to `NR_BOOL` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT32_to_NR_BOOL(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_BOOL);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_BOOL){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_BOOL);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_bool*)dst->data + i) = (nr_bool)*((nr_int32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)dst->data + i) = (nr_bool)*((nr_int32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)NIter_ITEM(&it)) = (nr_bool)*((nr_int32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_bool*)NIter_ITEM(&dit)) = (nr_bool)*((nr_int32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT32 to a
 * destination node with type NR_BOOL and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_BOOL. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT32` to `NR_BOOL` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT32_to_NR_BOOL(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_BOOL);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_BOOL){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_BOOL);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_bool*)dst->data + i) = (nr_bool)*((nr_uint32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)dst->data + i) = (nr_bool)*((nr_uint32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)NIter_ITEM(&it)) = (nr_bool)*((nr_uint32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_bool*)NIter_ITEM(&dit)) = (nr_bool)*((nr_uint32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT64 to a
 * destination node with type NR_BOOL and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_BOOL. If NULL, a new node is created.
 *   - src: The source node with type NR_INT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT64` to `NR_BOOL` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT64_to_NR_BOOL(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_BOOL);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_BOOL){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_BOOL);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_bool*)dst->data + i) = (nr_bool)*((nr_int64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)dst->data + i) = (nr_bool)*((nr_int64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)NIter_ITEM(&it)) = (nr_bool)*((nr_int64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_bool*)NIter_ITEM(&dit)) = (nr_bool)*((nr_int64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT64 to a
 * destination node with type NR_BOOL and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_BOOL. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT64` to `NR_BOOL` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT64_to_NR_BOOL(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_BOOL);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_BOOL){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_BOOL);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_bool*)dst->data + i) = (nr_bool)*((nr_uint64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)dst->data + i) = (nr_bool)*((nr_uint64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)NIter_ITEM(&it)) = (nr_bool)*((nr_uint64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_bool*)NIter_ITEM(&dit)) = (nr_bool)*((nr_uint64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT32 to a
 * destination node with type NR_BOOL and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_BOOL. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT32` to `NR_BOOL` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT32_to_NR_BOOL(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_BOOL);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_BOOL){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_BOOL);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_bool*)dst->data + i) = (nr_bool)*((nr_float32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)dst->data + i) = (nr_bool)*((nr_float32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)NIter_ITEM(&it)) = (nr_bool)*((nr_float32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_bool*)NIter_ITEM(&dit)) = (nr_bool)*((nr_float32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT64 to a
 * destination node with type NR_BOOL and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_BOOL. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT64` to `NR_BOOL` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT64_to_NR_BOOL(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_BOOL);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_BOOL){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_BOOL);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_bool*)dst->data + i) = (nr_bool)*((nr_float64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)dst->data + i) = (nr_bool)*((nr_float64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_bool*)NIter_ITEM(&it)) = (nr_bool)*((nr_float64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_bool*)NIter_ITEM(&dit)) = (nr_bool)*((nr_float64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/*
==================================================
         Type Convert Methods For NR_INT8         
==================================================
*/
/**
 * Converts data from a source node with type NR_BOOL to a
 * destination node with type NR_INT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT8. If NULL, a new node is created.
 *   - src: The source node with type NR_BOOL containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_BOOL` to `NR_INT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_BOOL_to_NR_INT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int8*)dst->data + i) = (nr_int8)*((nr_bool*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)dst->data + i) = (nr_int8)*((nr_bool*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)NIter_ITEM(&it)) = (nr_int8)*((nr_bool*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int8*)NIter_ITEM(&dit)) = (nr_int8)*((nr_bool*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT8 to a
 * destination node with type NR_INT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT8. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT8` to `NR_INT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT8_to_NR_INT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int8*)dst->data + i) = (nr_int8)*((nr_uint8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)dst->data + i) = (nr_int8)*((nr_uint8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)NIter_ITEM(&it)) = (nr_int8)*((nr_uint8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int8*)NIter_ITEM(&dit)) = (nr_int8)*((nr_uint8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT16 to a
 * destination node with type NR_INT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT8. If NULL, a new node is created.
 *   - src: The source node with type NR_INT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT16` to `NR_INT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT16_to_NR_INT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int8*)dst->data + i) = (nr_int8)*((nr_int16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)dst->data + i) = (nr_int8)*((nr_int16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)NIter_ITEM(&it)) = (nr_int8)*((nr_int16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int8*)NIter_ITEM(&dit)) = (nr_int8)*((nr_int16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT16 to a
 * destination node with type NR_INT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT8. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT16` to `NR_INT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT16_to_NR_INT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int8*)dst->data + i) = (nr_int8)*((nr_uint16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)dst->data + i) = (nr_int8)*((nr_uint16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)NIter_ITEM(&it)) = (nr_int8)*((nr_uint16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int8*)NIter_ITEM(&dit)) = (nr_int8)*((nr_uint16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT32 to a
 * destination node with type NR_INT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT8. If NULL, a new node is created.
 *   - src: The source node with type NR_INT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT32` to `NR_INT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT32_to_NR_INT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int8*)dst->data + i) = (nr_int8)*((nr_int32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)dst->data + i) = (nr_int8)*((nr_int32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)NIter_ITEM(&it)) = (nr_int8)*((nr_int32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int8*)NIter_ITEM(&dit)) = (nr_int8)*((nr_int32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT32 to a
 * destination node with type NR_INT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT8. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT32` to `NR_INT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT32_to_NR_INT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int8*)dst->data + i) = (nr_int8)*((nr_uint32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)dst->data + i) = (nr_int8)*((nr_uint32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)NIter_ITEM(&it)) = (nr_int8)*((nr_uint32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int8*)NIter_ITEM(&dit)) = (nr_int8)*((nr_uint32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT64 to a
 * destination node with type NR_INT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT8. If NULL, a new node is created.
 *   - src: The source node with type NR_INT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT64` to `NR_INT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT64_to_NR_INT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int8*)dst->data + i) = (nr_int8)*((nr_int64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)dst->data + i) = (nr_int8)*((nr_int64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)NIter_ITEM(&it)) = (nr_int8)*((nr_int64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int8*)NIter_ITEM(&dit)) = (nr_int8)*((nr_int64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT64 to a
 * destination node with type NR_INT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT8. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT64` to `NR_INT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT64_to_NR_INT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int8*)dst->data + i) = (nr_int8)*((nr_uint64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)dst->data + i) = (nr_int8)*((nr_uint64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)NIter_ITEM(&it)) = (nr_int8)*((nr_uint64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int8*)NIter_ITEM(&dit)) = (nr_int8)*((nr_uint64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT32 to a
 * destination node with type NR_INT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT8. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT32` to `NR_INT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT32_to_NR_INT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int8*)dst->data + i) = (nr_int8)*((nr_float32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)dst->data + i) = (nr_int8)*((nr_float32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)NIter_ITEM(&it)) = (nr_int8)*((nr_float32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int8*)NIter_ITEM(&dit)) = (nr_int8)*((nr_float32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT64 to a
 * destination node with type NR_INT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT8. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT64` to `NR_INT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT64_to_NR_INT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int8*)dst->data + i) = (nr_int8)*((nr_float64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)dst->data + i) = (nr_int8)*((nr_float64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int8*)NIter_ITEM(&it)) = (nr_int8)*((nr_float64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int8*)NIter_ITEM(&dit)) = (nr_int8)*((nr_float64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/*
==================================================
        Type Convert Methods For NR_UINT8         
==================================================
*/
/**
 * Converts data from a source node with type NR_BOOL to a
 * destination node with type NR_UINT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT8. If NULL, a new node is created.
 *   - src: The source node with type NR_BOOL containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_BOOL` to `NR_UINT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_BOOL_to_NR_UINT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_bool*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_bool*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)NIter_ITEM(&it)) = (nr_uint8)*((nr_bool*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint8*)NIter_ITEM(&dit)) = (nr_uint8)*((nr_bool*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT8 to a
 * destination node with type NR_UINT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT8. If NULL, a new node is created.
 *   - src: The source node with type NR_INT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT8` to `NR_UINT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT8_to_NR_UINT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_int8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_int8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)NIter_ITEM(&it)) = (nr_uint8)*((nr_int8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint8*)NIter_ITEM(&dit)) = (nr_uint8)*((nr_int8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT16 to a
 * destination node with type NR_UINT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT8. If NULL, a new node is created.
 *   - src: The source node with type NR_INT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT16` to `NR_UINT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT16_to_NR_UINT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_int16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_int16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)NIter_ITEM(&it)) = (nr_uint8)*((nr_int16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint8*)NIter_ITEM(&dit)) = (nr_uint8)*((nr_int16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT16 to a
 * destination node with type NR_UINT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT8. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT16` to `NR_UINT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT16_to_NR_UINT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_uint16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_uint16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)NIter_ITEM(&it)) = (nr_uint8)*((nr_uint16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint8*)NIter_ITEM(&dit)) = (nr_uint8)*((nr_uint16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT32 to a
 * destination node with type NR_UINT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT8. If NULL, a new node is created.
 *   - src: The source node with type NR_INT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT32` to `NR_UINT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT32_to_NR_UINT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_int32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_int32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)NIter_ITEM(&it)) = (nr_uint8)*((nr_int32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint8*)NIter_ITEM(&dit)) = (nr_uint8)*((nr_int32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT32 to a
 * destination node with type NR_UINT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT8. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT32` to `NR_UINT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT32_to_NR_UINT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_uint32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_uint32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)NIter_ITEM(&it)) = (nr_uint8)*((nr_uint32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint8*)NIter_ITEM(&dit)) = (nr_uint8)*((nr_uint32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT64 to a
 * destination node with type NR_UINT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT8. If NULL, a new node is created.
 *   - src: The source node with type NR_INT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT64` to `NR_UINT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT64_to_NR_UINT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_int64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_int64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)NIter_ITEM(&it)) = (nr_uint8)*((nr_int64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint8*)NIter_ITEM(&dit)) = (nr_uint8)*((nr_int64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT64 to a
 * destination node with type NR_UINT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT8. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT64` to `NR_UINT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT64_to_NR_UINT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_uint64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_uint64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)NIter_ITEM(&it)) = (nr_uint8)*((nr_uint64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint8*)NIter_ITEM(&dit)) = (nr_uint8)*((nr_uint64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT32 to a
 * destination node with type NR_UINT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT8. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT32` to `NR_UINT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT32_to_NR_UINT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_float32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_float32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)NIter_ITEM(&it)) = (nr_uint8)*((nr_float32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint8*)NIter_ITEM(&dit)) = (nr_uint8)*((nr_float32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT64 to a
 * destination node with type NR_UINT8 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT8. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT64` to `NR_UINT8` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT64_to_NR_UINT8(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT8);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT8){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT8);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_float64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)dst->data + i) = (nr_uint8)*((nr_float64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint8*)NIter_ITEM(&it)) = (nr_uint8)*((nr_float64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint8*)NIter_ITEM(&dit)) = (nr_uint8)*((nr_float64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/*
==================================================
        Type Convert Methods For NR_INT16         
==================================================
*/
/**
 * Converts data from a source node with type NR_BOOL to a
 * destination node with type NR_INT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT16. If NULL, a new node is created.
 *   - src: The source node with type NR_BOOL containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_BOOL` to `NR_INT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_BOOL_to_NR_INT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int16*)dst->data + i) = (nr_int16)*((nr_bool*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)dst->data + i) = (nr_int16)*((nr_bool*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)NIter_ITEM(&it)) = (nr_int16)*((nr_bool*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int16*)NIter_ITEM(&dit)) = (nr_int16)*((nr_bool*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT8 to a
 * destination node with type NR_INT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT16. If NULL, a new node is created.
 *   - src: The source node with type NR_INT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT8` to `NR_INT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT8_to_NR_INT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int16*)dst->data + i) = (nr_int16)*((nr_int8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)dst->data + i) = (nr_int16)*((nr_int8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)NIter_ITEM(&it)) = (nr_int16)*((nr_int8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int16*)NIter_ITEM(&dit)) = (nr_int16)*((nr_int8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT8 to a
 * destination node with type NR_INT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT16. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT8` to `NR_INT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT8_to_NR_INT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int16*)dst->data + i) = (nr_int16)*((nr_uint8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)dst->data + i) = (nr_int16)*((nr_uint8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)NIter_ITEM(&it)) = (nr_int16)*((nr_uint8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int16*)NIter_ITEM(&dit)) = (nr_int16)*((nr_uint8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT16 to a
 * destination node with type NR_INT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT16. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT16` to `NR_INT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT16_to_NR_INT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int16*)dst->data + i) = (nr_int16)*((nr_uint16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)dst->data + i) = (nr_int16)*((nr_uint16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)NIter_ITEM(&it)) = (nr_int16)*((nr_uint16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int16*)NIter_ITEM(&dit)) = (nr_int16)*((nr_uint16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT32 to a
 * destination node with type NR_INT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT16. If NULL, a new node is created.
 *   - src: The source node with type NR_INT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT32` to `NR_INT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT32_to_NR_INT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int16*)dst->data + i) = (nr_int16)*((nr_int32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)dst->data + i) = (nr_int16)*((nr_int32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)NIter_ITEM(&it)) = (nr_int16)*((nr_int32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int16*)NIter_ITEM(&dit)) = (nr_int16)*((nr_int32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT32 to a
 * destination node with type NR_INT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT16. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT32` to `NR_INT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT32_to_NR_INT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int16*)dst->data + i) = (nr_int16)*((nr_uint32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)dst->data + i) = (nr_int16)*((nr_uint32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)NIter_ITEM(&it)) = (nr_int16)*((nr_uint32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int16*)NIter_ITEM(&dit)) = (nr_int16)*((nr_uint32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT64 to a
 * destination node with type NR_INT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT16. If NULL, a new node is created.
 *   - src: The source node with type NR_INT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT64` to `NR_INT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT64_to_NR_INT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int16*)dst->data + i) = (nr_int16)*((nr_int64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)dst->data + i) = (nr_int16)*((nr_int64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)NIter_ITEM(&it)) = (nr_int16)*((nr_int64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int16*)NIter_ITEM(&dit)) = (nr_int16)*((nr_int64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT64 to a
 * destination node with type NR_INT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT16. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT64` to `NR_INT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT64_to_NR_INT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int16*)dst->data + i) = (nr_int16)*((nr_uint64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)dst->data + i) = (nr_int16)*((nr_uint64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)NIter_ITEM(&it)) = (nr_int16)*((nr_uint64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int16*)NIter_ITEM(&dit)) = (nr_int16)*((nr_uint64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT32 to a
 * destination node with type NR_INT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT16. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT32` to `NR_INT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT32_to_NR_INT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int16*)dst->data + i) = (nr_int16)*((nr_float32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)dst->data + i) = (nr_int16)*((nr_float32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)NIter_ITEM(&it)) = (nr_int16)*((nr_float32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int16*)NIter_ITEM(&dit)) = (nr_int16)*((nr_float32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT64 to a
 * destination node with type NR_INT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT16. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT64` to `NR_INT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT64_to_NR_INT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int16*)dst->data + i) = (nr_int16)*((nr_float64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)dst->data + i) = (nr_int16)*((nr_float64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int16*)NIter_ITEM(&it)) = (nr_int16)*((nr_float64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int16*)NIter_ITEM(&dit)) = (nr_int16)*((nr_float64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/*
==================================================
        Type Convert Methods For NR_UINT16        
==================================================
*/
/**
 * Converts data from a source node with type NR_BOOL to a
 * destination node with type NR_UINT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT16. If NULL, a new node is created.
 *   - src: The source node with type NR_BOOL containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_BOOL` to `NR_UINT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_BOOL_to_NR_UINT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_bool*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_bool*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)NIter_ITEM(&it)) = (nr_uint16)*((nr_bool*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint16*)NIter_ITEM(&dit)) = (nr_uint16)*((nr_bool*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT8 to a
 * destination node with type NR_UINT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT16. If NULL, a new node is created.
 *   - src: The source node with type NR_INT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT8` to `NR_UINT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT8_to_NR_UINT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_int8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_int8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)NIter_ITEM(&it)) = (nr_uint16)*((nr_int8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint16*)NIter_ITEM(&dit)) = (nr_uint16)*((nr_int8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT8 to a
 * destination node with type NR_UINT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT16. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT8` to `NR_UINT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT8_to_NR_UINT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_uint8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_uint8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)NIter_ITEM(&it)) = (nr_uint16)*((nr_uint8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint16*)NIter_ITEM(&dit)) = (nr_uint16)*((nr_uint8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT16 to a
 * destination node with type NR_UINT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT16. If NULL, a new node is created.
 *   - src: The source node with type NR_INT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT16` to `NR_UINT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT16_to_NR_UINT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_int16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_int16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)NIter_ITEM(&it)) = (nr_uint16)*((nr_int16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint16*)NIter_ITEM(&dit)) = (nr_uint16)*((nr_int16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT32 to a
 * destination node with type NR_UINT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT16. If NULL, a new node is created.
 *   - src: The source node with type NR_INT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT32` to `NR_UINT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT32_to_NR_UINT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_int32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_int32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)NIter_ITEM(&it)) = (nr_uint16)*((nr_int32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint16*)NIter_ITEM(&dit)) = (nr_uint16)*((nr_int32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT32 to a
 * destination node with type NR_UINT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT16. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT32` to `NR_UINT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT32_to_NR_UINT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_uint32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_uint32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)NIter_ITEM(&it)) = (nr_uint16)*((nr_uint32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint16*)NIter_ITEM(&dit)) = (nr_uint16)*((nr_uint32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT64 to a
 * destination node with type NR_UINT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT16. If NULL, a new node is created.
 *   - src: The source node with type NR_INT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT64` to `NR_UINT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT64_to_NR_UINT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_int64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_int64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)NIter_ITEM(&it)) = (nr_uint16)*((nr_int64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint16*)NIter_ITEM(&dit)) = (nr_uint16)*((nr_int64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT64 to a
 * destination node with type NR_UINT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT16. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT64` to `NR_UINT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT64_to_NR_UINT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_uint64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_uint64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)NIter_ITEM(&it)) = (nr_uint16)*((nr_uint64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint16*)NIter_ITEM(&dit)) = (nr_uint16)*((nr_uint64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT32 to a
 * destination node with type NR_UINT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT16. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT32` to `NR_UINT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT32_to_NR_UINT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_float32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_float32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)NIter_ITEM(&it)) = (nr_uint16)*((nr_float32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint16*)NIter_ITEM(&dit)) = (nr_uint16)*((nr_float32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT64 to a
 * destination node with type NR_UINT16 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT16. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT64` to `NR_UINT16` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT64_to_NR_UINT16(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT16);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT16){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT16);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_float64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)dst->data + i) = (nr_uint16)*((nr_float64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint16*)NIter_ITEM(&it)) = (nr_uint16)*((nr_float64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint16*)NIter_ITEM(&dit)) = (nr_uint16)*((nr_float64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/*
==================================================
        Type Convert Methods For NR_INT32         
==================================================
*/
/**
 * Converts data from a source node with type NR_BOOL to a
 * destination node with type NR_INT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT32. If NULL, a new node is created.
 *   - src: The source node with type NR_BOOL containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_BOOL` to `NR_INT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_BOOL_to_NR_INT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int32*)dst->data + i) = (nr_int32)*((nr_bool*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)dst->data + i) = (nr_int32)*((nr_bool*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)NIter_ITEM(&it)) = (nr_int32)*((nr_bool*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int32*)NIter_ITEM(&dit)) = (nr_int32)*((nr_bool*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT8 to a
 * destination node with type NR_INT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT32. If NULL, a new node is created.
 *   - src: The source node with type NR_INT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT8` to `NR_INT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT8_to_NR_INT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int32*)dst->data + i) = (nr_int32)*((nr_int8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)dst->data + i) = (nr_int32)*((nr_int8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)NIter_ITEM(&it)) = (nr_int32)*((nr_int8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int32*)NIter_ITEM(&dit)) = (nr_int32)*((nr_int8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT8 to a
 * destination node with type NR_INT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT32. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT8` to `NR_INT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT8_to_NR_INT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int32*)dst->data + i) = (nr_int32)*((nr_uint8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)dst->data + i) = (nr_int32)*((nr_uint8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)NIter_ITEM(&it)) = (nr_int32)*((nr_uint8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int32*)NIter_ITEM(&dit)) = (nr_int32)*((nr_uint8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT16 to a
 * destination node with type NR_INT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT32. If NULL, a new node is created.
 *   - src: The source node with type NR_INT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT16` to `NR_INT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT16_to_NR_INT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int32*)dst->data + i) = (nr_int32)*((nr_int16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)dst->data + i) = (nr_int32)*((nr_int16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)NIter_ITEM(&it)) = (nr_int32)*((nr_int16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int32*)NIter_ITEM(&dit)) = (nr_int32)*((nr_int16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT16 to a
 * destination node with type NR_INT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT32. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT16` to `NR_INT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT16_to_NR_INT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int32*)dst->data + i) = (nr_int32)*((nr_uint16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)dst->data + i) = (nr_int32)*((nr_uint16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)NIter_ITEM(&it)) = (nr_int32)*((nr_uint16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int32*)NIter_ITEM(&dit)) = (nr_int32)*((nr_uint16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT32 to a
 * destination node with type NR_INT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT32. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT32` to `NR_INT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT32_to_NR_INT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int32*)dst->data + i) = (nr_int32)*((nr_uint32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)dst->data + i) = (nr_int32)*((nr_uint32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)NIter_ITEM(&it)) = (nr_int32)*((nr_uint32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int32*)NIter_ITEM(&dit)) = (nr_int32)*((nr_uint32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT64 to a
 * destination node with type NR_INT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT32. If NULL, a new node is created.
 *   - src: The source node with type NR_INT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT64` to `NR_INT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT64_to_NR_INT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int32*)dst->data + i) = (nr_int32)*((nr_int64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)dst->data + i) = (nr_int32)*((nr_int64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)NIter_ITEM(&it)) = (nr_int32)*((nr_int64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int32*)NIter_ITEM(&dit)) = (nr_int32)*((nr_int64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT64 to a
 * destination node with type NR_INT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT32. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT64` to `NR_INT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT64_to_NR_INT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int32*)dst->data + i) = (nr_int32)*((nr_uint64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)dst->data + i) = (nr_int32)*((nr_uint64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)NIter_ITEM(&it)) = (nr_int32)*((nr_uint64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int32*)NIter_ITEM(&dit)) = (nr_int32)*((nr_uint64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT32 to a
 * destination node with type NR_INT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT32. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT32` to `NR_INT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT32_to_NR_INT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int32*)dst->data + i) = (nr_int32)*((nr_float32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)dst->data + i) = (nr_int32)*((nr_float32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)NIter_ITEM(&it)) = (nr_int32)*((nr_float32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int32*)NIter_ITEM(&dit)) = (nr_int32)*((nr_float32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT64 to a
 * destination node with type NR_INT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT32. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT64` to `NR_INT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT64_to_NR_INT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int32*)dst->data + i) = (nr_int32)*((nr_float64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)dst->data + i) = (nr_int32)*((nr_float64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int32*)NIter_ITEM(&it)) = (nr_int32)*((nr_float64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int32*)NIter_ITEM(&dit)) = (nr_int32)*((nr_float64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/*
==================================================
        Type Convert Methods For NR_UINT32        
==================================================
*/
/**
 * Converts data from a source node with type NR_BOOL to a
 * destination node with type NR_UINT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT32. If NULL, a new node is created.
 *   - src: The source node with type NR_BOOL containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_BOOL` to `NR_UINT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_BOOL_to_NR_UINT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_bool*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_bool*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)NIter_ITEM(&it)) = (nr_uint32)*((nr_bool*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint32*)NIter_ITEM(&dit)) = (nr_uint32)*((nr_bool*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT8 to a
 * destination node with type NR_UINT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT32. If NULL, a new node is created.
 *   - src: The source node with type NR_INT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT8` to `NR_UINT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT8_to_NR_UINT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_int8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_int8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)NIter_ITEM(&it)) = (nr_uint32)*((nr_int8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint32*)NIter_ITEM(&dit)) = (nr_uint32)*((nr_int8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT8 to a
 * destination node with type NR_UINT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT32. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT8` to `NR_UINT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT8_to_NR_UINT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_uint8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_uint8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)NIter_ITEM(&it)) = (nr_uint32)*((nr_uint8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint32*)NIter_ITEM(&dit)) = (nr_uint32)*((nr_uint8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT16 to a
 * destination node with type NR_UINT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT32. If NULL, a new node is created.
 *   - src: The source node with type NR_INT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT16` to `NR_UINT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT16_to_NR_UINT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_int16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_int16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)NIter_ITEM(&it)) = (nr_uint32)*((nr_int16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint32*)NIter_ITEM(&dit)) = (nr_uint32)*((nr_int16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT16 to a
 * destination node with type NR_UINT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT32. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT16` to `NR_UINT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT16_to_NR_UINT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_uint16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_uint16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)NIter_ITEM(&it)) = (nr_uint32)*((nr_uint16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint32*)NIter_ITEM(&dit)) = (nr_uint32)*((nr_uint16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT32 to a
 * destination node with type NR_UINT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT32. If NULL, a new node is created.
 *   - src: The source node with type NR_INT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT32` to `NR_UINT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT32_to_NR_UINT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_int32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_int32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)NIter_ITEM(&it)) = (nr_uint32)*((nr_int32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint32*)NIter_ITEM(&dit)) = (nr_uint32)*((nr_int32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT64 to a
 * destination node with type NR_UINT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT32. If NULL, a new node is created.
 *   - src: The source node with type NR_INT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT64` to `NR_UINT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT64_to_NR_UINT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_int64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_int64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)NIter_ITEM(&it)) = (nr_uint32)*((nr_int64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint32*)NIter_ITEM(&dit)) = (nr_uint32)*((nr_int64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT64 to a
 * destination node with type NR_UINT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT32. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT64` to `NR_UINT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT64_to_NR_UINT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_uint64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_uint64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)NIter_ITEM(&it)) = (nr_uint32)*((nr_uint64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint32*)NIter_ITEM(&dit)) = (nr_uint32)*((nr_uint64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT32 to a
 * destination node with type NR_UINT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT32. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT32` to `NR_UINT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT32_to_NR_UINT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_float32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_float32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)NIter_ITEM(&it)) = (nr_uint32)*((nr_float32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint32*)NIter_ITEM(&dit)) = (nr_uint32)*((nr_float32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT64 to a
 * destination node with type NR_UINT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT32. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT64` to `NR_UINT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT64_to_NR_UINT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_float64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)dst->data + i) = (nr_uint32)*((nr_float64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint32*)NIter_ITEM(&it)) = (nr_uint32)*((nr_float64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint32*)NIter_ITEM(&dit)) = (nr_uint32)*((nr_float64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/*
==================================================
        Type Convert Methods For NR_INT64         
==================================================
*/
/**
 * Converts data from a source node with type NR_BOOL to a
 * destination node with type NR_INT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT64. If NULL, a new node is created.
 *   - src: The source node with type NR_BOOL containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_BOOL` to `NR_INT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_BOOL_to_NR_INT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int64*)dst->data + i) = (nr_int64)*((nr_bool*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)dst->data + i) = (nr_int64)*((nr_bool*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)NIter_ITEM(&it)) = (nr_int64)*((nr_bool*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int64*)NIter_ITEM(&dit)) = (nr_int64)*((nr_bool*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT8 to a
 * destination node with type NR_INT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT64. If NULL, a new node is created.
 *   - src: The source node with type NR_INT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT8` to `NR_INT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT8_to_NR_INT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int64*)dst->data + i) = (nr_int64)*((nr_int8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)dst->data + i) = (nr_int64)*((nr_int8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)NIter_ITEM(&it)) = (nr_int64)*((nr_int8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int64*)NIter_ITEM(&dit)) = (nr_int64)*((nr_int8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT8 to a
 * destination node with type NR_INT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT64. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT8` to `NR_INT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT8_to_NR_INT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int64*)dst->data + i) = (nr_int64)*((nr_uint8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)dst->data + i) = (nr_int64)*((nr_uint8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)NIter_ITEM(&it)) = (nr_int64)*((nr_uint8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int64*)NIter_ITEM(&dit)) = (nr_int64)*((nr_uint8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT16 to a
 * destination node with type NR_INT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT64. If NULL, a new node is created.
 *   - src: The source node with type NR_INT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT16` to `NR_INT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT16_to_NR_INT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int64*)dst->data + i) = (nr_int64)*((nr_int16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)dst->data + i) = (nr_int64)*((nr_int16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)NIter_ITEM(&it)) = (nr_int64)*((nr_int16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int64*)NIter_ITEM(&dit)) = (nr_int64)*((nr_int16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT16 to a
 * destination node with type NR_INT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT64. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT16` to `NR_INT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT16_to_NR_INT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int64*)dst->data + i) = (nr_int64)*((nr_uint16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)dst->data + i) = (nr_int64)*((nr_uint16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)NIter_ITEM(&it)) = (nr_int64)*((nr_uint16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int64*)NIter_ITEM(&dit)) = (nr_int64)*((nr_uint16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT32 to a
 * destination node with type NR_INT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT64. If NULL, a new node is created.
 *   - src: The source node with type NR_INT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT32` to `NR_INT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT32_to_NR_INT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int64*)dst->data + i) = (nr_int64)*((nr_int32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)dst->data + i) = (nr_int64)*((nr_int32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)NIter_ITEM(&it)) = (nr_int64)*((nr_int32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int64*)NIter_ITEM(&dit)) = (nr_int64)*((nr_int32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT32 to a
 * destination node with type NR_INT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT64. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT32` to `NR_INT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT32_to_NR_INT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int64*)dst->data + i) = (nr_int64)*((nr_uint32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)dst->data + i) = (nr_int64)*((nr_uint32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)NIter_ITEM(&it)) = (nr_int64)*((nr_uint32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int64*)NIter_ITEM(&dit)) = (nr_int64)*((nr_uint32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT64 to a
 * destination node with type NR_INT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT64. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT64` to `NR_INT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT64_to_NR_INT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int64*)dst->data + i) = (nr_int64)*((nr_uint64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)dst->data + i) = (nr_int64)*((nr_uint64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)NIter_ITEM(&it)) = (nr_int64)*((nr_uint64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int64*)NIter_ITEM(&dit)) = (nr_int64)*((nr_uint64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT32 to a
 * destination node with type NR_INT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT64. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT32` to `NR_INT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT32_to_NR_INT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int64*)dst->data + i) = (nr_int64)*((nr_float32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)dst->data + i) = (nr_int64)*((nr_float32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)NIter_ITEM(&it)) = (nr_int64)*((nr_float32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int64*)NIter_ITEM(&dit)) = (nr_int64)*((nr_float32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT64 to a
 * destination node with type NR_INT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_INT64. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT64` to `NR_INT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT64_to_NR_INT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_INT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_INT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_INT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_int64*)dst->data + i) = (nr_int64)*((nr_float64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)dst->data + i) = (nr_int64)*((nr_float64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_int64*)NIter_ITEM(&it)) = (nr_int64)*((nr_float64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_int64*)NIter_ITEM(&dit)) = (nr_int64)*((nr_float64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/*
==================================================
        Type Convert Methods For NR_UINT64        
==================================================
*/
/**
 * Converts data from a source node with type NR_BOOL to a
 * destination node with type NR_UINT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT64. If NULL, a new node is created.
 *   - src: The source node with type NR_BOOL containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_BOOL` to `NR_UINT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_BOOL_to_NR_UINT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_bool*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_bool*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)NIter_ITEM(&it)) = (nr_uint64)*((nr_bool*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint64*)NIter_ITEM(&dit)) = (nr_uint64)*((nr_bool*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT8 to a
 * destination node with type NR_UINT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT64. If NULL, a new node is created.
 *   - src: The source node with type NR_INT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT8` to `NR_UINT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT8_to_NR_UINT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_int8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_int8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)NIter_ITEM(&it)) = (nr_uint64)*((nr_int8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint64*)NIter_ITEM(&dit)) = (nr_uint64)*((nr_int8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT8 to a
 * destination node with type NR_UINT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT64. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT8` to `NR_UINT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT8_to_NR_UINT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_uint8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_uint8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)NIter_ITEM(&it)) = (nr_uint64)*((nr_uint8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint64*)NIter_ITEM(&dit)) = (nr_uint64)*((nr_uint8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT16 to a
 * destination node with type NR_UINT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT64. If NULL, a new node is created.
 *   - src: The source node with type NR_INT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT16` to `NR_UINT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT16_to_NR_UINT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_int16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_int16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)NIter_ITEM(&it)) = (nr_uint64)*((nr_int16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint64*)NIter_ITEM(&dit)) = (nr_uint64)*((nr_int16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT16 to a
 * destination node with type NR_UINT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT64. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT16` to `NR_UINT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT16_to_NR_UINT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_uint16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_uint16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)NIter_ITEM(&it)) = (nr_uint64)*((nr_uint16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint64*)NIter_ITEM(&dit)) = (nr_uint64)*((nr_uint16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT32 to a
 * destination node with type NR_UINT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT64. If NULL, a new node is created.
 *   - src: The source node with type NR_INT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT32` to `NR_UINT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT32_to_NR_UINT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_int32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_int32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)NIter_ITEM(&it)) = (nr_uint64)*((nr_int32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint64*)NIter_ITEM(&dit)) = (nr_uint64)*((nr_int32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT32 to a
 * destination node with type NR_UINT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT64. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT32` to `NR_UINT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT32_to_NR_UINT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_uint32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_uint32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)NIter_ITEM(&it)) = (nr_uint64)*((nr_uint32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint64*)NIter_ITEM(&dit)) = (nr_uint64)*((nr_uint32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT64 to a
 * destination node with type NR_UINT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT64. If NULL, a new node is created.
 *   - src: The source node with type NR_INT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT64` to `NR_UINT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT64_to_NR_UINT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_int64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_int64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)NIter_ITEM(&it)) = (nr_uint64)*((nr_int64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint64*)NIter_ITEM(&dit)) = (nr_uint64)*((nr_int64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT32 to a
 * destination node with type NR_UINT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT64. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT32` to `NR_UINT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT32_to_NR_UINT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_float32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_float32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)NIter_ITEM(&it)) = (nr_uint64)*((nr_float32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint64*)NIter_ITEM(&dit)) = (nr_uint64)*((nr_float32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT64 to a
 * destination node with type NR_UINT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_UINT64. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT64` to `NR_UINT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT64_to_NR_UINT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_UINT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_UINT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_UINT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_float64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)dst->data + i) = (nr_uint64)*((nr_float64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_uint64*)NIter_ITEM(&it)) = (nr_uint64)*((nr_float64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_uint64*)NIter_ITEM(&dit)) = (nr_uint64)*((nr_float64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/*
==================================================
       Type Convert Methods For NR_FLOAT32        
==================================================
*/
/**
 * Converts data from a source node with type NR_BOOL to a
 * destination node with type NR_FLOAT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT32. If NULL, a new node is created.
 *   - src: The source node with type NR_BOOL containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_BOOL` to `NR_FLOAT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_BOOL_to_NR_FLOAT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float32*)dst->data + i) = (nr_float32)*((nr_bool*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)dst->data + i) = (nr_float32)*((nr_bool*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)NIter_ITEM(&it)) = (nr_float32)*((nr_bool*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float32*)NIter_ITEM(&dit)) = (nr_float32)*((nr_bool*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT8 to a
 * destination node with type NR_FLOAT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT32. If NULL, a new node is created.
 *   - src: The source node with type NR_INT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT8` to `NR_FLOAT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT8_to_NR_FLOAT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float32*)dst->data + i) = (nr_float32)*((nr_int8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)dst->data + i) = (nr_float32)*((nr_int8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)NIter_ITEM(&it)) = (nr_float32)*((nr_int8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float32*)NIter_ITEM(&dit)) = (nr_float32)*((nr_int8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT8 to a
 * destination node with type NR_FLOAT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT32. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT8` to `NR_FLOAT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT8_to_NR_FLOAT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float32*)dst->data + i) = (nr_float32)*((nr_uint8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)dst->data + i) = (nr_float32)*((nr_uint8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)NIter_ITEM(&it)) = (nr_float32)*((nr_uint8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float32*)NIter_ITEM(&dit)) = (nr_float32)*((nr_uint8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT16 to a
 * destination node with type NR_FLOAT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT32. If NULL, a new node is created.
 *   - src: The source node with type NR_INT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT16` to `NR_FLOAT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT16_to_NR_FLOAT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float32*)dst->data + i) = (nr_float32)*((nr_int16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)dst->data + i) = (nr_float32)*((nr_int16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)NIter_ITEM(&it)) = (nr_float32)*((nr_int16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float32*)NIter_ITEM(&dit)) = (nr_float32)*((nr_int16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT16 to a
 * destination node with type NR_FLOAT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT32. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT16` to `NR_FLOAT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT16_to_NR_FLOAT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float32*)dst->data + i) = (nr_float32)*((nr_uint16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)dst->data + i) = (nr_float32)*((nr_uint16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)NIter_ITEM(&it)) = (nr_float32)*((nr_uint16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float32*)NIter_ITEM(&dit)) = (nr_float32)*((nr_uint16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT32 to a
 * destination node with type NR_FLOAT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT32. If NULL, a new node is created.
 *   - src: The source node with type NR_INT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT32` to `NR_FLOAT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT32_to_NR_FLOAT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float32*)dst->data + i) = (nr_float32)*((nr_int32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)dst->data + i) = (nr_float32)*((nr_int32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)NIter_ITEM(&it)) = (nr_float32)*((nr_int32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float32*)NIter_ITEM(&dit)) = (nr_float32)*((nr_int32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT32 to a
 * destination node with type NR_FLOAT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT32. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT32` to `NR_FLOAT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT32_to_NR_FLOAT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float32*)dst->data + i) = (nr_float32)*((nr_uint32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)dst->data + i) = (nr_float32)*((nr_uint32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)NIter_ITEM(&it)) = (nr_float32)*((nr_uint32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float32*)NIter_ITEM(&dit)) = (nr_float32)*((nr_uint32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT64 to a
 * destination node with type NR_FLOAT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT32. If NULL, a new node is created.
 *   - src: The source node with type NR_INT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT64` to `NR_FLOAT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT64_to_NR_FLOAT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float32*)dst->data + i) = (nr_float32)*((nr_int64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)dst->data + i) = (nr_float32)*((nr_int64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)NIter_ITEM(&it)) = (nr_float32)*((nr_int64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float32*)NIter_ITEM(&dit)) = (nr_float32)*((nr_int64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT64 to a
 * destination node with type NR_FLOAT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT32. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT64` to `NR_FLOAT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT64_to_NR_FLOAT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float32*)dst->data + i) = (nr_float32)*((nr_uint64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)dst->data + i) = (nr_float32)*((nr_uint64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)NIter_ITEM(&it)) = (nr_float32)*((nr_uint64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float32*)NIter_ITEM(&dit)) = (nr_float32)*((nr_uint64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT64 to a
 * destination node with type NR_FLOAT32 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT32. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT64` to `NR_FLOAT32` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT64_to_NR_FLOAT32(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT32);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT32){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT32);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float32*)dst->data + i) = (nr_float32)*((nr_float64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)dst->data + i) = (nr_float32)*((nr_float64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float32*)NIter_ITEM(&it)) = (nr_float32)*((nr_float64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float32*)NIter_ITEM(&dit)) = (nr_float32)*((nr_float64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/*
==================================================
       Type Convert Methods For NR_FLOAT64        
==================================================
*/
/**
 * Converts data from a source node with type NR_BOOL to a
 * destination node with type NR_FLOAT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT64. If NULL, a new node is created.
 *   - src: The source node with type NR_BOOL containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_BOOL` to `NR_FLOAT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_BOOL_to_NR_FLOAT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float64*)dst->data + i) = (nr_float64)*((nr_bool*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)dst->data + i) = (nr_float64)*((nr_bool*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)NIter_ITEM(&it)) = (nr_float64)*((nr_bool*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float64*)NIter_ITEM(&dit)) = (nr_float64)*((nr_bool*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT8 to a
 * destination node with type NR_FLOAT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT64. If NULL, a new node is created.
 *   - src: The source node with type NR_INT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT8` to `NR_FLOAT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT8_to_NR_FLOAT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float64*)dst->data + i) = (nr_float64)*((nr_int8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)dst->data + i) = (nr_float64)*((nr_int8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)NIter_ITEM(&it)) = (nr_float64)*((nr_int8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float64*)NIter_ITEM(&dit)) = (nr_float64)*((nr_int8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT8 to a
 * destination node with type NR_FLOAT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT64. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT8 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT8` to `NR_FLOAT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT8_to_NR_FLOAT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float64*)dst->data + i) = (nr_float64)*((nr_uint8*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)dst->data + i) = (nr_float64)*((nr_uint8*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)NIter_ITEM(&it)) = (nr_float64)*((nr_uint8*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float64*)NIter_ITEM(&dit)) = (nr_float64)*((nr_uint8*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT16 to a
 * destination node with type NR_FLOAT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT64. If NULL, a new node is created.
 *   - src: The source node with type NR_INT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT16` to `NR_FLOAT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT16_to_NR_FLOAT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float64*)dst->data + i) = (nr_float64)*((nr_int16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)dst->data + i) = (nr_float64)*((nr_int16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)NIter_ITEM(&it)) = (nr_float64)*((nr_int16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float64*)NIter_ITEM(&dit)) = (nr_float64)*((nr_int16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT16 to a
 * destination node with type NR_FLOAT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT64. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT16 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT16` to `NR_FLOAT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT16_to_NR_FLOAT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float64*)dst->data + i) = (nr_float64)*((nr_uint16*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)dst->data + i) = (nr_float64)*((nr_uint16*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)NIter_ITEM(&it)) = (nr_float64)*((nr_uint16*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float64*)NIter_ITEM(&dit)) = (nr_float64)*((nr_uint16*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT32 to a
 * destination node with type NR_FLOAT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT64. If NULL, a new node is created.
 *   - src: The source node with type NR_INT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT32` to `NR_FLOAT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT32_to_NR_FLOAT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float64*)dst->data + i) = (nr_float64)*((nr_int32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)dst->data + i) = (nr_float64)*((nr_int32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)NIter_ITEM(&it)) = (nr_float64)*((nr_int32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float64*)NIter_ITEM(&dit)) = (nr_float64)*((nr_int32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT32 to a
 * destination node with type NR_FLOAT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT64. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT32` to `NR_FLOAT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT32_to_NR_FLOAT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float64*)dst->data + i) = (nr_float64)*((nr_uint32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)dst->data + i) = (nr_float64)*((nr_uint32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)NIter_ITEM(&it)) = (nr_float64)*((nr_uint32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float64*)NIter_ITEM(&dit)) = (nr_float64)*((nr_uint32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_INT64 to a
 * destination node with type NR_FLOAT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT64. If NULL, a new node is created.
 *   - src: The source node with type NR_INT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_INT64` to `NR_FLOAT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_INT64_to_NR_FLOAT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float64*)dst->data + i) = (nr_float64)*((nr_int64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)dst->data + i) = (nr_float64)*((nr_int64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)NIter_ITEM(&it)) = (nr_float64)*((nr_int64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float64*)NIter_ITEM(&dit)) = (nr_float64)*((nr_int64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_UINT64 to a
 * destination node with type NR_FLOAT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT64. If NULL, a new node is created.
 *   - src: The source node with type NR_UINT64 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_UINT64` to `NR_FLOAT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_UINT64_to_NR_FLOAT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float64*)dst->data + i) = (nr_float64)*((nr_uint64*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)dst->data + i) = (nr_float64)*((nr_uint64*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)NIter_ITEM(&it)) = (nr_float64)*((nr_uint64*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float64*)NIter_ITEM(&dit)) = (nr_float64)*((nr_uint64*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Converts data from a source node with type NR_FLOAT32 to a
 * destination node with type NR_FLOAT64 and performs type casting.
 *
 * Parameters:
 *   - dst: The destination node with type NR_FLOAT64. If NULL, a new node is created.
 *   - src: The source node with type NR_FLOAT32 containing the data to be converted.
 *
 * Behavior:
 *   - Ensures the source (`src`) and destination (`dst`) nodes share the same shape.
 *   - Allocates a new destination node if `dst` is NULL.
 *   - Optimizes conversion for memory-contiguous nodes. Uses `NIter` for non-contiguous memory.
 *   - Performs type casting from `NR_FLOAT32` to `NR_FLOAT64` during the data transfer.
 *
 * Return:
 *   - A pointer to the destination node (`dst`) after conversion.
 *   - NULL if an error occurs (e.g., shape mismatch, memory allocation failure).
 */NR_STATIC Node*
Node_TypeConvert_NR_FLOAT32_to_NR_FLOAT64(Node* dst, const Node* src){
    if (!dst){
        dst = Node_NewEmpty(src->ndim, src->shape, NR_FLOAT64);
        if (!dst){
            return NULL;
        }
    }
    else if (!Node_SameShape(dst, src)){
        _TC_RAISE_MISS_SHAPE(dst, src);
        return NULL;
    }
    else if (dst->dtype.dtype != NR_FLOAT64){
        _TC_RAISE_MISS_DTYPE(dst->dtype.dtype, NR_FLOAT64);
        return NULL;
    }

    int dcon = NODE_IS_CONTIGUOUS(dst);
    int scon = NODE_IS_CONTIGUOUS(src);

    if (dcon && scon){
        nr_size_t nitems = Node_NItems(dst);
        for (nr_size_t i = 0; i < nitems; i++){
            *((nr_float64*)dst->data + i) = (nr_float64)*((nr_float32*)src->data + i);
        }
    }
    else if (dcon | scon){
        if (dcon){
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)dst->data + i) = (nr_float64)*((nr_float32*)NIter_ITEM(&it));
                i++;
                NIter_NEXT(&it);
            }
        }
        else{
            NIter it;
            NIter_FromNode(&it, src, NITER_MODE_STRIDED);
            NIter_ITER(&it);
            int i = 0;
            while (NIter_NOTDONE(&it))
            {
                *((nr_float64*)NIter_ITEM(&it)) = (nr_float64)*((nr_float32*)src->data + i);
                i++;
                NIter_NEXT(&it);
            }
        }
        
    }
    else{
        NIter dit;
        NIter sit;
        NIter_FromNode(&dit, src, NITER_MODE_STRIDED);
        NIter_FromNode(&sit, src, NITER_MODE_STRIDED);
        NIter_ITER(&dit);
        NIter_ITER(&sit);
        while (NIter_NOTDONE(&sit))
        {
            *((nr_float64*)NIter_ITEM(&dit)) = (nr_float64)*((nr_float32*)NIter_ITEM(&sit));
            NIter_NEXT(&dit);
            NIter_NEXT(&sit);
        }
    }

    return dst;
}

/**
 * Registers the type-conversion methods for all supported source and destination data types.
 *
 * This array contains function pointers to the type-conversion functions,
 * parameterized by source and destination data types.
 */
Node2NodeFunc __NODE_TC_METHODS_ARRAY__[] = {
	//To NR_BOOL
	Node_Copy,
	Node_TypeConvert_NR_INT8_to_NR_BOOL,
	Node_TypeConvert_NR_UINT8_to_NR_BOOL,
	Node_TypeConvert_NR_INT16_to_NR_BOOL,
	Node_TypeConvert_NR_UINT16_to_NR_BOOL,
	Node_TypeConvert_NR_INT32_to_NR_BOOL,
	Node_TypeConvert_NR_UINT32_to_NR_BOOL,
	Node_TypeConvert_NR_INT64_to_NR_BOOL,
	Node_TypeConvert_NR_UINT64_to_NR_BOOL,
	Node_TypeConvert_NR_FLOAT32_to_NR_BOOL,
	Node_TypeConvert_NR_FLOAT64_to_NR_BOOL,

	//To NR_INT8
	Node_TypeConvert_NR_BOOL_to_NR_INT8,
	Node_Copy,
	Node_TypeConvert_NR_UINT8_to_NR_INT8,
	Node_TypeConvert_NR_INT16_to_NR_INT8,
	Node_TypeConvert_NR_UINT16_to_NR_INT8,
	Node_TypeConvert_NR_INT32_to_NR_INT8,
	Node_TypeConvert_NR_UINT32_to_NR_INT8,
	Node_TypeConvert_NR_INT64_to_NR_INT8,
	Node_TypeConvert_NR_UINT64_to_NR_INT8,
	Node_TypeConvert_NR_FLOAT32_to_NR_INT8,
	Node_TypeConvert_NR_FLOAT64_to_NR_INT8,

	//To NR_UINT8
	Node_TypeConvert_NR_BOOL_to_NR_UINT8,
	Node_TypeConvert_NR_INT8_to_NR_UINT8,
	Node_Copy,
	Node_TypeConvert_NR_INT16_to_NR_UINT8,
	Node_TypeConvert_NR_UINT16_to_NR_UINT8,
	Node_TypeConvert_NR_INT32_to_NR_UINT8,
	Node_TypeConvert_NR_UINT32_to_NR_UINT8,
	Node_TypeConvert_NR_INT64_to_NR_UINT8,
	Node_TypeConvert_NR_UINT64_to_NR_UINT8,
	Node_TypeConvert_NR_FLOAT32_to_NR_UINT8,
	Node_TypeConvert_NR_FLOAT64_to_NR_UINT8,

	//To NR_INT16
	Node_TypeConvert_NR_BOOL_to_NR_INT16,
	Node_TypeConvert_NR_INT8_to_NR_INT16,
	Node_TypeConvert_NR_UINT8_to_NR_INT16,
	Node_Copy,
	Node_TypeConvert_NR_UINT16_to_NR_INT16,
	Node_TypeConvert_NR_INT32_to_NR_INT16,
	Node_TypeConvert_NR_UINT32_to_NR_INT16,
	Node_TypeConvert_NR_INT64_to_NR_INT16,
	Node_TypeConvert_NR_UINT64_to_NR_INT16,
	Node_TypeConvert_NR_FLOAT32_to_NR_INT16,
	Node_TypeConvert_NR_FLOAT64_to_NR_INT16,

	//To NR_UINT16
	Node_TypeConvert_NR_BOOL_to_NR_UINT16,
	Node_TypeConvert_NR_INT8_to_NR_UINT16,
	Node_TypeConvert_NR_UINT8_to_NR_UINT16,
	Node_TypeConvert_NR_INT16_to_NR_UINT16,
	Node_Copy,
	Node_TypeConvert_NR_INT32_to_NR_UINT16,
	Node_TypeConvert_NR_UINT32_to_NR_UINT16,
	Node_TypeConvert_NR_INT64_to_NR_UINT16,
	Node_TypeConvert_NR_UINT64_to_NR_UINT16,
	Node_TypeConvert_NR_FLOAT32_to_NR_UINT16,
	Node_TypeConvert_NR_FLOAT64_to_NR_UINT16,

	//To NR_INT32
	Node_TypeConvert_NR_BOOL_to_NR_INT32,
	Node_TypeConvert_NR_INT8_to_NR_INT32,
	Node_TypeConvert_NR_UINT8_to_NR_INT32,
	Node_TypeConvert_NR_INT16_to_NR_INT32,
	Node_TypeConvert_NR_UINT16_to_NR_INT32,
	Node_Copy,
	Node_TypeConvert_NR_UINT32_to_NR_INT32,
	Node_TypeConvert_NR_INT64_to_NR_INT32,
	Node_TypeConvert_NR_UINT64_to_NR_INT32,
	Node_TypeConvert_NR_FLOAT32_to_NR_INT32,
	Node_TypeConvert_NR_FLOAT64_to_NR_INT32,

	//To NR_UINT32
	Node_TypeConvert_NR_BOOL_to_NR_UINT32,
	Node_TypeConvert_NR_INT8_to_NR_UINT32,
	Node_TypeConvert_NR_UINT8_to_NR_UINT32,
	Node_TypeConvert_NR_INT16_to_NR_UINT32,
	Node_TypeConvert_NR_UINT16_to_NR_UINT32,
	Node_TypeConvert_NR_INT32_to_NR_UINT32,
	Node_Copy,
	Node_TypeConvert_NR_INT64_to_NR_UINT32,
	Node_TypeConvert_NR_UINT64_to_NR_UINT32,
	Node_TypeConvert_NR_FLOAT32_to_NR_UINT32,
	Node_TypeConvert_NR_FLOAT64_to_NR_UINT32,

	//To NR_INT64
	Node_TypeConvert_NR_BOOL_to_NR_INT64,
	Node_TypeConvert_NR_INT8_to_NR_INT64,
	Node_TypeConvert_NR_UINT8_to_NR_INT64,
	Node_TypeConvert_NR_INT16_to_NR_INT64,
	Node_TypeConvert_NR_UINT16_to_NR_INT64,
	Node_TypeConvert_NR_INT32_to_NR_INT64,
	Node_TypeConvert_NR_UINT32_to_NR_INT64,
	Node_Copy,
	Node_TypeConvert_NR_UINT64_to_NR_INT64,
	Node_TypeConvert_NR_FLOAT32_to_NR_INT64,
	Node_TypeConvert_NR_FLOAT64_to_NR_INT64,

	//To NR_UINT64
	Node_TypeConvert_NR_BOOL_to_NR_UINT64,
	Node_TypeConvert_NR_INT8_to_NR_UINT64,
	Node_TypeConvert_NR_UINT8_to_NR_UINT64,
	Node_TypeConvert_NR_INT16_to_NR_UINT64,
	Node_TypeConvert_NR_UINT16_to_NR_UINT64,
	Node_TypeConvert_NR_INT32_to_NR_UINT64,
	Node_TypeConvert_NR_UINT32_to_NR_UINT64,
	Node_TypeConvert_NR_INT64_to_NR_UINT64,
	Node_Copy,
	Node_TypeConvert_NR_FLOAT32_to_NR_UINT64,
	Node_TypeConvert_NR_FLOAT64_to_NR_UINT64,

	//To NR_FLOAT32
	Node_TypeConvert_NR_BOOL_to_NR_FLOAT32,
	Node_TypeConvert_NR_INT8_to_NR_FLOAT32,
	Node_TypeConvert_NR_UINT8_to_NR_FLOAT32,
	Node_TypeConvert_NR_INT16_to_NR_FLOAT32,
	Node_TypeConvert_NR_UINT16_to_NR_FLOAT32,
	Node_TypeConvert_NR_INT32_to_NR_FLOAT32,
	Node_TypeConvert_NR_UINT32_to_NR_FLOAT32,
	Node_TypeConvert_NR_INT64_to_NR_FLOAT32,
	Node_TypeConvert_NR_UINT64_to_NR_FLOAT32,
	Node_Copy,
	Node_TypeConvert_NR_FLOAT64_to_NR_FLOAT32,

	//To NR_FLOAT64
	Node_TypeConvert_NR_BOOL_to_NR_FLOAT64,
	Node_TypeConvert_NR_INT8_to_NR_FLOAT64,
	Node_TypeConvert_NR_UINT8_to_NR_FLOAT64,
	Node_TypeConvert_NR_INT16_to_NR_FLOAT64,
	Node_TypeConvert_NR_UINT16_to_NR_FLOAT64,
	Node_TypeConvert_NR_INT32_to_NR_FLOAT64,
	Node_TypeConvert_NR_UINT32_to_NR_FLOAT64,
	Node_TypeConvert_NR_INT64_to_NR_FLOAT64,
	Node_TypeConvert_NR_UINT64_to_NR_FLOAT64,
	Node_TypeConvert_NR_FLOAT32_to_NR_FLOAT64,
	Node_Copy,
};