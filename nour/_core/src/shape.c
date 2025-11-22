/*
 * Shape transformation operations.
 * Each function returns a new Node view (sharing memory) unless:
 *   - Operation requires data size change (resize) => allocates new memory.
 *   - 'copy' argument is set to non-zero AND the original node has ref_count == 1;
 *     in that case we modify the original node in-place and return it.
 * All functions validate axes and dimensionality. On error they return NULL
 * after raising an appropriate NError.
 */

#include "shape.h"
#include "node_core.h"
#include "ntools.h"
#include "nerror.h"
#include "niter.h"

/* -------------------------------------------------------------------------- */
/* Helper utilities                                                           */
/* -------------------------------------------------------------------------- */

NR_STATIC_INLINE int _can_inplace(Node* node, int copy){
    return copy && node && NODE_REFCOUNT(node) == 1;
}

NR_STATIC_INLINE void _apply_inplace(Node* node, int new_ndim, nr_intp* new_shape){
    /* Reallocate shape/strides if ndim changed */
    if (new_ndim != node->ndim){
        free(node->shape);
        free(node->strides);
        node->shape = (nr_intp*)malloc(sizeof(nr_intp) * new_ndim);
        node->strides = (nr_intp*)malloc(sizeof(nr_intp) * new_ndim);
        if (!node->shape || !node->strides){
            NError_RaiseMemoryError();
            return; /* caller must detect allocation failure separately if needed */
        }
    }
    memcpy(node->shape, new_shape, sizeof(nr_intp) * new_ndim);
    NTools_CalculateStrides(new_ndim, new_shape, NODE_ITEMSIZE(node), node->strides);
    node->ndim = new_ndim;
}

NR_STATIC_INLINE Node* _new_view(Node* src, int ndim, nr_intp* shape, nr_intp* strides){
    return Node_NewChild(src, ndim, shape, strides, 0);
}

NR_STATIC_INLINE int _validate_axis(int axis, int ndim){
    return axis >= 0 && axis < ndim;
}

/* Build contiguous strides for a given shape */
NR_STATIC_INLINE void _build_strides(int ndim, const nr_intp* shape, nr_intp itemsize, nr_intp* strides){
    NTools_CalculateStrides(ndim, (nr_intp*)shape, itemsize, strides);
}

/* -------------------------------------------------------------------------- */
/* Reshape (view)                                                             */
/* -------------------------------------------------------------------------- */
NR_PUBLIC Node* Node_Reshape(Node* node, nr_intp* new_shape, int new_ndim, int copy){
    if (!node){
        NError_RaiseError(NError_ValueError, "reshape: NULL node");
        return NULL;
    }
    if (new_ndim < 0 || new_ndim > NR_NODE_MAX_NDIM){
        NError_RaiseError(NError_ValueError, "reshape: invalid new ndim %d", new_ndim);
        return NULL;
    }
    nr_intp old_items = Node_NItems(node);
    nr_intp new_items = NR_NItems(new_ndim, new_shape);
    if (old_items != new_items){
        NError_RaiseError(NError_ValueError, "reshape: item count mismatch %lld -> %lld", (long long)old_items, (long long)new_items);
        return NULL;
    }
    if (!NODE_IS_CONTIGUOUS(node)){
        NError_RaiseError(NError_ValueError, "reshape: only contiguous arrays supported for view reshape");
        return NULL;
    }
    if (_can_inplace(node, copy)){
        _apply_inplace(node, new_ndim, new_shape);
        return node;
    }
    nr_intp strides[NR_NODE_MAX_NDIM];
    _build_strides(new_ndim, new_shape, NODE_ITEMSIZE(node), strides);
    return _new_view(node, new_ndim, new_shape, strides);
}

/* -------------------------------------------------------------------------- */
/* Ravel / Flatten (1-D view)                                                 */
/* -------------------------------------------------------------------------- */
NR_PUBLIC Node* Node_Ravel(Node* node, int copy){
    if (!node){ NError_RaiseError(NError_ValueError, "ravel: NULL node"); return NULL; }
    nr_intp nitems = Node_NItems(node);
    nr_intp shape1[1] = { nitems };
    if (_can_inplace(node, copy)){
        _apply_inplace(node, 1, shape1);
        return node;
    }
    if (!NODE_IS_CONTIGUOUS(node)){
        /* Fallback: create a contiguous copy */
        Node* out = Node_NewEmpty(1, shape1, NODE_DTYPE(node));
        if (!out) return NULL;
        Node_Copy(out, node);
        return out;
    }
    nr_intp strides[1]; _build_strides(1, shape1, NODE_ITEMSIZE(node), strides);
    return _new_view(node, 1, shape1, strides);
}

