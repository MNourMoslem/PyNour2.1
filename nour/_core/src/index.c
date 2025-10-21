#include "index.h"
#include "nerror.h"
#include "node_core.h"
#include "niter.h"
#include "narray.h"
#include "free.h"

NR_PUBLIC Node*
Node_Slice(Node* nout, const Node* node, const Slice slice, int dim){
    if (!node){
        NError_RaiseError(NError_ValueError, "Node cannot be NULL.");
        return NULL;
    }

    if (NODE_NDIM(node) == 0 || dim < 0 || dim >= NODE_NDIM(node)){
        NError_RaiseError(NError_IndexError, 
            "Invalid dimension for slicing. got %d for array with %d dimensions.", 
            dim, NODE_NDIM(node));
        return NULL;
    }

    nr_intp start = slice.start;
    nr_intp stop = slice.stop;
    nr_intp step = slice.step;
    nr_intp dim_size = NODE_SHAPE(node)[dim];

    if (step == 0){
        NError_RaiseError(NError_ValueError, "Slice step cannot be zero.");
        return NULL;
    }

    // Normalize negative indices
    if (start < 0) start += dim_size;
    if (stop < 0) stop += dim_size;

    // Clamp to valid range
    if (start < 0) start = (step > 0) ? 0 : -1;
    if (start > dim_size) start = (step > 0) ? dim_size : dim_size;
    if (stop < 0) stop = (step > 0) ? 0 : -1;
    if (stop > dim_size) stop = (step > 0) ? dim_size : dim_size;

    // Calculate new dimension size
    nr_intp new_dim_size;
    if (step > 0) {
        new_dim_size = (stop > start) ? ((stop - start + step - 1) / step) : 0;
    } else {
        new_dim_size = (start > stop) ? ((start - stop - step - 1) / (-step)) : 0;
    }

    // Create output node if not provided
    if (!nout){
        nout = Node_CopyWithReference(node);
        if (!nout) return NULL;
    }

    // Calculate the offset for the new data pointer
    nr_intp offset = start * NODE_STRIDES(node)[dim];
    nout->data = (char*)node->data + offset;
    
    // Update shape and strides
    NODE_SHAPE(nout)[dim] = new_dim_size;
    NODE_STRIDES(nout)[dim] = NODE_STRIDES(node)[dim] * step;

    // Set base reference to source node and increment refcount
    if (nout->base != node) {
        nout->base = (Node*)node;
        ((Node*)node)->ref_count++;
    }

    // Mark as not owning data and update flags
    NR_RMVFLG(nout->flags, NR_NODE_OWNDATA);
    NR_SETFLG(nout->flags, NR_NODE_STRIDED);
    NR_RMVFLG(nout->flags, NR_NODE_CONTIGUOUS);

    return nout;
}


