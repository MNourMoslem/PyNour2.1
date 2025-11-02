#include "main.h"
#include "../src/node_core.h"
#include "../src/niter.h"
#include "../src/free.h"

NR_STATIC int test_basic_allocation(){
    int data[6] = {1, 2, 3, 4, 5, 6};
    nr_intp shape[2] = {2, 3};

    Node* node = Node_New(data, 1, 2, shape, NR_INT32);

    NIter iter;
    NIter_FromNode(&iter, node, NITER_MODE_STRIDED);
    NIter_ITER(&iter);

    int idx = 0;
    while (NIter_NOTDONE(&iter))
    {
        int* val = (int*)NIter_ITEM(&iter);
        if (*val != data[idx]) {
            Node_Free(node);
            return 0; // Failure
        }
        NIter_NEXT(&iter);
        idx++;
    }
    
    Node_Free(node);
    return 1; // Success
}


int test_basic_copy(){
    float data[12] = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2};
    nr_intp shape[3] = {2, 2, 3};
    Node* n1 = Node_New(data, 1, 3, shape, NR_FLOAT32);
    Node* n2 = Node_Copy(NULL, n1);

    NMultiIter mit;
    NMultiIter_FromNodes((Node*[]){n1, n2}, 2, &mit);
    NMultiIter_ITER(&mit);

    int idx = 0;
    while (NMultiIter_NOTDONE(&mit))
    {
        float* val1 = (float*)NMultiIter_ITEM(&mit, 0);
        float* val2 = (float*)NMultiIter_ITEM(&mit, 1);
        if (*val1 != data[idx] || *val2 != data[idx]) {
            Node_Free(n1);
            Node_Free(n2);
            return 0; // Failure
        }
        NMultiIter_NEXT(&mit);
        idx++;
    }

    Node_Free(n1);
    Node_Free(n2);
    return 1; // Success
}

void test_allocation_test(){
    run_test("Allocation Test", 2, (test_callback_t[]){
        test_basic_allocation,
        test_basic_copy
    });
}