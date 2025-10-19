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
}__nmath_iotypes;

NR_PRIVATE void
_CheckDtypes_3Nodes(__nmath_iotypes* iot, Node* c, Node* a, Node* b, int outtype, int intype){
    NR_DTYPE at = a->dtype.dtype;
    NR_DTYPE bt = b->dtype.dtype;
    NR_DTYPE abt = at == bt ? at : NTools_BroadcastDtypes(at, bt);
    NR_DTYPE outt = c ? c->dtype.dtype : abt;

    if (intype == NDTYPE_NONE){
        iot->intype = abt;
    }
    else if (intype == NDTYPE_FLOAT){
        iot->intype = (abt > NR_UINT64) && (abt < NR_NUM_NUMIRC_DT) ? abt : NR_FLOAT64;  
    }
    else if (intype == NDTYPE_BOOL){
        iot->intype = NR_BOOL;
    }
    else if (intype == NDTYPE_INT){
        iot->intype = abt <= NR_UINT64 ? abt : NR_INT64;
    }
    else{
        iot->intype = abt;
    }

    if (outtype == NDTYPE_NONE){
        iot->outtype = iot->intype;
    }
    else if (outtype == NDTYPE_FLOAT){
        iot->outtype = (outt > NR_UINT64) && (outt < NR_NUM_NUMIRC_DT) ? outt : NR_FLOAT64;  
    }
    else if (outtype == NDTYPE_BOOL){
        iot->outtype = NR_BOOL;
    }
    else if (outtype == NDTYPE_INT){
        iot->outtype = outt <= NR_UINT64 ? outt : NR_INT64;
    }
    else{
        iot->outtype = iot->intype;
    }
}

NR_PRIVATE void
_CheckDtypes_2Nodes(__nmath_iotypes* iot, Node* c, Node* a, int outtype, int intype){
    NR_DTYPE at = a->dtype.dtype;
    NR_DTYPE outt = c ? c->dtype.dtype : at;

    // Resolve input type
    if (intype == NDTYPE_NONE){
        iot->intype = at;
    }
    else if (intype == NDTYPE_FLOAT){
        iot->intype = (at > NR_UINT64) && (at < NR_NUM_NUMIRC_DT) ? at : NR_FLOAT64;  
    }
    else if (intype == NDTYPE_BOOL){
        iot->intype = NR_BOOL;
    }
    else if (intype == NDTYPE_INT){
        iot->intype = at <= NR_UINT64 ? at : NR_INT64;
    }
    else{
        iot->intype = at;
    }

    // Resolve output type
    if (outtype == NDTYPE_NONE){
        iot->outtype = iot->intype;
    }
    else if (outtype == NDTYPE_FLOAT){
        iot->outtype = (outt > NR_UINT64) && (outt < NR_NUM_NUMIRC_DT) ? outt : NR_FLOAT64;  
    }
    else if (outtype == NDTYPE_BOOL){
        iot->outtype = NR_BOOL;
    }
    else if (outtype == NDTYPE_INT){
        iot->outtype = outt <= NR_UINT64 ? outt : NR_INT64;
    }
    else{
        iot->outtype = iot->intype;
    }
}

NR_PRIVATE void
_IOType_AsString(int iotype, char dst[]){
    if (iotype == NDTYPE_FLOAT){
        strcpy(dst, "float");  
    }
    else if (iotype == NDTYPE_BOOL){
        strcpy(dst, "boolean");  
    }
    else if (iotype == NDTYPE_INT){
        strcpy(dst, "integer");  
    }
    else{
        strcpy(dst, "unknown");  
    }
}