NR_PUBLIC Node* Node_Flatten(Node* node, int copy){
    return Node_Ravel(node, copy);
}

/* -------------------------------------------------------------------------- */
/* SwapAxes                                                                  */
/* -------------------------------------------------------------------------- */
NR_PUBLIC Node* Node_SwapAxes(Node* node, int axis1, int axis2, int copy){
    if (!node){ NError_RaiseError(NError_ValueError, "swapaxes: NULL node"); return NULL; }
    if (!_validate_axis(axis1, node->ndim) || !_validate_axis(axis2, node->ndim)){
        NError_RaiseError(NError_ValueError, "swapaxes: invalid axes %d, %d", axis1, axis2);
        return NULL;
    }
    if (axis1 == axis2){
        return copy ? node : _new_view(node, node->ndim, node->shape, node->strides);
    }
    nr_intp new_shape[NR_NODE_MAX_NDIM];
    nr_intp new_strides[NR_NODE_MAX_NDIM];
    memcpy(new_shape, node->shape, sizeof(nr_intp)*node->ndim);
    memcpy(new_strides, node->strides, sizeof(nr_intp)*node->ndim);
    nr_intp tmp = new_shape[axis1]; new_shape[axis1] = new_shape[axis2]; new_shape[axis2] = tmp;
    tmp = new_strides[axis1]; new_strides[axis1] = new_strides[axis2]; new_strides[axis2] = tmp;
    if (_can_inplace(node, copy)){
        _apply_inplace(node, node->ndim, new_shape);
        memcpy(node->strides, new_strides, sizeof(nr_intp)*node->ndim);
        return node;
    }
    return Node_NewChild(node, node->ndim, new_shape, new_strides, 0);
}

/* -------------------------------------------------------------------------- */
/* Transpose (reverse axes)                                                   */
/* -------------------------------------------------------------------------- */
NR_PUBLIC Node* Node_Transpose(Node* node, int copy){
    if (!node){ NError_RaiseError(NError_ValueError, "transpose: NULL node"); return NULL; }
    if (node->ndim <= 1){ return copy ? node : _new_view(node, node->ndim, node->shape, node->strides); }
    nr_intp new_shape[NR_NODE_MAX_NDIM];
    nr_intp new_strides[NR_NODE_MAX_NDIM];
    for (int i=0;i<node->ndim;i++){ new_shape[i] = node->shape[node->ndim-1-i]; new_strides[i] = node->strides[node->ndim-1-i]; }
    if (_can_inplace(node, copy)){
        _apply_inplace(node, node->ndim, new_shape);
        memcpy(node->strides, new_strides, sizeof(nr_intp)*node->ndim);
        return node;
    }
    return Node_NewChild(node, node->ndim, new_shape, new_strides, 0);
}

/* -------------------------------------------------------------------------- */
/* Permute Dims (arbitrary order)                                             */
/* -------------------------------------------------------------------------- */
NR_PUBLIC Node* Node_PermuteDims(Node* node, const int* order, int copy){
    if (!node){ NError_RaiseError(NError_ValueError, "permute_dims: NULL node"); return NULL; }
    if (!order){ NError_RaiseError(NError_ValueError, "permute_dims: NULL order"); return NULL; }
    int ndim = node->ndim;
    int seen[NR_NODE_MAX_NDIM] = {0};
    for (int i=0;i<ndim;i++){ if (order[i] < 0 || order[i] >= ndim || seen[order[i]]){ NError_RaiseError(NError_ValueError, "permute_dims: invalid or duplicate axis %d", order[i]); return NULL; } seen[order[i]] = 1; }
    nr_intp new_shape[NR_NODE_MAX_NDIM]; nr_intp new_strides[NR_NODE_MAX_NDIM];
    for (int i=0;i<ndim;i++){ new_shape[i] = node->shape[order[i]]; new_strides[i] = node->strides[order[i]]; }
    if (_can_inplace(node, copy)){
        _apply_inplace(node, ndim, new_shape);
        memcpy(node->strides, new_strides, sizeof(nr_intp)*ndim);
        return node;
    }
    return Node_NewChild(node, ndim, new_shape, new_strides, 0);
}

