#include "nour/nour.h"
#include "nfunc.h"
#include "ntools.h"
#include "nerror.h"
#include "node_core.h"
#include "free.h"
#include "tc_methods.h"

#define DT_VALID(dtype) NDtype_IsValid(dtype)
#define SELF_CREATED_OUT_NODES_STACK_SIZE 16

NR_PRIVATE NR_DTYPE
resolve_dtype(NR_DTYPE dtype, int type_constraint) {
    switch (type_constraint) {
        case NDTYPE_FLOAT:
            /* If dtype is already a valid dtype we return it, otherwise default to NR_FLOAT64.
               (This keeps behavior similar to original intent but uses DT_VALID for safety.) */
            return NDtype_IsFloat(dtype) ? dtype : NR_FLOAT64;
        case NDTYPE_BOOL:
            return NR_BOOL;
        case NDTYPE_INT:
            /* Accept integer-like dtypes; if dtype is invalid return NR_INT64 */
            return NDtype_IsInteger(dtype) ? dtype : NR_INT64;
        default:
            return DT_VALID(dtype) ? dtype : NR_NONE;
    }
}

NR_PRIVATE int
check_in_nums(const NFunc* nfunc, NFuncArgs* args){
    if (nfunc->nin != args->nin){
        NError_RaiseError(
            NError_ValueError,
            "%s function expected %d input nodes, but got %d",
            nfunc->name,
            nfunc->nin,
            args->nin
        );
        return -1;
    }

    if (nfunc->nout != args->nout){
        NError_RaiseError(
            NError_ValueError,
            "%s function expected %d output nodes, but got %d",
            nfunc->name,
            nfunc->nout,
            args->nout
        );
        return -1;
    }

    return 0;
}

NR_PRIVATE int
understand_dtypes(const NFunc* nfunc, NFuncArgs* args, NR_DTYPE* in_dtype ,NR_DTYPE* out_dtype){
    int flags = nfunc->flags;
    int type_broadcastable = flags & NFUNC_FLAG_TYPE_BROADCASTABLE;

    int in_type = nfunc->in_type;
    int out_type = nfunc->out_type;

    NR_DTYPE in_dtype_local = nfunc->in_dtype;
    NR_DTYPE out_dtype_local = nfunc->out_dtype;

    // Start with checking input dtypes
    if (DT_VALID(in_dtype_local)){
        // All inputs must be of this dtype
        for (int i = 0; i < args->nin; i++){
            Node* node = args->in_nodes[i];
            NR_DTYPE node_dtype = NODE_DTYPE(node);
            if (node_dtype != in_dtype_local){
                NError_RaiseError(
                    NError_TypeError,
                    "Function '%s' expected all input nodes to have data type %d, but got %d",
                    nfunc->name,
                    in_dtype_local,
                    node_dtype
                );
                return -1;
            }
        }

        *in_dtype = in_dtype_local;
    }
    else{
        if (type_broadcastable || in_type == NDTYPE_NONE){
            // input dtypes is the broadcasted dtype of all inputs
            NR_DTYPE broadcasted_dtype = NODE_DTYPE(args->in_nodes[0]);
            for (int i = 1; i < args->nin; i++){
                broadcasted_dtype = NTools_BroadcastDtypes(broadcasted_dtype, NODE_DTYPE(args->in_nodes[i]));
            }
            *in_dtype = resolve_dtype(broadcasted_dtype, in_type);
        }
        else{
            // input dtypes is resolved from the type constraint
            NR_DTYPE resolved_dtype = resolve_dtype(-1, in_type);
            *in_dtype = resolved_dtype;
        }
    }

    *out_dtype = DT_VALID(out_dtype_local) ? out_dtype_local : resolve_dtype(*in_dtype, out_type);
    return 0;
}

/*
 * understand_user_out_nodes:
 *  - Fills `so` (stack array) with indices of NULL output slots up to SELF_CREATED_OUT_NODES_STACK_SIZE.
 *  - If more nulls exist, allocates an int array and sets *so2 to it.
 *  - Returns user_nout (number of non-NULL user provided outputs) or -1 on error.
 *
 * Ownership / cleanup:
 *  - If this function allocates *so2 it is the caller's responsibility to free it.
 *  - However we standardize on calling clear_self_created_out_nodes_info(args, so, so2, user_nout)
 *    to free so2 (and DECREF created nodes if they were created).
 */
