#include "index.h"
#include "nerror.h"
#include "node_core.h"
#include "free.h"
#include "niter.h"
#include "tc_methods.h"
#include "ntools.h"
#include "node2str.h"  


#define NIndex_INT NR_INT64

#define HAS_INT       1
#define HAS_SLICE     2
#define HAS_NODE      4
#define HAS_ELLIPSIS  8
#define HAS_NEW_AXIS  16
#define HAS_BOOL      32


typedef struct {
    int index_type;          // bitmask of index types present
    int copy_needed;        // whether a copy of data is needed
    int fancy_dims;         // number of fancy indexing dimensions
    int new_axis_dims;      // number of new axis dimensions
    int keeped_dims;        // number of kept dimensions
    int risky_indexing;    // whether risky indexing is used
}
indices_unpack_info;


typedef struct {
    int out_ndim;                           // output number of dimensions
    nr_intp out_shape[NR_NODE_MAX_NDIM];       // output shape
    nr_intp out_strides[NR_NODE_MAX_NDIM];     // output strides
} no_node_indices_info;


typedef struct
{
    Node* nodes[NR_MULTIITER_MAX_NITER];    // array of index nodes
    int is_temp[NR_MULTIITER_MAX_NITER];    // positions of temporary nodes to free
    int in_node_dims[NR_NODE_MAX_NDIM];     // input node dimensions
    int node_count;                         // total number of node indices
}node_indices_info;



NR_PUBLIC NIndexRuleSet
NIndexRuleSet_New() {
    NIndexRuleSet rs;
    NIndexRuleSet_NUM_RULES(&rs) = 0;
    return rs;
}