NR_PUBLIC Node*
Node_MultiSlice(Node* nout, const Node* node, const Slice* slices, int num_slices){
    if (!node || !slices){
        NError_RaiseError(NError_ValueError, "Node and slices cannot be NULL.");
        return NULL;
    }

    if (NODE_NDIM(node) == 0){
        NError_RaiseError(NError_IndexError, "Cannot slice a 0-dimensional array.");
        return NULL;
    }

    if (num_slices > NODE_NDIM(node)){
        NError_RaiseError(NError_IndexError, 
            "Too many slices: got %d slices for array with %d dimensions.", 
            num_slices, NODE_NDIM(node));
        return NULL;
    }

    // Create output node if not provided
    if (!nout){
        nout = Node_CopyWithReference(node);
        if (!nout) return NULL;
    }

    // Calculate total offset and new shapes/strides for all dimensions
    nr_intp total_offset = 0;
    
    for (int dim = 0; dim < num_slices; dim++){
        const Slice* slice_ptr = &slices[dim];
        
        // Skip NULL slices (equivalent to ':')
        if (slice_ptr == NULL || 
            (slice_ptr->start == 0 && slice_ptr->stop == 0 && slice_ptr->step == 0)){
            continue;
        }

        nr_intp start = slice_ptr->start;
        nr_intp stop = slice_ptr->stop;
        nr_intp step = slice_ptr->step;
        nr_intp dim_size = NODE_SHAPE(node)[dim];

        if (step == 0){
            NError_RaiseError(NError_ValueError, 
                "Slice step cannot be zero at dimension %d.", dim);
            return NULL;
        }

        // Normalize negative indices
        if (start < 0) start += dim_size;
        if (stop < 0) stop += dim_size;

        // Clamp to valid range
        if (start < 0) start = (step > 0) ? 0 : -1;
        if (start > dim_size) start = (step > 0) ? dim_size : dim_size;
        if (stop < 0) stop = (step > 0) ? 0 : -1;
        if (stop > dim_size) stop = (step > 0) ? dim_size : dim_size;

        // Calculate new dimension size
        nr_intp new_dim_size;
        if (step > 0) {
            new_dim_size = (stop > start) ? ((stop - start + step - 1) / step) : 0;
        } else {
            new_dim_size = (start > stop) ? ((start - stop - step - 1) / (-step)) : 0;
        }

        // Accumulate offset for this dimension
        total_offset += start * NODE_STRIDES(node)[dim];
        
        // Update shape and stride for this dimension
        NODE_SHAPE(nout)[dim] = new_dim_size;
        NODE_STRIDES(nout)[dim] = NODE_STRIDES(node)[dim] * step;
    }

    // Apply the total offset to the data pointer
    nout->data = (char*)node->data + total_offset;

    // Set base reference to source node and increment refcount
    if (nout->base != node) {
        nout->base = (Node*)node;
        ((Node*)node)->ref_count++;
    }

    // Mark as not owning data and update flags
    NR_RMVFLG(nout->flags, NR_NODE_OWNDATA);
    NR_SETFLG(nout->flags, NR_NODE_STRIDED);
    NR_RMVFLG(nout->flags, NR_NODE_CONTIGUOUS);

    return nout;
}


NR_PUBLIC Node*
Node_BooleanMask(const Node* node, const Node* bool_mask){
    if (!node || !bool_mask){
        NError_RaiseError(NError_ValueError, "Node and boolean mask cannot be NULL.");
        return NULL;
    }

    if (NODE_DTYPE(bool_mask) != NR_BOOL){
        NError_RaiseError(NError_ValueError, 
            "Boolean mask has invalid data type. Expected boolean type.");
        return NULL;
    }

    // Both must have the same shape
    if (!Node_SameShape(node, bool_mask)){
        NError_RaiseError(NError_ValueError, 
            "Node and boolean mask must have the same shape.");
        return NULL;
    }

    // Count number of true elements in the boolean mask
    nr_intp nitems = Node_NItems(node);
    nr_intp true_count = 0;
    
    if (NODE_IS_CONTIGUOUS(bool_mask)){
        // Fast path for contiguous boolean mask
        char* mask_data = (char*)NODE_DATA(bool_mask);
        for (nr_intp i = 0; i < nitems; i++){
            if (mask_data[i]) true_count++;
        }
    } else {
        // General case for strided boolean mask
        NIter it;
        NIter_FromNode(&it, bool_mask, NITER_MODE_STRIDED);
        NIter_ITER(&it);
        while (NIter_NOTDONE(&it)){
            if (*(char*)NIter_ITEM(&it)) true_count++;
            NIter_NEXT(&it);
        }
    }


    // Create new node for the result
    Node* result = Node_NewEmpty(1, &true_count, node->dtype.dtype);
    if (!result){
        NError_RaiseMemoryError();
        return NULL;
    }


    // Fill the new node with selected elements
    nr_intp bsize = NODE_ITEMSIZE(node);
    char* src_data = (char*)NODE_DATA(node);
    char* dst_data = (char*)NODE_DATA(result);

    if (NODE_IS_CONTIGUOUS(bool_mask)){
        // Fast path for contiguous boolean mask
        char* mask_data = (char*)NODE_DATA(bool_mask);
        for (nr_intp i = 0; i < nitems; i++){
            if (mask_data[i]){
                memcpy(dst_data, src_data, bsize);
                dst_data += bsize;
            }
            src_data += bsize;
        }
    } else {
        // General case for strided boolean mask
        NIter it;
        NIter_FromNode(&it, bool_mask, NITER_MODE_STRIDED);
        NIter_ITER(&it);
        while (NIter_NOTDONE(&it)){
            if (*(char*)NIter_ITEM(&it)){
                memcpy(dst_data, src_data, bsize);
                dst_data += bsize;
            }
            NIter_NEXT(&it);
        }
    }

    return result;
}