NR_PRIVATE int
understand_user_out_nodes(const NFunc* nfunc, NFuncArgs* args, NR_DTYPE out_dtype, int so[], int** so2){
    *so2 = NULL; /* default to NULL so caller can always call cleanup safely */

    if (!args->out_nodes || args->nout <= 0){
        return 0;
    }
    
    // Check if function prohibits user-provided output nodes
    if (nfunc->flags & NFUNC_FLAG_NO_USER_OUT_NODES) {
        // Count non-NULL output nodes
        for (int i = 0; i < args->nout; i++) {
            if (args->out_nodes[i]) {
                NError_RaiseError(
                    NError_ValueError,
                    "Function '%s' does not allow user-provided output nodes",
                    nfunc->name
                );
                return -1;
            }
        }
    }

    int check_out_dtype = DT_VALID(out_dtype) && !(nfunc->flags & NFUNC_FLAG_OUT_DTYPES_NOT_SAME);
    int nout = args->nout;
    int user_nout = 0;
    for (int i = 0; i < nout; i++){
        Node* out_node = args->out_nodes[i];
        if (out_node){
            if (check_out_dtype && (NODE_DTYPE(out_node) != out_dtype)){
                NError_RaiseError(
                    NError_TypeError,
                    "%s function expected output node %d to have data type %d, but got %d",
                    nfunc->name,
                    i,
                    out_dtype,
                    NODE_DTYPE(out_node)
                );
                return -1;
            }

            user_nout++;
        }
    }

    int current = 0;
    int null_count = nout - user_nout;
    if (null_count <= SELF_CREATED_OUT_NODES_STACK_SIZE){
        for (int i = 0; i < nout; i++){
            Node* out_node = args->out_nodes[i];
            if (!out_node){
                so[current++] = i;
            }
        }
        *so2 = NULL;
    } else {
        int* so2_local = (int*)malloc(sizeof(int) * null_count);
        if (!so2_local){
            NError_RaiseMemoryError();
            return -1;
        }
        for (int i = 0; i < nout; i++){
            Node* out_node = args->out_nodes[i];
            if (!out_node){
                so2_local[current++] = i;
            }
        }
        *so2 = so2_local;
    }
    
    return user_nout;
}

/*
 * clear_self_created_out_nodes_info:
 *  - Clean up any self-created output nodes. Called after the nfunc executes or when a later
 *    step fails and we need to roll back. It will DECREF only if args->out_nodes[pos] is non-NULL.
 *  - Also frees the dynamically allocated so2 array if present.
 */
NR_PRIVATE void
clear_self_created_out_nodes_info(NFuncArgs* args, int so[], int* so2, int user_nout){
    if (!args){
        if (so2){
            free(so2);
        }
        return;
    }

    if (user_nout == args->nout){
        /* Nothing to cleanup - ensure so2 freed if allocated */
        if (so2){
            free(so2);
        }
        return;
    }

    int* arr = so2 ? so2 : so;
    int num_self_created = args->nout - user_nout;

    /* Only DECREF if the output node is non-NULL; some error paths call this before the
       function has created outputs, so we must be null-safe. */
    for (int i = 0; i < num_self_created; i++){
        int pos = arr[i];
        if (args->out_nodes && args->out_nodes[pos]){
            NODE_DECREF(args->out_nodes[pos]);
            args->out_nodes[pos] = NULL;
        }
    }

    if (so2){
        free(so2);
    }
}

/*
 * broadcast_nodes:
 *  - If dtype promotion is required, create promoted copies (Node_ToType).
 *  - On any internal failure it frees any already created promoted nodes and the array.
 *  - Returns the `promoted_nodes` array on success (caller must free with clear_broadcasted_nodes).
 *  - Returns args->in_nodes itself if no promotion required.
 */
NR_PRIVATE Node**
broadcast_nodes(const NFunc* nfunc, NFuncArgs* args, NR_DTYPE in_dtype){
    int nin = nfunc->nin;
    int flags = nfunc->flags;
    int broadcastable = flags & NFUNC_FLAG_TYPE_BROADCASTABLE;
    if (!broadcastable || nin <= 0 || !DT_VALID(in_dtype)){
        return args->in_nodes;
    }

    Node** promoted_nodes = (Node**)malloc(sizeof(Node*) * nin);
    if (!promoted_nodes){
        NError_RaiseMemoryError();
        return NULL;
    }

    for (int i = 0; i < nin; i++){
        Node* node = args->in_nodes[i];
        NR_DTYPE node_dtype = NODE_DTYPE(node);
        if (node_dtype != in_dtype){
            Node* promoted_node = Node_ToType(NULL, node, in_dtype);
            if (!promoted_node){
                /* Free any promoted nodes created so far */
                for (int j = 0; j < i; j++){
                    if (promoted_nodes[j] != args->in_nodes[j]){
                        Node_Free(promoted_nodes[j]);
                    }
                }
                free(promoted_nodes);
                return NULL;
            }
            promoted_nodes[i] = promoted_node;
        } else {
            promoted_nodes[i] = node;
        }
    }

    return promoted_nodes;
}

