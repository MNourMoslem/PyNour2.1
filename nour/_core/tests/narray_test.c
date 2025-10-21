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
    nr_intp indices[] = {0, 2, 4};
    nr_intp size = 3;
    
    NArray* arr = NArray_FromIntArray(indices, size, 1);
    if (!arr){
        return 0;
    }
    
    if (NARRAY_SIZE(arr) != 3){
        NArray_Free(arr);
        return 0;
    }
    
    if (NARRAY_DTYPE(arr) != NR_INT64){
        NArray_Free(arr);
        return 0;
    }
    
    NArray_Free(arr);
    return 1;
}

/*
    Test 2: Index a Node with integer array
*/
int test_node_index_with_int_array(){
    // Create a 1D node with values [10, 20, 30, 40, 50]
    nr_int32 data[] = {10, 20, 30, 40, 50};
    nr_intp shape[] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node){
        return 0;
    }
    
    // Create index array [0, 2, 4]
    nr_intp indices[] = {0, 2, 4};
    NArray* idx_arr = NArray_FromIntArray(indices, 3, 1);
    if (!idx_arr){
        Node_Free(node);
        return 0;
    }
    
    // Index the node
    Node* result = Node_IndexWithIntArray(node, idx_arr, 0);
    if (!result){
        NArray_Free(idx_arr);
        Node_Free(node);
        return 0;
    }
    
    // Check result shape
    if (NODE_SHAPE(result)[0] != 3){
        Node_Free(result);
        NArray_Free(idx_arr);
        Node_Free(node);
        return 0;
    }
    
    // Check result values
    nr_int32* result_data = (nr_int32*)NODE_DATA(result);
    if (result_data[0] != 10 || result_data[1] != 30 || result_data[2] != 50){
        Node_Free(result);
        NArray_Free(idx_arr);
        Node_Free(node);
        return 0;
    }
    
    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

/*
    Test 3: Index a Node with boolean array
*/
int test_node_index_with_boolean_array(){
    // Create a 1D node with values [10, 20, 30, 40, 50]
    nr_int32 data[] = {10, 20, 30, 40, 50};
    nr_intp shape[] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node){
        return 0;
    }
    
    // Create boolean mask [true, false, true, false, true]
    nr_bool mask[] = {1, 0, 1, 0, 1};
    NArray* mask_arr = NArray_FromBoolArray(mask, 5, 1);
    if (!mask_arr){
        Node_Free(node);
        return 0;
    }
    
    // Index the node
    Node* result = Node_IndexWithBooleanArray(node, mask_arr);
    if (!result){
        NArray_Free(mask_arr);
        Node_Free(node);
        return 0;
    }
    
    // Check result shape (should be 3 true values)
    if (NODE_SHAPE(result)[0] != 3){
        Node_Free(result);
        NArray_Free(mask_arr);
        Node_Free(node);
        return 0;
    }
    
    // Check result values
    nr_int32* result_data = (nr_int32*)NODE_DATA(result);
    if (result_data[0] != 10 || result_data[1] != 30 || result_data[2] != 50){
        Node_Free(result);
        NArray_Free(mask_arr);
        Node_Free(node);
        return 0;
    }
    
    Node_Free(result);
    NArray_Free(mask_arr);
    Node_Free(node);
    return 1;
}

/*
    Test 4: Node_Take with different modes
*/
int test_node_take(){
    // Create a 1D node with values [10, 20, 30, 40, 50]
    nr_int32 data[] = {10, 20, 30, 40, 50};
    nr_intp shape[] = {5};
    Node* node = Node_New(data, 1, 1, shape, NR_INT32);
    if (!node){
        return 0;
    }
    
    // Test with wrap mode: indices [0, 2, -1, 7]
    // -1 should wrap to 4, 7 should wrap to 2
    nr_intp indices[] = {0, 2, -1, 7};
    NArray* idx_arr = NArray_FromIntArray(indices, 4, 1);
    if (!idx_arr){
        Node_Free(node);
        return 0;
    }
    
    Node* result = Node_Take(node, idx_arr, 0, 1); // mode 1 = wrap
    if (!result){
        NArray_Free(idx_arr);
        Node_Free(node);
        return 0;
    }
    
    // Check result
    nr_int32* result_data = (nr_int32*)NODE_DATA(result);
    if (result_data[0] != 10 || result_data[1] != 30 || 
        result_data[2] != 50 || result_data[3] != 30){
        Node_Free(result);
        NArray_Free(idx_arr);
        Node_Free(node);
        return 0;
    }
    
    Node_Free(result);
    NArray_Free(idx_arr);
    Node_Free(node);
    return 1;
}

/*
    Test 5: NArray_FromNode conversion
*/
int test_narray_from_node(){
    nr_int32 data[] = {1, 2, 3, 4, 5, 6};
    nr_intp shape[] = {2, 3};
    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node){
        return 0;
    }
    
    NArray* arr = NArray_FromNode(node, 1); // copy data
    if (!arr){
        Node_Free(node);
        return 0;
    }
    
    if (NARRAY_NDIM(arr) != 2){
        NArray_Free(arr);
        Node_Free(node);
        return 0;
    }
    
    if (NARRAY_SIZE(arr) != 6){
        NArray_Free(arr);
        Node_Free(node);
        return 0;
    }
    
    NArray_Free(arr);
    Node_Free(node);
    return 1;
}

/*
    Run all NArray tests
*/
int test_narray(){
    int passed = 0;
    int total = 5;
    
    passed += test_narray_from_int_array();
    passed += test_node_index_with_int_array();
    passed += test_node_index_with_boolean_array();
    passed += test_node_take();
    passed += test_narray_from_node();
    
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
