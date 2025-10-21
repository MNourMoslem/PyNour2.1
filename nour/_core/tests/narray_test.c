/*
    Test file for NArray functionality
    
    This file demonstrates how to use NArray for indexing operations.
*/

#include "../src/narray.h"
#include "../src/index.h"
#include "../src/node_core.h"
#include "../src/free.h"
#include "../src/nerror.h"
#include "main.h"
#include <stdio.h>

/*
    Test 1: Create NArray from integer array
*/
int test_narray_from_int_array(){
    printf("Running test_narray_from_int_array...\n");
    
    nr_intp indices[] = {0, 2, 4};
    nr_intp size = 3;
    
    NArray* arr = NArray_FromIntArray(indices, size, 1);
    if (!arr){
        printf("Failed to create NArray from int array\n");
        return 0;
    }
    
    if (NARRAY_SIZE(arr) != 3){
        printf("Expected size 3, got %lld\n", (long long)NARRAY_SIZE(arr));
        NArray_Free(arr);
        return 0;
    }
    
    if (NARRAY_DTYPE(arr) != NR_INT64){
        printf("Expected dtype NR_INT64\n");
        NArray_Free(arr);
        return 0;
    }
    
    NArray_Free(arr);
    printf("test_narray_from_int_array passed\n");
    return 1;
}

/*
    Test 2: Index a Node with integer array
*/
int test_node_index_with_int_array(){
    printf("Running test_node_index_with_int_array...\n");
    
    // Create a 1D node with values [10, 20, 30, 40, 50]
    nr_int32 data[] = {10, 20, 30, 40, 50};
    nr_intp shape[] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node){
        printf("Failed to create node\n");
        return 0;
    }
    
    // Create index array [0, 2, 4]
    nr_intp indices[] = {0, 2, 4};
    NArray* idx_arr = NArray_FromIntArray(indices, 3, 1);
    if (!idx_arr){
        printf("Failed to create index array\n");
        Node_Free(node);
        return 0;
    }
    
    // Index the node
    Node* result = Node_IndexWithIntArray(node, idx_arr, 0);
    if (!result){
        printf("Failed to index node\n");
        NArray_Free(idx_arr);
        Node_Free(node);
        return 0;
    }
    
    // Check result shape
    if (NODE_SHAPE(result)[0] != 3){
        printf("Expected result shape [3], got [%lld]\n", 
               (long long)NODE_SHAPE(result)[0]);
        Node_Free(result);
        NArray_Free(idx_arr);
        Node_Free(node);
        return 0;
    }
    
    // Check result values
    nr_int32* result_data = (nr_int32*)NODE_DATA(result);
    if (result_data[0] != 10 || result_data[1] != 30 || result_data[2] != 50){
        printf("Expected [10, 30, 50], got [%d, %d, %d]\n",
               result_data[0], result_data[1], result_data[2]);
        Node_Free(result);
        NArray_Free(idx_arr);
        Node_Free(node);
        return 0;
    }
    
    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    printf("test_node_index_with_int_array passed\n");
    return 1;
}

/*
    Test 3: Index a Node with boolean array
*/
int test_node_index_with_boolean_array(){
    printf("Running test_node_index_with_boolean_array...\n");
    
    // Create a 1D node with values [10, 20, 30, 40, 50]
    nr_int32 data[] = {10, 20, 30, 40, 50};
    nr_intp shape[] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node){
        printf("Failed to create node\n");
        return 0;
    }
    
    // Create boolean mask [true, false, true, false, true]
    nr_bool mask[] = {1, 0, 1, 0, 1};
    NArray* mask_arr = NArray_FromBoolArray(mask, 5, 1);
    if (!mask_arr){
        printf("Failed to create boolean array\n");
        Node_Free(node);
        return 0;
    }
    
    // Index the node
    Node* result = Node_IndexWithBooleanArray(node, mask_arr);
    if (!result){
        printf("Failed to index node with boolean array\n");
        NArray_Free(mask_arr);
        Node_Free(node);
        return 0;
    }
    
    // Check result shape (should be 3 true values)
    if (NODE_SHAPE(result)[0] != 3){
        printf("Expected result shape [3], got [%lld]\n", 
               (long long)NODE_SHAPE(result)[0]);
        Node_Free(result);
        NArray_Free(mask_arr);
        Node_Free(node);
        return 0;
    }
    
    // Check result values
    nr_int32* result_data = (nr_int32*)NODE_DATA(result);
    if (result_data[0] != 10 || result_data[1] != 30 || result_data[2] != 50){
        printf("Expected [10, 30, 50], got [%d, %d, %d]\n",
               result_data[0], result_data[1], result_data[2]);
        Node_Free(result);
        NArray_Free(mask_arr);
        Node_Free(node);
        return 0;
    }
    
    Node_Free(result);
    NArray_Free(mask_arr);
    Node_Free(node);
    printf("test_node_index_with_boolean_array passed\n");
    return 1;
}