NR_PRIVATE void
clear_broadcasted_nodes(const NFunc* nfunc, Node** original_nodes, Node** promoted_nodes){
    if (!promoted_nodes || promoted_nodes == original_nodes){
        return;
    }
    int nin = nfunc->nin;
    for (int i = 0; i < nin; i++){
        if (promoted_nodes[i] != original_nodes[i]){
            Node_Free(promoted_nodes[i]);
        }
    }
    free(promoted_nodes);
}

/*
 * track_out_node_if_needed:
 *  - If output nodes are tracked, registers the function info to the nodes.
 *  - On failure it will unregister any nodes that were registered in this call.
 */
NR_PRIVATE int
track_out_node_if_needed(const NFunc* nfunc, NFuncArgs* args){
    if (!args->out_nodes || args->nout <= 0){
        return 0;
    }

    for (int i = 0; i < args->nout; i++){
        Node* out_node = args->out_nodes[i];
        if (NODE_IS_TRACK(out_node)){
            int res = _NFuncFuncInfo_RegisterToNode(out_node, nfunc, args, i);
            if (res < 0){
                /* Unregister previous nodes that were registered in this loop */
                for (int j = 0; j < i; j++){
                    Node* prev_out_node = args->out_nodes[j];
                    if (prev_out_node && NODE_IS_TRACK(prev_out_node)){
                        if (prev_out_node->nfunc_info){
                            _NFuncFuncInfo_Free((NFuncFuncInfo*)prev_out_node->nfunc_info);
                            prev_out_node->nfunc_info = NULL;
                        }
                    }
                }
                return -1;
            }
        }
    }
    
    return 0;
}

/*
 * NOTE: This function had a wrong NULL check previously (it rejected nodes with NULL nfunc_info).
 * Correct behavior:
 *  - `node` must be non-NULL.
 *  - We create a new NFuncFuncInfo and assign it to node->nfunc_info.
 *  - If the node already has a nfunc_info we treat it as an error (avoid overwrite) unless your
 *    design allows replacing — modify as needed.
 */
NR_PUBLIC int
_NFuncFuncInfo_RegisterToNode(Node* node, const NFunc* nfunc, NFuncArgs* args, int out_idx){
    if (!node){
        NError_RaiseError(NError_ValueError, "_NFuncFuncInfo_RegisterToNode received NULL node");
        return -1;
    }

    /* Create new NFuncFuncInfo that we will assign to node */
    NFuncFuncInfo* new_nfunc_info = _NFuncFuncInfo_New(nfunc, args, out_idx);
    if (!new_nfunc_info){
        return -1;
    }

    /* Validate that the node pointer matches the expected output node from the original info */
    Node* expected_out_node = args->out_nodes[out_idx];
    if (expected_out_node && node != expected_out_node){
        NError_RaiseError(
            NError_ValueError,
            "_NFuncFuncInfo_RegisterToNode: node address does not match the expected output node"
        );
        _NFuncFuncInfo_Free(new_nfunc_info);
        return -1;
    }

    /* Defensive: avoid overwriting an existing nfunc_info. If your design needs to replace it,
       change this to free then assign. */
    if (node->nfunc_info){
        NError_RaiseError(
            NError_ValueError,
            "_NFuncFuncInfo_RegisterToNode: node already has nfunc_info (refuse to overwrite)"
        );
        _NFuncFuncInfo_Free(new_nfunc_info);
        return -1;
    }

    /* Assign the created info to the node. Ownership transferred to the node. */
    node->nfunc_info = (struct NFuncFuncInfo*)new_nfunc_info;
    return 0;
}

NR_PUBLIC void
_NFuncFuncInfo_Free(NFuncFuncInfo* nfunc_info){
    if (!nfunc_info){
        return;
    }
    NFuncArgs_DECREF(nfunc_info->args);
    free(nfunc_info);
}

