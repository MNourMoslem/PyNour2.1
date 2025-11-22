#include "main.h"
#include <stdio.h>
#include <string.h>

/* Shape API under test */
/* All symbol prototypes come via cnour.h -> shape.h */

#define VERIFY_SHAPE(node, expected_ndim, ...) do { \
    nr_intp expected[] = {__VA_ARGS__}; \
    if (NODE_NDIM(node) != (expected_ndim)) { printf("Shape mismatch: expected ndim=%d got %d\n", (int)(expected_ndim), NODE_NDIM(node)); return 0; } \
    for (int _i=0; _i<(expected_ndim); _i++){ if (NODE_SHAPE(node)[_i] != expected[_i]) { printf("Dim %d mismatch: expected %lld got %lld\n", _i, expected[_i], NODE_SHAPE(node)[_i]); return 0; } } \
} while(0)

#define VERIFY_DATA_INT32(node, length, ...) do { \
    nr_int32 expected[] = {__VA_ARGS__}; \
    if (Node_NItems(node) != (nr_intp)(length)) { printf("Items mismatch: expected %d got %lld\n", (int)(length), Node_NItems(node)); return 0; } \
    nr_int32 *data = (nr_int32*)NODE_DATA(node); \
    for (int _i=0; _i<(length); _i++){ if (data[_i] != expected[_i]) { printf("Int32 mismatch at %d: expected %d got %d\n", _i, (int)expected[_i], (int)data[_i]); return 0; } } \
} while(0)

#define VERIFY_VIEW(base, view) do { \
    if ((view) == NULL) { printf("Expected non-NULL view\n"); return 0; } \
    if (NODE_BASE(view) != (base)) { printf("View base mismatch\n"); return 0; } \
    if (NODE_DATA(view) != NODE_DATA(base)) { printf("View data pointer mismatch\n"); return 0; } \
} while(0)

#define VERIFY_NEW_ALLOC(base, other) do { \
    if ((other) == NULL) { printf("Expected non-NULL new alloc\n"); return 0; } \
    if (NODE_BASE(other) != NULL) { printf("Expected new alloc without base\n"); return 0; } \
    if (NODE_DATA(other) == NODE_DATA(base)) { printf("Expected different data pointer for allocation\n"); return 0; } \
} while(0)

static Node* make_i32(const int* vals, int ndim, const nr_intp* shape){
    return Node_New((void*)vals, 0, ndim, (nr_intp*)shape, NR_INT32);
}

