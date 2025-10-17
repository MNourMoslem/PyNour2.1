#include <string.h>

#include "node_core.h"
#include "nerror.h"
#include "niter.h"
#include "free.h"

/* ============================================================================
 * Constants and Globals
 * ============================================================================ */

char* NR_NODE_NAME = "node";

/* ============================================================================
 * Helper Macros
 * ============================================================================ */

#define CHECK_NULL(ptr, cleanup_label) \
    do { \
        if (!(ptr)) { \
            NError_RaiseMemoryError(); \
            goto cleanup_label; \
        } \
    } while(0)

#define ALLOC_AND_CHECK(ptr, size, cleanup_label) \
    do { \
        (ptr) = malloc(size); \
        CHECK_NULL(ptr, cleanup_label); \
    } while(0)

#define ALLOC_COPY_AND_CHECK(dst, src, size, cleanup_label) \
    do { \
        ALLOC_AND_CHECK(dst, size, cleanup_label); \
        memcpy((dst), (src), (size)); \
    } while(0)

#define SHAPE_STRIDE_SIZE(ndim) (sizeof(nr_intp) * (ndim))

#define SET_DEFAULT_FLAGS(node) \
    NR_SETFLG((node)->flags, (NR_NODE_C_ORDER | NR_NODE_CONTIGUOUS | NR_NODE_WRITABLE))

#define SET_OWNDATA_FLAGS(node) \
    do { \
        NR_SETFLG((node)->flags, NR_NODE_OWNDATA); \
        SET_DEFAULT_FLAGS(node); \
    } while(0)

/* ============================================================================
 * Internal Helper Structures
 * ============================================================================ */

typedef struct {
    Node* node;
    nr_intp nitems;
} NodeInitResult;

/* ============================================================================
 * Memory Allocation Helpers
 * ============================================================================ */

NR_PRIVATE int
AllocateShapeAndStrides(Node* node, nr_intp* shape) {
    nr_intp size = SHAPE_STRIDE_SIZE(node->ndim);
    
    ALLOC_COPY_AND_CHECK(node->shape, shape, size, error_shape);
    ALLOC_AND_CHECK(node->strides, size, error_strides);
    
    return 0;

error_strides:
    free(node->shape);
error_shape:
    return -1;
}

NR_PRIVATE void
FreeShapeAndStrides(Node* node) {
    if (node) {
        free(node->shape);
        free(node->strides);
    }
}

NR_PRIVATE void
FreeNodePartial(Node* node) {
    if (node) {
        FreeShapeAndStrides(node);
        free(node);
    }
}

/* ============================================================================
 * Node Initialization
 * ============================================================================ */

NR_PRIVATE Node*
InitializeNodeMetadata(int ndim, NR_DTYPE dtype) {
    Node* node;
    
    ALLOC_AND_CHECK(node, sizeof(Node), error);
    
    node->name = NR_NODE_NAME;
    
    if (ndim > NR_NODE_MAX_NDIM) {
        NError_RaiseError(
            NError_ValueError,
            "%s object could not have more than %i dimensions. got %i\n",
            node->name, NR_NODE_MAX_NDIM, ndim
        );
        goto error_ndim;
    }
    
    node->ndim = ndim;
    node->dtype = NDtype_New(dtype);
    node->base = NULL;
    node->ref_count = 1;
    node->flags = 0;
    
    return node;

error_ndim:
    free(node);
error:
    return NULL;
}

NR_PRIVATE nr_intp
ComputeStridesAndItems(Node* node) {
    nr_intp nitems = 1;
    
    for (int i = node->ndim - 1; i >= 0; i--) {
        node->strides[i] = nitems * node->dtype.size;
        nitems *= node->shape[i];
    }
    
    return nitems;
}

NR_PRIVATE NodeInitResult
CreateNodeStructure(int ndim, nr_intp* shape, NR_DTYPE dtype) {
    NodeInitResult result = {NULL, -1};
    
    result.node = InitializeNodeMetadata(ndim, dtype);
    if (!result.node) {
        return result;
    }
    
    if (AllocateShapeAndStrides(result.node, shape) < 0) {
        free(result.node);
        result.node = NULL;
        return result;
    }
    
    result.nitems = ComputeStridesAndItems(result.node);
    return result;
}

/* ============================================================================
 * Data Allocation and Copying
 * ============================================================================ */

NR_PRIVATE int
AllocateAndCopyData(Node* node, void* src_data, nr_intp nitems) {
    nr_intp data_size = nitems * node->dtype.size;
    
    ALLOC_AND_CHECK(node->data, data_size, error);
    memcpy(node->data, src_data, data_size);
    
    return 0;

error:
    return -1;
}

NR_PRIVATE int
AllocateEmptyData(Node* node, nr_intp nitems) {
    nr_intp data_size = nitems * node->dtype.size;
    
    ALLOC_AND_CHECK(node->data, data_size, error);
    
    return 0;

error:
    return -1;
}

/* ============================================================================
 * Shape and Type Validation
 * ============================================================================ */

NR_PRIVATE int
ValidateShape(const Node* dst, const Node* src) {
    if (Node_SameShape(dst, src)) {
        return 0;
    }
    
    char dst_shape[30], src_shape[30];
    NTools_ShapeAsString(dst->shape, dst->ndim, dst_shape);
    NTools_ShapeAsString(src->shape, src->ndim, src_shape);
    
    NError_RaiseError(
        NError_ValueError,
        "%s object has to have the same shape as src %s. got %s and %s",
        dst->name, src->name, dst_shape, src_shape
    );
    
    return -1;
}