/* -------------------------------------------------------------------------- */
/* MoveAxis (move single axis to new position)                                */
/* -------------------------------------------------------------------------- */
NR_PUBLIC Node* Node_MoveAxis(Node* node, int src_axis, int dst_axis, int copy){
    if (!node){ NError_RaiseError(NError_ValueError, "moveaxis: NULL node"); return NULL; }
    int ndim = node->ndim;
    if (!_validate_axis(src_axis, ndim) || dst_axis < 0 || dst_axis >= ndim){
        NError_RaiseError(NError_ValueError, "moveaxis: invalid src %d or dst %d", src_axis, dst_axis);
        return NULL;
    }
    if (src_axis == dst_axis){ return copy ? node : _new_view(node, ndim, node->shape, node->strides); }
    int order[NR_NODE_MAX_NDIM];
    int j=0;
    for (int i=0;i<ndim;i++){ if (i != src_axis){ order[j++] = i; } }
    for (int i=ndim-1;i>=dst_axis;i--){ order[i] = order[i-1]; }
    order[dst_axis] = src_axis; /* This algorithm ensures insertion at dst_axis */
    /* But adjust: simpler approach build new order manually */
    j=0; int k=0; int tmp_order[NR_NODE_MAX_NDIM];
    for (int i=0;i<ndim;i++){ tmp_order[i] = -1; }
    for (int i=0;i<ndim;i++){ if (i==src_axis) continue; tmp_order[j++] = i; }
    /* insert src_axis at dst_axis */
    for (int i=ndim-1;i>dst_axis;i--){ tmp_order[i] = tmp_order[i-1]; }
    tmp_order[dst_axis] = src_axis;
    for (int i=0;i<ndim;i++){ order[i] = tmp_order[i]; }
    return Node_PermuteDims(node, order, copy);
}

/* -------------------------------------------------------------------------- */
/* RollAxis (move axis toward front until position 'start')                   */
/* -------------------------------------------------------------------------- */
NR_PUBLIC Node* Node_RollAxis(Node* node, int axis, int start, int copy){
    if (!node){ NError_RaiseError(NError_ValueError, "rollaxis: NULL node"); return NULL; }
    int ndim = node->ndim;
    if (!_validate_axis(axis, ndim) || start < 0 || start >= ndim){
        NError_RaiseError(NError_ValueError, "rollaxis: invalid axis %d or start %d", axis, start);
        return NULL;
    }
    if (axis < start){ /* move axis forward (toward end) until start */
        return Node_MoveAxis(node, axis, start, copy);
    }
    /* axis > start: move backwards */
    int order[NR_NODE_MAX_NDIM];
    int idx=0;
    for (int i=0;i<ndim;i++){ order[i] = i; }
    while (axis > start){
        int a = order[axis-1];
        order[axis-1] = order[axis];
        order[axis] = a;
        axis--;
    }
    return Node_PermuteDims(node, order, copy);
}

/* -------------------------------------------------------------------------- */
/* MatrixTranspose (2-D only)                                                 */
/* -------------------------------------------------------------------------- */
NR_PUBLIC Node* Node_MatrixTranspose(Node* node, int copy){
    if (!node){ NError_RaiseError(NError_ValueError, "matrix_transpose: NULL node"); return NULL; }
    if (node->ndim != 2){ NError_RaiseError(NError_ValueError, "matrix_transpose: requires 2D, got %d", node->ndim); return NULL; }
    int order[2] = {1,0};
    return Node_PermuteDims(node, order, copy);
}

/* -------------------------------------------------------------------------- */
/* ExpandDims (insert axis of length 1)                                       */
/* -------------------------------------------------------------------------- */
NR_PUBLIC Node* Node_ExpandDims(Node* node, int axis, int copy){
    if (!node){ NError_RaiseError(NError_ValueError, "expand_dims: NULL node"); return NULL; }
    int ndim = node->ndim;
    if (axis < 0 || axis > ndim){ NError_RaiseError(NError_ValueError, "expand_dims: invalid axis %d", axis); return NULL; }
    nr_intp new_shape[NR_NODE_MAX_NDIM];
    nr_intp new_strides[NR_NODE_MAX_NDIM];
    for (int i=0;i<axis;i++){ new_shape[i] = node->shape[i]; new_strides[i] = node->strides[i]; }
    new_shape[axis] = 1; new_strides[axis] = (ndim==0 ? NODE_ITEMSIZE(node) : node->strides[axis==ndim?axis-1:axis]);
    for (int i=axis;i<ndim;i++){ new_shape[i+1] = node->shape[i]; new_strides[i+1] = node->strides[i]; }
    if (_can_inplace(node, copy)){
        _apply_inplace(node, ndim+1, new_shape);
        memcpy(node->strides, new_strides, sizeof(nr_intp)*(ndim+1));
        return node;
    }
    return Node_NewChild(node, ndim+1, new_shape, new_strides, 0);
}

