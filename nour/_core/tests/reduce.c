#include "main.h"
#include "../src/cnour.h"
#include <math.h>

#define VERIFY_SCALAR_INT64(node, expected) do { \
    if (NODE_NDIM(node) != 0) { printf("Expected scalar ndim=0 got %d\n", NODE_NDIM(node)); return 0; } \
    nr_int64 got = *(nr_int64*)NODE_DATA(node); \
    if (got != (nr_int64)(expected)) { printf("Scalar int64 mismatch: expected %lld got %lld\n", (nr_int64)(expected), got); return 0; } \
} while(0)

#define VERIFY_SCALAR_FLOAT64_APPROX(node, expected, eps) do { \
    if (NODE_NDIM(node) != 0) { printf("Expected scalar ndim=0\n"); return 0; } \
    double got = *(nr_float64*)NODE_DATA(node); \
    if (isnan(expected)) { if (!isnan(got)) { printf("Expected NaN got %f\n", got); return 0; } } \
    else if (fabs(got - (expected)) > (eps)) { printf("Scalar float64 mismatch: expected %.6f got %.6f\n", (expected), got); return 0; } \
} while(0)

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

#define VERIFY_ARRAY_FLOAT64_APPROX(node, length, eps, ...) do { \
    double expected[] = {__VA_ARGS__}; \
    if (Node_NItems(node) != (nr_intp)(length)) { printf("Items mismatch: expected %d got %lld\n", (int)(length), Node_NItems(node)); return 0; } \
    double* data = (double*)NODE_DATA(node); \
    for (int i=0;i<(length);i++){ if (isnan(expected[i])) { if(!isnan(data[i])){ printf("Expected NaN at %d got %f\n", i, data[i]); return 0; } } else if (fabs(data[i]-expected[i])> (eps)) { printf("Float64 array mismatch at %d: expected %.6f got %.6f\n", i, expected[i], data[i]); return 0; }} \
} while(0)

static Node* make_node_i32(const int* vals, int ndim, const nr_intp* shape){
    return Node_New((void*)vals, 0, ndim, (nr_intp*)shape, NR_INT32);
}
static Node* make_node_f64(const double* vals, int ndim, const nr_intp* shape){
    return Node_New((void*)vals, 0, ndim, (nr_intp*)shape, NR_FLOAT64);
}