/* ========================================
   NArray-based indexing implementations
   ======================================== */

NR_PUBLIC Node*
Node_IndexWithIntArray(const Node* node, const NArray* indices, int axis){
    if (!node || !indices){
        NError_RaiseError(NError_ValueError, "Node and indices cannot be NULL.");
        return NULL;
    }

    if (NARRAY_DTYPE(indices) != NR_INT64){
        NError_RaiseError(NError_TypeError, 
            "Indices array must be of type NR_INT64 (nr_intp).");
        return NULL;
    }

    // Handle scalar node
    if (NODE_NDIM(node) == 0){
        NError_RaiseError(NError_IndexError, 
            "Cannot index a 0-dimensional array.");
        return NULL;
    }

    // Normalize axis
    if (axis < 0){
        axis += NODE_NDIM(node);
    }

    if (axis < 0 || axis >= NODE_NDIM(node)){
        NError_RaiseError(NError_IndexError, 
            "Axis %d out of bounds for array with %d dimensions.", 
            axis, NODE_NDIM(node));
        return NULL;
    }

    nr_intp num_indices = NARRAY_SIZE(indices);
    nr_intp axis_size = NODE_SHAPE(node)[axis];

    // Calculate output shape
    int out_ndim = NODE_NDIM(node) + NARRAY_NDIM(indices) - 1;
    nr_intp out_shape[NR_NODE_MAX_NDIM];
    
    // Copy dimensions before axis
    int out_idx = 0;
    for (int i = 0; i < axis; i++){
        out_shape[out_idx++] = NODE_SHAPE(node)[i];
    }
    
    // Add index array dimensions
    for (int i = 0; i < NARRAY_NDIM(indices); i++){
        out_shape[out_idx++] = NARRAY_SHAPE(indices)[i];
    }
    
    // Copy dimensions after axis
    for (int i = axis + 1; i < NODE_NDIM(node); i++){
        out_shape[out_idx++] = NODE_SHAPE(node)[i];
    }

    // Create output node
    Node* result = Node_NewEmpty(out_ndim, out_shape, NODE_DTYPE(node));
    if (!result){
        return NULL;
    }

    // Get pointers to index data
    nr_intp* index_data = (nr_intp*)NARRAY_DATA(indices);
    
    nr_intp itemsize = NODE_ITEMSIZE(node);
    nr_intp axis_stride = NODE_STRIDES(node)[axis];
    
    // Calculate size of sub-arrays we're copying
    nr_intp sub_size = 1;
    for (int i = axis + 1; i < NODE_NDIM(node); i++){
        sub_size *= NODE_SHAPE(node)[i];
    }
    nr_intp sub_bytes = sub_size * itemsize;

    // Calculate outer iterations (before axis)
    nr_intp outer_size = 1;
    for (int i = 0; i < axis; i++){
        outer_size *= NODE_SHAPE(node)[i];
    }

    char* dst_ptr = (char*)NODE_DATA(result);
    char* src_base = (char*)NODE_DATA(node);

    // Iterate through all combinations
    for (nr_intp outer = 0; outer < outer_size; outer++){
        // Calculate offset for this outer iteration
        nr_intp outer_offset = 0;
        nr_intp temp = outer;
        for (int i = axis - 1; i >= 0; i--){
            nr_intp coord = temp % NODE_SHAPE(node)[i];
            outer_offset += coord * NODE_STRIDES(node)[i];
            temp /= NODE_SHAPE(node)[i];
        }

        // Iterate through indices
        for (nr_intp idx = 0; idx < num_indices; idx++){
            nr_intp index = index_data[idx];
            
            // Handle negative indices
            if (index < 0){
                index += axis_size;
            }

            // Check bounds
            if (index < 0 || index >= axis_size){
                NError_RaiseError(NError_IndexError, 
                    "Index %lld out of bounds for axis %d with size %lld.",
                    (long long)index_data[idx], axis, (long long)axis_size);
                Node_Free(result);
                return NULL;
            }

            // Calculate source offset
            nr_intp src_offset = outer_offset + index * axis_stride;
            char* src_ptr = src_base + src_offset;

            // Copy sub-array
            memcpy(dst_ptr, src_ptr, sub_bytes);
            dst_ptr += sub_bytes;
        }
    }

    return result;
}


