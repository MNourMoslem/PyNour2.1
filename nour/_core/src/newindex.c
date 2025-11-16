#include "newindex.h"
#include "nerror.h"
#include "node_core.h"
#include "free.h"
#include "niter.h"
#include "niter.h"
#include "tc_methods.h"

#define NIndex_INT NR_INT64

#define HAS_INT 1
#define HAS_SLICE 2
#define HAS_NODE 4
#define HAS_ELLIPSIS 8
#define HAS_NEW_AXIS 16

#define NINDEXRULESET_MAX_NUM_ERR(rs) \
    if ((rs)->num_rules >= NINDEXRULESET_MAX_RULES){                                   \
        NError_RaiseError(NError_IndexError,                                           \
            "Exceeded maximum number of index rules (%d).", NINDEXRULESET_MAX_RULES);  \
        return -1;                                                                     \
    }

#define CHECK_INT_INDEX_BOUNDS(index, dim_size, current_dim) \
    if ((index) < 0 || (index) >= (dim_size)){                                         \
        NError_RaiseError(NError_IndexError,                                           \
            "Index %lld is out of bounds for dimension %lld with size %lld",           \
            (long long)(index), (long long)(current_dim), (long long)(dim_size));      \
        return -1;                                                                     \
    }


NR_PUBLIC nr_intp
NIndexSlice_Length(NIndexSlice* slice){
    nr_intp length = 0;
    if (slice){
        nr_intp start = NIndexSlice_START(*slice);
        nr_intp stop = NIndexSlice_STOP(*slice);
        nr_intp step = NIndexSlice_STEP(*slice);

        if (step > 0) {
            length = (stop - start + step - 1) / step;
        } else {
            length = (start - stop - step - 1) / (-step);
        }
    }
    return length;
}


NR_PUBLIC NIndexRuleSet
NIndexRuleSet_New(Node* base_array){
    NIndexRuleSet rs;
    NIndexRuleSet_NUM_RULES(&rs) = 0;
    return rs;
}


