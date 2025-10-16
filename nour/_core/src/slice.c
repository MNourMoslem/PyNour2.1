#include "slice.h"
#include "nerror.h"
#include "node_core.h"
#include "niter.h"

int 
Node_Slice(Node* node, const Slice slice, int dim){
    if (NODE_NDIM(node) == 0 || dim < 0 || dim >= NODE_NDIM(node)){
        NError_RaiseError(NError_IndexError, 
            "Invalid dimension for slicing. got %d for array with %d dimensions.", 
            dim, NODE_NDIM(node));
        return -1;
    }

    nr_intp start = slice.start;
    nr_intp stop = slice.stop;
    nr_intp step = slice.step;
    nr_intp dim_size = NODE_SHAPE(node)[dim];

    if (step == 0){
        NError_RaiseError(NError_ValueError, "Slice step cannot be zero.");
        return -1;
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

    // Calculate the offset for the new data pointer
    nr_intp offset = start * NODE_STRIDES(node)[dim];
    node->data = (char*)node->data + offset;
    
    // Update shape and strides
    NODE_SHAPE(node)[dim] = new_dim_size;
    NODE_STRIDES(node)[dim] *= step;

    // Mark as strided if not already
    if (!NODE_IS_STRIDED(node)){
        NR_SETFLG(node->flags, NR_NODE_STRIDED);
        NR_RMVFLG(node->flags, NR_NODE_CONTIGUOUS);
    }

    return 0;
}


int 
Node_MultiSlice(Node* node, const Slice* slices, int num_slices){
    if (node == NULL || slices == NULL){
        NError_RaiseError(NError_ValueError, "Node and slices cannot be NULL.");
        return -1;
    }

    if (NODE_NDIM(node) == 0){
        NError_RaiseError(NError_IndexError, "Cannot slice a 0-dimensional array.");
        return -1;
    }

    if (num_slices > NODE_NDIM(node)){
        NError_RaiseError(NError_IndexError, 
            "Too many slices: got %d slices for array with %d dimensions.", 
            num_slices, NODE_NDIM(node));
        return -1;
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
            return -1;
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
        NODE_SHAPE(node)[dim] = new_dim_size;
        NODE_STRIDES(node)[dim] *= step;
    }

    // Apply the total offset to the data pointer
    node->data = (char*)node->data + total_offset;

    // Mark as strided if not already
    if (!NODE_IS_STRIDED(node)){
        NR_SETFLG(node->flags, NR_NODE_STRIDED);
        NR_RMVFLG(node->flags, NR_NODE_CONTIGUOUS);
    }

    return 0;
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