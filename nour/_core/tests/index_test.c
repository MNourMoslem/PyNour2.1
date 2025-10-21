#include "main.h"
#include "../src/node_core.h"
#include "../src/index.h"
#include "../src/free.h"
#include "../src/nerror.h"
#include "../src/niter.h"
#include "../src/narray.h"

// Basic slice: [start:stop]
int test_slice_basic(){
    int data[6] = {1,2,3,4,5,6};
    nr_intp shape[1] = {6};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    Slice s = {.start = 1, .stop = 4, .step = 1}; // should give elements 2,3,4
    Node* sliced = Node_Slice(NULL, node, s, 0);
    if (!sliced){Node_Free(node); return 0; }

    if (NODE_SHAPE(sliced)[0] != 3){Node_Free(sliced); Node_Free(node); return 0; }

    // Check first element points to original data[1]
    int* first = (int*)NODE_DATA(sliced);
    if (*first != 2){Node_Free(sliced); Node_Free(node); return 0; }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}

// Step > 1
int test_slice_step(){
    int data[6] = {1,2,3,4,5,6};
    nr_intp shape[1] = {6};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    Slice s = {.start = 0, .stop = 6, .step = 2}; // 1,3,5
    Node* sliced = Node_Slice(NULL, node, s, 0);
    if (!sliced){ Node_Free(node); return 0; }

    if (NODE_SHAPE(sliced)[0] != 3){ Node_Free(sliced); Node_Free(node); return 0; }
    int* first = (int*)NODE_DATA(sliced);
    if (*first != 1){ Node_Free(sliced); Node_Free(node); return 0; }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}

// Negative indices
int test_slice_negative_indices(){
    int data[6] = {1,2,3,4,5,6};
    nr_intp shape[1] = {6};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    Slice s = {.start = -4, .stop = -1, .step = 1}; // start -> 2, stop -> 5 -> elements 3,4,5
    Node* sliced = Node_Slice(NULL, node, s, 0);
    if (!sliced){ Node_Free(node); return 0; }

    if (NODE_SHAPE(sliced)[0] != 3){ Node_Free(sliced); Node_Free(node); return 0; }
    int* first = (int*)NODE_DATA(sliced);
    if (*first != 3){ Node_Free(sliced); Node_Free(node); return 0; }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}

// Zero step should error
int test_slice_zero_step(){
    int data[6] = {1,2,3,4,5,6};
    nr_intp shape[1] = {6};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    Slice s = {.start = 0, .stop = 6, .step = 0};
    Node* sliced = Node_Slice(NULL, node, s, 0);
    if (sliced){ Node_Free(sliced); Node_Free(node); return 0; } // should fail

    Node_Free(node);
    return 1;
}

// Empty slice
int test_slice_empty(){
    int data[6] = {1,2,3,4,5,6};
    nr_intp shape[1] = {6};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    Slice s = {.start = 2, .stop = 2, .step = 1}; // empty
    Node* sliced = Node_Slice(NULL, node, s, 0);
    if (!sliced){ Node_Free(node); return 0; }

    if (NODE_SHAPE(sliced)[0] != 0){ Node_Free(sliced); Node_Free(node); return 0; }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}


int test_4x4_slice(){
    float data[16] = { 1.,  2.,  3.,  4.,
                       5.,  6.,  7.,  8.,
                       9., 10., 11., 12.,
                      13., 14., 15., 16. };
    nr_intp shape[2] = {4,4};
    Node* node = Node_New(data, 1, 2, shape, NR_FLOAT32);
    if (!node) return 0;

    Slice s = {.start = 1, .stop = 3, .step = 1}; // should give elements 2,3
    Node* sliced = Node_Slice(NULL, node, s, 1);
    if (!sliced){ Node_Free(node); return 0; }

    if (NODE_SHAPE(sliced)[1] != 2){ Node_Free(sliced); Node_Free(node); return 0; }

    // Check first element points to original data[1]
    float* first = (float*)NODE_DATA(sliced);
    if (*first != 2.0f){ Node_Free(sliced); Node_Free(node); return 0; }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}