/*
    Test 4: Node_Take with different modes
*/
int test_node_take(){
    printf("Running test_node_take...\n");
    
    // Create a 1D node with values [10, 20, 30, 40, 50]
    nr_int32 data[] = {10, 20, 30, 40, 50};
    nr_intp shape[] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node){
        printf("Failed to create node\n");
        return 0;
    }
    
    // Test with wrap mode: indices [0, 2, -1, 7]
    // -1 should wrap to 4, 7 should wrap to 2
    nr_intp indices[] = {0, 2, -1, 7};
    NArray* idx_arr = NArray_FromIntArray(indices, 4, 1);
    if (!idx_arr){
        printf("Failed to create index array\n");
        Node_Free(node);
        return 0;
    }
    
    Node* result = Node_Take(node, idx_arr, 0, 1); // mode 1 = wrap
    if (!result){
        printf("Failed to take from node\n");
        NArray_Free(idx_arr);
        Node_Free(node);
        return 0;
    }
    
    // Check result
    nr_int32* result_data = (nr_int32*)NODE_DATA(result);
    if (result_data[0] != 10 || result_data[1] != 30 || 
        result_data[2] != 50 || result_data[3] != 30){
        printf("Expected [10, 30, 50, 30], got [%d, %d, %d, %d]\n",
               result_data[0], result_data[1], result_data[2], result_data[3]);
        Node_Free(result);
        NArray_Free(idx_arr);
        Node_Free(node);
        return 0;
    }
    
    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    printf("test_node_take passed\n");
    return 1;
}

/*
    Test 5: NArray_FromNode conversion
*/
int test_narray_from_node(){
    printf("Running test_narray_from_node...\n");
    
    nr_int32 data[] = {1, 2, 3, 4, 5, 6};
    nr_intp shape[] = {2, 3};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node){
        printf("Failed to create node\n");
        return 0;
    }
    
    NArray* arr = NArray_FromNode(node, 1); // copy data
    if (!arr){
        printf("Failed to create NArray from node\n");
        Node_Free(node);
        return 0;
    }
    
    if (NARRAY_NDIM(arr) != 2){
        printf("Expected ndim 2, got %d\n", NARRAY_NDIM(arr));
        NArray_Free(arr);
        Node_Free(node);
        return 0;
    }
    
    if (NARRAY_SIZE(arr) != 6){
        printf("Expected size 6, got %lld\n", (long long)NARRAY_SIZE(arr));
        NArray_Free(arr);
        Node_Free(node);
        return 0;
    }
    
    NArray_Free(arr);
    Node_Free(node);
    printf("test_narray_from_node passed\n");
    return 1;
}

/*
    Run all NArray tests
*/
int test_narray(){
    printf("\n=== Running NArray Tests ===\n");
    
    int passed = 0;
    int total = 5;
    
    passed += test_narray_from_int_array();
    passed += test_node_index_with_int_array();
    passed += test_node_index_with_boolean_array();
    passed += test_node_take();
    passed += test_narray_from_node();
    
    printf("\n=== NArray Tests Complete: %d/%d passed ===\n\n", passed, total);
    
    return (passed == total);
}

void test_narray_test(){
    run_test("NArray Test", 5, (test_callback_t[]){
        test_narray_from_int_array,
        test_node_index_with_int_array,
        test_node_index_with_boolean_array,
        test_node_take,
        test_narray_from_node
    });
}