NR_PRIVATE Node*
_NMath_Func_2in1out(Node* c, Node* a, Node* b, NFunc func_list[], int outtype, int intype, int in_castable, char* func_name){
    if (!in_castable){
        int atype = NDtype_GetDtypeType(a->dtype.dtype);
        int btype = NDtype_GetDtypeType(b->dtype.dtype);
        if (atype != intype || btype != intype){
            char intype_str[20];
            char adtype_str[20];
            char bdtype_str[20];
            
            NDtype_AsString(intype, intype_str);
            NDtype_AsString(a->dtype.dtype, adtype_str);
            NDtype_AsString(b->dtype.dtype, bdtype_str);

            NError_RaiseError(NError_TypeError, "%s operation not supported for input types %s and %s. Expected input types of type %s.", 
                func_name,
                adtype_str,
                bdtype_str,
                intype_str
            );
            return NULL;
        }
    }


    int copy_a = 0, copy_b = 0;
    int cast_out_to_c = 0;
    int func_idx;
    __nmath_iotypes iot;
     _CheckDtypes_3Nodes(&iot, c, a, b, outtype, intype);

    if (a->dtype.dtype != iot.intype){
        a = Node_ToType(NULL, a, iot.intype);
        if (!a){
            return NULL;
        }

        copy_a = 1;
    }

    if (b->dtype.dtype != iot.intype){
        b = Node_ToType(NULL, b, iot.intype);
        if (!b){
            if (copy_a){Node_Free(a);}
            return NULL;
        }
        copy_b = 1;
    }

    Node* out;
    if (c && c->dtype.dtype != iot.outtype){
        out = NULL;
        cast_out_to_c = 1;
    }
    else{
        out = c;
    }

    if (intype != NDTYPE_FLOAT){
        func_idx = (int)iot.intype;
    }
    else{
        func_idx = (int)iot.intype - (int)NR_UINT64 - 1;
    }

    NFunc func = func_list[func_idx];

    Node* in[] = {a, b};
    NFuncArgs args = {
        .nodes = in,
        .out = out,
        .outtype = iot.outtype,
        .n_nodes = 2
    };

    int res = func(&args);

    if (copy_a){
        Node_Free(a);
    }

    if (copy_b){
        Node_Free(b);
    }

    if (cast_out_to_c){
        out = args.out;
        args.out = Node_ToType(c, args.out, c->dtype.dtype);
        Node_Free(out);
    }

    return res != 0 ? NULL : args.out;
}


NR_PRIVATE Node*
_NMath_Func_1in1out(Node* c, Node* a, NFunc func_list[], int outtype, int intype, int in_castable, char* func_name) {
    if (!in_castable) {
        int atype = NDtype_GetDtypeType(a->dtype.dtype);
        if (atype != intype) {
            char intype_str[20];
            char adtype_str[20];
            
            NDtype_AsString(intype, intype_str);
            NDtype_AsString(a->dtype.dtype, adtype_str);

            NError_RaiseError(NError_TypeError, "%s operation not supported for input type %s. Expected input type %s.", 
                func_name,
                adtype_str,
                intype_str
            );
            return NULL;
        }
    }

    int copy_a = 0;
    int cast_out_to_c = 0;
    int func_idx;
    __nmath_iotypes iot;

    _CheckDtypes_2Nodes(&iot, c, a, outtype, intype);

    if (a->dtype.dtype != iot.intype) {
        a = Node_ToType(NULL, a, iot.intype);
        if (!a) {
            return NULL;
        }
        copy_a = 1;
    }

    Node* out;
    if (c && c->dtype.dtype != iot.outtype) {
        out = NULL;
        cast_out_to_c = 1;
    } else {
        out = c;
    }

    if (intype != NDTYPE_FLOAT) {
        func_idx = (int)iot.intype;
    } else {
        func_idx = (int)iot.intype - (int)NR_UINT64 - 1;
    }

    NFunc func = func_list[func_idx];

    Node* in[] = {a};
    NFuncArgs args = {
        .nodes = in,
        .out = out,
        .outtype = iot.outtype,
        .n_nodes = 1
    };

    int res = func(&args);

    if (copy_a) {
        Node_Free(a);
    }

    if (cast_out_to_c) {
        out = args.out;
        args.out = Node_ToType(c, args.out, c->dtype.dtype);
        Node_Free(out);
    }

    return res != 0 ? NULL : args.out;
}