/* ---------------- Reshape Tests ---------------- */
int test_shape_reshape_success(){ nr_intp shp[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* a=make_i32(data,2,shp); nr_intp new_shape[3]={3,2,1}; Node* r=Node_Reshape(a,new_shape,3,0); if(!r){ printf("Reshape returned NULL\n"); Node_Free(a); return 0;} VERIFY_SHAPE(r,3,3,2,1); VERIFY_VIEW(a,r); Node_Free(r); Node_Free(a); return 1; }
int test_shape_reshape_size_mismatch(){ nr_intp shp[2]={2,2}; int data[4]={1,2,3,4}; Node* a=make_i32(data,2,shp); nr_intp bad[2]={3,3}; Node* r=Node_Reshape(a,bad,2,0); if(r){ printf("Expected size mismatch failure\n"); Node_Free(r); Node_Free(a); return 0;} Node_Free(a); return 1; }
int test_shape_reshape_noncontiguous_error(){ nr_intp shp[2]={4,3}; int data[12]; for(int i=0;i<12;i++) data[i]=i; Node* base=make_i32(data,2,shp); /* create manual non-contiguous (negative stride) view */ nr_intp view_shape[2]={2,3}; nr_intp view_strides[2]={ -base->strides[0], base->strides[1] }; Node* view = Node_NewChild(base,2,view_shape,view_strides, base->strides[0]); nr_intp new_shape[1]={6}; Node* r=Node_Reshape(view,new_shape,1,0); if(r){ printf("Expected reshape non-contiguous failure\n"); Node_Free(r); Node_Free(view); Node_Free(base); return 0;} Node_Free(view); Node_Free(base); return 1; }
int test_shape_reshape_inplace_copy_flag(){ nr_intp shp[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* a=make_i32(data,2,shp); nr_intp new_shape[1]={6}; Node* r=Node_Reshape(a,new_shape,1,1); if(!r||r!=a){ printf("Expected in-place reshape\n"); Node_Free(a); return 0;} VERIFY_SHAPE(a,1,6); Node_Free(a); return 1; }
int test_shape_reshape_not_inplace_due_to_refcount(){ nr_intp shp[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* a=make_i32(data,2,shp); NODE_INCREF(a); nr_intp new_shape[1]={6}; Node* r=Node_Reshape(a,new_shape,1,1); if(!r){ printf("Reshape failed unexpectedly\n"); Node_Free(a); return 0;} if(r==a){ printf("Should not be in-place when refcount>1\n"); Node_Free(r); Node_Free(a); return 0;} VERIFY_VIEW(a,r); VERIFY_SHAPE(r,1,6); Node_Free(r); Node_Free(a); return 1; }

/* ---------------- Ravel / Flatten ---------------- */
int test_shape_ravel_view(){ nr_intp shp[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* a=make_i32(data,2,shp); Node* r=Node_Ravel(a,0); if(!r){ printf("Ravel NULL\n"); Node_Free(a); return 0;} VERIFY_VIEW(a,r); VERIFY_SHAPE(r,1,6); Node_Free(r); Node_Free(a); return 1; }
int test_shape_ravel_inplace(){ nr_intp shp[2]={2,2}; int data[4]={1,2,3,4}; Node* a=make_i32(data,2,shp); Node* r=Node_Ravel(a,1); if(!r||r!=a){ printf("Ravel expected inplace\n"); Node_Free(a); return 0;} VERIFY_SHAPE(a,1,4); Node_Free(a); return 1; }
int test_shape_ravel_refcount_prevents_inplace(){ nr_intp shp[2]={2,2}; int data[4]={1,2,3,4}; Node* a=make_i32(data,2,shp); NODE_INCREF(a); Node* r=Node_Ravel(a,1); if(!r||r==a){ printf("Ravel should create view when refcount>1\n"); if(r) Node_Free(r); Node_Free(a); return 0;} VERIFY_VIEW(a,r); Node_Free(r); Node_Free(a); return 1; }
int test_shape_flatten_alias(){ nr_intp shp[2]={2,2}; int data[4]={5,6,7,8}; Node* a=make_i32(data,2,shp); Node* f=Node_Flatten(a,0); if(!f){ printf("Flatten NULL\n"); Node_Free(a); return 0;} VERIFY_SHAPE(f,1,4); VERIFY_VIEW(a,f); Node_Free(f); Node_Free(a); return 1; }

/* ---------------- Transpose / SwapAxes / Permute ---------------- */
int test_shape_transpose_basic(){ nr_intp shp[3]={2,3,4}; int data[24]; for(int i=0;i<24;i++) data[i]=i; Node* a=make_i32(data,3,shp); Node* t=Node_Transpose(a,0); if(!t){ printf("Transpose NULL\n"); Node_Free(a); return 0;} VERIFY_SHAPE(t,3,4,3,2); VERIFY_VIEW(a,t); Node_Free(t); Node_Free(a); return 1; }
int test_shape_swapaxes_success(){ nr_intp shp[3]={2,3,4}; int data[24]; for(int i=0;i<24;i++) data[i]=i; Node* a=make_i32(data,3,shp); Node* s=Node_SwapAxes(a,0,2,0); if(!s){ printf("SwapAxes NULL\n"); Node_Free(a); return 0;} VERIFY_SHAPE(s,3,4,3,2); VERIFY_VIEW(a,s); Node_Free(s); Node_Free(a); return 1; }
int test_shape_swapaxes_invalid_axis(){ nr_intp shp[2]={2,3}; int data[6]; for(int i=0;i<6;i++) data[i]=i; Node* a=make_i32(data,2,shp); Node* s=Node_SwapAxes(a,0,5,0); if(s){ printf("Expected invalid axis failure\n"); Node_Free(s); Node_Free(a); return 0;} Node_Free(a); return 1; }
int test_shape_permute_dims_success(){ nr_intp shp[4]={2,3,4,5}; int data[120]; for(int i=0;i<120;i++) data[i]=i; Node* a=make_i32(data,4,shp); int order[4]={3,1,0,2}; Node* p=Node_PermuteDims(a,order,0); if(!p){ printf("Permute NULL\n"); Node_Free(a); return 0;} VERIFY_SHAPE(p,4,5,3,2,4); VERIFY_VIEW(a,p); Node_Free(p); Node_Free(a); return 1; }
int test_shape_permute_dims_duplicate_axis_error(){ nr_intp shp[3]={2,3,4}; int data[24]; for(int i=0;i<24;i++) data[i]=i; Node* a=make_i32(data,3,shp); int bad[3]={0,0,2}; Node* p=Node_PermuteDims(a,bad,0); if(p){ printf("Expected duplicate axis failure\n"); Node_Free(p); Node_Free(a); return 0;} Node_Free(a); return 1; }
int test_shape_moveaxis_success(){ nr_intp shp[3]={2,3,4}; int data[24]; for(int i=0;i<24;i++) data[i]=i; Node* a=make_i32(data,3,shp); Node* m=Node_MoveAxis(a,2,0,0); if(!m){ printf("MoveAxis NULL\n"); Node_Free(a); return 0;} VERIFY_SHAPE(m,3,4,2,3); Node_Free(m); Node_Free(a); return 1; }
int test_shape_moveaxis_invalid(){ nr_intp shp[2]={2,3}; int data[6]; for(int i=0;i<6;i++) data[i]=i; Node* a=make_i32(data,2,shp); Node* m=Node_MoveAxis(a,5,0,0); if(m){ printf("Expected moveaxis invalid failure\n"); Node_Free(m); Node_Free(a); return 0;} Node_Free(a); return 1; }
int test_shape_rollaxis_forward(){ nr_intp shp[3]={2,3,4}; int data[24]; for(int i=0;i<24;i++) data[i]=i; Node* a=make_i32(data,3,shp); Node* r=Node_RollAxis(a,0,2,0); if(!r){ printf("RollAxis NULL\n"); Node_Free(a); return 0;} VERIFY_SHAPE(r,3,3,4,2); Node_Free(r); Node_Free(a); return 1; }
int test_shape_rollaxis_backward(){ nr_intp shp[3]={2,3,4}; int data[24]; for(int i=0;i<24;i++) data[i]=i; Node* a=make_i32(data,3,shp); Node* r=Node_RollAxis(a,2,0,0); if(!r){ printf("RollAxis NULL bwd\n"); Node_Free(a); return 0;} VERIFY_SHAPE(r,3,4,2,3); Node_Free(r); Node_Free(a); return 1; }

/* ---------------- Matrix Transpose ---------------- */
int test_shape_matrix_transpose_success(){ nr_intp shp[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* a=make_i32(data,2,shp); Node* mt=Node_MatrixTranspose(a,0); if(!mt){ printf("MatrixTranspose NULL\n"); Node_Free(a); return 0;} VERIFY_SHAPE(mt,2,3,2); Node_Free(mt); Node_Free(a); return 1; }
int test_shape_matrix_transpose_error_ndim(){ nr_intp shp[3]={2,2,2}; int data[8]; for(int i=0;i<8;i++) data[i]=i; Node* a=make_i32(data,3,shp); Node* mt=Node_MatrixTranspose(a,0); if(mt){ printf("Expected 2D error\n"); Node_Free(mt); Node_Free(a); return 0;} Node_Free(a); return 1; }

/* ---------------- Expand / Squeeze ---------------- */
int test_shape_expand_dims_start(){ nr_intp shp[2]={3,4}; int data[12]; for(int i=0;i<12;i++) data[i]=i; Node* a=make_i32(data,2,shp); Node* e=Node_ExpandDims(a,0,0); if(!e){ printf("ExpandDims NULL\n"); Node_Free(a); return 0;} VERIFY_SHAPE(e,3,1,3,4); Node_Free(e); Node_Free(a); return 1; }
int test_shape_expand_dims_middle(){ nr_intp shp[2]={3,4}; int data[12]; for(int i=0;i<12;i++) data[i]=i; Node* a=make_i32(data,2,shp); Node* e=Node_ExpandDims(a,1,0); if(!e){ printf("ExpandDims NULL mid\n"); Node_Free(a); return 0;} VERIFY_SHAPE(e,3,3,1,4); Node_Free(e); Node_Free(a); return 1; }
int test_shape_expand_dims_invalid_axis(){ nr_intp shp[1]={5}; int data[5]; for(int i=0;i<5;i++) data[i]=i; Node* a=make_i32(data,1,shp); Node* e=Node_ExpandDims(a,2,0); if(e){ printf("Expected expand invalid axis failure\n"); Node_Free(e); Node_Free(a); return 0;} Node_Free(a); return 1; }
int test_shape_squeeze_removes_ones(){ nr_intp shp[4]={1,3,1,2}; int data[6]; for(int i=0;i<6;i++) data[i]=i; Node* a=make_i32(data,4,shp); Node* s=Node_Squeeze(a,0); if(!s){ printf("Squeeze NULL\n"); Node_Free(a); return 0;} VERIFY_SHAPE(s,2,3,2); Node_Free(s); Node_Free(a); return 1; }
int test_shape_squeeze_scalar(){ int val=42; Node* scalar=Node_NewScalar(&val, NR_INT32); Node* s=Node_Squeeze(scalar,0); if(!s){ printf("Squeeze scalar NULL\n"); Node_Free(scalar); return 0;} if(s!=scalar){ if(NODE_BASE(s)!=scalar || NODE_DATA(s)!=NODE_DATA(scalar)){ printf("Scalar squeeze view incorrect\n"); Node_Free(s); Node_Free(scalar); return 0;} Node_Free(s); Node_Free(scalar); return 1; } Node_Free(scalar); return 1; }

/* ---------------- Resize ---------------- */
int test_shape_resize_grow(){ nr_intp shp[1]={4}; int data[4]={1,2,3,4}; Node* a=make_i32(data,1,shp); nr_intp new_shape[1]={6}; Node* r=Node_Resize(a,new_shape,1,0); if(!r){ printf("Resize grow NULL\n"); Node_Free(a); return 0;} VERIFY_NEW_ALLOC(a,r); VERIFY_SHAPE(r,1,6); nr_int32* d=(nr_int32*)NODE_DATA(r); if(d[0]!=1||d[3]!=4||d[4]!=0||d[5]!=0){ printf("Resize grow content mismatch\n"); Node_Free(r); Node_Free(a); return 0;} Node_Free(r); Node_Free(a); return 1; }
int test_shape_resize_shrink(){ nr_intp shp[1]={5}; int data[5]={9,8,7,6,5}; Node* a=make_i32(data,1,shp); nr_intp new_shape[1]={3}; Node* r=Node_Resize(a,new_shape,1,0); if(!r){ printf("Resize shrink NULL\n"); Node_Free(a); return 0;} VERIFY_SHAPE(r,1,3); nr_int32* d=(nr_int32*)NODE_DATA(r); if(d[0]!=9||d[2]!=7){ printf("Resize shrink mismatch\n"); Node_Free(r); Node_Free(a); return 0;} Node_Free(r); Node_Free(a); return 1; }
int test_shape_resize_inplace(){ nr_intp shp[1]={3}; int data[3]={5,6,7}; Node* a=make_i32(data,1,shp); nr_intp new_shape[1]={5}; Node* r=Node_Resize(a,new_shape,1,1); if(!r||r!=a){ printf("Resize inplace expected\n"); Node_Free(a); return 0;} VERIFY_SHAPE(a,1,5); nr_int32* d=(nr_int32*)NODE_DATA(a); if(d[0]!=5){ printf("Resize inplace data mismatch\n"); Node_Free(a); return 0;} Node_Free(a); return 1; }

/* Aggregate runner */
void test_shape(){ TestFunc tests[]={
    test_shape_reshape_success,
    test_shape_reshape_size_mismatch,
    test_shape_reshape_noncontiguous_error,
    test_shape_reshape_inplace_copy_flag,
    test_shape_reshape_not_inplace_due_to_refcount,
    test_shape_ravel_view,
    test_shape_ravel_inplace,
    test_shape_ravel_refcount_prevents_inplace,
    test_shape_flatten_alias,
    test_shape_transpose_basic,
    test_shape_swapaxes_success,
    test_shape_swapaxes_invalid_axis,
    test_shape_permute_dims_success,
    test_shape_permute_dims_duplicate_axis_error,
    test_shape_moveaxis_success,
    test_shape_moveaxis_invalid,
    test_shape_rollaxis_forward,
    test_shape_rollaxis_backward,
    test_shape_matrix_transpose_success,
    test_shape_matrix_transpose_error_ndim,
    test_shape_expand_dims_start,
    test_shape_expand_dims_middle,
    test_shape_expand_dims_invalid_axis,
    test_shape_squeeze_removes_ones,
    test_shape_squeeze_scalar,
    test_shape_resize_grow,
    test_shape_resize_shrink,
    test_shape_resize_inplace
}; int num=sizeof(tests)/sizeof(tests[0]); run_all_tests(tests, "Shape Tests", num); }