/* -------------------------------------------------------------------------- */
/* Squeeze (remove axes of length 1)                                          */
/* -------------------------------------------------------------------------- */
NR_PUBLIC Node* Node_Squeeze(Node* node, int copy){
    if (!node){ NError_RaiseError(NError_ValueError, "squeeze: NULL node"); return NULL; }
    if (NODE_IS_SCALAR(node)){ return copy ? node : _new_view(node, 0, NULL, NULL); }
    nr_intp new_shape[NR_NODE_MAX_NDIM]; nr_intp new_strides[NR_NODE_MAX_NDIM];
    int new_ndim = 0;
    for (int i=0;i<node->ndim;i++){ if (node->shape[i] != 1){ new_shape[new_ndim] = node->shape[i]; new_strides[new_ndim] = node->strides[i]; new_ndim++; } }
    if (new_ndim == node->ndim){ return copy ? node : _new_view(node, node->ndim, node->shape, node->strides); }
    if (new_ndim == 0){ /* becomes scalar */
        if (_can_inplace(node, copy)){
            free(node->shape); free(node->strides); node->shape=NULL; node->strides=NULL; node->ndim=0; return node; }
        return Node_NewChild(node, 0, NULL, NULL, 0);
    }
    if (_can_inplace(node, copy)){
        _apply_inplace(node, new_ndim, new_shape);
        memcpy(node->strides, new_strides, sizeof(nr_intp)*new_ndim);
        return node;
    }
    return Node_NewChild(node, new_ndim, new_shape, new_strides, 0);
}

/* -------------------------------------------------------------------------- */
/* Resize (allocate new memory, copy overlap)                                 */
/* -------------------------------------------------------------------------- */
NR_PUBLIC Node* Node_Resize(Node* node, nr_intp* new_shape, int new_ndim, int copy){
    if (!node){ NError_RaiseError(NError_ValueError, "resize: NULL node"); return NULL; }
    if (new_ndim < 0 || new_ndim > NR_NODE_MAX_NDIM){ NError_RaiseError(NError_ValueError, "resize: invalid ndim %d", new_ndim); return NULL; }
    nr_intp new_items = NR_NItems(new_ndim, new_shape);
    nr_intp itemsize = NODE_ITEMSIZE(node);
    void* new_data = malloc(new_items * itemsize);
    if (!new_data){ NError_RaiseMemoryError(); return NULL; }
    /* copy min(old_items, new_items) */
    nr_intp old_items = Node_NItems(node);
    nr_intp to_copy = old_items < new_items ? old_items : new_items;
    if (NODE_IS_CONTIGUOUS(node)){
        memcpy(new_data, node->data, to_copy * itemsize);
    } else {
        /* Use iterator copy for non-contiguous source */
        NIter it; NIter_New(&it, node->data, node->ndim, node->shape, node->strides, NITER_MODE_STRIDED);
        NIter_ITER(&it);
        char* dst = (char*)new_data; nr_intp copied = 0;
        while (NIter_NOTDONE(&it) && copied < to_copy){ memcpy(dst + copied*itemsize, NIter_ITEM(&it), itemsize); NIter_NEXT(&it); copied++; }
    }
    /* zero-fill remainder */
    if (to_copy < new_items){ memset((char*)new_data + to_copy*itemsize, 0, (new_items - to_copy)*itemsize); }
    if (_can_inplace(node, copy)){
        if (NODE_IS_OWNDATA(node)){ free(node->data); }
        node->data = new_data; node->flags |= NR_NODE_OWNDATA;
        _apply_inplace(node, new_ndim, new_shape);
        return node;
    }
    Node* out = Node_New(new_data, 1, new_ndim, new_shape, NODE_DTYPE(node));
    return out;
}
