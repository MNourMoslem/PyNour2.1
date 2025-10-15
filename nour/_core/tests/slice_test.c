#include "main.h"
#include "../src/node_core.h"
#include "../src/slice.h"
#include "../src/free.h"
#include "../src/nerror.h"
#include "../src/niter.h"

// Basic slice: [start:stop]
int test_slice_basic(){
    int data[6] = {1,2,3,4,5,6};
    nr_intp shape[1] = {6};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    Slice s = {.start = 1, .stop = 4, .step = 1}; // should give elements 2,3,4
    if (Node_Slice(node, s, 0) != 0){Node_Free(node); return 0; }

    if (NODE_SHAPE(node)[0] != 3){Node_Free(node); return 0; }

    // Check first element points to original data[1]
    int* first = (int*)NODE_DATA(node);
    if (*first != 2){Node_Free(node); return 0; }

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
    if (Node_Slice(node, s, 0) != 0){ Node_Free(node); return 0; }

    if (NODE_SHAPE(node)[0] != 3){ Node_Free(node); return 0; }
    int* first = (int*)NODE_DATA(node);
    if (*first != 1){ Node_Free(node); return 0; }

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
    if (Node_Slice(node, s, 0) != 0){ Node_Free(node); return 0; }

    if (NODE_SHAPE(node)[0] != 3){ Node_Free(node); return 0; }
    int* first = (int*)NODE_DATA(node);
    if (*first != 3){ Node_Free(node); return 0; }

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
    if (Node_Slice(node, s, 0) == 0){ Node_Free(node); return 0; } // should fail

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
    if (Node_Slice(node, s, 0) != 0){ Node_Free(node); return 0; }

    if (NODE_SHAPE(node)[0] != 0){ Node_Free(node); return 0; }

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
    if (Node_Slice(node, s, 1) != 0){ Node_Free(node); return 0; }

    if (NODE_SHAPE(node)[1] != 2){ Node_Free(node); return 0; }

    // Check first element points to original data[1]
    float* first = (float*)NODE_DATA(node);
    if (*first != 2.0f){ Node_Free(node); return 0; }

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
    if (Node_MultiSlice(node, slices, 2) != 0){ Node_Free(node); return 0; }

    if (NODE_SHAPE(node)[0] != 2 || NODE_SHAPE(node)[1] != 2){ Node_Free(node); return 0; }
    
    // First element should be data[1*4 + 1] = data[5] = 6
    int* first = (int*)NODE_DATA(node);
    if (*first != 6){ Node_Free(node); return 0; }

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
    if (Node_MultiSlice(node, slices, 2) != 0){ Node_Free(node); return 0; }

    if (NODE_SHAPE(node)[0] != 2 || NODE_SHAPE(node)[1] != 4){ Node_Free(node); return 0; }
    
    // First element should be data[1*4] = data[4] = 5
    int* first = (int*)NODE_DATA(node);
    if (*first != 5){ Node_Free(node); return 0; }

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
    if (Node_MultiSlice(node, slices, 2) != 0){ Node_Free(node); return 0; }

    if (NODE_SHAPE(node)[0] != 2 || NODE_SHAPE(node)[1] != 2){ Node_Free(node); return 0; }
    
    // First element should be data[0] = 1
    int* first = (int*)NODE_DATA(node);
    if (*first != 1){ Node_Free(node); return 0; }

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
    if (Node_MultiSlice(node, slices, 3) != 0){ Node_Free(node); return 0; }

    if (NODE_SHAPE(node)[0] != 1 || NODE_SHAPE(node)[1] != 2 || NODE_SHAPE(node)[2] != 2){
        Node_Free(node); 
        return 0; 
    }
    
    // First element: data[1*12 + 1*4 + 2] = data[18] = 19
    int* first = (int*)NODE_DATA(node);
    if (*first != 19){ Node_Free(node); return 0; }

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
    if (Node_MultiSlice(node, slices, 2) != 0){ Node_Free(node); return 0; }

    if (NODE_SHAPE(node)[0] != 2 || NODE_SHAPE(node)[1] != 2){ Node_Free(node); return 0; }
    
    // First element: data[1*4 + 1] = data[5] = 6
    int* first = (int*)NODE_DATA(node);
    if (*first != 6){ Node_Free(node); return 0; }

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
    if (Node_MultiSlice(node, slices, 2) != 0){ Node_Free(node); return 0; }

    // Shape should be [1, 2, 4] - third dimension unchanged
    if (NODE_SHAPE(node)[0] != 1 || NODE_SHAPE(node)[1] != 2 || NODE_SHAPE(node)[2] != 4){
        Node_Free(node); 
        return 0; 
    }

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
    if (Node_MultiSlice(node, slices, 2) == 0){ Node_Free(node); return 0; } // should fail

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
    if (Node_MultiSlice(node, slices, 2) != 0){ Node_Free(node); return 0; }

    if (NODE_SHAPE(node)[0] != 0){ Node_Free(node); return 0; }

    Node_Free(node);
    return 1;
}

void test_multislice_test(){
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
}

void test_slice_test(){
    run_test("Slice Test", 6, (test_callback_t[]){
        test_slice_basic,
        test_slice_step,
        test_slice_negative_indices,
        test_slice_zero_step,
        test_slice_empty,
        test_4x4_slice
    });
}
