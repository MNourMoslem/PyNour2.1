#include "nour/nour.h"
#include "../src/nmath.h"
#include "../src/node_core.h"
#include "../src/free.h"
#include "../src/niter.h"
#include "main.h"


int
test_addition(){
    int data1[6] = {1,2,3,4,5,6};
    int data2[6] = {10,20,30,40,50,60};
    nr_intp shape[1] = {6};

    Node* node1 = Node_New(data1, 1, 1, shape, NR_INT32);
    Node* node2 = Node_New(data2, 1, 1, shape, NR_INT32);
    if (!node1 || !node2){
        if (node1) Node_Free(node1);
        if (node2) Node_Free(node2);
        return 0;
    }

    Node* result = NMath_Add(NULL, node1, node2);
    if (!result){
        Node_Free(node1);
        Node_Free(node2);
        return 0;
    }

    // Check result data
    int* res_data = (int*)NODE_DATA(result);
    for (int i = 0; i < 6; i++){
        if (res_data[i] != data1[i] + data2[i]){
            Node_Free(node1);
            Node_Free(node2);
            Node_Free(result);
            return 0;
        }
    }

    Node_Free(node1);
    Node_Free(node2);
    Node_Free(result);
    return 1;
}

int
test_subtraction(){
    int data1[6] = {10,20,30,40,50,60};
    int data2[6] = {1,2,3,4,5,6};
    nr_intp shape[1] = {6};

    Node* node1 = Node_New(data1, 1, 1, shape, NR_INT32);
    Node* node2 = Node_New(data2, 1, 1, shape, NR_INT32);
    if (!node1 || !node2){
        if (node1) Node_Free(node1);
        if (node2) Node_Free(node2);
        return 0;
    }

    Node* result = NMath_Sub(NULL, node1, node2);
    if (!result){
        Node_Free(node1);
        Node_Free(node2);
        return 0;
    }

    // Check result data
    int* res_data = (int*)NODE_DATA(result);
    for (int i = 0; i < 6; i++){
        if (res_data[i] != data1[i] - data2[i]){
            Node_Free(node1);
            Node_Free(node2);
            Node_Free(result);
            return 0;
        }
    }

    Node_Free(node1);
    Node_Free(node2);
    Node_Free(result);
    return 1;
}

int
test_multiplication(){
    int data1[6] = {1,2,3,4,5,6};
    int data2[6] = {10,20,30,40,50,60};
    nr_intp shape[1] = {6};

    Node* node1 = Node_New(data1, 1, 1, shape, NR_INT32);
    Node* node2 = Node_New(data2, 1, 1, shape, NR_INT32);
    if (!node1 || !node2){
        if (node1) Node_Free(node1);
        if (node2) Node_Free(node2);
        return 0;
    }

    Node* result = NMath_Mul(NULL, node1, node2);
    if (!result){
        Node_Free(node1);
        Node_Free(node2);
        return 0;
    }

    // Check result data
    int* res_data = (int*)NODE_DATA(result);
    for (int i = 0; i < 6; i++){
        if (res_data[i] != data1[i] * data2[i]){
            Node_Free(node1);
            Node_Free(node2);
            Node_Free(result);
            return 0;
        }
    }

    Node_Free(node1);
    Node_Free(node2);
    Node_Free(result);
    return 1;
}

void test_nmath_test(){
    run_test("NMath Test", 3, (test_callback_t[]){
        test_addition,
        test_subtraction,
        test_multiplication
    });
}