// Basic multi-slice on 2D array
int test_multislice_2d_basic(){
    int data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    nr_intp shape[2] = {3,4}; // 3x4 array
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    // Slice [1:3, 1:3] -> 2x2 array starting at (1,1)
    Slice slices[2] = {
        {.start = 1, .stop = 3, .step = 1},
        {.start = 1, .stop = 3, .step = 1}
    };
    Node* sliced = Node_MultiSlice(NULL, node, slices, 2);
    if (!sliced){ Node_Free(node); return 0; }

    if (NODE_SHAPE(sliced)[0] != 2 || NODE_SHAPE(sliced)[1] != 2){ Node_Free(sliced); Node_Free(node); return 0; }
    
    // First element should be data[1*4 + 1] = data[5] = 6
    int* first = (int*)NODE_DATA(sliced);
    if (*first != 6){ Node_Free(sliced); Node_Free(node); return 0; }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}

// Multi-slice with NULL/skip slices
int test_multislice_skip(){
    int data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    nr_intp shape[2] = {3,4};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    // Slice [1:3, :] -> slice only first dimension
    Slice slices[2] = {
        {.start = 1, .stop = 3, .step = 1},
        {.start = 0, .stop = 0, .step = 0}  // NULL slice
    };
    Node* sliced = Node_MultiSlice(NULL, node, slices, 2);
    if (!sliced){ Node_Free(node); return 0; }

    if (NODE_SHAPE(sliced)[0] != 2 || NODE_SHAPE(sliced)[1] != 4){ Node_Free(sliced); Node_Free(node); return 0; }
    
    // First element should be data[1*4] = data[4] = 5
    int* first = (int*)NODE_DATA(sliced);
    if (*first != 5){ Node_Free(sliced); Node_Free(node); return 0; }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}

