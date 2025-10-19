#include "nour/nour.h"
#include "../src/nmath.h"
#include "main.h"


int test_basic_add(){
    float data1[6] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
    float data2[6] = {10.0, 20.0, 30.0, 40.0, 50.0, 60.0};
    float expected[6] = {11.0, 22.0, 33.0, 44.0, 55.0, 66.0};
    nr_intp shape[2] = {2, 3};

    Node* n1 = Node_New(data1, 1, 2, shape, NR_FLOAT32);
    Node* n2 = Node_New(data2, 1, 2, shape, NR_FLOAT32);
    Node* out = NMath_Add(NULL, n1, n2);

    NIter iter;
    NIter_FromNode(&iter, out, NITER_MODE_STRIDED);
    NIter_ITER(&iter);

    int idx = 0;
    while (NIter_NOTDONE(&iter))
    {
        float* val = (float*)NIter_ITEM(&iter);
        if (*val != expected[idx]) {
            Node_Free(n1);
            Node_Free(n2);
            Node_Free(out);
            return 0; // Failure
        }
        NIter_NEXT(&iter);
        idx++;
    }

    Node_Free(n1);
    Node_Free(n2);
    Node_Free(out);
    return 1; // Success
}

void test_nmath_test(){
    run_test("nmath_test", 1, (test_callback_t[]){
        test_basic_add
    });
}