NR_PUBLIC Node*
Node_IndexWithBooleanArray(const Node* node, const NArray* bool_mask){
    if (!node || !bool_mask){
        NError_RaiseError(NError_ValueError, "Node and boolean mask cannot be NULL.");
        return NULL;
    }

    if (NARRAY_DTYPE(bool_mask) != NR_BOOL){
        NError_RaiseError(NError_ValueError, 
            "Boolean mask has invalid data type. Expected boolean type.");
        return NULL;
    }

    // Check that shapes match
    if (NARRAY_NDIM(bool_mask) != NODE_NDIM(node)){
        NError_RaiseError(NError_ValueError, 
            "Boolean mask must have same number of dimensions as node. Got %d, expected %d.",
            NARRAY_NDIM(bool_mask), NODE_NDIM(node));
        return NULL;
    }

    for (int i = 0; i < NODE_NDIM(node); i++){
        if (NARRAY_SHAPE(bool_mask)[i] != NODE_SHAPE(node)[i]){
            NError_RaiseError(NError_ValueError, 
                "Boolean mask shape mismatch at dimension %d. Got %lld, expected %lld.",
                i, (long long)NARRAY_SHAPE(bool_mask)[i], (long long)NODE_SHAPE(node)[i]);
            return NULL;
        }
    }

    // Count number of true elements in the boolean mask
    nr_intp true_count = 0;
    nr_bool* mask_data = (nr_bool*)NARRAY_DATA(bool_mask);
    
    if (NArray_IsContiguous(bool_mask)){
        // Fast path for contiguous mask
        for (nr_intp i = 0; i < NARRAY_SIZE(bool_mask); i++){
            if (mask_data[i]) true_count++;
        }
    } else {
        // Strided mask - iterate properly
        for (nr_intp i = 0; i < NARRAY_SIZE(bool_mask); i++){
            nr_intp coords[NR_NODE_MAX_NDIM];
            nr_intp temp = i;
            for (int j = NARRAY_NDIM(bool_mask) - 1; j >= 0; j--){
                coords[j] = temp % NARRAY_SHAPE(bool_mask)[j];
                temp /= NARRAY_SHAPE(bool_mask)[j];
            }
            
            nr_bool* mask_item = (nr_bool*)NArray_GetItem(bool_mask, coords);
            if (mask_item && *mask_item) true_count++;
        }
    }

    // Create result node (1D array with true_count elements)
    Node* result = Node_NewEmpty(1, &true_count, NODE_DTYPE(node));
    if (!result){
        return NULL;
    }

    // Fill result with selected elements
    nr_intp itemsize = NODE_ITEMSIZE(node);
    char* dst_data = (char*)NODE_DATA(result);
    nr_intp dst_idx = 0;

    if (NODE_IS_CONTIGUOUS(node) && NArray_IsContiguous(bool_mask)){
        // Fast path: both contiguous
        char* src_data = (char*)NODE_DATA(node);
        for (nr_intp i = 0; i < NARRAY_SIZE(bool_mask); i++){
            if (mask_data[i]){
                memcpy(dst_data + dst_idx * itemsize, src_data + i * itemsize, itemsize);
                dst_idx++;
            }
        }
    } else {
        // General case: iterate through elements
        for (nr_intp i = 0; i < NARRAY_SIZE(bool_mask); i++){
            nr_intp coords[NR_NODE_MAX_NDIM];
            nr_intp temp = i;
            for (int j = NARRAY_NDIM(bool_mask) - 1; j >= 0; j--){
                coords[j] = temp % NARRAY_SHAPE(bool_mask)[j];
                temp /= NARRAY_SHAPE(bool_mask)[j];
            }
            
            nr_bool* mask_item = (nr_bool*)NArray_GetItem(bool_mask, coords);
            if (mask_item && *mask_item){
                // Calculate offset into node data
                nr_intp offset = 0;
                for (int j = 0; j < NODE_NDIM(node); j++){
                    offset += coords[j] * NODE_STRIDES(node)[j];
                }
                
                char* src_ptr = (char*)NODE_DATA(node) + offset;
                memcpy(dst_data + dst_idx * itemsize, src_ptr, itemsize);
                dst_idx++;
            }
        }
    }

    return result;
}


