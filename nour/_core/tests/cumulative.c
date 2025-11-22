#include "main.h"

/* Forward declarations (header not yet public) */
struct Node; /* opaque forward */
Node* NMath_Cumsum(Node* c, Node* a, int axis);
Node* NMath_Cumprod(Node* c, Node* a, int axis);
Node* NMath_Cummin(Node* c, Node* a, int axis);
Node* NMath_Cummax(Node* c, Node* a, int axis);
Node* NMath_Diff(Node* c, Node* a, int axis);
Node* NMath_Gradient(Node* c, Node* a, int axis);
Node* NMath_NanCumsum(Node* c, Node* a, int axis);
Node* NMath_NanCumprod(Node* c, Node* a, int axis);
Node* NMath_NanCummin(Node* c, Node* a, int axis);
Node* NMath_NanCummax(Node* c, Node* a, int axis);
#include <math.h>

#define VERIFY_SHAPE(node, expected_ndim, ...) do { \
    nr_intp expected[] = {__VA_ARGS__}; \
    if (NODE_NDIM(node) != expected_ndim) { printf("Shape mismatch: expected ndim=%d got %d\n", expected_ndim, NODE_NDIM(node)); return 0; } \
    for (int i=0;i<expected_ndim;i++){ if (NODE_SHAPE(node)[i] != expected[i]) { printf("Shape dim %d mismatch: expected %lld got %lld\n", i, expected[i], NODE_SHAPE(node)[i]); return 0; }} \
} while(0)

#define VERIFY_ARRAY_INT64(node, length, ...) do { \
    nr_int64 expected[] = {__VA_ARGS__}; \
    if (Node_NItems(node) != (nr_intp)(length)) { printf("Items mismatch: expected %d got %lld\n", (int)(length), Node_NItems(node)); return 0; } \
    nr_int64* data = (nr_int64*)NODE_DATA(node); \
    for (int i=0;i<(length);i++){ if (data[i] != expected[i]) { printf("Int64 array mismatch at %d: expected %lld got %lld\n", i, expected[i], data[i]); return 0; }} \
} while(0)

#define VERIFY_ARRAY_INT32(node, length, ...) do { \
    nr_int32 expected[] = {__VA_ARGS__}; \
    if (Node_NItems(node) != (nr_intp)(length)) { printf("Items mismatch: expected %d got %lld\n", (int)(length), Node_NItems(node)); return 0; } \
    nr_int32* data = (nr_int32*)NODE_DATA(node); \
    for (int i=0;i<(length);i++){ if (data[i] != expected[i]) { printf("Int32 array mismatch at %d: expected %d got %d\n", i, (int)expected[i], (int)data[i]); return 0; }} \
} while(0)

#define VERIFY_ARRAY_UINT64(node, length, ...) do { \
    nr_uint64 expected[] = {__VA_ARGS__}; \
    if (Node_NItems(node) != (nr_intp)(length)) { printf("Items mismatch: expected %d got %lld\n", (int)(length), Node_NItems(node)); return 0; } \
    nr_uint64* data = (nr_uint64*)NODE_DATA(node); \
    for (int i=0;i<(length);i++){ if (data[i] != expected[i]) { printf("UInt64 array mismatch at %d: expected %llu got %llu\n", i, (unsigned long long)expected[i], (unsigned long long)data[i]); return 0; }} \
} while(0)

#define VERIFY_ARRAY_FLOAT64_APPROX(node, length, eps, ...) do { \
    double expected[] = {__VA_ARGS__}; \
    if (Node_NItems(node) != (nr_intp)(length)) { printf("Items mismatch: expected %d got %lld\n", (int)(length), Node_NItems(node)); return 0; } \
    double* data = (double*)NODE_DATA(node); \
    for (int i=0;i<(length);i++){ if (fabs(data[i]-expected[i])> (eps)) { printf("Float64 array mismatch at %d: expected %.6f got %.6f\n", i, expected[i], data[i]); return 0; }} \
} while(0)

static Node* make_node_i32(const int* vals, int ndim, const nr_intp* shape){
    return Node_New((void*)vals, 0, ndim, (nr_intp*)shape, NR_INT32);
}
static Node* make_node_f64(const double* vals, int ndim, const nr_intp* shape){
    return Node_New((void*)vals, 0, ndim, (nr_intp*)shape, NR_FLOAT64);
}
static Node* make_node_f32(const float* vals, int ndim, const nr_intp* shape){
    return Node_New((void*)vals, 0, ndim, (nr_intp*)shape, NR_FLOAT32);
}
static Node* make_node_u8(const unsigned char* vals, int ndim, const nr_intp* shape){
    return Node_New((void*)vals, 0, ndim, (nr_intp*)shape, NR_UINT8);
}

