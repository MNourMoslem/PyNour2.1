#include "nour/nour.h"
#include "nfunc.h"
#include "getset.h"
#include "nerror.h"
#include "node_core.h"
#include "free.h"
#include "niter.h"
#include "tc_methods.h"
#include "ntools.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ============================================================================
 * Internal Structures
 * ============================================================================ */

typedef struct {
    NIndexRuleSet* rs;
    int risky;
} IndexOpArgs;

/* ============================================================================
 * NIndexRuleSet Implementation
 * ============================================================================ */

NR_PUBLIC NIndexRuleSet
NIndexRuleSet_New()
{
    NIndexRuleSet rs;
    NIndexRuleSet_Init(&rs);
    return rs;
}

NR_PUBLIC void
NIndexRuleSet_Init(NIndexRuleSet* rs)
{
    rs->num_rules = 0;
}

NR_PUBLIC int
NIndexRuleSet_AddInt(NIndexRuleSet* rs, nr_intp index)
{
    if (rs->num_rules >= NINDEXRULESET_MAX_RULES) return -1;
    NIndexRule* rule = &rs->rules[rs->num_rules++];
    rule->type = NIndexRuleType_Int;
    rule->data.int_data.index = index;
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddSlice(NIndexRuleSet* rs, nr_intp start, nr_intp stop, nr_intp step)
{
    return NIndexRuleSet_AddSliceAdvanced(rs, start, stop, step, 1, 1);
}

NR_PUBLIC int
NIndexRuleSet_AddSliceAdvanced(NIndexRuleSet* rs, nr_intp start, nr_intp stop, nr_intp step,
                                 nr_bool has_start, nr_bool has_stop)
{
    if (rs->num_rules >= NINDEXRULESET_MAX_RULES) return -1;
    NIndexRule* rule = &rs->rules[rs->num_rules++];
    rule->type = NIndexRuleType_Slice;
    rule->data.slice_data.start = start;
    rule->data.slice_data.stop = stop;
    rule->data.slice_data.step = step;
    rule->data.slice_data.has_start = has_start;
    rule->data.slice_data.has_stop = has_stop;
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddNewAxis(NIndexRuleSet* rs)
{
    if (rs->num_rules >= NINDEXRULESET_MAX_RULES) return -1;
    NIndexRule* rule = &rs->rules[rs->num_rules++];
    rule->type = NIndexRuleType_NewAxis;
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddEllipsis(NIndexRuleSet* rs)
{
    if (rs->num_rules >= NINDEXRULESET_MAX_RULES) return -1;
    NIndexRule* rule = &rs->rules[rs->num_rules++];
    rule->type = NIndexRuleType_Ellipsis;
    return 0;
}

NR_PUBLIC int
NIndexRuleSet_AddNode(NIndexRuleSet* rs, Node* index_node)
{
    if (rs->num_rules >= NINDEXRULESET_MAX_RULES) return -1;
    NIndexRule* rule = &rs->rules[rs->num_rules++];
    rule->type = NIndexRuleType_Node;
    rule->data.node_data.node = index_node;
    NODE_INCREF(index_node);
    return 0;
}

// Helper to trim whitespace
static char* trim_whitespace(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

NR_PUBLIC NIndexRuleSet
NIndexRuleSet_NewFromString(const char* index_string)
{
    NIndexRuleSet rs = NIndexRuleSet_New();
    if (!index_string) return rs;

    char* str = strdup(index_string);
    if (!str) return rs; // Memory error

    char* p = str;
    
    // Remove brackets
    char* start = strchr(p, '[');
    if (start) p = start + 1;
    char* end = strrchr(p, ']');
    if (end) *end = '\0';

    // Split by comma
    char* token = strtok(p, ",");
    while (token) {
        char* part = trim_whitespace(token);
        
        if (strlen(part) == 0) {
            token = strtok(NULL, ",");
            continue;
        }

        if (strcmp(part, "None") == 0 || strcmp(part, "np.newaxis") == 0) {
            NIndexRuleSet_AddNewAxis(&rs);
        } else if (strcmp(part, "...") == 0) {
            NIndexRuleSet_AddEllipsis(&rs);
        } else if (strchr(part, ':')) {
            // Slice parsing
            nr_intp start_val = 0, stop_val = 0, step_val = 1;
            nr_bool has_start = 0, has_stop = 0;
            
            // Count colons
            int colons = 0;
            for (int i = 0; part[i]; i++) if (part[i] == ':') colons++;
            
            char* first = part;
            char* second = strchr(first, ':');
            *second = '\0';
            char* third = (colons > 1) ? strchr(second + 1, ':') : NULL;
            if (third) *third = '\0';
            
            // Parse start
            char* start_str = trim_whitespace(first);
            if (strlen(start_str) > 0) {
                start_val = atoll(start_str);
                has_start = 1;
            }
            
            // Parse stop
            char* stop_str = trim_whitespace(second + 1);
            if (strlen(stop_str) > 0) {
                stop_val = atoll(stop_str);
                has_stop = 1;
            }
            
            // Parse step
            if (third) {
                char* step_str = trim_whitespace(third + 1);
                if (strlen(step_str) > 0) {
                    step_val = atoll(step_str);
                }
            }
            
            NIndexRuleSet_AddSliceAdvanced(&rs, start_val, stop_val, step_val, has_start, has_stop);
        } else {
            // Integer
            nr_intp val = atoll(part);
            NIndexRuleSet_AddInt(&rs, val);
        }

        token = strtok(NULL, ",");
    }

    free(str);
    return rs;
}

NR_PUBLIC void
NIndexRuleSet_Cleanup(NIndexRuleSet* rs)
{
    for (int i = 0; i < rs->num_rules; i++) {
        if (rs->rules[i].type == NIndexRuleType_Node) {
            NODE_DECREF(rs->rules[i].data.node_data.node);
        }
    }
    rs->num_rules = 0;
}

NR_PUBLIC int
NIndexRuleSet_AddFullSlice(NIndexRuleSet* rs)
{
    return NIndexRuleSet_AddSliceAdvanced(rs, 0, 0, 1, 0, 0);
}

NR_PUBLIC int
NIndexRuleSet_AddRange(NIndexRuleSet* rs, nr_intp start, nr_intp stop)
{
    return NIndexRuleSet_AddSliceAdvanced(rs, start, stop, 1, 1, 1);
}

/* ============================================================================
 * Index Type Flags
 * ============================================================================ */

#define HAS_INT       1
#define HAS_SLICE     2
#define HAS_NODE      4
#define HAS_ELLIPSIS  8
#define HAS_NEW_AXIS  16
#define HAS_BOOL      32

/* ============================================================================
 * Internal Structures
 * ============================================================================ */

typedef struct {
    int index_type;
    int copy_needed;
    int fancy_dims;
    int new_axis_dims;
    int keeped_dims;
    int risky_indexing;
} indices_unpack_info;

typedef struct {
    int out_ndim;
    nr_intp out_shape[NR_NODE_MAX_NDIM];
    nr_intp out_strides[NR_NODE_MAX_NDIM];
} no_node_indices_info;

typedef struct {
    Node* nodes[NR_MULTIITER_MAX_NITER];
    int is_temp[NR_MULTIITER_MAX_NITER];
    int in_node_dims[NR_NODE_MAX_NDIM];
    int node_count;
} node_indices_info;

/* Indexing context - holds all indexing computation results */
typedef struct {
    /* Input */
    Node* base_node;
    NIndexRuleSet* rs;
    int risky_indexing;
    
    /* Computed info */
    indices_unpack_info unpack_info;
    no_node_indices_info no_node_info;
    node_indices_info node_info;
    
    /* Results */
    char* data_offset;          /* Pointer offset for non-node indices */
    nr_intp byte_offset;        /* Byte offset from base data */
    int remaining_dims;         /* Remaining dimensions after node indexing */
    nr_intp* remaining_shape;   /* Remaining shape */
    nr_intp* remaining_strides; /* Remaining strides */
    
    /* Flags */
    int has_node_indices;
    int is_flat_bool;
    int computation_done;
} IndexContext;

/* ============================================================================
 * Helper: Free node indices
 * ============================================================================ */

NR_STATIC_INLINE void
free_nodes_in_info(node_indices_info* info) {
    for (int i = 0; i < info->node_count; i++) {
        if (info->is_temp[i]) {
            Node_Free(info->nodes[i]);
        }
    }
}

/* ============================================================================
 * Step 1: Unpack indices (determine what types are present)
 * ============================================================================ */

NR_STATIC_INLINE int
unpack_indices(NIndexRuleSet* rs, indices_unpack_info* info) {
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
                    NError_RaiseError(NError_IndexError, 
                        "Multiple ellipses found in indexing.");
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

/* ============================================================================
 * Step 2: Handle non-node indices (int, slice, newaxis, ellipsis)
 * ============================================================================ */

NR_STATIC_INLINE char*
handle_non_node_indices(char* data_ptr, int ndim,
                        nr_intp* base_shape, nr_intp* base_strides,
                        NIndexRuleSet* rs, indices_unpack_info* info,
                        no_node_indices_info* nnii)
{
    int num_rules = NIndexRuleSet_NUM_RULES(rs);
    int dim = 0;
    int tdim = 0;
    nnii->out_ndim = 0;
    nr_intp* tshape = nnii->out_shape;
    nr_intp* tstrides = nnii->out_strides;
    nr_intp offset = 0;

    for (int i = 0; i < num_rules; i++) {
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        NIndexRuleType type = NIndexRule_TYPE(rule);
        NIndexData data = NIndexRule_DATA(rule);

        switch (type) {
            case NIndexRuleType_Int: {
                NIndexInt idx = NIndexData_AS_INT(data);
                nr_intp real_index = NIndexInt_INDEX(idx);
                nr_intp index = (real_index < 0) ? 
                    (base_shape[dim] + real_index) : real_index;

                if (index < 0 || index >= base_shape[dim]) {
                    NError_RaiseError(NError_IndexError,
                        "Index %lld out of bounds for axis %d with size %lld",
                        (nr_long)real_index, dim, (nr_long)base_shape[dim]);
                    return NULL;
                }
                offset += index * base_strides[dim];
                dim++;
                break;
            }

            case NIndexRuleType_Slice: {
                NIndexSlice slice = NIndexData_AS_SLICE(data);
                nr_intp step = NIndexSlice_STEP(slice);
                nr_intp dim_size = base_shape[dim];

                if (step == 0) {
                    NError_RaiseError(NError_IndexError, 
                        "Slice step cannot be zero for axis %d", dim);
                    return NULL;
                }

                nr_intp start = NIndexSlice_HAS_START(slice) ? 
                    NIndexSlice_START(slice) : (step > 0 ? 0 : dim_size - 1);
                nr_intp stop = NIndexSlice_HAS_STOP(slice) ? 
                    NIndexSlice_STOP(slice) : (step > 0 ? dim_size : -1);
                
                if (start < 0) start += dim_size;
                if (stop < 0 && step > 0) stop += dim_size;
                
                start = start < 0 ? (step > 0 ? 0 : -1) : 
                    (start > dim_size ? (step > 0 ? dim_size : dim_size - 1) : start);
                stop = stop < 0 ? -1 : (stop > dim_size ? dim_size : stop);

                if ((step > 0 && start > stop) || (step < 0 && start < stop)) {
                    NError_RaiseError(NError_IndexError,
                        "Slice [%lld:%lld:%lld] invalid on axis %d",
                        (nr_long)NIndexSlice_START(slice), 
                        (nr_long)NIndexSlice_STOP(slice),
                        (nr_long)step, dim);
                    return NULL;
                }

                nr_intp length = (step > 0) ? 
                    (stop - start + step - 1) / step : 
                    (start - stop - step - 1) / (-step);

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
                for (int j = 0; j < remaining_dims; j++) {
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

    while (dim < ndim) {
        tshape[tdim] = base_shape[dim];
        tstrides[tdim] = base_strides[dim];
        dim++;
        tdim++;
    }

    nnii->out_ndim = tdim;
    return data_ptr + offset;
}

/* ============================================================================
 * Step 3: Handle node indices (prepare node index arrays)
 * ============================================================================ */

NR_STATIC_INLINE int
handle_node_indices(Node* base_node, NIndexRuleSet* rs,
                    indices_unpack_info* info, node_indices_info* nii)
{
    int num_rules = NIndexRuleSet_NUM_RULES(rs);
    int dim = 0;
    nii->node_count = 0;

    for (int i = 0; i < num_rules; i++) {
        NIndexRule* rule = &NIndexRuleSet_RULES(rs)[i];
        NIndexRuleType type = NIndexRule_TYPE(rule);
        int temp = 0;

        if (type == NIndexRuleType_Node) {
            NIndexData data = NIndexRule_DATA(rule);
            NIndexNode index_node_data = NIndexData_AS_NODE(data);
            Node* indexed_node = NIndexNode_NODE(index_node_data);

            NR_DTYPE ndtype = NODE_DTYPE(indexed_node);
            if (ndtype != NR_INT64) {
                Node* converted_node = Node_ToType(NULL, indexed_node, NR_INT64);
                if (!converted_node) {
                    free_nodes_in_info(nii);
                    return -1;
                }
                indexed_node = converted_node;
                temp = 1;
            }

            if (!info->risky_indexing) {
                NIter iter;
                NIter_FromNode(&iter, indexed_node, NITER_MODE_NONE);
                NIter_ITER(&iter);
                while (NIter_NOTDONE(&iter)) {
                    nr_int64 index_val = *(nr_int64*)NIter_ITEM(&iter);
                    nr_intp dim_size = NODE_SHAPE(base_node)[dim];
                    nr_intp idx = (index_val < 0) ? dim_size + index_val : index_val;

                    if (idx < 0 || idx >= dim_size) {
                        NError_RaiseError(NError_IndexError,
                            "Index %lld out of bounds for axis %d with size %lld",
                            (nr_long)index_val, nii->node_count, 
                            (nr_long)base_node->shape[nii->node_count]);
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
        } else {
            if (type != NIndexRuleType_NewAxis) {
                dim++;
            }
        }
    }
    return 0;
}

/* ============================================================================
 * Step 4: Compute indexing context (centralized)
 * ============================================================================ */

NR_STATIC_INLINE int
compute_index_context(IndexContext* ctx)
{
    if (ctx->computation_done) return 0;
    
    int num_rules = NIndexRuleSet_NUM_RULES(ctx->rs);
    
    /* Check for flat boolean indexing */
    ctx->is_flat_bool = 0;
    if (num_rules == 1 && (ctx->unpack_info.index_type & HAS_BOOL)) {
        ctx->is_flat_bool = 1;
        ctx->computation_done = 1;
        return 0;
    }
    
    /* Handle non-node indices first */
    ctx->data_offset = handle_non_node_indices(
        (char*)NODE_DATA(ctx->base_node),
        NODE_NDIM(ctx->base_node),
        NODE_SHAPE(ctx->base_node),
        NODE_STRIDES(ctx->base_node),
        ctx->rs,
        &ctx->unpack_info,
        &ctx->no_node_info
    );
    
    if (!ctx->data_offset) return -1;
    
    ctx->byte_offset = ctx->data_offset - (char*)NODE_DATA(ctx->base_node);
    
    /* Handle node indices if present */
    ctx->has_node_indices = (ctx->unpack_info.index_type & HAS_NODE) != 0;
    
    if (ctx->has_node_indices) {
        if (handle_node_indices(ctx->base_node, ctx->rs, 
                                &ctx->unpack_info, &ctx->node_info) < 0) {
            return -1;
        }
        
        /* Determine remaining dimensions */
        if (ctx->unpack_info.index_type &~ (HAS_NODE | HAS_BOOL)) {
            /* Mixed indexing */
            ctx->remaining_dims = ctx->no_node_info.out_ndim;
            ctx->remaining_shape = ctx->no_node_info.out_shape;
            ctx->remaining_strides = ctx->no_node_info.out_strides;
        } else {
            /* Only node indexing */
            ctx->remaining_dims = ctx->base_node->ndim - ctx->node_info.node_count;
            ctx->remaining_shape = &ctx->base_node->shape[ctx->node_info.node_count];
            ctx->remaining_strides = &ctx->base_node->strides[ctx->node_info.node_count];
        }
    }
    
    ctx->computation_done = 1;
    return 0;
}

/* ============================================================================
 * GET Operation: Flat Boolean Indexing
 * ============================================================================ */

NR_STATIC_INLINE Node*
get_flat_bool_indexing(Node* base_node, NIndexRuleSet* rs)
{
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[0];
    Node* index_node = NIndexRule_DATA_AS_NODE(rule).node;
    char* base_data = (char*)NODE_DATA(base_node);

    int is_same_shape = Node_SameShape(base_node, index_node);
    int is_index_c = NODE_IS_CONTIGUOUS(index_node);
    int is_base_c = NODE_IS_CONTIGUOUS(base_node);

    nr_intp nitems = NR_NItems(base_node->ndim, base_node->shape);
    nr_size_t bsize = NDtype_Size(NODE_DTYPE(base_node));
    char* temp_data = malloc(nitems * bsize);
    if (!temp_data) {
        NError_RaiseMemoryError();
        return NULL;
    }

    nr_intp correct_count = 0;

    if (is_same_shape && is_index_c && is_base_c) {
        nr_intp current = 0;
        for (nr_intp i = 0; i < nitems; i++) {
            nr_bool val = ((nr_bool*)index_node->data)[i];
            if (val) {
                memcpy(temp_data + current, base_data + i * bsize, bsize);
                current += bsize;
                correct_count++;
            }
        }
    } else {
        Node* nodes[] = {base_node, index_node};
        NMultiIter mit;
        if (NMultiIter_FromNodes(nodes, 2, &mit) < 0) {
            free(temp_data);
            return NULL;
        }

        char* temp_data_ptr = temp_data;
        NMultiIter_ITER(&mit);
        while (NMultiIter_NOTDONE(&mit)) {
            nr_bool val = *(nr_bool*)NMultiIter_ITEM(&mit, 1);
            if (val) {
                memcpy(temp_data_ptr, NMultiIter_ITEM(&mit, 0), bsize);
                temp_data_ptr += bsize;
                correct_count++;
            }
            NMultiIter_NEXT2(&mit);
        }
    }

    char* out_data = malloc(correct_count * bsize);
    if (!out_data) {
        free(temp_data);
        NError_RaiseMemoryError();
        return NULL;
    }

    memcpy(out_data, temp_data, correct_count * bsize);
    free(temp_data);

    return Node_New(out_data, 1, 1, (nr_intp[]){correct_count}, 
                    NODE_DTYPE(base_node));
}

/* ============================================================================
 * GET Operation: Fancy Indexing (node indices)
 * ============================================================================ */

NR_STATIC_INLINE Node*
get_fancy_indexing(IndexContext* ctx)
{
    NMultiIter* mit = malloc(sizeof(NMultiIter));
    if (!mit) {
        NError_RaiseError(NError_ValueError, 
            "Failed to allocate memory for NMultiIter");
        return NULL;
    }
    
    if (NMultiIter_FromNodes(ctx->node_info.nodes, 
                             ctx->node_info.node_count, mit) < 0) {
        free(mit);
        return NULL;
    }
    
    char* node_data = (char*)NODE_DATA(ctx->base_node) + ctx->byte_offset;
    nr_size_t bsize = NDtype_Size(NODE_DTYPE(ctx->base_node));
    NR_DTYPE dtype = NODE_DTYPE(ctx->base_node);

    /* No remaining dimensions - output is just broadcast shape */
    if (ctx->remaining_dims <= 0) {
        nr_size_t nitems = NR_NItems(mit->out_ndim, mit->out_shape);
        char* out_data = malloc(nitems * bsize);
        if (!out_data) {
            free(mit);
            NError_RaiseMemoryError();
            return NULL;
        }

        char* temp_data_ptr = out_data;
        int num_iters = mit->n_iter;
        
        NMultiIter_ITER(mit);
        while (NMultiIter_NOTDONE(mit)) {
            nr_intp step = 0;
            for (int i = 0; i < num_iters; i++) {
                int in_node_dim = ctx->node_info.in_node_dims[i];
                step += (*(nr_int64*)NMultiIter_ITEM(mit, i)) * 
                        NODE_STRIDES(ctx->base_node)[in_node_dim];
            }
            memcpy(temp_data_ptr, node_data + step, bsize);
            temp_data_ptr += bsize;
            NMultiIter_NEXT(mit);
        }

        Node* result = Node_New(out_data, 1, mit->out_ndim, 
                               mit->out_shape, dtype);
        free(mit);
        return result;
    }

    /* Has remaining dimensions - output is broadcast_shape + remaining_shape */
    int tndim = mit->out_ndim + ctx->remaining_dims;
    nr_intp tshape[NR_NODE_MAX_NDIM];
    memcpy(tshape, mit->out_shape, sizeof(nr_intp) * mit->out_ndim);
    memcpy(&tshape[mit->out_ndim], ctx->remaining_shape, 
           sizeof(nr_intp) * ctx->remaining_dims);

    nr_size_t nitems = NR_NItems(tndim, tshape);
    char* out_data = malloc(nitems * bsize);
    if (!out_data) {
        free(mit);
        NError_RaiseMemoryError();
        return NULL;
    }

    char* temp_data_ptr = out_data;
    int num_iters = mit->n_iter;
    
    NCoordIter citer;
    NCoordIter_New(&citer, ctx->remaining_dims, ctx->remaining_shape);
    
    NMultiIter_ITER(mit);
    while (NMultiIter_NOTDONE(mit)) {
        nr_intp base_offset = 0;
        for (int i = 0; i < num_iters; i++) {
            nr_int64 index_val = *(nr_int64*)NMultiIter_ITEM(mit, i);
            base_offset += index_val * NODE_STRIDES(ctx->base_node)[i];
        }

        NCoordIter_ITER(&citer);
        while (NCoordIter_NOTDONE(&citer)) {
            nr_intp offset = base_offset;
            for (int j = 0; j < ctx->remaining_dims; j++) {
                nr_intp coord = NCoordIter_COORD(&citer, j);
                offset += coord * ctx->remaining_strides[j];
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

/* ============================================================================
 * GET Operation: Simple Indexing
 * ============================================================================ */

NR_STATIC_INLINE Node*
get_simple_indexing(IndexContext* ctx)
{
    no_node_indices_info* nnii = &ctx->no_node_info;
    
    if (!ctx->unpack_info.copy_needed) {
        return Node_NewChild(ctx->base_node, nnii->out_ndim, 
                             nnii->out_shape, nnii->out_strides, ctx->byte_offset);
    }

    nr_size_t bsize = NDtype_Size(NODE_DTYPE(ctx->base_node));
    NR_DTYPE dtype = NODE_DTYPE(ctx->base_node);
    
    nr_size_t nitems = NR_NItems(nnii->out_ndim, nnii->out_shape);
    char* out_data = malloc(nitems * bsize);
    if (!out_data) {
        NError_RaiseMemoryError();
        return NULL;
    }
    
    NIter iter;
    NIter_New(&iter, ctx->data_offset, nnii->out_ndim, 
              nnii->out_shape, nnii->out_strides, NITER_MODE_STRIDED);
    NIter_ITER(&iter);
    
    char* temp_data_ptr = out_data;
    while (NIter_NOTDONE(&iter)) {
        memcpy(temp_data_ptr, NIter_ITEM(&iter), bsize);
        temp_data_ptr += bsize;
        NIter_NEXT_STRIDED(&iter);
    }
    
    return Node_New(out_data, 1, nnii->out_ndim, nnii->out_shape, dtype);
}

/* ============================================================================
 * SET Operation: Simple Indexing
 * ============================================================================ */

NR_STATIC_INLINE int
set_simple_indexing(IndexContext* ctx, Node* value)
{
    no_node_indices_info* nnii = &ctx->no_node_info;
    nr_size_t bsize = NDtype_Size(NODE_DTYPE(ctx->base_node));
    
    int is_scalar = Node_NItems(value) == 1;
    char* scalar_data = is_scalar ? (char*)NODE_DATA(value) : NULL;

    NIter iter;
    NIter_New(&iter, ctx->data_offset, nnii->out_ndim, 
              nnii->out_shape, nnii->out_strides, NITER_MODE_STRIDED);
    NIter_ITER(&iter);

    nr_intp value_idx = 0;
    while (NIter_NOTDONE(&iter)) {
        if (is_scalar) {
            memcpy(NIter_ITEM(&iter), scalar_data, bsize);
        } else {
            memcpy(NIter_ITEM(&iter), 
                   (char*)NODE_DATA(value) + value_idx * bsize, bsize);
            value_idx++;
        }
        NIter_NEXT_STRIDED(&iter);
    }

    return 0;
}

/* ============================================================================
 * SET Operation: Flat Boolean Indexing
 * ============================================================================ */

NR_STATIC_INLINE int
set_flat_bool_indexing(Node* base_node, NIndexRuleSet* rs, Node* value)
{
    NIndexRule* rule = &NIndexRuleSet_RULES(rs)[0];
    Node* index_node = NIndexRule_DATA_AS_NODE(rule).node;
    char* base_data = (char*)NODE_DATA(base_node);
    
    int is_scalar = Node_NItems(value) == 1;
    char* scalar_data = is_scalar ? (char*)NODE_DATA(value) : NULL;
    nr_size_t bsize = NDtype_Size(NODE_DTYPE(base_node));
    
    int is_same_shape = Node_SameShape(base_node, index_node);
    int is_index_c = NODE_IS_CONTIGUOUS(index_node);
    int is_base_c = NODE_IS_CONTIGUOUS(base_node);
    
    nr_intp nitems = NR_NItems(base_node->ndim, base_node->shape);
    nr_intp value_idx = 0;
    
    if (is_same_shape && is_index_c && is_base_c) {
        for (nr_intp i = 0; i < nitems; i++) {
            nr_bool val = ((nr_bool*)index_node->data)[i];
            if (val) {
                if (is_scalar) {
                    memcpy(base_data + i * bsize, scalar_data, bsize);
                } else {
                    memcpy(base_data + i * bsize, 
                           (char*)NODE_DATA(value) + value_idx * bsize, bsize);
                    value_idx++;
                }
            }
        }
    } else {
        Node* nodes[] = {base_node, index_node};
        NMultiIter mit;
        if (NMultiIter_FromNodes(nodes, 2, &mit) < 0) {
            return -1;
        }
        
        NMultiIter_ITER(&mit);
        while (NMultiIter_NOTDONE(&mit)) {
            nr_bool val = *(nr_bool*)NMultiIter_ITEM(&mit, 1);
            if (val) {
                if (is_scalar) {
                    memcpy(NMultiIter_ITEM(&mit, 0), scalar_data, bsize);
                } else {
                    memcpy(NMultiIter_ITEM(&mit, 0), 
                           (char*)NODE_DATA(value) + value_idx * bsize, bsize);
                    value_idx++;
                }
            }
            NMultiIter_NEXT2(&mit);
        }
    }
    
    return 0;
}

/* ============================================================================
 * SET Operation: Fancy Indexing (node indices)
 * ============================================================================ */

NR_STATIC_INLINE int
set_fancy_indexing(IndexContext* ctx, Node* value)
{
    NMultiIter* mit = malloc(sizeof(NMultiIter));
    if (!mit) {
        NError_RaiseError(NError_ValueError, 
            "Failed to allocate memory for NMultiIter");
        return -1;
    }
    
    if (NMultiIter_FromNodes(ctx->node_info.nodes, 
                             ctx->node_info.node_count, mit) < 0) {
        free(mit);
        return -1;
    }
    
    char* node_data = (char*)NODE_DATA(ctx->base_node) + ctx->byte_offset;
    nr_size_t bsize = NDtype_Size(NODE_DTYPE(ctx->base_node));
    
    int is_scalar = Node_NItems(value) == 1;
    char* scalar_data = is_scalar ? (char*)NODE_DATA(value) : NULL;
    nr_intp value_idx = 0;

    /* No remaining dimensions */
    if (ctx->remaining_dims <= 0) {
        int num_iters = mit->n_iter;
        
        NMultiIter_ITER(mit);
        while (NMultiIter_NOTDONE(mit)) {
            nr_intp step = 0;
            for (int i = 0; i < num_iters; i++) {
                int in_node_dim = ctx->node_info.in_node_dims[i];
                step += (*(nr_int64*)NMultiIter_ITEM(mit, i)) * 
                        NODE_STRIDES(ctx->base_node)[in_node_dim];
            }
            
            if (is_scalar) {
                memcpy(node_data + step, scalar_data, bsize);
            } else {
                memcpy(node_data + step, 
                       (char*)NODE_DATA(value) + value_idx * bsize, bsize);
                value_idx++;
            }
            
            NMultiIter_NEXT(mit);
        }
        
        free(mit);
        return 0;
    }

    /* Has remaining dimensions */
    int num_iters = mit->n_iter;
    
    NCoordIter citer;
    NCoordIter_New(&citer, ctx->remaining_dims, ctx->remaining_shape);
    
    NMultiIter_ITER(mit);
    while (NMultiIter_NOTDONE(mit)) {
        nr_intp base_offset = 0;
        for (int i = 0; i < num_iters; i++) {
            nr_int64 index_val = *(nr_int64*)NMultiIter_ITEM(mit, i);
            base_offset += index_val * NODE_STRIDES(ctx->base_node)[i];
        }

        NCoordIter_ITER(&citer);
        while (NCoordIter_NOTDONE(&citer)) {
            nr_intp offset = base_offset;
            for (int j = 0; j < ctx->remaining_dims; j++) {
                nr_intp coord = NCoordIter_COORD(&citer, j);
                offset += coord * ctx->remaining_strides[j];
            }
            
            if (is_scalar) {
                memcpy(node_data + offset, scalar_data, bsize);
            } else {
                memcpy(node_data + offset, 
                       (char*)NODE_DATA(value) + value_idx * bsize, bsize);
                value_idx++;
            }
            NCoordIter_NEXT(&citer);
        }
        
        NMultiIter_NEXT(mit);
    }

    free(mit);
    return 0;
}


NR_STATIC_INLINE int
set_no_rules_indexing(Node* base_node, Node* value)
{
    if (Node_SameShape(base_node, value)) {
        nr_size_t bsize = NDtype_Size(NODE_DTYPE(base_node));
        nr_intp nitems = Node_NItems(base_node);
        
        if (NODE_IS_CONTIGUOUS(base_node) && NODE_IS_CONTIGUOUS(value)) {
             memcpy(NODE_DATA(base_node), NODE_DATA(value), nitems * bsize);
        } else {
            NIter base_iter, val_iter;
            NIter_FromNode(&base_iter, base_node, NITER_MODE_STRIDED);
            NIter_FromNode(&val_iter, value, NITER_MODE_STRIDED);
            
            NIter_ITER(&base_iter);
            NIter_ITER(&val_iter);
            
            while (NIter_NOTDONE(&base_iter)) {
                memcpy(NIter_ITEM(&base_iter), NIter_ITEM(&val_iter), bsize);
                NIter_NEXT_STRIDED(&base_iter);
                NIter_NEXT_STRIDED(&val_iter);
            }
        }
    } else {
        Node* nodes[] = {base_node, value};
        NMultiIter mit;
        if (NMultiIter_FromNodes(nodes, 2, &mit) < 0) {
            return -1;
        }
        
        nr_size_t bsize = NDtype_Size(NODE_DTYPE(base_node));
        NMultiIter_ITER(&mit);
        while (NMultiIter_NOTDONE(&mit)) {
            memcpy(NMultiIter_ITEM(&mit, 0), NMultiIter_ITEM(&mit, 1), bsize);
            NMultiIter_NEXT2(&mit);
        }
    }
    return 0;
}

/* ============================================================================
 * Public API: Node_Get (GET operation)
 * ============================================================================ */

NR_STATIC_INLINE Node*
node_index_internal(Node* base_node, NIndexRuleSet* rs, int risky_indexing)
{
    int num_rules = NIndexRuleSet_NUM_RULES(rs);
    
    /* No indexing rules - return view */
    if (num_rules == 0 || !rs) {
        return Node_NewChild(base_node, base_node->ndim, 
                            base_node->shape, base_node->strides, 0);
    }
    
    /* Initialize context */
    IndexContext ctx = {0};
    ctx.base_node = base_node;
    ctx.rs = rs;
    ctx.risky_indexing = risky_indexing;
    
    /* Unpack indices */
    if (unpack_indices(rs, &ctx.unpack_info) < 0) {
        return NULL;
    }
    ctx.unpack_info.risky_indexing = risky_indexing;
    
    /* Validate dimensions */
    int ndim = base_node->ndim;
    int num_used_dims = num_rules - ctx.unpack_info.new_axis_dims;
    int ellipsis_dims = ndim - num_rules + 1 + ctx.unpack_info.new_axis_dims;
    
    if (ctx.unpack_info.index_type & HAS_ELLIPSIS) {
        num_used_dims += ellipsis_dims - 1;
    }

    if (num_used_dims > base_node->ndim) {
        NError_RaiseError(NError_IndexError,
            "Too many indices for array: array is %d-dimensional, "
            "but %d were indexed", base_node->ndim, num_used_dims);
        return NULL;
    }

    int num_keeped_dims = ctx.unpack_info.keeped_dims;
    if (ctx.unpack_info.index_type & HAS_ELLIPSIS) {
        num_keeped_dims += ellipsis_dims;
    }
    int out_dim = num_keeped_dims + ctx.unpack_info.new_axis_dims;
    
    if (out_dim > NR_NODE_MAX_NDIM) {
        NError_RaiseError(NError_IndexError,
            "Resulting array has too many dimensions: %d > %d",
            out_dim, NR_NODE_MAX_NDIM);
        return NULL;
    }
    
    /* Compute indexing context */
    if (compute_index_context(&ctx) < 0) {
        if (ctx.has_node_indices) {
            free_nodes_in_info(&ctx.node_info);
        }
        return NULL;
    }
    
    /* Execute GET based on index type */
    Node* result = NULL;
    
    if (ctx.is_flat_bool) {
        result = get_flat_bool_indexing(base_node, rs);
    } else if (ctx.has_node_indices) {
        result = get_fancy_indexing(&ctx);
        free_nodes_in_info(&ctx.node_info);
    } else {
        result = get_simple_indexing(&ctx);
    }
    
    return result;
}

/* GetItem NFunc */
NR_PRIVATE int
GetItem_function(NFuncArgs* args)
{
    Node* base_node = args->in_nodes[0];
    IndexOpArgs* op_args = (IndexOpArgs*)args->extra;
    
    Node* result = node_index_internal(base_node, op_args->rs, op_args->risky);
    if (!result) return -1;
    
    args->out_nodes[0] = result;
    return 0;
}

const NFunc getitem_nfunc = {
    .name = "getitem",
    .flags = 0,
    .nin = 1,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_NONE,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = GetItem_function,
    .grad_func = NULL
};

NR_PUBLIC Node*
Node_Get(Node* base_node, NIndexRuleSet* rs)
{
    IndexOpArgs args = {rs, 0};
    NFuncArgs* fargs = NFuncArgs_New(1, 1);
    fargs->in_nodes[0] = base_node;
    fargs->extra = &args;
    
    int result = NFunc_Call(&getitem_nfunc, fargs);
    Node* out = fargs->out_nodes[0];
    if (out) NODE_INCREF(out);
    NFuncArgs_DECREF(fargs);
    
    return result != 0 ? NULL : out;
}

NR_PUBLIC Node*
Node_RiskyGet(Node* base_node, NIndexRuleSet* rs)
{
    IndexOpArgs args = {rs, 1};
    NFuncArgs* fargs = NFuncArgs_New(1, 1);
    fargs->in_nodes[0] = base_node;
    fargs->extra = &args;
    
    int result = NFunc_Call(&getitem_nfunc, fargs);
    Node* out = fargs->out_nodes[0];
    if (out) NODE_INCREF(out);
    NFuncArgs_DECREF(fargs);
    
    return result != 0 ? NULL : out;
}

/* ============================================================================
 * Public API: Node_SetItem (SET operation)
 * ============================================================================ */

NR_STATIC_INLINE int
node_setitem_internal(Node* base_node, NIndexRuleSet* rs, 
                      Node* value, int risky_indexing)
{
    /* Type Check & Cast */
    Node* casted_value = value;
    int is_temp = 0;
    if (NODE_DTYPE(base_node) != NODE_DTYPE(value)) {
        casted_value = Node_ToType(NULL, value, NODE_DTYPE(base_node));
        if (!casted_value) return -1;
        is_temp = 1;
    }

    int num_rules = rs ? NIndexRuleSet_NUM_RULES(rs) : 0;
    int result = 0;
    
    /* No indexing rules */
    if (num_rules == 0) {
        result = set_no_rules_indexing(base_node, casted_value);
        if (is_temp) Node_Free(casted_value);
        return result;
    }
    
    /* Initialize context */
    IndexContext ctx = {0};
    ctx.base_node = base_node;
    ctx.rs = rs;
    ctx.risky_indexing = risky_indexing;
    
    /* Unpack indices */
    if (unpack_indices(rs, &ctx.unpack_info) < 0) {
        if (is_temp) Node_Free(casted_value);
        return -1;
    }
    ctx.unpack_info.risky_indexing = risky_indexing;
    
    /* Validate dimensions */
    int ndim = base_node->ndim;
    int num_used_dims = num_rules - ctx.unpack_info.new_axis_dims;
    int ellipsis_dims = ndim - num_rules + 1 + ctx.unpack_info.new_axis_dims;
    
    if (ctx.unpack_info.index_type & HAS_ELLIPSIS) {
        num_used_dims += ellipsis_dims - 1;
    }

    if (num_used_dims > base_node->ndim) {
        NError_RaiseError(NError_IndexError,
            "Too many indices for array: array is %d-dimensional, "
            "but %d were indexed", base_node->ndim, num_used_dims);
        if (is_temp) Node_Free(casted_value);
        return -1;
    }
    
    /* Compute indexing context */
    if (compute_index_context(&ctx) < 0) {
        if (ctx.has_node_indices) {
            free_nodes_in_info(&ctx.node_info);
        }
        if (is_temp) Node_Free(casted_value);
        return -1;
    }
    
    /* Execute SET based on index type */
    if (ctx.is_flat_bool) {
        result = set_flat_bool_indexing(base_node, rs, casted_value);
    } else if (ctx.has_node_indices) {
        result = set_fancy_indexing(&ctx, casted_value);
        free_nodes_in_info(&ctx.node_info);
    } else {
        result = set_simple_indexing(&ctx, casted_value);
    }
    
    if (is_temp) Node_Free(casted_value);
    return result;
}


/* SetItem NFunc */
NR_PRIVATE int
SetItem_function(NFuncArgs* args)
{
    Node* base_node = args->in_nodes[0];
    Node* value = args->in_nodes[1];
    IndexOpArgs* op_args = (IndexOpArgs*)args->extra;
    
    if (node_setitem_internal(base_node, op_args->rs, value, op_args->risky) < 0) {
        return -1;
    }
    
    /* SetItem modifies in-place, return base node */
    args->out_nodes[0] = base_node;
    NODE_INCREF(base_node);
    return 0;
}

const NFunc setitem_nfunc = {
    .name = "setitem",
    .flags = 0,
    .nin = 2,
    .nout = 1,
    .in_type = NDTYPE_NONE,
    .out_type = NDTYPE_NONE,
    .in_dtype = NR_NONE,
    .out_dtype = NR_NONE,
    .func = SetItem_function,
    .grad_func = NULL
};

NR_PUBLIC int
Node_Set(Node* base_node, NIndexRuleSet* rs, Node* value)
{
    IndexOpArgs args = {rs, 0};
    NFuncArgs* fargs = NFuncArgs_New(2, 1);
    fargs->in_nodes[0] = base_node;
    fargs->in_nodes[1] = value;
    fargs->extra = &args;
    
    int result = NFunc_Call(&setitem_nfunc, fargs);
    NFuncArgs_DECREF(fargs);
    
    return result;
}

NR_PUBLIC int
Node_RiskySet(Node* base_node, NIndexRuleSet* rs, Node* value)
{
    IndexOpArgs args = {rs, 1};
    NFuncArgs* fargs = NFuncArgs_New(2, 1);
    fargs->in_nodes[0] = base_node;
    fargs->in_nodes[1] = value;
    fargs->extra = &args;
    
    int result = NFunc_Call(&setitem_nfunc, fargs);
    NFuncArgs_DECREF(fargs);
    
    return result;
}

/* ============================================================================
 * Set Helpers & Shortcuts
 * ============================================================================ */

NR_PUBLIC int
Node_SetNumber(Node* base_node, NIndexRuleSet* rs, void* num, NR_DTYPE dtype)
{
    Node* scalar = Node_NewScalar(num, dtype);
    if (!scalar) return -1;
    int result = Node_Set(base_node, rs, scalar);
    Node_Free(scalar);
    return result;
}

NR_PUBLIC int
Node_SetArray(Node* base_node, NIndexRuleSet* rs, void* data, int ndim, nr_intp* shape, nr_intp* strides, NR_DTYPE dtype)
{
    /* Create a view of the data (copy_data=0) */
    Node* value_node = Node_NewAdvanced(data, 0, ndim, shape, strides, dtype, 0, NULL);
    if (!value_node) {
        return -1;
    }
    
    int result = Node_Set(base_node, rs, value_node);
    
    /* Node_NewAdvanced with copy_data=0 does not own the data, so Node_Free will just free the struct */
    Node_Free(value_node);
    return result;
}

NR_PUBLIC int Node_SetBool(Node* n, NIndexRuleSet* rs, nr_bool val) { return Node_SetNumber(n, rs, &val, NR_BOOL); }
NR_PUBLIC int Node_SetByte(Node* n, NIndexRuleSet* rs, nr_int8 val) { return Node_SetNumber(n, rs, &val, NR_INT8); }
NR_PUBLIC int Node_SetUByte(Node* n, NIndexRuleSet* rs, nr_uint8 val) { return Node_SetNumber(n, rs, &val, NR_UINT8); }
NR_PUBLIC int Node_SetShort(Node* n, NIndexRuleSet* rs, nr_int16 val) { return Node_SetNumber(n, rs, &val, NR_INT16); }
NR_PUBLIC int Node_SetUShort(Node* n, NIndexRuleSet* rs, nr_uint16 val) { return Node_SetNumber(n, rs, &val, NR_UINT16); }
NR_PUBLIC int Node_SetInt(Node* n, NIndexRuleSet* rs, nr_int32 val) { return Node_SetNumber(n, rs, &val, NR_INT32); }
NR_PUBLIC int Node_SetUInt(Node* n, NIndexRuleSet* rs, nr_uint32 val) { return Node_SetNumber(n, rs, &val, NR_UINT32); }
NR_PUBLIC int Node_SetLong(Node* n, NIndexRuleSet* rs, nr_int64 val) { return Node_SetNumber(n, rs, &val, NR_INT64); }
NR_PUBLIC int Node_SetULong(Node* n, NIndexRuleSet* rs, nr_uint64 val) { return Node_SetNumber(n, rs, &val, NR_UINT64); }
NR_PUBLIC int Node_SetFloat(Node* n, NIndexRuleSet* rs, nr_float32 val) { return Node_SetNumber(n, rs, &val, NR_FLOAT32); }
NR_PUBLIC int Node_SetDouble(Node* n, NIndexRuleSet* rs, nr_float64 val) { return Node_SetNumber(n, rs, &val, NR_FLOAT64); }