NR_PUBLIC int
NIndexRuleSet_AddInt(NIndexRuleSet* rs, nr_intp index){
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    NIndexRule_TYPE(rule) = NIndexRuleType_Int;
    NIndexData data = NIndexRule_DATA(rule);
    NIndexInt* int_data = &data.int_data;
    NIndexInt_INDEX(*int_data) = index;
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddSlice(NIndexRuleSet* rs, nr_intp start, nr_intp stop, nr_intp step){
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
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddEllipsis(NIndexRuleSet* rs){
    NINDEXRULESET_MAX_NUM_ERR(rs);
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    NIndexRule_TYPE(rule) = NIndexRuleType_Ellipsis;
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddNode(NIndexRuleSet* rs, Node* index_node){
    nr_intp current_dim = NIndexRuleSet_NUM_RULES(rs);
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    NIndexRule_TYPE(rule) = NIndexRuleType_Node;
    NIndexRule_DATA(rule).node_data.node = index_node;
    return 0;
}


NR_PUBLIC void
NIndexRuleSet_Cleanup(NIndexRuleSet* rs){
    if (!rs) return;
    rs->num_rules = 0;
}

NR_PUBLIC int
NIndexRuleSet_AddFullSlice(NIndexRuleSet* rs){
    return NIndexRuleSet_AddSlice(rs, 0, -1, 1);
}

NR_PUBLIC int
NIndexRuleSet_AddRange(NIndexRuleSet* rs, nr_intp start, nr_intp stop){
    return NIndexRuleSet_AddSlice(rs, start, stop, 1);
}

NR_STATIC_INLINE int
get_indexing_types(NIndexRuleSet* rs){
    int types = 0;
    for (nr_intp i = 0; i < NIndexRuleSet_NUM_RULES(rs); i++){
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        switch (NIndexRule_TYPE(rule))
        {
            case NIndexRuleType_Int:
                types |= HAS_INT;
                break;
            case NIndexRuleType_Slice:
                types |= HAS_SLICE;
                break;
            case NIndexRuleType_Node:
                types |= HAS_NODE;
                break;
            case NIndexRuleType_Ellipsis:
                types |= HAS_ELLIPSIS;
                break;
            case NIndexRuleType_NewAxis:
                types |= HAS_NEW_AXIS;
                break;
            default:
                break;
        }
    }
    return types;
}

NR_STATIC_INLINE int
NIndexEllipsis_Length(NIndexRuleSet* rs, int found_index){
    int num_rules = NIndexRuleSet_NUM_RULES(rs);
    if (found_index == 0 || found_index == num_rules - 1){ 
        return num_rules - 1;
    }

    int dims_before = found_index;
    int dims_after = num_rules - found_index - 1;
    return num_rules - dims_before - dims_after - 1;
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


NR_STATIC_INLINE Node*
node_index_handle_node_indices_only_nodes(NIndexRuleSet* rs){

}

NR_STATIC_INLINE Node*
node_index_handle_node_indices(Node* base_node, NIndexRuleSet* rs){
    char* data_ptr = NODE_DATA(base_node);
    int base_node_ndim = NODE_NDIM(base_node);

    Node* nodes[NINDEXRULESET_MAX_RULES];
    int num_nodes = 0;

    NIndexRule other_rules[NINDEXRULESET_MAX_RULES];
    int num_other_rules = 0;

    int keep_dim = 0;
    int new_dim = 0;
    int ellipsis_found = 0;

    for (int i = 0; i < NIndexRuleSet_NUM_RULES(rs); i++){
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        NIndexRuleType rule_type = NIndexRule_TYPE(rule);

        switch (rule_type)
        {
            case NIndexRuleType_Int:
                keep_dim++;
                other_rules[num_other_rules++] = *rule;
                break;
            case NIndexRuleType_Slice:
                keep_dim++;
                other_rules[num_other_rules++] = *rule;
                break;
            case NIndexRuleType_NewAxis:
                new_dim++;
                break;
            case NIndexRuleType_Ellipsis:
                if (ellipsis_found){
                    NError_RaiseError(NError_IndexError, "Multiple ellipses found in indexing.");
                    return NULL;
                }
                ellipsis_found = 1;

                int remaining_dims;
                if (i == 0 || i == NIndexRuleSet_NUM_RULES(rs) - 1){
                    // Ellipsis at the beginning
                    remaining_dims = base_node_ndim - (NIndexRuleSet_NUM_RULES(rs) - 1);
                }
                else{
                    int dims_before = i;
                    int dims_after = NIndexRuleSet_NUM_RULES(rs) - i - 1;
                    int total_dims = base_node_ndim;
                    remaining_dims = total_dims - dims_before - dims_after;
                }

                keep_dim += remaining_dims;
                other_rules[num_other_rules++] = *rule;
                break;

            case NIndexRuleType_Node:
                nodes[num_nodes++] = NIndexNode_NODE(NIndexRule_DATA_AS_NODE(rule));
        }
    }

    nr_intp bshape[NR_NODE_MAX_NDIM];
    int bndim = 0;
    if (NTools_BroadcastShapes(nodes, num_nodes, bshape, &bndim) != 0){
        return NULL;
    }
    int remaining_dims = base_node_ndim - bndim - keep_dim;
    return NULL;
}

NR_STATIC_INLINE Node*
node_index_handle_non_node_indices(Node* base_node, NIndexRuleSet* rs){
    int ellipsis_found = 0;
    int copy_needed = 0;  // Use the consistent copy logic
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
            copy_needed = 1;
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
            tshape[tdim] = NIndexSlice_Length(&NIndexRule_DATA_AS_SLICE(rule));
            
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
                remaining_dims = NODE_NDIM(base_node) - (NIndexRuleSet_NUM_RULES(rs) - 1);
            } else if (i == NIndexRuleSet_NUM_RULES(rs) - 1){ // means ellipsis is at the end
                remaining_dims = NODE_NDIM(base_node) - (NIndexRuleSet_NUM_RULES(rs) - 1);
            } else {
                int dims_before = i;
                int dims_after = NIndexRuleSet_NUM_RULES(rs) - i - 1;
                int total_dims = NODE_NDIM(base_node);
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

    return Node_NewChild(base_node, tdim, tshape);
}

NR_STATIC_INLINE Node*
node_index_handle_int_indices(Node* base_node, NIndexRuleSet* rs){
    char* data_ptr = NODE_DATA(base_node);
    int dim = 0;

    for (nr_intp i = 0; i < NIndexRuleSet_NUM_RULES(rs); i++){
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        if (NIndexRule_TYPE(rule) != NIndexRuleType_Int){
            NError_RaiseError(NError_IndexError, "Expected only integer indices.");
            return NULL;
        }
        nr_intp dim_size = NODE_SHAPE(base_node)[dim];
        int index = NIndexInt_INDEX(NIndexRule_DATA_AS_INT(rule));
        
        // CHECK_INT_INDEX_BOUNDS(index, dim_size, dim);

        data_ptr += NODE_STRIDES(base_node)[dim] * index;
        dim++;
    }

    int ndim = NODE_NDIM(base_node);
    if (dim == ndim){ // output is a scalar
        Node* result_node = Node_NewScalar(data_ptr, NODE_DTYPE(base_node));
        if (!result_node){
            NError_RaiseMemoryError();
            return NULL;
        }
        return result_node;
    }

    NR_DTYPE dtype = NODE_DTYPE(base_node);
    nr_intp dtype_size = NDtype_Size(NODE_DTYPE(base_node));
    nr_intp* out_shape = &NODE_SHAPE(base_node)[dim];
    nr_intp* out_strides = &NODE_STRIDES(base_node)[dim];
    int ndim_out = ndim - dim;
    nr_intp nitems = NR_NItems(ndim_out, out_shape);

    void* out_data = malloc(nitems * dtype_size);
    if (!out_data){
        NError_RaiseMemoryError();
        return NULL;
    }

    if (NODE_IS_CONTIGUOUS(base_node)){
        memcpy(out_data, data_ptr, nitems * dtype_size);
    } else {
        NIter iter;
        NIter_New(&iter, data_ptr, ndim_out, out_shape, out_strides, NITER_MODE_STRIDED);
        NIter_ITER(&iter);

        nr_intp offset = 0;
        while (NIter_NOTDONE(&iter)){
            memcpy((char*)out_data + offset, NIter_ITEM(&iter), dtype_size);
            offset += dtype_size;
            NIter_NEXT(&iter);
        }
    }

    Node* result_node = Node_New(out_data, 0, ndim_out, out_shape, dtype);
    if (!result_node){
        free(out_data);
        NError_RaiseMemoryError();
        return NULL;
    }

    return result_node;
}

NR_PUBLIC Node*
Node_Index(NIndexRuleSet* rs){
    Node* base_node = NIndexRuleSet_BASE_ARRAY(rs);
    int index_types = get_indexing_types(rs);
    int node_in_indices = index_types & HAS_NODE;

    if (index_types == HAS_INT){ // only int
        return node_index_handle_int_indices(base_node, rs);
    }

    if (node_in_indices){
        return node_index_handle_node_indices(base_node, rs);
    }
    return node_index_handle_non_node_indices(base_node, rs);
}