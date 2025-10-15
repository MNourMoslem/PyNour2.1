#include "main.h"
#include "../src/node_core.h"
#include "../src/free.h"

NR_STATIC int test_copy_with_reference_shares_data_and_refcount(){
    int data[4] = {10, 20, 30, 40};
    nr_intp shape[2] = {2, 2};

    // create owning node
    Node* owner = Node_New(data, 1, 2, shape, NR_INT32);
    if (!owner) return 0;

    int initial_ref = owner->ref_count;

    // create a copy that references the same data
    Node* view = Node_CopyWithReference(owner);
    if (!view){ Node_Free(owner); return 0; }

    // src refcount should have incremented
    if (owner->ref_count != initial_ref + 1){ Node_Free(view); Node_Free(owner); return 0; }

    // modify data through view
    int* p = (int*)view->data;
    p[1] = 99; // change second element

    // change should reflect on owner
    int* q = (int*)owner->data;
    if (q[1] != 99){ Node_Free(view); Node_Free(owner); return 0; }

    // free view: should decrement owner's refcount but not free the data
    Node_Free(view);
    if (owner->ref_count != initial_ref){ Node_Free(owner); return 0; }

    // free owner: should free the underlying data and fully release
    Node_Free(owner);

    return 1;
}

void test_refcount_test(){
    run_test("Refcount/CopyWithReference Test", 1, (test_callback_t[]){
        test_copy_with_reference_shares_data_and_refcount
    });
}