NR_PUBLIC int
NIndexRuleSet_AddInt(NIndexRuleSet* rs, nr_intp index) {
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    rule->type = NIndexRuleType_Int;
    rule->data.int_data.index = index;
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddSliceAdvanced(NIndexRuleSet* rs, nr_intp start, nr_intp stop, nr_intp step,
                                 nr_bool has_start, nr_bool has_stop) 
{
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    rule->type = NIndexRuleType_Slice;
    NIndexSlice* slice = &rule->data.slice_data;

    slice->start = start;
    slice->stop = stop;
    slice->step = step;

    slice->has_start = has_start;
    slice->has_stop = has_stop;
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddSlice(NIndexRuleSet* rs, nr_intp start, nr_intp stop, nr_intp step) {
    return NIndexRuleSet_AddSliceAdvanced(rs, start, stop, step, 1, 1);
}

NR_PUBLIC int
NIndexRuleSet_AddNewAxis(NIndexRuleSet* rs) {
    NINDEXRULESET_MAX_NUM_ERR(rs);
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    rule->type = NIndexRuleType_NewAxis;
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddEllipsis(NIndexRuleSet* rs) {
    NINDEXRULESET_MAX_NUM_ERR(rs);
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    rule->type = NIndexRuleType_Ellipsis;
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddNode(NIndexRuleSet* rs, Node* index_node) {
    NINDEXRULESET_MAX_NUM_ERR(rs);
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[NIndexRuleSet_NUM_RULES(rs)++];
    rule->type = NIndexRuleType_Node;
    rule->data.node_data.node = index_node;
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddFullSlice(NIndexRuleSet* rs) {
    return NIndexRuleSet_AddSliceAdvanced(rs, 0, 0, 1, 0, 0);
}

NR_PUBLIC int
NIndexRuleSet_AddRange(NIndexRuleSet* rs, nr_intp start, nr_intp stop) {
    return NIndexRuleSet_AddSlice(rs, start, stop, 1);
}

NR_PUBLIC void
NIndexRuleSet_Cleanup(NIndexRuleSet* rs) {
    if (!rs) return;
    rs->num_rules = 0;
}


NR_STATIC_INLINE void
free_nodes_in_info(node_indices_info* info) {
    for (int i = 0; i < info->node_count; i++) {
        if (info->is_temp[i]) {
            Node_Free(info->nodes[i]);
        }
    }
}

NR_STATIC_INLINE int
unpack_indices(NIndexRuleSet* rs, indices_unpack_info* info){
    info->copy_needed = 0;
    info->fancy_dims = 0;
    info->new_axis_dims = 0;
    info->index_type = 0;
    info->keeped_dims = 0;
    
    for (nr_intp i = 0; i < NIndexRuleSet_NUM_RULES(rs); i++) {
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        NIndexRuleType type = NIndexRule_TYPE(rule);
        
        switch (type) {
            case NIndexRuleType_Int:
                info->index_type |= HAS_INT;
                info->copy_needed = 1;
                break;
                
            case NIndexRuleType_Slice:
                info->index_type |= HAS_SLICE;
                info->keeped_dims += 1;
                break;
                
            case NIndexRuleType_Node: {
                info->index_type |= HAS_NODE;
                info->copy_needed = 1;
                info->fancy_dims += 1;

                Node* index_node = NIndexRule_DATA_AS_NODE(rule).node;
                if (NODE_DTYPE(index_node) == NR_BOOL) {
                    info->index_type |= HAS_BOOL;
                }
                break;
            }
                
            case NIndexRuleType_Ellipsis:
                if (info->index_type & HAS_ELLIPSIS) {
                    NError_RaiseError(NError_IndexError, "Multiple ellipses found in indexing.");
                    return -1;
                }
                info->index_type |= HAS_ELLIPSIS;
                break;
                
            case NIndexRuleType_NewAxis:
                info->index_type |= HAS_NEW_AXIS;
                info->new_axis_dims += 1;
                break;
                
            default:
                break;
        }
    }

    return 0;
}


NR_STATIC_INLINE char*
handle_non_node_indices(char* data_ptr, int ndim,
                        nr_intp* base_shape, nr_intp* base_strides,
                        NIndexRuleSet* rs, indices_unpack_info* info,
                        no_node_indices_info* nnii)
{
    int num_rules = NIndexRuleSet_NUM_RULES(rs);
    int dim = 0;      // current dimension in base node
    int tdim = 0;     // current dimension in target node    

    nnii->out_ndim = 0;
    nr_intp* tshape = nnii->out_shape;
    nr_intp* tstrides = nnii->out_strides;

    nr_intp offset = 0;

    for (int i = 0; i < num_rules; i++){
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        NIndexRuleType type = NIndexRule_TYPE(rule);
        NIndexData data = NIndexRule_DATA(rule);

        switch (type){
            case NIndexRuleType_Int: {
                NIndexInt idx = NIndexData_AS_INT(data);
                nr_intp real_index = NIndexInt_INDEX(idx);
                nr_intp index = (real_index < 0) ? (base_shape[dim] + real_index) : real_index;

                if (index < 0 || index >= base_shape[dim]){
                    NError_RaiseError(
                        NError_IndexError,
                        "Index %lld out of bounds for axis %d with size %lld",
                        (nr_long)real_index, dim, (nr_long)base_shape[dim]
                    );
                    return NULL;
                }

                offset += index * base_strides[dim];
                dim ++;
                break;
            }

            case NIndexRuleType_Slice: {
                NIndexSlice slice = NIndexData_AS_SLICE(data);
                nr_intp step = NIndexSlice_STEP(slice);
                nr_intp dim_size = base_shape[dim];

                if (step == 0) {
                    NError_RaiseError(NError_IndexError, "Slice step cannot be zero for axis %d", dim);
                    return NULL;
                }

                /* Apply defaults and normalize */
                nr_intp start = NIndexSlice_HAS_START(slice) ? NIndexSlice_START(slice) : (step > 0 ? 0 : dim_size - 1);
                nr_intp stop = NIndexSlice_HAS_STOP(slice) ? NIndexSlice_STOP(slice) : (step > 0 ? dim_size : -1);
                
                if (start < 0) start += dim_size;
                if (stop < 0 && step > 0) stop += dim_size;
                
                /* Clamp */
                start = start < 0 ? (step > 0 ? 0 : -1) : (start > dim_size ? (step > 0 ? dim_size : dim_size - 1) : start);
                stop = stop < 0 ? -1 : (stop > dim_size ? dim_size : stop);

                /* Error check for invalid slice direction */
                if ((step > 0 && start > stop) || (step < 0 && start < stop)) {
                    NError_RaiseError(
                        NError_IndexError,
                        "Slice [%lld:%lld:%lld] would select no elements on axis %d with size %lld",
                        (nr_long)NIndexSlice_START(slice), (nr_long)NIndexSlice_STOP(slice),
                        (nr_long)NIndexSlice_STEP(slice), dim, (nr_long)dim_size
                    );
                    return NULL;
                }

                /* Compute length */
                nr_intp length = (step > 0) ? (stop - start + step - 1) / step : (start - stop - step - 1) / (-step);

                /* Update output */
                offset += start * base_strides[dim];
                tstrides[tdim] = base_strides[dim] * step;
                tshape[tdim] = length;
                tdim++;
                dim++;
                break;
            }

            case NIndexRuleType_NewAxis: {
                tshape[tdim] = 1;
                tstrides[tdim] = 0;
                tdim++;
                break;
            }

            case NIndexRuleType_Ellipsis: {
                int remaining_dims = ndim - num_rules + 1 + info->new_axis_dims;

                for (int j = 0; j < remaining_dims; j++){
                    tstrides[tdim] = base_strides[dim];
                    tshape[tdim] = base_shape[dim];
                    dim++;
                    tdim++;
                }
                break;
            }

            default: {
                dim++;
                break;
            }
        }
    }

    // Remaining dimensions
    while (dim < ndim){
        tshape[tdim] = base_shape[dim];
        tstrides[tdim] = base_strides[dim];
        dim++;
        tdim++;
    }

    nnii->out_ndim = tdim;
    return data_ptr + offset;
}

NR_STATIC_INLINE Node*
index_through_multi_index_node(Node* base_node, nr_intp offset, 
                               node_indices_info* nii,
                               int remaining_dims,
                               nr_intp* remaining_shape,
                               nr_intp* remaining_strides)
{
    // Allocate NMultiIter on heap to prevent stack overflow
    NMultiIter* mit = malloc(sizeof(NMultiIter));
    if (!mit) {
        NError_RaiseError(NError_ValueError, "Failed to allocate memory for NMultiIter");
        return NULL;
    }
    
    if (NMultiIter_FromNodes(nii->nodes, nii->node_count, mit) < 0) {
        free(mit);
        return NULL;
    }
    
    char* node_data = (char*)NODE_DATA(base_node) + offset;
    char* out_data = NULL;
    nr_size_t bsize = NDtype_Size(NODE_DTYPE(base_node));
    NR_DTYPE dtype = NODE_DTYPE(base_node);

    if (remaining_dims <= 0){
        nr_size_t nitems = NR_NItems(mit->out_ndim, mit->out_shape);
        out_data = malloc(nitems * bsize);
        if (!out_data){
            free(mit);
            NError_RaiseMemoryError();
            return NULL;
        }

        char* temp_data_ptr = out_data;
        int num_iters = mit->n_iter;
        nr_intp step = 0;
        
        NMultiIter_ITER(mit);
        while (NMultiIter_NOTDONE(mit)){
            for (int i = 0; i < num_iters; i++){
                int in_node_dim = nii->in_node_dims[i];
                step += (*(nr_int64*)NMultiIter_ITEM(mit, i)) * NODE_STRIDES(base_node)[in_node_dim];
            }

            memcpy(temp_data_ptr, node_data + step, bsize);
            temp_data_ptr += bsize;
            step = 0;
            NMultiIter_NEXT(mit);
        }

        Node* result = Node_New(out_data, 1, mit->out_ndim, mit->out_shape, dtype);
        free(mit);
        return result;
    }

    int tndim = mit->out_ndim + remaining_dims;
    nr_intp tshape[NR_NODE_MAX_NDIM];

    memcpy(tshape, mit->out_shape, sizeof(nr_intp) * mit->out_ndim);
    memcpy(&tshape[mit->out_ndim], remaining_shape, sizeof(nr_intp) * remaining_dims);

    nr_size_t nitems = NR_NItems(tndim, tshape);
    out_data = malloc(nitems * bsize);
    if (!out_data){
        free(mit);
        NError_RaiseMemoryError();
        return NULL;
    }

    char* temp_data_ptr = out_data;
    int num_iters = mit->n_iter;
    
    NCoordIter citer;
    NCoordIter_New(&citer, remaining_dims, remaining_shape);
    
    NMultiIter_ITER(mit);
    while (NMultiIter_NOTDONE(mit)){
        // Calculate the base offset from fancy indices with bounds checking
        nr_intp base_offset = 0;
        for (int i = 0; i < num_iters; i++) {
            nr_int64 index_val = *(nr_int64*)NMultiIter_ITEM(mit, i);
            base_offset += index_val * NODE_STRIDES(base_node)[i];
        }

        NCoordIter_ITER(&citer);
        while (NCoordIter_NOTDONE(&citer)) {
            nr_intp offset = base_offset;
            
            // Calculate offset using coordinate iterator with bounds checking
            for (int j = 0; j < remaining_dims; j++) {
                nr_intp coord = NCoordIter_COORD(&citer, j);
                offset += coord * remaining_strides[j];
            }
            
            memcpy(temp_data_ptr, node_data + offset, bsize);
            temp_data_ptr += bsize;
            NCoordIter_NEXT(&citer);
        }

        NMultiIter_NEXT(mit);
    }

    Node* result = Node_New(out_data, 1, tndim, tshape, dtype);
    free(mit);
    return result;
}

NR_STATIC_INLINE int
handle_node_indecies(Node* base_node, NIndexRuleSet* rs,
                    indices_unpack_info* info,
                    node_indices_info* nii)
{
    int num_rules = NIndexRuleSet_NUM_RULES(rs);
    int dim = 0;
    nii->node_count = 0;

    for (int i = 0; i < num_rules; i++){
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        NIndexRuleType type = NIndexRule_TYPE(rule);
        int temp = 0;

        if (type == NIndexRuleType_Node){
            NIndexData data = NIndexRule_DATA(rule);
            NIndexNode index_node_data = NIndexData_AS_NODE(data);
            Node* indexed_node = NIndexNode_NODE(index_node_data);

            NR_DTYPE ndtype = NODE_DTYPE(indexed_node);
            if (ndtype != NIndex_INT){
                Node* converted_node = Node_ToType(NULL, indexed_node, NIndex_INT);
                if (!converted_node){
                    free_nodes_in_info(nii);
                    return -1;
                }
                indexed_node = converted_node;
                temp = 1;   
            }

            if (!info->risky_indexing){
                // Check index bounds
                NIter iter;
                NIter_FromNode(&iter, indexed_node, NITER_MODE_NONE);
                NIter_ITER(&iter);
                while (NIter_NOTDONE(&iter)){
                    nr_int64 index_val = *(nr_int64*)NIter_ITEM(&iter);
                    nr_intp dim_size = NODE_SHAPE(base_node)[dim];
                    nr_intp idx = (index_val < 0) ? dim_size + index_val : index_val;

                    if (idx < 0 || idx >= dim_size){
                        NError_RaiseError(
                            NError_IndexError,
                            "Index %lld out of bounds for axis %d with size %lld",
                            (nr_long)index_val, nii->node_count, (nr_long)base_node->shape[nii->node_count]
                        );
                        free_nodes_in_info(nii);
                        return -1;
                    }

                    NIter_NEXT(&iter);
                }
            }

            nii->nodes[nii->node_count] = indexed_node;
            nii->is_temp[nii->node_count] = temp;
            nii->in_node_dims[nii->node_count] = dim;
            nii->node_count++;
            dim++;
        }
        else{
            if (type != NIndexRuleType_NewAxis){
                dim++;
            }
        }
    }
    
    return 0;
}


NR_STATIC_INLINE Node*
index_flat_bool_indexing(Node* base_node, NIndexRuleSet* rs){
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[0];
    NIndexData data = NIndexRule_DATA(rule);
    NIndexNode index_node_data = NIndexData_AS_NODE(data);
    Node* index_node = NIndexNode_NODE(index_node_data);

    char* base_data = (char*)NODE_DATA(base_node);

    int is_same_shape = Node_SameShape(base_node, index_node);
    int is_index_c = NODE_IS_CONTIGUOUS(index_node);
    int is_base_c = NODE_IS_CONTIGUOUS(base_node);

    char* out_data = NULL;
    nr_intp correct_count = 0;

    if (is_same_shape){
        nr_intp nitems = NR_NItems(base_node->ndim, base_node->shape);
        nr_size_t bsize = NDtype_Size(NODE_DTYPE(base_node));
        char* temp_data = malloc(nitems * bsize);
        if (!temp_data){
            NError_RaiseMemoryError();
            return NULL;
        }

        // Same Shape, both nodes contiguous
        if (is_index_c && is_base_c){
            nr_intp current = 0;
            for (nr_intp i = 0; i < nitems; i++){
                nr_bool val = ((nr_bool*)index_node->data)[i];
                if (val){
                    memcpy(temp_data + current, base_data + i * bsize, bsize);
                    current += bsize;
                    correct_count++;
                }
            }
        }

        // Same Shape, index is contiguous but base is not
        else if (is_index_c){
            nr_intp current = 0;
            NIter iter;
            NIter_FromNode(&iter, base_node, NITER_MODE_NONE);

            NIter_ITER(&iter);
            nr_intp i = 0;
            while (NIter_NOTDONE(&iter)){
                nr_bool val = ((nr_bool*)index_node->data)[i];
                if (val){
                    memcpy(temp_data + current, NIter_ITEM(&iter), bsize);
                    current += bsize;
                    correct_count++;
                }
                NIter_NEXT(&iter);
                i++;
            }
        }

        // Same Shape, base is contiguous but index is not
        else if (is_base_c){
            nr_intp current = 0;
            NIter iter;
            NIter_FromNode(&iter, index_node, NITER_MODE_NONE);

            NIter_ITER(&iter);
            nr_intp i = 0;
            while (NIter_NOTDONE(&iter)){
                nr_bool val = *(nr_bool*)NIter_ITEM(&iter);
                if (val){
                    memcpy(temp_data + current, base_data + i * bsize, bsize);
                    current += bsize;
                    correct_count++;
                }
                NIter_NEXT(&iter);
                i++;
            }
        }
        
        else{
            nr_intp current = 0;
            NIter base_iter;
            NIter_FromNode(&base_iter, base_node, NITER_MODE_NONE);
            NIter index_iter;
            NIter_FromNode(&index_iter, index_node, NITER_MODE_NONE);

            NIter_ITER(&base_iter);
            NIter_ITER(&index_iter);
            while (NIter_NOTDONE(&base_iter) && NIter_NOTDONE(&index_iter)){
                nr_bool val = *(nr_bool*)NIter_ITEM(&index_iter);
                if (val){
                    memcpy(temp_data + current, NIter_ITEM(&base_iter), bsize);
                    current += bsize;
                    correct_count++;
                }
                NIter_NEXT(&base_iter);
                NIter_NEXT(&index_iter);
            }
        }

        out_data = malloc(correct_count * bsize);
        if (!out_data){
            free(temp_data);
            NError_RaiseMemoryError();
            return NULL;
        }

        memcpy(out_data, temp_data, correct_count * bsize);
        free(temp_data);
    }
    else{
        Node* nodes[] = {base_node, index_node};
        NMultiIter mit;
        if (NMultiIter_FromNodes(nodes, 2, &mit) < 0){
            return NULL;
        }

        nr_intp nitems = NR_NItems(mit.out_ndim, mit.out_shape);
        nr_size_t bsize = NDtype_Size(NODE_DTYPE(base_node));
        char* temp_data = malloc(nitems * bsize);
        if (!temp_data){
            NError_RaiseMemoryError();
            return NULL;
        }

        char* temp_data_ptr = temp_data;

        NMultiIter_ITER(&mit);
        while (NMultiIter_NOTDONE(&mit)){
            nr_bool val = *(nr_bool*)NMultiIter_ITEM(&mit, 1);
            if (val){
                memcpy(temp_data_ptr, NMultiIter_ITEM(&mit, 0), bsize);
                temp_data_ptr += bsize;
                correct_count++;
            }
            NMultiIter_NEXT2(&mit);
        }
    }

    if (!out_data){
        return NULL;
    }

    return Node_New(
        out_data,
        1,
        1,
        (nr_intp[]){correct_count},
        NODE_DTYPE(base_node)
    );
}

NR_STATIC_INLINE Node*
index_mixed_indexing(Node* base_node, NIndexRuleSet* rs, indices_unpack_info* info){
    no_node_indices_info nnii;
    char* data_ptr = handle_non_node_indices(
        (char*)NODE_DATA(base_node),
        NODE_NDIM(base_node),
        NODE_SHAPE(base_node),
        NODE_STRIDES(base_node),
        rs,
        info,
        &nnii
    );
    if (!data_ptr){
        return NULL;
    }

    int remaining_dims = nnii.out_ndim;
    nr_intp* remaining_shape = nnii.out_shape;
    nr_intp* remaining_strides = nnii.out_strides;

    node_indices_info nii;
    if (handle_node_indecies(base_node, rs, info, &nii) < 0){
        return NULL;
    }

    Node* result_node = index_through_multi_index_node(
        base_node,
        data_ptr - (char*)NODE_DATA(base_node),
        &nii,
        remaining_dims,
        remaining_shape,
        remaining_strides
    );

    free_nodes_in_info(&nii);
    return result_node;
}

NR_STATIC_INLINE Node*
index_only_node_indexing(Node* base_node, NIndexRuleSet* rs, indices_unpack_info* info){
    node_indices_info nii;
    if (handle_node_indecies(base_node, rs, info, &nii) < 0){
        return NULL;
    }

    int remaining_dims = base_node->ndim - nii.node_count;
    nr_intp* remaining_shape = &base_node->shape[nii.node_count];
    nr_intp* remaining_strides = &base_node->strides[nii.node_count];
    
    Node* result_node = index_through_multi_index_node(
        base_node,
        0,
        &nii,
        remaining_dims,
        remaining_shape,
        remaining_strides
    );

    free_nodes_in_info(&nii);
    return result_node;
}

NR_STATIC_INLINE Node*
index_node(Node* base_node, NIndexRuleSet* rs, indices_unpack_info* info){
    int has_bool_index = info->index_type & HAS_BOOL;
    int num_rules = NIndexRuleSet_NUM_RULES(rs);

    if (num_rules == 1 && has_bool_index){
        return index_flat_bool_indexing(base_node, rs);
    }

    if (info->index_type &~ (HAS_NODE | HAS_BOOL)) {
        return index_mixed_indexing(base_node, rs, info);
    }

    return index_only_node_indexing(base_node, rs, info);
}

NR_STATIC_INLINE Node*
index_non_nodes(Node* base_node, NIndexRuleSet* rs, indices_unpack_info* info){
    no_node_indices_info nnii;
    char* data_ptr = handle_non_node_indices(
        (char*)NODE_DATA(base_node),
        NODE_NDIM(base_node),
        NODE_SHAPE(base_node),
        NODE_STRIDES(base_node),
        rs,
        info,
        &nnii
    );

    if (!data_ptr){
        return NULL;
    }

    if (info->copy_needed){
        nr_intp nitems = NR_NItems(nnii.out_ndim, nnii.out_shape);
        nr_size_t bsize = NDtype_Size(NODE_DTYPE(base_node));
        char* data = malloc(nitems * bsize);
        if (!data){
            NError_RaiseMemoryError();
            return NULL;
        }

        NIter iter;
        NIter_New(&iter, data_ptr, nnii.out_ndim, nnii.out_shape, nnii.out_strides, NITER_MODE_STRIDED);
        NIter_ITER(&iter);

        char* dest_ptr = data;
        while (NIter_NOTDONE(&iter)){
            memcpy(dest_ptr, NIter_ITEM(&iter), bsize);
            dest_ptr += bsize;
            NIter_NEXT(&iter);
        }

        return Node_New(
            data,
            1,
            nnii.out_ndim,
            nnii.out_shape,
            NODE_DTYPE(base_node)
        );
    }

    return Node_NewChild(
        base_node,
        nnii.out_ndim,
        nnii.out_shape,
        nnii.out_strides,
        data_ptr - (char*)NODE_DATA(base_node)
    );
}


NR_STATIC_INLINE Node*
node_index(Node* base_node, NIndexRuleSet* rs, int risky_indexing){
    int num_rules = NIndexRuleSet_NUM_RULES(rs);
    if (num_rules == 0){
        // No indexing rules, return a view of the base node
        return Node_NewChild(
            base_node,
            base_node->ndim,
            base_node->shape,
            base_node->strides,
            0
        );
    }
    
    indices_unpack_info info;
    if (unpack_indices(rs, &info) < 0){
        return NULL;
    }
    info.risky_indexing = risky_indexing;
    
    int ndim = base_node->ndim;
    int num_used_dims = num_rules - info.new_axis_dims;
    int ellipsis_dims = ndim - num_rules + 1 + info.new_axis_dims;
    if (info.index_type & HAS_ELLIPSIS){
        num_used_dims += ellipsis_dims - 1;
    }

    if (num_used_dims > base_node->ndim){
        NError_RaiseError(
            NError_IndexError,
            "Too many indices for array: array is %d-dimensional, "
            "but %d were indexed",
            base_node->ndim,
            num_used_dims
        );
        return NULL;
    }

    int num_keeped_dims = info.keeped_dims;
    if (info.index_type & HAS_ELLIPSIS){
        num_keeped_dims += ellipsis_dims;
    }
    int out_dim = num_keeped_dims + info.new_axis_dims;
    
    if (out_dim > NR_NODE_MAX_NDIM){
        NError_RaiseError(
            NError_IndexError,
            "Resulting array has too many dimensions: %d > %d",
            out_dim,
            NR_NODE_MAX_NDIM
        );
        return NULL;
    }


    if (info.index_type & HAS_NODE){
        return index_node(base_node, rs, &info);
    }

    return index_non_nodes(base_node, rs, &info);
}

NR_PUBLIC Node*
Node_Index(Node* base_node, NIndexRuleSet* rs){
    return node_index(base_node, rs, 0);
}

NR_PUBLIC Node*
Node_RiskyIndex(Node* base_node, NIndexRuleSet* rs){
    return node_index(base_node, rs, 1);
}