int test_cumsum_int32_default_axis(){ nr_intp shape[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* n=make_node_i32(data,2,shape); Node* r=NMath_Cumsum(NULL,n,-1); if(!r){ printf("Cumsum failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,2,2,3); VERIFY_ARRAY_INT64(r,6,1,3,6,4,9,15); Node_Free(n); Node_Free(r); return 1; }
int test_cumsum_int32_axis0(){ nr_intp shape[2]={3,2}; int data[6]={1,2,3,4,5,6}; Node* n=make_node_i32(data,2,shape); Node* r=NMath_Cumsum(NULL,n,0); if(!r){ printf("Cumsum axis0 failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,2,3,2); VERIFY_ARRAY_INT64(r,6,1,2,4,6,9,12); Node_Free(n); Node_Free(r); return 1; }
int test_cumsum_int32_axis_negative(){ nr_intp shape[2]={2,3}; int data[6]={2,1,3,1,2,4}; Node* n=make_node_i32(data,2,shape); Node* r=NMath_Cumsum(NULL,n,-1); if(!r){ printf("Cumsum negative axis failed\n"); Node_Free(n); return 0;} VERIFY_ARRAY_INT64(r,6,2,3,6,1,3,7); Node_Free(n); Node_Free(r); return 1; }
int test_cumsum_user_out_shape_error(){ nr_intp shape[1]={4}; int data[4]={1,2,3,4}; Node* n=make_node_i32(data,1,shape); nr_intp bad_shape[1]={3}; Node* out=Node_NewEmpty(1,bad_shape,NR_INT64); Node* r=NMath_Cumsum(out,n,0); if(r){ printf("Expected failure wrong output shape for cumsum\n"); Node_Free(n); Node_Free(out); return 0;} Node_Free(n); Node_Free(out); return 1; }

int test_cumprod_uint8_overflow_promotion(){ nr_intp shape[1]={5}; unsigned char data[5]={2,3,4,5,6}; Node* n=make_node_u8(data,1,shape); Node* r=NMath_Cumprod(NULL,n,0); if(!r){ printf("Cumprod failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,5); VERIFY_ARRAY_UINT64(r,5,2ULL,6ULL,24ULL,120ULL,720ULL); Node_Free(n); Node_Free(r); return 1; }

int test_cummin_int32_axis1(){ nr_intp shape[2]={2,4}; int data[8]={5,3,4,2, 7,6,9,5}; Node* n=make_node_i32(data,2,shape); Node* r=NMath_Cummin(NULL,n,1); if(!r){ printf("Cummin axis1 failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,2,2,4); VERIFY_ARRAY_INT32(r,8,5,3,3,2, 7,6,6,5); Node_Free(n); Node_Free(r); return 1; }
int test_cummax_int32_axis1(){ nr_intp shape[2]={2,4}; int data[8]={5,3,4,2, 7,6,9,5}; Node* n=make_node_i32(data,2,shape); Node* r=NMath_Cummax(NULL,n,1); if(!r){ printf("Cummax axis1 failed\n"); Node_Free(n); return 0;} VERIFY_ARRAY_INT32(r,8,5,5,5,5, 7,7,9,9); Node_Free(n); Node_Free(r); return 1; }

int test_diff_int32_axis0(){ nr_intp shape[1]={5}; int data[5]={1,4,6,9,10}; Node* n=make_node_i32(data,1,shape); Node* r=NMath_Diff(NULL,n,0); if(!r){ printf("Diff axis0 failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,4); VERIFY_ARRAY_INT64(r,4,3,2,3,1); Node_Free(n); Node_Free(r); return 1; }
int test_diff_float64_axis_last(){ nr_intp shape[2]={2,5}; double data[10]={1,2,4,7,11, 0,1,1.5,2.5,3.0}; Node* n=make_node_f64(data,2,shape); Node* r=NMath_Diff(NULL,n,-1); if(!r){ printf("Diff last axis failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,2,2,4); /* Expected float64 differences */ VERIFY_ARRAY_FLOAT64_APPROX(r,8,1e-9,1.0,2.0,3.0,4.0, 1.0,0.5,1.0,0.5); Node_Free(n); Node_Free(r); return 1; }
int test_diff_axis_length_error(){ nr_intp shape[1]={1}; int data[1]={5}; Node* n=make_node_i32(data,1,shape); Node* r=NMath_Diff(NULL,n,0); if(r){ printf("Expected diff failure axis length 1\n"); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); return 1; }

int test_gradient_int32_axis0(){ nr_intp shape[1]={5}; int data[5]={1,3,5,7,9}; Node* n=make_node_i32(data,1,shape); Node* r=NMath_Gradient(NULL,n,0); if(!r){ printf("Gradient axis0 failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,5); VERIFY_ARRAY_FLOAT64_APPROX(r,5,1e-9,2.0,2.0,2.0,2.0,2.0); Node_Free(n); Node_Free(r); return 1; }
int test_gradient_float64_axis_last(){ nr_intp shape[2]={2,5}; double data[10]={1,2,4,7,11, 10,13,15,16,18}; Node* n=make_node_f64(data,2,shape); Node* r=NMath_Gradient(NULL,n,-1); if(!r){ printf("Gradient last axis failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,2,2,5); double expected[10]={1,1.5,2.5,3.5,4, 3,2.5,1.5,1.5,2}; double* got=(double*)NODE_DATA(r); for(int i=0;i<10;i++){ if(fabs(got[i]-expected[i])>1e-9){ printf("Gradient mismatch at %d expected %.6f got %.6f\n", i, expected[i], got[i]); Node_Free(n); Node_Free(r); return 0; }} Node_Free(n); Node_Free(r); return 1; }

int test_nancumsum_float32(){ nr_intp shape[1]={5}; float data[5]={1.0f,(float)NAN,2.0f,(float)NAN,3.0f}; Node* n=make_node_f32(data,1,shape); Node* r=NMath_NanCumsum(NULL,n,0); if(!r){ printf("NanCumsum failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,5); VERIFY_ARRAY_FLOAT64_APPROX(r,5,1e-6,1.0,1.0,3.0,3.0,6.0); Node_Free(n); Node_Free(r); return 1; }
int test_nancumprod_float64(){ nr_intp shape[1]={4}; double data[4]={2.0,NAN,3.0,4.0}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_NanCumprod(NULL,n,0); if(!r){ printf("NanCumprod failed\n"); Node_Free(n); return 0;} VERIFY_ARRAY_FLOAT64_APPROX(r,4,1e-9,2.0,2.0,6.0,24.0); Node_Free(n); Node_Free(r); return 1; }
int test_nancummin_behavior(){ nr_intp shape[1]={5}; double data[5]={NAN,5.0,3.0,NAN,2.0}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_NanCummin(NULL,n,0); if(!r){ printf("NanCummin failed\n"); Node_Free(n); return 0;} VERIFY_ARRAY_FLOAT64_APPROX(r,5,1e-9,0.0,5.0,3.0,3.0,2.0); Node_Free(n); Node_Free(r); return 1; }
int test_nancummax_behavior(){ nr_intp shape[1]={5}; double data[5]={NAN,5.0,3.0,NAN,7.0}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_NanCummax(NULL,n,0); if(!r){ printf("NanCummax failed\n"); Node_Free(n); return 0;} VERIFY_ARRAY_FLOAT64_APPROX(r,5,1e-9,0.0,5.0,5.0,5.0,7.0); Node_Free(n); Node_Free(r); return 1; }

void test_cumulative(){ TestFunc tests[]={
    test_cumsum_int32_default_axis,
    test_cumsum_int32_axis0,
    test_cumsum_int32_axis_negative,
    test_cumsum_user_out_shape_error,
    test_cumprod_uint8_overflow_promotion,
    test_cummin_int32_axis1,
    test_cummax_int32_axis1,
    test_diff_int32_axis0,
    test_diff_float64_axis_last,
    test_diff_axis_length_error,
    test_gradient_int32_axis0,
    test_gradient_float64_axis_last,
    test_nancumsum_float32,
    test_nancumprod_float64,
    test_nancummin_behavior,
    test_nancummax_behavior
}; int num=sizeof(tests)/sizeof(tests[0]); run_all_tests(tests, "Cumulative Tests", num); }