NR_PUBLIC Node*
Node_AdvancedIndex(const Node* node, const NArray** indices, int num_indices, const int* axes){
    if (!node || !indices){
        NError_RaiseError(NError_ValueError, "Node and indices cannot be NULL.");
        return NULL;
    }

    if (num_indices <= 0 || num_indices > NODE_NDIM(node)){
        NError_RaiseError(NError_IndexError, 
            "Invalid number of indices: %d for array with %d dimensions.",
            num_indices, NODE_NDIM(node));
        return NULL;
    }

    // Validate all indices are integer type and get broadcast shape
    int broadcast_ndim = 0;
    nr_intp broadcast_shape[NR_NODE_MAX_NDIM];
    
    for (int i = 0; i < num_indices; i++){
        if (!indices[i]){
            NError_RaiseError(NError_ValueError, "Index array %d is NULL.", i);
            return NULL;
        }
        
        if (NARRAY_DTYPE(indices[i]) != NR_INT64){
            NError_RaiseError(NError_TypeError, 
                "Index array %d must be of type NR_INT64.", i);
            return NULL;
        }
        
        // Update broadcast shape
        if (i == 0){
            broadcast_ndim = NARRAY_NDIM(indices[i]);
            memcpy(broadcast_shape, NARRAY_SHAPE(indices[i]), 
                   broadcast_ndim * sizeof(nr_intp));
        } else {
            // Simple broadcast check (arrays must have same shape for now)
            if (NARRAY_NDIM(indices[i]) != broadcast_ndim){
                NError_RaiseError(NError_ValueError, 
                    "All index arrays must have the same number of dimensions for broadcasting.");
                return NULL;
            }
            for (int j = 0; j < broadcast_ndim; j++){
                if (NARRAY_SHAPE(indices[i])[j] != broadcast_shape[j]){
                    NError_RaiseError(NError_ValueError, 
                        "Index arrays have incompatible shapes for broadcasting.");
                    return NULL;
                }
            }
        }
    }

    // Default axes if not provided
    int default_axes[NR_NODE_MAX_NDIM];
    if (!axes){
        for (int i = 0; i < num_indices; i++){
            default_axes[i] = i;
        }
        axes = default_axes;
    }

    // Calculate output shape
    nr_intp total_indices = 1;
    for (int i = 0; i < broadcast_ndim; i++){
        total_indices *= broadcast_shape[i];
    }

    // Output shape = broadcast_shape + remaining node dimensions
    int out_ndim = broadcast_ndim;
    nr_intp out_shape[NR_NODE_MAX_NDIM];
    memcpy(out_shape, broadcast_shape, broadcast_ndim * sizeof(nr_intp));
    
    for (int i = 0; i < NODE_NDIM(node); i++){
        int is_indexed = 0;
        for (int j = 0; j < num_indices; j++){
            if (axes[j] == i){
                is_indexed = 1;
                break;
            }
        }
        if (!is_indexed){
            out_shape[out_ndim++] = NODE_SHAPE(node)[i];
        }
    }

    // Create output node
    Node* result = Node_NewEmpty(out_ndim, out_shape, NODE_DTYPE(node));
    if (!result){
        return NULL;
    }

    // Calculate sub-element size (elements in non-indexed dimensions)
    nr_intp itemsize = NODE_ITEMSIZE(node);
    nr_intp sub_size = 1;
    for (int i = 0; i < NODE_NDIM(node); i++){
        int is_indexed = 0;
        for (int j = 0; j < num_indices; j++){
            if (axes[j] == i){
                is_indexed = 1;
                break;
            }
        }
        if (!is_indexed){
            sub_size *= NODE_SHAPE(node)[i];
        }
    }
    nr_intp sub_bytes = sub_size * itemsize;

    char* dst_ptr = (char*)NODE_DATA(result);
    char* src_base = (char*)NODE_DATA(node);

    // Iterate through all index combinations
    for (nr_intp idx = 0; idx < total_indices; idx++){
        // Get coordinates in the broadcast array
        nr_intp coords[NR_NODE_MAX_NDIM];
        nr_intp temp = idx;
        for (int i = broadcast_ndim - 1; i >= 0; i--){
            coords[i] = temp % broadcast_shape[i];
            temp /= broadcast_shape[i];
        }

        // Get the index values for each axis
        nr_intp node_coords[NR_NODE_MAX_NDIM] = {0};
        for (int i = 0; i < num_indices; i++){
            nr_intp* idx_ptr = (nr_intp*)NArray_GetItem(indices[i], coords);
            if (!idx_ptr){
                Node_Free(result);
                return NULL;
            }
            
            nr_intp index = *idx_ptr;
            int axis = axes[i];
            
            // Handle negative indices
            if (index < 0){
                index += NODE_SHAPE(node)[axis];
            }
            
            // Check bounds
            if (index < 0 || index >= NODE_SHAPE(node)[axis]){
                NError_RaiseError(NError_IndexError, 
                    "Index %lld out of bounds for axis %d with size %lld.",
                    (long long)*idx_ptr, axis, (long long)NODE_SHAPE(node)[axis]);
                Node_Free(result);
                return NULL;
            }
            
            node_coords[axis] = index;
        }

        // Calculate source offset
        nr_intp src_offset = 0;
        for (int i = 0; i < NODE_NDIM(node); i++){
            src_offset += node_coords[i] * NODE_STRIDES(node)[i];
        }

        // Copy data
        memcpy(dst_ptr, src_base + src_offset, sub_bytes);
        dst_ptr += sub_bytes;
    }

    return result;
}