int test_reduce_sum_full_int32(){ nr_intp shape[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* n=make_node_i32(data,2,shape); Node* r=NMath_Sum(NULL,n,NULL,0); if(!r){ printf("Sum failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_INT64(r,21); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_sum_axis0(){ nr_intp shape[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* n=make_node_i32(data,2,shape); int axes[1]={0}; Node* r=NMath_Sum(NULL,n,axes,1); if(!r){ printf("Sum axis0 failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,3); VERIFY_ARRAY_INT64(r,3,5,7,9); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_sum_axis1(){ nr_intp shape[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* n=make_node_i32(data,2,shape); int axes[1]={1}; Node* r=NMath_Sum(NULL,n,axes,1); if(!r){ printf("Sum axis1 failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,2); VERIFY_ARRAY_INT64(r,2,6,15); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_sum_negative_axis(){ nr_intp shape[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* n=make_node_i32(data,2,shape); int axes[1]={-1}; Node* r=NMath_Sum(NULL,n,axes,1); if(!r){ printf("Sum negative axis failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,2); VERIFY_ARRAY_INT64(r,2,6,15); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_sum_all_axes_list(){ nr_intp shape[3]={2,2,2}; int data[8]={1,2,3,4,5,6,7,8}; Node* n=make_node_i32(data,3,shape); int axes[3]={0,1,2}; Node* r=NMath_Sum(NULL,n,axes,3); if(!r){ printf("Sum all axes failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_INT64(r,36); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_sum_duplicate_axes(){ nr_intp shape[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* n=make_node_i32(data,2,shape); int axes[3]={0,0,1}; Node* r=NMath_Sum(NULL,n,axes,3); if(!r){ printf("Sum duplicate axes failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_INT64(r,21); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_sum_axis_out_of_bounds(){ nr_intp shape[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* n=make_node_i32(data,2,shape); int axes[1]={2}; Node* r=NMath_Sum(NULL,n,axes,1); if(r){ printf("Expected failure for out-of-bounds axis\n"); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); return 1; }
int test_reduce_sum_user_output_correct(){ nr_intp shape[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* n=make_node_i32(data,2,shape); int axes[1]={0}; nr_intp out_shape[1]={3}; Node* out=Node_NewEmpty(1,out_shape,NR_INT64); Node* r=NMath_Sum(out,n,axes,1); if(!r||r!=out){ printf("User output sum axis0 failed\n"); Node_Free(n); Node_Free(out); return 0;} VERIFY_ARRAY_INT64(out,3,5,7,9); Node_Free(n); Node_Free(out); return 1; }
int test_reduce_sum_user_output_wrong_shape(){ nr_intp shape[2]={2,3}; int data[6]={1,2,3,4,5,6}; Node* n=make_node_i32(data,2,shape); int axes[1]={0}; nr_intp bad_shape[1]={2}; Node* out=Node_NewEmpty(1,bad_shape,NR_INT64); Node* r=NMath_Sum(out,n,axes,1); if(r){ printf("Expected failure wrong output shape\n"); Node_Free(n); Node_Free(out); return 0;} Node_Free(n); Node_Free(out); return 1; }
int test_reduce_sum_user_output_dtype_downcast(){ nr_intp shape[1]={4}; int data[4]={100,50,60,40}; Node* n=make_node_i32(data,1,shape); nr_intp out_shape[]={1}; Node* out=Node_NewEmpty(0,out_shape,NR_INT8); Node* r=NMath_Sum(out,n,NULL,0); if(!r){ printf("Sum int8 out failed\n"); Node_Free(n); Node_Free(out); return 0;} nr_int8 got = *(nr_int8*)NODE_DATA(r); nr_int8 expected = (nr_int8)250; if(got!=expected){ printf("Overflow cast mismatch expected %d got %d\n", expected, got); Node_Free(n); Node_Free(out); return 0;} Node_Free(n); Node_Free(out); return 1; }

int test_reduce_prod_full(){ nr_intp shape[1]={5}; int data[5]={1,2,3,4,5}; Node* n=make_node_i32(data,1,shape); Node* r=NMath_Prod(NULL,n,NULL,0); if(!r){ printf("Prod failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_INT64(r,120); Node_Free(n); Node_Free(r); return 1; }

int test_reduce_min_full(){ nr_intp shape[1]={5}; int data[5]={3,7,1,8,5}; Node* n=make_node_i32(data,1,shape); Node* r=NMath_Min(NULL,n,NULL,0); if(!r){ printf("Min failed\n"); Node_Free(n); return 0;} nr_int32 v=*(nr_int32*)NODE_DATA(r); if(v!=1){ printf("Min expected 1 got %d\n",(int)v); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); Node_Free(r); return 1; }
int test_reduce_max_full(){ nr_intp shape[1]={5}; int data[5]={3,7,1,8,5}; Node* n=make_node_i32(data,1,shape); Node* r=NMath_Max(NULL,n,NULL,0); if(!r){ printf("Max failed\n"); Node_Free(n); return 0;} nr_int32 v=*(nr_int32*)NODE_DATA(r); if(v!=8){ printf("Max expected 8 got %d\n",(int)v); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); Node_Free(r); return 1; }
int test_reduce_min_axis(){ nr_intp shape[2]={2,3}; int data[6]={4,2,9,1,5,7}; Node* n=make_node_i32(data,2,shape); int ax[1]={1}; Node* r=NMath_Min(NULL,n,ax,1); if(!r){ printf("Min axis failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,2); nr_int32* d=(nr_int32*)NODE_DATA(r); if(d[0]!=2||d[1]!=1){ printf("Min axis values mismatch\n"); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); Node_Free(r); return 1; }
int test_reduce_max_axis(){ nr_intp shape[2]={2,3}; int data[6]={4,2,9,1,5,7}; Node* n=make_node_i32(data,2,shape); int ax[1]={0}; Node* r=NMath_Max(NULL,n,ax,1); if(!r){ printf("Max axis failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,3); nr_int32* d=(nr_int32*)NODE_DATA(r); if(d[0]!=4||d[1]!=5||d[2]!=9){ printf("Max axis values mismatch\n"); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); Node_Free(r); return 1; }

int test_reduce_mean_full_int32(){ nr_intp shape[1]={4}; int data[4]={2,4,6,8}; Node* n=make_node_i32(data,1,shape); Node* r=NMath_Mean(NULL,n,NULL,0); if(!r){ printf("Mean failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_FLOAT64_APPROX(r,5.0,1e-9); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_mean_axis(){ nr_intp shape[2]={2,2}; int data[4]={1,3,5,7}; Node* n=make_node_i32(data,2,shape); int ax[1]={0}; Node* r=NMath_Mean(NULL,n,ax,1); if(!r){ printf("Mean axis failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,2); VERIFY_ARRAY_FLOAT64_APPROX(r,2,1e-9,3.0,5.0); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_var_full(){ nr_intp shape[1]={4}; double data[4]={1.0,2.0,3.0,4.0}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_Var(NULL,n,NULL,0); if(!r){ printf("Var failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_FLOAT64_APPROX(r,1.25,1e-9); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_std_full(){ nr_intp shape[1]={4}; double data[4]={1.0,2.0,3.0,4.0}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_Std(NULL,n,NULL,0); if(!r){ printf("Std failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_FLOAT64_APPROX(r,sqrt(1.25),1e-9); Node_Free(n); Node_Free(r); return 1; }

int test_reduce_argmin_full(){ nr_intp shape[1]={5}; double data[5]={5.0,2.0,2.0,9.0,7.0}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_Argmin(NULL,n,NULL,0); if(!r){ printf("Argmin failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_INT64(r,1); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_argmax_full(){ nr_intp shape[1]={5}; double data[5]={5.0,9.0,9.0,1.0,7.0}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_Argmax(NULL,n,NULL,0); if(!r){ printf("Argmax failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_INT64(r,1); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_argmin_axis(){ nr_intp shape[2]={2,3}; int data[6]={7,1,5,0,9,2}; Node* n=make_node_i32(data,2,shape); int ax[1]={1}; Node* r=NMath_Argmin(NULL,n,ax,1); if(!r){ printf("Argmin axis failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,2); nr_int64* d=(nr_int64*)NODE_DATA(r); if(d[0]!=1||d[1]!=0){ printf("Argmin axis mismatch got [%lld,%lld]\n",d[0],d[1]); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); Node_Free(r); return 1; }

int test_reduce_all_full(){ nr_intp shape[1]={4}; int data[4]={1,2,3,0}; Node* n=make_node_i32(data,1,shape); Node* r=NMath_All(NULL,n,NULL,0); if(!r){ printf("All failed\n"); Node_Free(n); return 0;} nr_bool v=*(nr_bool*)NODE_DATA(r); if(v!=0){ printf("All expected 0 got %d\n",(int)v); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); Node_Free(r); return 1; }
int test_reduce_any_full(){ nr_intp shape[1]={4}; int data[4]={0,0,0,2}; Node* n=make_node_i32(data,1,shape); Node* r=NMath_Any(NULL,n,NULL,0); if(!r){ printf("Any failed\n"); Node_Free(n); return 0;} nr_bool v=*(nr_bool*)NODE_DATA(r); if(v!=1){ printf("Any expected 1 got %d\n",(int)v); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); Node_Free(r); return 1; }
int test_reduce_count_nonzero_axis(){ nr_intp shape[2]={2,3}; int data[6]={0,1,0,2,3,0}; Node* n=make_node_i32(data,2,shape); int ax[1]={0}; Node* r=NMath_CountNonzero(NULL,n,ax,1); if(!r){ printf("CountNonzero axis failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,3); VERIFY_ARRAY_INT64(r,3,1,2,0); Node_Free(n); Node_Free(r); return 1; }

int test_reduce_nansum_full(){ nr_intp shape[1]={5}; double data[5]={1.0,NAN,2.0,NAN,3.0}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_NanSum(NULL,n,NULL,0); if(!r){ printf("NanSum failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_FLOAT64_APPROX(r,6.0,1e-9); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_nanmean_full(){ nr_intp shape[1]={4}; double data[4]={NAN,2.0,4.0,NAN}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_NanMean(NULL,n,NULL,0); if(!r){ printf("NanMean failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_FLOAT64_APPROX(r,3.0,1e-9); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_nanmin_full(){ nr_intp shape[1]={5}; double data[5]={NAN,5.0,2.0,NAN,3.0}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_NanMin(NULL,n,NULL,0); if(!r){ printf("NanMin failed\n"); Node_Free(n); return 0;} nr_float64 v=*(nr_float64*)NODE_DATA(r); if(v!=2.0){ printf("NanMin expected 2 got %f\n",v); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); Node_Free(r); return 1; }
int test_reduce_nanmax_full(){ nr_intp shape[1]={5}; double data[5]={NAN,5.0,2.0,NAN,3.0}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_NanMax(NULL,n,NULL,0); if(!r){ printf("NanMax failed\n"); Node_Free(n); return 0;} nr_float64 v=*(nr_float64*)NODE_DATA(r); if(v!=5.0){ printf("NanMax expected 5 got %f\n",v); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); Node_Free(r); return 1; }
int test_reduce_nanvar_full(){ nr_intp shape[1]={5}; double data[5]={1.0,NAN,3.0,5.0,NAN}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_NanVar(NULL,n,NULL,0); if(!r){ printf("NanVar failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_FLOAT64_APPROX(r,2.6666666667,1e-6); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_nanstd_full(){ nr_intp shape[1]={5}; double data[5]={1.0,NAN,3.0,5.0,NAN}; Node* n=make_node_f64(data,1,shape); Node* r=NMath_NanStd(NULL,n,NULL,0); if(!r){ printf("NanStd failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_FLOAT64_APPROX(r,sqrt(2.6666666667),1e-6); Node_Free(n); Node_Free(r); return 1; }

int test_reduce_empty_sum(){ nr_intp shape[1]={0}; Node* n=Node_NewEmpty(1,shape,NR_INT32); if(!n){ printf("Empty node alloc failed\n"); return 0;} Node* r=NMath_Sum(NULL,n,NULL,0); if(!r){ printf("Sum empty failed\n"); Node_Free(n); return 0;} VERIFY_SCALAR_INT64(r,0); Node_Free(n); Node_Free(r); return 1; }
int test_reduce_empty_min(){ nr_intp shape[1]={0}; Node* n=Node_NewEmpty(1,shape,NR_INT32); if(!n) return 0; Node* r=NMath_Min(NULL,n,NULL,0); if(!r){ printf("Min empty failed\n"); Node_Free(n); return 0;} nr_int32 v=*(nr_int32*)NODE_DATA(r); if(v!=0){ printf("Min empty expected 0 got %d\n",(int)v); Node_Free(n); Node_Free(r); return 0;} Node_Free(n); Node_Free(r); return 1; }
int test_reduce_small_var_axis(){ nr_intp shape[2]={1,4}; double data[4]={2.0,2.0,2.0,2.0}; Node* n=make_node_f64(data,2,shape); int ax[1]={0}; Node* r=NMath_Var(NULL,n,ax,1); if(!r){ printf("Var axis failed\n"); Node_Free(n); return 0;} VERIFY_SHAPE(r,1,4); VERIFY_ARRAY_FLOAT64_APPROX(r,4,1e-12,0.0,0.0,0.0,0.0); Node_Free(n); Node_Free(r); return 1; }

void test_reduce(){ TestFunc tests[]={
    test_reduce_sum_full_int32, test_reduce_sum_axis0, test_reduce_sum_axis1, test_reduce_sum_negative_axis, test_reduce_sum_all_axes_list, test_reduce_sum_duplicate_axes, test_reduce_sum_axis_out_of_bounds, test_reduce_sum_user_output_correct, test_reduce_sum_user_output_wrong_shape, test_reduce_sum_user_output_dtype_downcast,
    test_reduce_prod_full,
    test_reduce_min_full, test_reduce_max_full, test_reduce_min_axis, test_reduce_max_axis,
    test_reduce_mean_full_int32, test_reduce_mean_axis, test_reduce_var_full, test_reduce_std_full,
    test_reduce_argmin_full, test_reduce_argmax_full, test_reduce_argmin_axis,
    test_reduce_all_full, test_reduce_any_full, test_reduce_count_nonzero_axis,
    test_reduce_nansum_full, test_reduce_nanmean_full, test_reduce_nanmin_full, test_reduce_nanmax_full, test_reduce_nanvar_full, test_reduce_nanstd_full,
    test_reduce_empty_sum, test_reduce_empty_min, test_reduce_small_var_axis
}; int num_tests=sizeof(tests)/sizeof(tests[0]); run_all_tests(tests, "Reduce Tests", num_tests); }
