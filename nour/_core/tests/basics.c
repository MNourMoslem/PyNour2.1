#include "main.h"

int test_basic_1(){
    Node* n1;
    TEST_NEW_NODE_32_INT_FLAT(n1);

    for (int i = 0; i < 32; i++) {
        nr_int* data = (nr_int*)NODE_DATA(n1);
        if (data[i] != i) {
            Node_Free(n1);
            return 0;
        }
    }

    Node_Free(n1);
    return 1;
}


void test_basic() {
    run_all_tests((TestFunc[]){
        test_basic_1,
    }, "Basic", 1);
}