NR_PUBLIC Node*
Node_Take(const Node* node, const NArray* indices, int axis, int mode){
    if (!node || !indices){
        NError_RaiseError(NError_ValueError, "Node and indices cannot be NULL.");
        return NULL;
    }

    if (NARRAY_DTYPE(indices) != NR_INT64){
        NError_RaiseError(NError_TypeError, 
            "Indices array must be of type NR_INT64.");
        return NULL;
    }

    // Handle scalar node
    if (NODE_NDIM(node) == 0){
        NError_RaiseError(NError_IndexError, "Cannot take from 0-dimensional array.");
        return NULL;
    }

    // Normalize axis
    if (axis < 0){
        axis += NODE_NDIM(node);
    }

    if (axis < 0 || axis >= NODE_NDIM(node)){
        NError_RaiseError(NError_IndexError, 
            "Axis %d out of bounds for array with %d dimensions.", 
            axis, NODE_NDIM(node));
        return NULL;
    }

    nr_intp axis_size = NODE_SHAPE(node)[axis];
    nr_intp num_indices = NARRAY_SIZE(indices);
    nr_intp* index_data = (nr_intp*)NARRAY_DATA(indices);

    // Calculate output shape
    nr_intp out_shape[NR_NODE_MAX_NDIM];
    for (int i = 0; i < NODE_NDIM(node); i++){
        if (i == axis){
            out_shape[i] = num_indices;
        } else {
            out_shape[i] = NODE_SHAPE(node)[i];
        }
    }

    // Create output node
    Node* result = Node_NewEmpty(NODE_NDIM(node), out_shape, NODE_DTYPE(node));
    if (!result){
        return NULL;
    }

    nr_intp itemsize = NODE_ITEMSIZE(node);
    nr_intp axis_stride = NODE_STRIDES(node)[axis];

    // Calculate size of sub-arrays
    nr_intp sub_size = 1;
    for (int i = axis + 1; i < NODE_NDIM(node); i++){
        sub_size *= NODE_SHAPE(node)[i];
    }
    nr_intp sub_bytes = sub_size * itemsize;

    // Calculate outer size
    nr_intp outer_size = 1;
    for (int i = 0; i < axis; i++){
        outer_size *= NODE_SHAPE(node)[i];
    }

    char* dst_ptr = (char*)NODE_DATA(result);
    char* src_base = (char*)NODE_DATA(node);

    // Process indices with mode handling
    for (nr_intp outer = 0; outer < outer_size; outer++){
        nr_intp outer_offset = 0;
        nr_intp temp = outer;
        for (int i = axis - 1; i >= 0; i--){
            nr_intp coord = temp % NODE_SHAPE(node)[i];
            outer_offset += coord * NODE_STRIDES(node)[i];
            temp /= NODE_SHAPE(node)[i];
        }

        for (nr_intp idx = 0; idx < num_indices; idx++){
            nr_intp index = index_data[idx];

            // Apply mode
            switch (mode){
                case 0: // error mode
                    if (index < 0) index += axis_size;
                    if (index < 0 || index >= axis_size){
                        NError_RaiseError(NError_IndexError, 
                            "Index %lld out of bounds for axis %d with size %lld.",
                            (long long)index_data[idx], axis, (long long)axis_size);
                        Node_Free(result);
                        return NULL;
                    }
                    break;
                
                case 1: // wrap mode
                    index = ((index % axis_size) + axis_size) % axis_size;
                    break;
                
                case 2: // clip mode
                    if (index < 0) index = 0;
                    if (index >= axis_size) index = axis_size - 1;
                    break;
                
                default:
                    NError_RaiseError(NError_ValueError, 
                        "Invalid mode: %d. Must be 0 (error), 1 (wrap), or 2 (clip).", mode);
                    Node_Free(result);
                    return NULL;
            }

            nr_intp src_offset = outer_offset + index * axis_stride;
            memcpy(dst_ptr, src_base + src_offset, sub_bytes);
            dst_ptr += sub_bytes;
        }
    }

    return result;
}


