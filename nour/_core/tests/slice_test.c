#include "main.h"
#include "../src/node_core.h"
#include "../src/slice.h"
#include "../src/free.h"
#include "../src/nerror.h"

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

void test_slice_test(){
    run_test("Slice Test", 5, (test_callback_t[]){
        test_slice_basic,
        test_slice_step,
        test_slice_negative_indices,
        test_slice_zero_step,
        test_slice_empty
    });
}
