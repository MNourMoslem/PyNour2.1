#include "main.h"
#include "../src/node_core.h"
#include "../src/index.h"
#include "../src/free.h"

NR_STATIC int test_boolean_mask_basic(){
    // Create a simple 1D array: [1, 2, 3, 4, 5]
    int data[5] = {1, 2, 3, 4, 5};
    nr_intp shape[1] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    // Create a boolean mask: [true, false, true, false, true]
    char mask_data[5] = {1, 0, 1, 0, 1};
    Node* mask = Node_New(mask_data, 1, 1, shape, NR_BOOL);
    if (!mask) { Node_Free(node); return 0; }

    // Apply boolean mask
    Node* result = Node_BooleanMask(node, mask);
    if (!result) { Node_Free(mask); Node_Free(node); return 0; }

    // Result should have shape [3] and contain [1, 3, 5]
    if (NODE_NDIM(result) != 1 || NODE_SHAPE(result)[0] != 3) {
        Node_Free(result); Node_Free(mask); Node_Free(node);
        return 0;
    }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 1 || result_data[1] != 3 || result_data[2] != 5) {
        Node_Free(result); Node_Free(mask); Node_Free(node);
        return 0;
    }

    Node_Free(result);
    Node_Free(mask);
    Node_Free(node);
    return 1;
}

NR_STATIC int test_boolean_mask_2d(){
    // Create a 2D array: [[1, 2, 3], [4, 5, 6]]
    int data[6] = {1, 2, 3, 4, 5, 6};
    nr_intp shape[2] = {2, 3};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    // Create a boolean mask: [[true, false, true], [false, true, false]]
    char mask_data[6] = {1, 0, 1, 0, 1, 0};
    Node* mask = Node_New(mask_data, 1, 2, shape, NR_BOOL);
    if (!mask) { Node_Free(node); return 0; }

    // Apply boolean mask
    Node* result = Node_BooleanMask(node, mask);
    if (!result) { Node_Free(mask); Node_Free(node); return 0; }

    // Result should have shape [3] and contain [1, 3, 5]
    if (NODE_NDIM(result) != 1 || NODE_SHAPE(result)[0] != 3) {
        Node_Free(result); Node_Free(mask); Node_Free(node);
        return 0;
    }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 1 || result_data[1] != 3 || result_data[2] != 5) {
        Node_Free(result); Node_Free(mask); Node_Free(node);
        return 0;
    }

    Node_Free(result);
    Node_Free(mask);
    Node_Free(node);
    return 1;
}

NR_STATIC int test_boolean_mask_all_false(){
    // Create a simple 1D array: [1, 2, 3]
    int data[3] = {1, 2, 3};
    nr_intp shape[1] = {3};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    // Create a boolean mask with all false
    char mask_data[3] = {0, 0, 0};
    Node* mask = Node_New(mask_data, 1, 1, shape, NR_BOOL);
    if (!mask) { Node_Free(node); return 0; }

    // Apply boolean mask
    Node* result = Node_BooleanMask(node, mask);
    if (!result) { Node_Free(mask); Node_Free(node); return 0; }

    // Result should have shape [0]
    if (NODE_NDIM(result) != 1 || NODE_SHAPE(result)[0] != 0) {
        Node_Free(result); Node_Free(mask); Node_Free(node);
        return 0;
    }

    Node_Free(result);
    Node_Free(mask);
    Node_Free(node);
    return 1;
}

NR_STATIC int test_boolean_mask_all_true(){
    // Create a simple 1D array: [10, 20, 30]
    int data[3] = {10, 20, 30};
    nr_intp shape[1] = {3};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node) return 0;

    // Create a boolean mask with all true
    char mask_data[3] = {1, 1, 1};
    Node* mask = Node_New(mask_data, 1, 1, shape, NR_BOOL);
    if (!mask) { Node_Free(node); return 0; }

    // Apply boolean mask
    Node* result = Node_BooleanMask(node, mask);
    if (!result) { Node_Free(mask); Node_Free(node); return 0; }

    // Result should have shape [3] and contain [10, 20, 30]
    if (NODE_NDIM(result) != 1 || NODE_SHAPE(result)[0] != 3) {
        Node_Free(result); Node_Free(mask); Node_Free(node);
        return 0;
    }

    int* result_data = (int*)NODE_DATA(result);
    if (result_data[0] != 10 || result_data[1] != 20 || result_data[2] != 30) {
        Node_Free(result); Node_Free(mask); Node_Free(node);
        return 0;
    }

    Node_Free(result);
    Node_Free(mask);
    Node_Free(node);
    return 1;
}

NR_STATIC int test_boolean_mask_shape_mismatch(){
    // Create a simple 1D array: [1, 2, 3]
    int data[3] = {1, 2, 3};
    nr_intp shape1[1] = {3};
    Node* node = Node_New(data, 1, 1, shape1, NR_INT32);
    if (!node) return 0;

    // Create a boolean mask with different shape [4]
    char mask_data[4] = {1, 0, 1, 0};
    nr_intp shape2[1] = {4};
    Node* mask = Node_New(mask_data, 1, 1, shape2, NR_BOOL);
    if (!mask) { Node_Free(node); return 0; }

    // Apply boolean mask - should fail
    Node* result = Node_BooleanMask(node, mask);
    int success = (result == NULL && NError_IsError());

    Node_Free(mask);
    Node_Free(node);
    if (result) Node_Free(result);
    
    return success;
}

void test_boolean_mask_test(){
    run_test("Boolean Mask Test", 5, (test_callback_t[]){
        test_boolean_mask_basic,
        test_boolean_mask_2d,
        test_boolean_mask_all_false,
        test_boolean_mask_all_true,
        test_boolean_mask_shape_mismatch
    });
}