NR_PUBLIC int
Node_Put(Node* node, const NArray* indices, const Node* values, int mode){
    if (!node || !indices || !values){
        NError_RaiseError(NError_ValueError, "Node, indices, and values cannot be NULL.");
        return -1;
    }

    if (NARRAY_DTYPE(indices) != NR_INT64){
        NError_RaiseError(NError_TypeError, 
            "Indices array must be of type NR_INT64.");
        return -1;
    }

    if (NODE_DTYPE(node) != NODE_DTYPE(values)){
        NError_RaiseError(NError_TypeError, 
            "Node and values must have the same dtype.");
        return -1;
    }

    nr_intp total_size = Node_NItems(node);
    nr_intp num_indices = NARRAY_SIZE(indices);
    nr_intp* index_data = (nr_intp*)NARRAY_DATA(indices);

    // Values should have same size as indices or be broadcastable
    nr_intp num_values = Node_NItems(values);
    if (num_values != num_indices && num_values != 1){
        NError_RaiseError(NError_ValueError, 
            "Values array size (%lld) must match indices size (%lld) or be 1 for broadcasting.",
            (long long)num_values, (long long)num_indices);
        return -1;
    }

    nr_intp itemsize = NODE_ITEMSIZE(node);
    char* node_data = (char*)NODE_DATA(node);
    char* values_data = (char*)NODE_DATA(values);

    for (nr_intp i = 0; i < num_indices; i++){
        nr_intp index = index_data[i];

        // Apply mode
        switch (mode){
            case 0: // error mode
                if (index < 0) index += total_size;
                if (index < 0 || index >= total_size){
                    NError_RaiseError(NError_IndexError, 
                        "Index %lld out of bounds for flattened array with size %lld.",
                        (long long)index_data[i], (long long)total_size);
                    return -1;
                }
                break;
            
            case 1: // wrap mode
                index = ((index % total_size) + total_size) % total_size;
                break;
            
            case 2: // clip mode
                if (index < 0) index = 0;
                if (index >= total_size) index = total_size - 1;
                break;
            
            default:
                NError_RaiseError(NError_ValueError, 
                    "Invalid mode: %d. Must be 0 (error), 1 (wrap), or 2 (clip).", mode);
                return -1;
        }

        // Calculate offset in flattened array
        nr_intp offset = index * itemsize;
        
        // Get value to put (broadcast if only one value)
        char* value_ptr = (num_values == 1) ? values_data : (values_data + i * itemsize);
        
        // Copy value
        memcpy(node_data + offset, value_ptr, itemsize);
    }

    return 0;
}