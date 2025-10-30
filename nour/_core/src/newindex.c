#include "newindex.h"
#include "nerror.h"
#include "node_core.h"
#include "free.h"
#include "niter.h"
#include "niter.h"


#define NINDEXRULESET_MAX_NUM_ERR(rs) \
    if ((rs)->num_rules >= NINDEXRULESET_MAX_RULES){                                   \
        NError_RaiseError(NError_IndexError,                                           \
            "Exceeded maximum number of index rules (%d).", NINDEXRULESET_MAX_RULES);  \
        return -1;                                                                     \
    }

NR_PUBLIC NIndexRuleSet
NIndexRuleSet_New(Node* base_array){
    NIndexRuleSet rs;
    NIndexRuleSet_NUM_RULES(&rs) = 0;
    NIndexRuleSet_BASE_ARRAY(&rs) = base_array;
    return rs;
}


NR_PUBLIC int
NIndexRuleSet_AddInt(NIndexRuleSet* rs, nr_intp index){
    NINDEXRULESET_MAX_NUM_ERR(rs);
    
    nr_intp current_dim = NIndexRuleSet_NUM_RULES(rs);
    Node* base_array = NIndexRuleSet_BASE_ARRAY(rs);
    
    // Check if we're trying to index beyond available dimensions
    if (current_dim >= NODE_NDIM(base_array)) {
        NError_RaiseError(NError_IndexError, 
            "Too many indices for array: array has %d dimensions but %lld indices given",
            NODE_NDIM(base_array), (long long)(current_dim + 1));
        return -1;
    }
    
    nr_intp dim_size = NODE_SHAPE(base_array)[current_dim];
    
    // if index is negative, convert to positive
    if (index < 0){
        index += dim_size;
    }
    
    // Bounds checking
    if (index < 0 || index >= dim_size){
        NError_RaiseError(NError_IndexError, 
            "Index %lld is out of bounds for dimension %lld with size %lld", 
            (long long)index, (long long)current_dim, (long long)dim_size);
        return -1;
    }

    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    NIndexRule_TYPE(rule) = NIndexRuleType_Int;
    NIndexRule_DATA(rule).int_data.index = index;

    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddSlice(NIndexRuleSet* rs, nr_intp start, nr_intp stop, nr_intp step){
    NINDEXRULESET_MAX_NUM_ERR(rs);

    // Check possible error cases for slice parameters
    if (step == 0){
        NError_RaiseError(NError_ValueError, "Slice step cannot be zero.");
        return -1;
    }

    nr_intp current_dim = NIndexRuleSet_NUM_RULES(rs);
    Node* base_array = NIndexRuleSet_BASE_ARRAY(rs);
    
    // Check if we're trying to index beyond available dimensions
    if (current_dim >= NODE_NDIM(base_array)) {
        NError_RaiseError(NError_IndexError, 
            "Too many indices for array: array has %d dimensions but %lld indices given",
            NODE_NDIM(base_array), (long long)(current_dim + 1));
        return -1;
    }
    
    nr_intp dim_size = NODE_SHAPE(base_array)[current_dim];

    // Handle negative indices
    if (start < 0) start += dim_size;
    if (stop < 0) stop += dim_size;

    // Clamp bounds to valid range
    start = NR_MAX(0, NR_MIN(start, dim_size - 1));
    stop = NR_MAX(0, NR_MIN(stop, dim_size));

    // Validate slice range
    if (step > 0) {
        if (start >= stop) {
            // Empty slice - this is valid, will result in 0-length dimension
            start = 0;
            stop = 0;
        }
    } else {
        if (start <= stop) {
            // Empty slice for negative step
            start = 0;
            stop = 0;
        }
    }

    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    NIndexRule_TYPE(rule) = NIndexRuleType_Slice;
    NIndexSlice* slice = &NIndexRule_DATA_AS_SLICE(rule);

    NIndexSlice_START(*slice) = start;
    NIndexSlice_STOP(*slice) = stop;
    NIndexSlice_STEP(*slice) = step;

    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddNewAxis(NIndexRuleSet* rs){
    NINDEXRULESET_MAX_NUM_ERR(rs);

    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    NIndexRule_TYPE(rule) = NIndexRuleType_NewAxis;
    // NewAxis doesn't need any data, just the type

    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddEllipsis(NIndexRuleSet* rs){
    NINDEXRULESET_MAX_NUM_ERR(rs);

    // Check if ellipsis already exists in the rule set
    for (nr_intp i = 0; i < NIndexRuleSet_NUM_RULES(rs); i++){
        NIndexRule* existing_rule = &NIndexRuleSet_RULES(rs)[i];
        if (NIndexRule_TYPE(existing_rule) == NIndexRuleType_Ellipsis){
            NError_RaiseError(NError_IndexError, 
                "Multiple ellipses (...) are not allowed in a single indexing operation");
            return -1;
        }
    }

    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    NIndexRule_TYPE(rule) = NIndexRuleType_Ellipsis;
    // Ellipsis doesn't need any data, just the type

    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddNode(NIndexRuleSet* rs, Node* index_node){
    NINDEXRULESET_MAX_NUM_ERR(rs);

    if (!index_node) {
        NError_RaiseError(NError_ValueError, "Index node cannot be NULL");
        return -1;
    }

    // Validate that the node contains integer indices
    if (NDtype_GetDtypeType(NODE_DTYPE(index_node)) != NDTYPE_INT) {
        char dtype_str[50];
        NDtype_AsString(NODE_DTYPE(index_node), dtype_str);
        NError_RaiseError(NError_TypeError, 
            "Index arrays must be of integer type, got %s", dtype_str);
        return -1;
    }

    nr_intp current_dim = NIndexRuleSet_NUM_RULES(rs);
    Node* base_array = NIndexRuleSet_BASE_ARRAY(rs);
    
    // Check if we're trying to index beyond available dimensions
    if (current_dim >= NODE_NDIM(base_array)) {
        NError_RaiseError(NError_IndexError, 
            "Too many indices for array: array has %d dimensions but %lld indices given",
            NODE_NDIM(base_array), (long long)(current_dim + 1));
        return -1;
    }

    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    NIndexRule_TYPE(rule) = NIndexRuleType_Node;
    NIndexRule_DATA(rule).node_data.node = index_node;

    // Increment reference count since we're storing the node
    index_node->ref_count++;

    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddBoolNode(NIndexRuleSet* rs, Node* bool_node){
    NINDEXRULESET_MAX_NUM_ERR(rs);

    if (!bool_node) {
        NError_RaiseError(NError_ValueError, "Boolean node cannot be NULL");
        return -1;
    }

    // Validate that the node contains boolean values
    if (NODE_DTYPE(bool_node) != NR_BOOL) {
        char dtype_str[50];
        NDtype_AsString(NODE_DTYPE(bool_node), dtype_str);
        NError_RaiseError(NError_TypeError, 
            "Boolean index arrays must be of boolean type, got %s", dtype_str);
        return -1;
    }

    // Boolean indexing consumes all remaining dimensions
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    NIndexRule_TYPE(rule) = NIndexRuleType_BoolNode;
    NIndexRule_DATA(rule).node_data.node = bool_node;

    // Increment reference count since we're storing the node
    bool_node->ref_count++;

    return 0;
}

NR_PUBLIC void
NIndexRuleSet_Cleanup(NIndexRuleSet* rs){
    if (!rs) return;

    // Decrement reference counts for any stored nodes
    for (nr_intp i = 0; i < NIndexRuleSet_NUM_RULES(rs); i++){
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        if (NIndexRule_TYPE(rule) == NIndexRuleType_Node || 
            NIndexRule_TYPE(rule) == NIndexRuleType_BoolNode) {
            Node* node = NIndexNode_NODE(NIndexRule_DATA_AS_NODE(rule));
            if (node) {
                node->ref_count--;
                // TODO: Add proper node cleanup/freeing logic if ref_count reaches 0
            }
        }
    }

    // Reset the rule set
    NIndexRuleSet_NUM_RULES(rs) = 0;
    NIndexRuleSet_BASE_ARRAY(rs) = NULL;
}

NR_PUBLIC int
NIndexRuleSet_AddFullSlice(NIndexRuleSet* rs){
    // Add a full slice (:) which selects the entire dimension
    nr_intp current_dim = NIndexRuleSet_NUM_RULES(rs);
    Node* base_array = NIndexRuleSet_BASE_ARRAY(rs);
    
    if (current_dim >= NODE_NDIM(base_array)) {
        NError_RaiseError(NError_IndexError, 
            "Too many indices for array: array has %d dimensions but %lld indices given",
            NODE_NDIM(base_array), (long long)(current_dim + 1));
        return -1;
    }
    
    nr_intp dim_size = NODE_SHAPE(base_array)[current_dim];
    return NIndexRuleSet_AddSlice(rs, 0, dim_size, 1);
}

NR_PUBLIC int
NIndexRuleSet_AddRange(NIndexRuleSet* rs, nr_intp start, nr_intp stop){
    // Add a range slice with step=1
    return NIndexRuleSet_AddSlice(rs, start, stop, 1);
}

NR_STATIC_INLINE int
is_node_in_indices(NIndexRuleSet* rs){
    for (nr_intp i = 0; i < NIndexRuleSet_NUM_RULES(rs); i++){
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        if (NIndexRule_TYPE(rule) == NIndexRuleType_Node){
            return 1;
        }
    }
    return 0;
}

NR_STATIC_INLINE int
is_copy_required(NIndexRuleSet* rs){
    // The only case where copy is not required is when all indices are slices
    for (nr_intp i = 0; i < NIndexRuleSet_NUM_RULES(rs); i++){
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        if (NIndexRule_TYPE(rule) != NIndexRuleType_Slice){
            return 1;
        }
    }
    return 0;
}


NR_STATIC_INLINE Node*
node_index_handle_node_indices(NIndexRuleSet* rs){
    // TODO: Implement node-based indexing (fancy indexing)
    NError_RaiseError(NError_NotImplementedError, "Node-based indexing not yet implemented");
    return NULL;
}

NR_STATIC_INLINE Node*
node_index_handle_non_node_indices(NIndexRuleSet* rs){
    Node* base_node = NIndexRuleSet_BASE_ARRAY(rs);
    int ellipsis_found = 0;
    int copy_needed = is_copy_required(rs);  // Use the consistent copy logic
    int dim = 0;
    int tdim = 0;

    char* data_ptr = NODE_DATA(base_node);

    nr_intp tshape[NR_NODE_MAX_NDIM];
    nr_intp tstrides[NR_NODE_MAX_NDIM];
    // Check if any of the rules contain a Node (not implemented yet)
    for (nr_intp i = 0; i < NIndexRuleSet_NUM_RULES(rs); i++){
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        switch (NIndexRule_TYPE(rule))
        {
        case NIndexRuleType_Int:
            // copy_needed is already determined by is_copy_required()
            int index = NIndexInt_INDEX(NIndexRule_DATA_AS_INT(rule));
            data_ptr += NODE_STRIDES(base_node)[dim] * index;
            dim++;
            break;

        case NIndexRuleType_Slice:
            nr_intp start = NIndexSlice_START(NIndexRule_DATA_AS_SLICE(rule));
            nr_intp step = NIndexSlice_STEP(NIndexRule_DATA_AS_SLICE(rule));
            nr_intp stop = NIndexSlice_STOP(NIndexRule_DATA_AS_SLICE(rule));

            if (step == 0){
                NError_RaiseError(NError_ValueError, "Slice step cannot be zero.");
                return NULL;
            }

            data_ptr += NODE_STRIDES(base_node)[dim] * start;
            tstrides[tdim] = NODE_STRIDES(base_node)[dim] * step;
            
            // Calculate slice length properly for both positive and negative steps
            if (step > 0) {
                tshape[tdim] = (start >= stop) ? 0 : (stop - start + step - 1) / step;
            } else {
                tshape[tdim] = (start <= stop) ? 0 : (start - stop - step - 1) / (-step);
            }
            
            dim++;
            tdim++;
            break;

        case NIndexRuleType_NewAxis:
            // NewAxis doesn't consume a dimension from the base array
            // We need to compute a stride that doesn't interfere with data access
            nr_intp stride = (dim < NODE_NDIM(base_node)) ? NODE_STRIDES(base_node)[dim] : NODE_ITEMSIZE(base_node);
            tshape[tdim] = 1;
            tstrides[tdim] = stride;
            tdim++;
            // Note: dim is NOT incremented for NewAxis
            break;

        case NIndexRuleType_Ellipsis:
            if (ellipsis_found){
                NError_RaiseError(NError_IndexError, "Multiple ellipses found in indexing.");
                return NULL;
            }

            int remaining_dims;
            if (i == 0) { // means that ellipsis is at the beginning
                remaining_dims = NODE_NDIM(NIndexRuleSet_BASE_ARRAY(rs)) - (NIndexRuleSet_NUM_RULES(rs) - 1);
            } else if (i == NIndexRuleSet_NUM_RULES(rs) - 1){ // means ellipsis is at the end
                remaining_dims = NODE_NDIM(NIndexRuleSet_BASE_ARRAY(rs)) - (NIndexRuleSet_NUM_RULES(rs) - 1);
            } else {
                int dims_before = i;
                int dims_after = NIndexRuleSet_NUM_RULES(rs) - i - 1;
                int total_dims = NODE_NDIM(NIndexRuleSet_BASE_ARRAY(rs));
                remaining_dims = total_dims - dims_before - dims_after;
            }

            for (int j = 0; j < remaining_dims; j++){
                tstrides[tdim] = NODE_STRIDES(base_node)[dim];
                tshape[tdim] = NODE_SHAPE(base_node)[dim];
                dim++;
                tdim++;
           }

            ellipsis_found = 1;
            break;

        default:
            break;
        }
    }

    // Handle remaining dimensions that weren't explicitly indexed
    // These dimensions should be included as full slices
    while (dim < NODE_NDIM(base_node)) {
        tshape[tdim] = NODE_SHAPE(base_node)[dim];
        tstrides[tdim] = NODE_STRIDES(base_node)[dim];
        dim++;
        tdim++;
    }

    nr_intp nitems = 1;
    for (int i = 0; i < tdim; i++){
        nitems *= tshape[i];
    }

    if (copy_needed){
        nr_size_t bsize = NDtype_Size(NODE_DTYPE(base_node));
        char* data = malloc(nitems * bsize);
        if (!data){
            NError_RaiseMemoryError();
            return NULL;
        }

        NIter iter;
        NIter_New(&iter, data_ptr, tdim, tshape, tstrides, NITER_MODE_STRIDED);
        NIter_ITER(&iter);

        nr_intp offset = 0;
        while (NIter_NOTDONE(&iter)){
            memcpy(data + offset, NIter_ITEM(&iter), bsize);
            offset += bsize;
            NIter_NEXT(&iter);
        }

        Node* result_node = Node_New(data, 0, tdim, tshape, NODE_DTYPE(base_node));
        if (!result_node){
            free(data);
            NError_RaiseMemoryError();
            return NULL;
        }
        return result_node;
    }

    // if no copy is needed, create a new Node that shares data
    Node* result_node = Node_NewAdvanced(data_ptr, 0, tdim, tshape, tstrides, NODE_DTYPE(base_node), 0, NULL);
    if (!result_node){
        NError_RaiseMemoryError();
        return NULL;
    }
    _Node_Inherit(result_node, base_node);

    return result_node;
}

NR_PUBLIC Node*
Node_Index(NIndexRuleSet* rs){
    int node_in_indices = is_node_in_indices(rs);

    if (node_in_indices){
        return node_index_handle_node_indices(rs);
    }
    return node_index_handle_non_node_indices(rs);
}