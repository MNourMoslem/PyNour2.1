#include "nour/nour.h"
#include "../niter.h"
#include "../node_core.h"
#include "../nerror.h"
#include "../tc_methods.h"
#include "../ntools.h"
#include "../free.h"
#include "loops.h"
#include <string.h>

typedef struct
{
    NR_DTYPE intype;
    NR_DTYPE outtype;
} __nmath_iotypes;

/* Helper function to resolve dtype based on type constraints */
NR_PRIVATE NR_DTYPE
_ResolveDtype(NR_DTYPE dtype, int type_constraint) {
    switch (type_constraint) {
        case NDTYPE_NONE:
            return dtype;
        case NDTYPE_FLOAT:
            return (dtype > NR_UINT64 && dtype < NR_NUM_NUMIRC_DT) ? dtype : NR_FLOAT64;
        case NDTYPE_BOOL:
            return NR_BOOL;
        case NDTYPE_INT:
            return (dtype <= NR_UINT64) ? dtype : NR_INT64;
        default:
            return dtype;
    }
}

/* Unified function to check and resolve dtypes */
NR_PRIVATE void
_CheckDtypes(__nmath_iotypes* iot, Node* c, Node* a, Node* b, int outtype, int intype) {
    NR_DTYPE input_dtype, output_dtype;
    
    if (b) {
        /* Two input nodes */
        NR_DTYPE at = a->dtype.dtype;
        NR_DTYPE bt = b->dtype.dtype;
        input_dtype = (at == bt) ? at : NTools_BroadcastDtypes(at, bt);
    } else {
        /* Single input node */
        input_dtype = a->dtype.dtype;
    }
    
    output_dtype = c ? c->dtype.dtype : input_dtype;
    
    iot->intype = _ResolveDtype(input_dtype, intype);
    iot->outtype = _ResolveDtype((outtype == NDTYPE_NONE) ? iot->intype : output_dtype, outtype);
}

/* Convert type category to string */
NR_PRIVATE void
_IOType_AsString(int iotype, char dst[]) {
    switch (iotype) {
        case NDTYPE_FLOAT:
            strcpy(dst, "float");
            break;
        case NDTYPE_BOOL:
            strcpy(dst, "boolean");
            break;
        case NDTYPE_INT:
            strcpy(dst, "integer");
            break;
        default:
            strcpy(dst, "unknown");
            break;
    }
}

/* Validate input types if not castable */
NR_PRIVATE int
_ValidateInputTypes(Node* a, Node* b, int intype, int in_castable, char* func_name) {
    if (in_castable) {
        return 1; /* No validation needed */
    }
    
    int atype = NDtype_GetDtypeType(a->dtype.dtype);
    if (atype != intype) {
        char intype_str[20], adtype_str[20];
        _IOType_AsString(intype, intype_str);
        NDtype_AsString(a->dtype.dtype, adtype_str);
        
        if (b) {
            char bdtype_str[20];
            NDtype_AsString(b->dtype.dtype, bdtype_str);
            int btype = NDtype_GetDtypeType(b->dtype.dtype);
            if (btype != intype) {
                NError_RaiseError(NError_TypeError, 
                    "%s operation not supported for input types %s and %s. Expected input types of type %s.",
                    func_name, adtype_str, bdtype_str, intype_str);
                return 0;
            }
        } else {
            NError_RaiseError(NError_TypeError,
                "%s operation not supported for input type %s. Expected input type %s.",
                func_name, adtype_str, intype_str);
            return 0;
        }
    }
    
    if (b) {
        int btype = NDtype_GetDtypeType(b->dtype.dtype);
        if (btype != intype) {
            char intype_str[20], bdtype_str[20];
            _IOType_AsString(intype, intype_str);
            NDtype_AsString(b->dtype.dtype, bdtype_str);
            NError_RaiseError(NError_TypeError,
                "%s operation not supported for input type %s. Expected input type %s.",
                func_name, bdtype_str, intype_str);
            return 0;
        }
    }
    
    return 1;
}

/* Cast node to target type if needed */
NR_PRIVATE Node*
_CastNodeIfNeeded(Node* node, NR_DTYPE target_type, int* copy_flag) {
    if (node->dtype.dtype != target_type) {
        Node* cast_node = Node_ToType(NULL, node, target_type);
        if (cast_node) {
            *copy_flag = 1;
        }
        return cast_node;
    }
    return node;
}

/* Clean up copied nodes */
NR_PRIVATE void
_CleanupCopiedNodes(Node* a, Node* b, int copy_a, int copy_b) {
    if (copy_a) Node_Free(a);
    if (copy_b) Node_Free(b);
}

/* Unified function for both 1-input and 2-input operations */
NR_PRIVATE Node*
_NMath_Func_Core(Node* c, Node* a, Node* b, NFuncFunc func_list[], int outtype, int intype, int in_castable, char* func_name) {
    /* Validate input types */
    if (!_ValidateInputTypes(a, b, intype, in_castable, func_name)) {
        return NULL;
    }
    
    /* Resolve data types */
    __nmath_iotypes iot;
    _CheckDtypes(&iot, c, a, b, outtype, intype);
    
    /* Cast input nodes if needed */
    int copy_a = 0, copy_b = 0;
    a = _CastNodeIfNeeded(a, iot.intype, &copy_a);
    if (!a) {
        return NULL;
    }
    
    if (b) {
        b = _CastNodeIfNeeded(b, iot.intype, &copy_b);
        if (!b) {
            if (copy_a) Node_Free(a);
            return NULL;
        }
    }
    
    /* Calculate function index */
    int func_idx = (intype != NDTYPE_FLOAT) ? 
        (int)iot.intype : 
        (int)iot.intype - (int)NR_UINT64 - 1;
    
    /* Execute function */
    NFuncFunc func = func_list[func_idx];
    Node* in[2] = {a, b};
    NFuncArgs args;
    args.nodes = in;
    args.out = c;
    args.outtype = iot.outtype;
    args.n_nodes = b ? 2 : 1;
    args.dim = 0;
    args.extra = NULL;
    
    int res = func(&args);
    
    /* Clean up */
    _CleanupCopiedNodes(a, b, copy_a, copy_b);
    
    return res != 0 ? NULL : args.out;
}

/* Wrapper functions for backward compatibility */
NR_PRIVATE Node*
_NMath_Func_2in1out(Node* c, Node* a, Node* b, NFuncFunc func_list[], int outtype, int intype, int in_castable, char* func_name) {
    return _NMath_Func_Core(c, a, b, func_list, outtype, intype, in_castable, func_name);
}

NR_PRIVATE Node*
_NMath_Func_1in1out(Node* c, Node* a, NFuncFunc func_list[], int outtype, int intype, int in_castable, char* func_name) {
    return _NMath_Func_Core(c, a, NULL, func_list, outtype, intype, in_castable, func_name);
}