NR_PUBLIC int
NFunc_Call(const NFunc* nfunc, NFuncArgs* args){
    if (!nfunc || !args){
        NError_RaiseError(NError_ValueError, "NFunc_Call received NULL arguments");
        return -1;
    }

    if (check_in_nums(nfunc, args) < 0){
        return -1;
    }

    /* Fast path for metadata-only functions that must not touch data buffers. */
    if (nfunc->flags & NFUNC_FLAG_NO_DATA){
        /* Enforce in-place semantics if declared; out node must be NULL or same pointer. */
        if (args->nout == 1){
            Node* in = args->in_nodes[0];
            Node* out = args->out_nodes[0];
            if (out && out != in){
                NError_RaiseError(
                    NError_ValueError,
                    "%s: NO_DATA functions require output node to be same as input or NULL",
                    nfunc->name
                );
                return -1;
            }
            if (!out){
                args->out_nodes[0] = in; /* implicit in-place */
            }
        }
        /* Assign dtype pass-through. */
        if (args->nin > 0 && args->in_nodes[0]){
            args->outtype = NODE_DTYPE(args->in_nodes[0]);
        } else {
            args->outtype = NR_NONE;
        }

        int result = nfunc->func(args);
        if (result < 0){
            return -1;
        }
        /* Tracking still allowed. */
        if (track_out_node_if_needed(nfunc, args) < 0){
            return -1;
        }
        return result;
    }

    NR_DTYPE in_dtype = -1;
    NR_DTYPE out_dtype = -1;
    if (understand_dtypes(nfunc, args, &in_dtype, &out_dtype) < 0){
        return -1;
    }
    args->outtype = out_dtype;

    int so[SELF_CREATED_OUT_NODES_STACK_SIZE];
    int* so2 = NULL;
    int user_nout = understand_user_out_nodes(nfunc, args, out_dtype, so, &so2);
    if (user_nout < 0){
        /* understand_user_out_nodes already raised appropriate error */
        return -1;
    }

    Node** broadcasted_nodes = broadcast_nodes(nfunc, args, in_dtype);
    if (!broadcasted_nodes){
        /* Nothing created in out_nodes by function yet; cleanup the so2 allocation */
        clear_self_created_out_nodes_info(args, so, so2, user_nout);
        return -1;
    }

    Node** original_nodes = args->in_nodes;
    args->in_nodes = broadcasted_nodes;
    
    int result = nfunc->func(args);

    /* restore original inputs and free promoted nodes */
    args->in_nodes = original_nodes;
    clear_broadcasted_nodes(nfunc, original_nodes, broadcasted_nodes);

    if (result < 0){
        /* clean up any so2 and (if any outputs were created before the failure) DECREF them */
        clear_self_created_out_nodes_info(args, so, so2, user_nout);
        return -1;
    }

    /* Register tracking info if needed. If that fails, rollback outputs created by this call */
    if (track_out_node_if_needed(nfunc, args) < 0){
        clear_self_created_out_nodes_info(args, so, so2, user_nout);
        return -1;
    }

    /* Successful path: ownership of any self-created output nodes remains with args. Free so2 if present. */
    if (so2){
        /* clear_self_created_out_nodes_info would have freed so2 earlier on error paths.
           Here we only free the allocation marker array because outputs were created successfully. */
        free(so2);
    }

    return result;
}

NR_PUBLIC NFuncArgs* 
NFuncArgs_New(int nin, int nout)
{
    NFuncArgs* args = (NFuncArgs*)malloc(sizeof(NFuncArgs));
    if (!args){
        NError_RaiseMemoryError();
        return NULL;
    }
    args->nin = nin;
    if (nin > 0){
        args->in_nodes = (Node**)malloc(sizeof(Node*) * nin);
        if (!args->in_nodes){
            free(args);
            NError_RaiseMemoryError();
            return NULL;
        }
        for (int i = 0; i < nin; i++){
            args->in_nodes[i] = NULL;
        }
    }
    else{
        args->in_nodes = NULL;
    }

    args->nout = nout;
    if (nout > 0){
        args->out_nodes = (Node**)malloc(sizeof(Node*) * nout);
        if (!args->out_nodes){
            if (args->in_nodes) free(args->in_nodes);
            free(args);
            NError_RaiseMemoryError();
            return NULL;
        }
        for (int i = 0; i < nout; i++){
            args->out_nodes[i] = NULL;
        }
    }
    else{
        args->out_nodes = NULL;
    }

    args->outtype = NR_NONE;
    args->extra = NULL;
    args->__ref_count = 1;
    return args;
}

NR_PUBLIC void
_NFuncArgs_DECREF(NFuncArgs* args){
    if (!args){
        return;
    }
    
    if (args->__ref_count > 1){
        args->__ref_count--;
        return;
    }

    if (args->in_nodes){
        free(args->in_nodes);
    }
    if (args->out_nodes){
        free(args->out_nodes);
    }
    free(args);
}

NR_PUBLIC NFuncFuncInfo*
_NFuncFuncInfo_New(const NFunc* nfunc, NFuncArgs* args, int out_idx){
    NFuncFuncInfo* nfunc_info = (NFuncFuncInfo*)malloc(sizeof(NFuncFuncInfo));
    if (!nfunc_info){
        NError_RaiseMemoryError();
        return NULL;
    }
    nfunc_info->nfunc = nfunc;
    nfunc_info->args = args;
    nfunc_info->out_idx = out_idx;
    NFuncArgs_INCREF(args);
    return nfunc_info;
}