// Multi-slice with steps
int test_multislice_with_steps(){
    int data[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    nr_intp shape[2] = {4,4};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    // Slice [::2, ::2] -> every other element in both dimensions
    Slice slices[2] = {
        {.start = 0, .stop = 4, .step = 2},
        {.start = 0, .stop = 4, .step = 2}
    };
    Node* sliced = Node_MultiSlice(NULL, node, slices, 2);
    if (!sliced){ Node_Free(node); return 0; }

    if (NODE_SHAPE(sliced)[0] != 2 || NODE_SHAPE(sliced)[1] != 2){ Node_Free(sliced); Node_Free(node); return 0; }
    
    // First element should be data[0] = 1
    int* first = (int*)NODE_DATA(sliced);
    if (*first != 1){ Node_Free(sliced); Node_Free(node); return 0; }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}

// Multi-slice on 3D array
int test_multislice_3d(){
    int data[24];
    for (int i = 0; i < 24; i++) data[i] = i + 1;
    
    nr_intp shape[3] = {2,3,4}; // 2x3x4 array
    Node* node = Node_New(data, 1, 3, shape, NR_INT32);
    if (!node) return 0;

    // Slice [1:2, 1:3, 2:4] 
    Slice slices[3] = {
        {.start = 1, .stop = 2, .step = 1},  // dim 0: 1 element
        {.start = 1, .stop = 3, .step = 1},  // dim 1: 2 elements
        {.start = 2, .stop = 4, .step = 1}   // dim 2: 2 elements
    };
    Node* sliced = Node_MultiSlice(NULL, node, slices, 3);
    if (!sliced){ Node_Free(node); return 0; }

    if (NODE_SHAPE(sliced)[0] != 1 || NODE_SHAPE(sliced)[1] != 2 || NODE_SHAPE(sliced)[2] != 2){
        Node_Free(sliced);
        Node_Free(node); 
        return 0; 
    }
    
    // First element: data[1*12 + 1*4 + 2] = data[18] = 19
    int* first = (int*)NODE_DATA(sliced);
    if (*first != 19){ Node_Free(sliced); Node_Free(node); return 0; }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}

// Multi-slice with negative indices
int test_multislice_negative(){
    int data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    nr_intp shape[2] = {3,4};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    // Slice [-2:, -3:-1]
    Slice slices[2] = {
        {.start = -2, .stop = 3, .step = 1},   // rows 1,2
        {.start = -3, .stop = -1, .step = 1}   // cols 1,2
    };
    Node* sliced = Node_MultiSlice(NULL, node, slices, 2);
    if (!sliced){ Node_Free(node); return 0; }

    if (NODE_SHAPE(sliced)[0] != 2 || NODE_SHAPE(sliced)[1] != 2){ Node_Free(sliced); Node_Free(node); return 0; }
    
    // First element: data[1*4 + 1] = data[5] = 6
    int* first = (int*)NODE_DATA(sliced);
    if (*first != 6){ Node_Free(sliced); Node_Free(node); return 0; }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}

// Multi-slice with fewer slices than dimensions
int test_multislice_partial(){
    int data[24];
    for (int i = 0; i < 24; i++) data[i] = i + 1;
    
    nr_intp shape[3] = {2,3,4};
    Node* node = Node_New(data, 1, 3, shape, NR_INT32);
    if (!node) return 0;

    // Only slice first two dimensions, leave third unchanged
    Slice slices[2] = {
        {.start = 1, .stop = 2, .step = 1},
        {.start = 0, .stop = 2, .step = 1}
    };
    Node* sliced = Node_MultiSlice(NULL, node, slices, 2);
    if (!sliced){ Node_Free(node); return 0; }

    // Shape should be [1, 2, 4] - third dimension unchanged
    if (NODE_SHAPE(sliced)[0] != 1 || NODE_SHAPE(sliced)[1] != 2 || NODE_SHAPE(sliced)[2] != 4){
        Node_Free(sliced);
        Node_Free(node); 
        return 0; 
    }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}

// Multi-slice with zero step should error
int test_multislice_zero_step(){
    int data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    nr_intp shape[2] = {3,4};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    Slice slices[2] = {
        {.start = 0, .stop = 2, .step = 1},
        {.start = 0, .stop = 2, .step = 0}  // Invalid step
    };
    Node* sliced = Node_MultiSlice(NULL, node, slices, 2);
    if (sliced){ Node_Free(sliced); Node_Free(node); return 0; } // should fail

    Node_Free(node);
    return 1;
}

// Multi-slice resulting in empty array
int test_multislice_empty(){
    int data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    nr_intp shape[2] = {3,4};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    // Empty slice on first dimension
    Slice slices[2] = {
        {.start = 2, .stop = 2, .step = 1},  // Empty
        {.start = 0, .stop = 4, .step = 1}
    };
    Node* sliced = Node_MultiSlice(NULL, node, slices, 2);
    if (!sliced){ Node_Free(node); return 0; }

    if (NODE_SHAPE(sliced)[0] != 0){ Node_Free(sliced); Node_Free(node); return 0; }

    Node_Free(sliced);
    Node_Free(node);
    return 1;
}

// Test Node_IndexWithIntArray - basic 1D indexing
int test_index_with_int_array_1d(){
    int data[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    nr_intp shape[1] = {10};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[4] = {0, 3, 5, 9};
    NArray* idx_arr = NArray_FromIntArray(indices, 4, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    Node* result = Node_IndexWithIntArray(node, idx_arr, 0);
    if (!result) { NArray_Free(idx_arr); Node_Free(node); return 0; }

    if (NODE_SHAPE(result)[0] != 4) { Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0; }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 10 || result_data[1] != 40 || result_data[2] != 60 || result_data[3] != 100) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

// Test Node_IndexWithIntArray - negative indices
int test_index_with_int_array_negative(){
    int data[5] = {10, 20, 30, 40, 50};
    nr_intp shape[1] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[3] = {-1, -3, -5};
    NArray* idx_arr = NArray_FromIntArray(indices, 3, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    Node* result = Node_IndexWithIntArray(node, idx_arr, 0);
    if (!result) { NArray_Free(idx_arr); Node_Free(node); return 0; }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 50 || result_data[1] != 30 || result_data[2] != 10) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

// Test Node_IndexWithIntArray - 2D array indexing along axis 0
int test_index_with_int_array_2d_axis0(){
    int data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    nr_intp shape[2] = {3,4};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[2] = {0, 2};
    NArray* idx_arr = NArray_FromIntArray(indices, 2, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    Node* result = Node_IndexWithIntArray(node, idx_arr, 0);
    if (!result) { NArray_Free(idx_arr); Node_Free(node); return 0; }

    if (NODE_SHAPE(result)[0] != 2 || NODE_SHAPE(result)[1] != 4) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 1 || result_data[4] != 9) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

// Test Node_IndexWithIntArray - 2D array indexing along axis 1
int test_index_with_int_array_2d_axis1(){
    int data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    nr_intp shape[2] = {3,4};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[2] = {1, 3};
    NArray* idx_arr = NArray_FromIntArray(indices, 2, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    Node* result = Node_IndexWithIntArray(node, idx_arr, 1);
    if (!result) { NArray_Free(idx_arr); Node_Free(node); return 0; }

    if (NODE_SHAPE(result)[0] != 3 || NODE_SHAPE(result)[1] != 2) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 2 || result_data[1] != 4) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

// Test Node_IndexWithIntArray - out of bounds should fail
int test_index_with_int_array_out_of_bounds(){
    int data[5] = {10, 20, 30, 40, 50};
    nr_intp shape[1] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[2] = {0, 10};
    NArray* idx_arr = NArray_FromIntArray(indices, 2, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    Node* result = Node_IndexWithIntArray(node, idx_arr, 0);
    if (result) { Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0; }

    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

// Test Node_IndexWithBooleanArray - basic 1D masking
int test_index_with_boolean_array_1d(){
    int data[5] = {10, 20, 30, 40, 50};
    nr_intp shape[1] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_bool mask[5] = {1, 0, 1, 0, 1};
    NArray* mask_arr = NArray_FromBoolArray(mask, 5, 1);
    if (!mask_arr) { Node_Free(node); return 0; }

    Node* result = Node_IndexWithBooleanArray(node, mask_arr);
    if (!result) { NArray_Free(mask_arr); Node_Free(node); return 0; }

    if (NODE_SHAPE(result)[0] != 3) {
        Node_Free(result); NArray_Free(mask_arr); Node_Free(node); return 0;
    }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 10 || result_data[1] != 30 || result_data[2] != 50) {
        Node_Free(result); NArray_Free(mask_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(mask_arr);
    Node_Free(node);
    return 1;
}

// Test Node_IndexWithBooleanArray - all false
int test_index_with_boolean_array_all_false(){
    int data[4] = {10, 20, 30, 40};
    nr_intp shape[1] = {4};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_bool mask[4] = {0, 0, 0, 0};
    NArray* mask_arr = NArray_FromBoolArray(mask, 4, 1);
    if (!mask_arr) { Node_Free(node); return 0; }

    Node* result = Node_IndexWithBooleanArray(node, mask_arr);
    if (!result) { NArray_Free(mask_arr); Node_Free(node); return 0; }

    if (NODE_SHAPE(result)[0] != 0) {
        Node_Free(result); NArray_Free(mask_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(mask_arr);
    Node_Free(node);
    return 1;
}

// Test Node_IndexWithBooleanArray - all true
int test_index_with_boolean_array_all_true(){
    int data[4] = {10, 20, 30, 40};
    nr_intp shape[1] = {4};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_bool mask[4] = {1, 1, 1, 1};
    NArray* mask_arr = NArray_FromBoolArray(mask, 4, 1);
    if (!mask_arr) { Node_Free(node); return 0; }

    Node* result = Node_IndexWithBooleanArray(node, mask_arr);
    if (!result) { NArray_Free(mask_arr); Node_Free(node); return 0; }

    if (NODE_SHAPE(result)[0] != 4) {
        Node_Free(result); NArray_Free(mask_arr); Node_Free(node); return 0;
    }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 10 || result_data[3] != 40) {
        Node_Free(result); NArray_Free(mask_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(mask_arr);
    Node_Free(node);
    return 1;
}

// Test Node_IndexWithBooleanArray - 2D array
int test_index_with_boolean_array_2d(){
    int data[6] = {1, 2, 3, 4, 5, 6};
    nr_intp shape[2] = {2, 3};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    nr_bool mask[6] = {1, 0, 1, 0, 1, 0};
    nr_intp mask_shape[2] = {2, 3};
    NArray* mask_arr = NArray_New(mask, 2, mask_shape, NULL, NR_BOOL, 1);
    if (!mask_arr) { Node_Free(node); return 0; }

    Node* result = Node_IndexWithBooleanArray(node, mask_arr);
    if (!result) { NArray_Free(mask_arr); Node_Free(node); return 0; }

    if (NODE_SHAPE(result)[0] != 3) {
        Node_Free(result); NArray_Free(mask_arr); Node_Free(node); return 0;
    }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 1 || result_data[1] != 3 || result_data[2] != 5) {
        Node_Free(result); NArray_Free(mask_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(mask_arr);
    Node_Free(node);
    return 1;
}

// Test Node_Take - basic take with error mode
int test_node_take_basic(){
    int data[5] = {10, 20, 30, 40, 50};
    nr_intp shape[1] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[3] = {0, 2, 4};
    NArray* idx_arr = NArray_FromIntArray(indices, 3, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    Node* result = Node_Take(node, idx_arr, 0, 0); // mode 0 = error
    if (!result) { NArray_Free(idx_arr); Node_Free(node); return 0; }

    if (NODE_SHAPE(result)[0] != 3) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 10 || result_data[1] != 30 || result_data[2] != 50) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

// Test Node_Take - wrap mode
int test_node_take_wrap(){
    int data[5] = {10, 20, 30, 40, 50};
    nr_intp shape[1] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[4] = {0, 7, -1, -8};
    NArray* idx_arr = NArray_FromIntArray(indices, 4, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    Node* result = Node_Take(node, idx_arr, 0, 1); // mode 1 = wrap
    if (!result) { NArray_Free(idx_arr); Node_Free(node); return 0; }

    int* result_data = (int*)NODE_DATA(result);
    // 0->10, 7%5=2->30, -1%5=4->50, -8%5=2->30
    if (result_data[0] != 10 || result_data[1] != 30 || result_data[2] != 50 || result_data[3] != 30) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

// Test Node_Take - clip mode
int test_node_take_clip(){
    int data[5] = {10, 20, 30, 40, 50};
    nr_intp shape[1] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[4] = {-10, 0, 4, 100};
    NArray* idx_arr = NArray_FromIntArray(indices, 4, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    Node* result = Node_Take(node, idx_arr, 0, 2); // mode 2 = clip
    if (!result) { NArray_Free(idx_arr); Node_Free(node); return 0; }

    int* result_data = (int*)NODE_DATA(result);
    // -10->0->10, 0->10, 4->50, 100->4->50
    if (result_data[0] != 10 || result_data[1] != 10 || result_data[2] != 50 || result_data[3] != 50) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

// Test Node_Take - 2D array along axis 1
int test_node_take_2d(){
    int data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    nr_intp shape[2] = {3,4};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[2] = {0, 2};
    NArray* idx_arr = NArray_FromIntArray(indices, 2, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    Node* result = Node_Take(node, idx_arr, 1, 0);
    if (!result) { NArray_Free(idx_arr); Node_Free(node); return 0; }

    if (NODE_SHAPE(result)[0] != 3 || NODE_SHAPE(result)[1] != 2) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 1 || result_data[1] != 3) {
        Node_Free(result); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

// Test Node_Put - basic put
int test_node_put_basic(){
    int data[5] = {10, 20, 30, 40, 50};
    nr_intp shape[1] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[2] = {1, 3};
    NArray* idx_arr = NArray_FromIntArray(indices, 2, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    int values_data[2] = {99, 88};
    nr_intp values_shape[1] = {2};
    Node* values = Node_New(values_data, 1, 1, values_shape, NR_INT32);
    if (!values) { NArray_Free(idx_arr); Node_Free(node); return 0; }

    int status = Node_Put(node, idx_arr, values, 0);
    if (status != 0) { Node_Free(values); NArray_Free(idx_arr); Node_Free(node); return 0; }

    int* result_data = (int*)NODE_DATA(node);
    if (result_data[1] != 99 || result_data[3] != 88) {
        Node_Free(values); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    Node_Free(values);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

// Test Node_Put - broadcast single value
int test_node_put_broadcast(){
    int data[5] = {10, 20, 30, 40, 50};
    nr_intp shape[1] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[3] = {0, 2, 4};
    NArray* idx_arr = NArray_FromIntArray(indices, 3, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    int value_data[1] = {77};
    nr_intp value_shape[1] = {1};
    Node* values = Node_New(value_data, 1, 1, value_shape, NR_INT32);
    if (!values) { NArray_Free(idx_arr); Node_Free(node); return 0; }

    int status = Node_Put(node, idx_arr, values, 0);
    if (status != 0) { Node_Free(values); NArray_Free(idx_arr); Node_Free(node); return 0; }

    int* result_data = (int*)NODE_DATA(node);
    if (result_data[0] != 77 || result_data[2] != 77 || result_data[4] != 77) {
        Node_Free(values); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    Node_Free(values);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

// Test Node_Put - wrap mode
int test_node_put_wrap(){
    int data[5] = {10, 20, 30, 40, 50};
    nr_intp shape[1] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    nr_intp indices[2] = {-1, 7};
    NArray* idx_arr = NArray_FromIntArray(indices, 2, 1);
    if (!idx_arr) { Node_Free(node); return 0; }

    int values_data[2] = {99, 88};
    nr_intp values_shape[1] = {2};
    Node* values = Node_New(values_data, 1, 1, values_shape, NR_INT32);
    if (!values) { NArray_Free(idx_arr); Node_Free(node); return 0; }

    int status = Node_Put(node, idx_arr, values, 1); // mode 1 = wrap
    if (status != 0) { Node_Free(values); NArray_Free(idx_arr); Node_Free(node); return 0; }

    int* result_data = (int*)NODE_DATA(node);
    // -1 wraps to 4, 7 wraps to 2
    if (result_data[4] != 99 || result_data[2] != 88) {
        Node_Free(values); NArray_Free(idx_arr); Node_Free(node); return 0;
    }

    Node_Free(values);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}


void test_index_test(){
    run_test("Slice Test", 6, (test_callback_t[]){
        test_slice_basic,
        test_slice_step,
        test_slice_negative_indices,
        test_slice_zero_step,
        test_slice_empty,
        test_4x4_slice
    });
    
    run_test("Multi-Slice Test", 8, (test_callback_t[]){
        test_multislice_2d_basic,
        test_multislice_skip,
        test_multislice_with_steps,
        test_multislice_3d,
        test_multislice_negative,
        test_multislice_partial,
        test_multislice_zero_step,
        test_multislice_empty
    });
    
    run_test("NArray Integer Indexing Test", 5, (test_callback_t[]){
        test_index_with_int_array_1d,
        test_index_with_int_array_negative,
        test_index_with_int_array_2d_axis0,
        test_index_with_int_array_2d_axis1,
        test_index_with_int_array_out_of_bounds
    });
    
    run_test("NArray Boolean Indexing Test", 4, (test_callback_t[]){
        test_index_with_boolean_array_1d,
        test_index_with_boolean_array_all_false,
        test_index_with_boolean_array_all_true,
        test_index_with_boolean_array_2d
    });
    
    run_test("Node Take Test", 4, (test_callback_t[]){
        test_node_take_basic,
        test_node_take_wrap,
        test_node_take_clip,
        test_node_take_2d
    });
    
    run_test("Node Put Test", 3, (test_callback_t[]){
        test_node_put_basic,
        test_node_put_broadcast,
        test_node_put_wrap
    });
}