NR_PRIVATE int
ValidateDtype(const Node* dst, const Node* src) {
    if (dst->dtype.dtype == src->dtype.dtype) {
        return 0;
    }
    
    char dst_type[30], src_type[30];
    NDtype_AsStringOnlyType(dst->dtype.dtype, dst_type);
    NDtype_AsStringOnlyType(src->dtype.dtype, src_type);
    
    NError_RaiseError(
        NError_ValueError,
        "%s object and src %s object must have the same dtype. got %s and %s",
        dst->name, src->name, dst_type, src_type
    );
    
    return -1;
}

/* ============================================================================
 * Data Copying Strategies
 * ============================================================================ */

NR_PRIVATE void
CopyContiguousToContiguous(Node* dst, const Node* src) {
    nr_intp nitems = Node_NItems(dst);
    nr_intp copy_size = nitems * dst->dtype.size;
    memcpy(dst->data, src->data, copy_size);
}

NR_PRIVATE void
CopyStridedToContiguous(Node* dst, const Node* src) {
    nr_intp bsize = dst->dtype.size;
    NIter it;
    
    NIter_FromNode(&it, src, NITER_MODE_STRIDED);
    NIter_ITER(&it);
    
    nr_intp offset = 0;
    while (NIter_NOTDONE(&it)) {
        memcpy((char*)dst->data + offset, NIter_ITEM(&it), bsize);
        offset += bsize;
        NIter_NEXT(&it);
    }
}

NR_PRIVATE void
CopyContiguousToStrided(Node* dst, const Node* src) {
    nr_intp bsize = dst->dtype.size;
    NIter it;
    
    NIter_FromNode(&it, dst, NITER_MODE_STRIDED);
    NIter_ITER(&it);
    
    nr_intp offset = 0;
    while (NIter_NOTDONE(&it)) {
        memcpy(NIter_ITEM(&it), (char*)src->data + offset, bsize);
        offset += bsize;
        NIter_NEXT(&it);
    }
}

NR_PRIVATE void
CopyStridedToStrided(Node* dst, const Node* src) {
    nr_intp bsize = dst->dtype.size;
    NIter dst_it, src_it;
    
    NIter_FromNode(&dst_it, dst, NITER_MODE_STRIDED);
    NIter_FromNode(&src_it, src, NITER_MODE_STRIDED);
    NIter_ITER(&dst_it);
    NIter_ITER(&src_it);
    
    while (NIter_NOTDONE(&src_it)) {
        memcpy(NIter_ITEM(&dst_it), NIter_ITEM(&src_it), bsize);
        NIter_NEXT(&dst_it);
        NIter_NEXT(&src_it);
    }
}

NR_PRIVATE void
CopyNodeData(Node* dst, const Node* src) {
    int dst_contiguous = NODE_IS_CONTIGUOUS(dst);
    int src_contiguous = NODE_IS_CONTIGUOUS(src);
    
    if (dst_contiguous && src_contiguous) {
        CopyContiguousToContiguous(dst, src);
    }
    else if (dst_contiguous) {
        CopyStridedToContiguous(dst, src);
    }
    else if (src_contiguous) {
        CopyContiguousToStrided(dst, src);
    }
    else {
        CopyStridedToStrided(dst, src);
    }
}

/* ============================================================================
 * Public API
 * ============================================================================ */

NR_PUBLIC Node*
Node_New(void* data_block, int copy_data, int ndim, nr_intp* shape, NR_DTYPE dtype) {
    NodeInitResult result = CreateNodeStructure(ndim, shape, dtype);
    
    if (!result.node || result.nitems < 0) {
        NError_RaiseMemoryError();
        return NULL;
    }
    
    if (copy_data) {
        if (AllocateAndCopyData(result.node, data_block, result.nitems) < 0) {
            FreeNodePartial(result.node);
            return NULL;
        }
        SET_OWNDATA_FLAGS(result.node);
    }
    else {
        result.node->data = data_block;
        SET_DEFAULT_FLAGS(result.node);
    }
    
    return result.node;
}

NR_PUBLIC Node*
Node_NewEmpty(int ndim, nr_intp* shape, NR_DTYPE dtype) {
    NodeInitResult result = CreateNodeStructure(ndim, shape, dtype);
    
    if (!result.node || result.nitems < 0) {
        NError_RaiseMemoryError();
        return NULL;
    }
    
    if (AllocateEmptyData(result.node, result.nitems) < 0) {
        FreeNodePartial(result.node);
        return NULL;
    }
    
    SET_OWNDATA_FLAGS(result.node);
    
    return result.node;
}

NR_PUBLIC Node*
Node_Copy(Node* dst, const Node* src) {
    if (!dst) {
        dst = Node_NewEmpty(src->ndim, src->shape, src->dtype.dtype);
        if (!dst) {
            return NULL;
        }
    }
    else {
        if (ValidateShape(dst, src) < 0 || ValidateDtype(dst, src) < 0) {
            return NULL;
        }
    }
    
    CopyNodeData(dst, src);
    return dst;
}

NR_PUBLIC Node*
Node_CopyWithReference(const Node* src) {
    if (!src) {
        return NULL;
    }

    Node* node;
    ALLOC_AND_CHECK(node, sizeof(Node), error);
    
    // Copy metadata
    node->name = NR_NODE_NAME;
    node->ndim = src->ndim;
    node->dtype = src->dtype;
    node->ref_count = 1;
    node->flags = src->flags;
    
    // Allocate and copy shape and strides
    if (AllocateShapeAndStrides(node, src->shape) < 0) {
        free(node);
        return NULL;
    }
    memcpy(node->strides, src->strides, SHAPE_STRIDE_SIZE(node->ndim));
    
    // Share data reference
    node->data = src->data;
    node->base = (Node*)src;
    ((Node*)src)->ref_count++;
    
    // Update flags for shared data
    NR_RMVFLG(node->flags, NR_NODE_OWNDATA);
    
    return node;

error:
    return NULL;
}