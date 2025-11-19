#include "main.h"

// ============================================================================
// HELPER MACROS AND FUNCTIONS
// ============================================================================

#define VERIFY_SHAPE(node, expected_ndim, ...) \
    do { \
        nr_intp expected[] = {__VA_ARGS__}; \
        if (NODE_NDIM(node) != expected_ndim) { \
            printf("Shape mismatch: expected ndim=%d, got %d\n", expected_ndim, NODE_NDIM(node)); \
            return 0; \
        } \
        for (int i = 0; i < expected_ndim; i++) { \
            if (NODE_SHAPE(node)[i] != expected[i]) { \
                printf("Shape mismatch at dim %d: expected %lld, got %lld\n", i, expected[i], NODE_SHAPE(node)[i]); \
                return 0; \
            } \
        } \
    } while(0)

#define VERIFY_DATA_INT(node, expected_len, ...) \
    do { \
        nr_int expected[] = {__VA_ARGS__}; \
        char* data = (char*)NODE_DATA(node); \
        nr_intp stride = NODE_STRIDES(node)[0]; \
        for (int i = 0; i < expected_len; i++) { \
            nr_int value = *(nr_int*)(data + i * stride); \
            if (value != expected[i]) { \
                printf("Data mismatch at position %d: expected %lld, got %lld\n", i, expected[i], value); \
                return 0; \
            } \
        } \
    } while(0)

// ============================================================================
// INTEGER INDEXING TESTS
// ============================================================================

int test_index_int_1d_positive() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 3);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 0, 0);  // Scalar result
    nr_int value = *(nr_int*)NODE_DATA(indexed);
    if (value != 3) {
        printf("Expected value 3, got %lld\n", value);
        Node_Free(n1);
        Node_Free(indexed);
        return 0;
    }
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_int_1d_negative() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, -2);  // Should index element 8
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 0, 0);
    nr_int value = *(nr_int*)NODE_DATA(indexed);
    if (value != 8) {
        printf("Expected value 8, got %lld\n", value);
        Node_Free(n1);
        Node_Free(indexed);
        return 0;
    }
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_int_2d_single() {
    Node* n1;
    nr_intp shape[2] = {4, 5};
    TEST_NEW_NODE_INT(n1, 20, 2, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 2);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 5);
    VERIFY_DATA_INT(indexed, 5, 10, 11, 12, 13, 14);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_int_2d_both() {
    Node* n1;
    nr_intp shape[2] = {4, 5};
    TEST_NEW_NODE_INT(n1, 20, 2, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 2);
    NIndexRuleSet_AddInt(&rs, 3);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 0, 0);  // Scalar
    nr_int value = *(nr_int*)NODE_DATA(indexed);
    if (value != 13) {  // 2*5 + 3 = 13
        printf("Expected value 13, got %lld\n", value);
        Node_Free(n1);
        Node_Free(indexed);
        return 0;
    }
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_int_3d() {
    Node* n1;
    nr_intp shape[3] = {3, 4, 5};
    TEST_NEW_NODE_INT(n1, 60, 3, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 1);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 4, 5);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_int_out_of_bounds() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 10);  // Out of bounds
    Node* indexed = Node_Index(n1, &rs);
    if (indexed) {
        printf("Expected indexing to fail for out of bounds index.\n");
        Node_Free(n1);
        Node_Free(indexed);
        return 0;
    }
    
    Node_Free(n1);
    return 1;
}

// ============================================================================
// SLICE INDEXING TESTS
// ============================================================================

int test_index_slice_1d_basic() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 2, 7, 1);  // [2:7:1]
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 5);
    VERIFY_DATA_INT(indexed, 5, 2, 3, 4, 5, 6);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_slice_1d_step() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 0, 10, 2);  // [0:10:2]
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 5);
    VERIFY_DATA_INT(indexed, 5, 0, 2, 4, 6, 8);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_slice_1d_negative_step() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 8, 2, -2);  // [8:2:-2]
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 3);
    VERIFY_DATA_INT(indexed, 3, 8, 6, 4);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_slice_1d_negative_indices() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, -8, -2, 1);  // [-8:-2:1] = [2:8:1]
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 6);
    VERIFY_DATA_INT(indexed, 6, 2, 3, 4, 5, 6, 7);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_slice_2d_first_dim() {
    Node* n1;
    nr_intp shape[2] = {5, 4};
    TEST_NEW_NODE_INT(n1, 20, 2, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 1, 4, 1);  // [1:4]
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 3, 4);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_slice_2d_both_dims() {
    Node* n1;
    nr_intp shape[2] = {5, 6};
    TEST_NEW_NODE_INT(n1, 30, 2, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 1, 4, 1);   // [1:4]
    NIndexRuleSet_AddSlice(&rs, 2, 5, 1);   // [2:5]
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 3, 3);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_slice_step_zero() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 0, 5, 0);  // Step cannot be zero
    Node* indexed = Node_Index(n1, &rs);
    
    if (indexed) {
        printf("Expected indexing to fail for zero step.\n");
        Node_Free(n1);
        Node_Free(indexed);
        return 0;
    }
    
    Node_Free(n1);
    return 1;
}

// ============================================================================
// MIXED INT AND SLICE TESTS
// ============================================================================

int test_index_mixed_int_slice() {
    Node* n1;
    nr_intp shape[3] = {4, 5, 6};
    TEST_NEW_NODE_INT(n1, 120, 3, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 2);           // Fix first dimension
    NIndexRuleSet_AddSlice(&rs, 1, 4, 1);   // Slice second dimension
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 3, 6);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_mixed_slice_int() {
    Node* n1;
    nr_intp shape[3] = {4, 5, 6};
    TEST_NEW_NODE_INT(n1, 120, 3, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 0, 3, 1);   // Slice first dimension
    NIndexRuleSet_AddInt(&rs, 2);           // Fix second dimension
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 3, 6);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_mixed_alternating() {
    Node* n1;
    nr_intp shape[4] = {3, 4, 5, 6};
    TEST_NEW_NODE_INT(n1, 360, 4, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 1);
    NIndexRuleSet_AddSlice(&rs, 1, 3, 1);
    NIndexRuleSet_AddInt(&rs, 2);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 2, 6);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

// ============================================================================
// NEWAXIS TESTS
// ============================================================================

int test_index_newaxis_front() {
    Node* n1;
    nr_intp shape[2] = {3, 4};
    TEST_NEW_NODE_INT(n1, 12, 2, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNewAxis(&rs);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 3, 1, 3, 4);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_newaxis_middle() {
    Node* n1;
    nr_intp shape[2] = {3, 4};
    TEST_NEW_NODE_INT(n1, 12, 2, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 0, 3, 1);
    NIndexRuleSet_AddNewAxis(&rs);
    NIndexRuleSet_AddSlice(&rs, 0, 4, 1);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 3, 3, 1, 4);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_newaxis_multiple() {
    Node* n1;
    nr_intp shape[2] = {3, 4};
    TEST_NEW_NODE_INT(n1, 12, 2, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNewAxis(&rs);
    NIndexRuleSet_AddNewAxis(&rs);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 4, 1, 1, 3, 4);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_newaxis_with_int() {
    Node* n1;
    nr_intp shape[2] = {3, 4};
    TEST_NEW_NODE_INT(n1, 12, 2, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 1);
    NIndexRuleSet_AddNewAxis(&rs);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 1, 4);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

// ============================================================================
// ELLIPSIS TESTS
// ============================================================================

int test_index_ellipsis_front() {
    Node* n1;
    nr_intp shape[3] = {3, 4, 5};
    TEST_NEW_NODE_INT(n1, 60, 3, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddEllipsis(&rs);
    NIndexRuleSet_AddInt(&rs, 2);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 3, 4);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_ellipsis_middle() {
    Node* n1;
    nr_intp shape[4] = {2, 3, 4, 5};
    TEST_NEW_NODE_INT(n1, 120, 4, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 1);
    NIndexRuleSet_AddEllipsis(&rs);
    NIndexRuleSet_AddInt(&rs, 2);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 3, 4);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_ellipsis_back() {
    Node* n1;
    nr_intp shape[3] = {3, 4, 5};
    TEST_NEW_NODE_INT(n1, 60, 3, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 1);
    NIndexRuleSet_AddEllipsis(&rs);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 4, 5);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_ellipsis_multiple_error() {
    Node* n1;
    nr_intp shape[3] = {3, 4, 5};
    TEST_NEW_NODE_INT(n1, 60, 3, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddEllipsis(&rs);
    NIndexRuleSet_AddEllipsis(&rs);
    Node* indexed = Node_Index(n1, &rs);
    
    if (indexed) {
        printf("Expected indexing to fail for multiple ellipses.\n");
        Node_Free(n1);
        Node_Free(indexed);
        return 0;
    }
    
    Node_Free(n1);
    return 1;
}

// ============================================================================
// FANCY (NODE) INDEXING TESTS
// ============================================================================

int test_index_node_1d_simple() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    // Create index node
    Node* idx;
    nr_intp idx_shape[1] = {3};
    idx = Node_NewEmpty(1, idx_shape, NR_INT64);
    nr_int64* idx_data = (nr_int64*)NODE_DATA(idx);
    idx_data[0] = 2;
    idx_data[1] = 5;
    idx_data[2] = 8;
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNode(&rs, idx);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        Node_Free(idx);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 3);
    VERIFY_DATA_INT(indexed, 3, 2, 5, 8);
    
    Node_Free(n1);
    Node_Free(idx);
    Node_Free(indexed);
    return 1;
}

int test_index_node_2d_single_axis() {
    Node* n1;
    nr_intp shape[2] = {5, 4};
    TEST_NEW_NODE_INT(n1, 20, 2, shape);
    
    // Create index node for first axis
    Node* idx;
    nr_intp idx_shape[1] = {3};
    idx = Node_NewEmpty(1, idx_shape, NR_INT64);
    nr_int64* idx_data = (nr_int64*)NODE_DATA(idx);
    idx_data[0] = 0;
    idx_data[1] = 2;
    idx_data[2] = 4;
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNode(&rs, idx);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        Node_Free(idx);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 3, 4);
    
    Node_Free(n1);
    Node_Free(idx);
    Node_Free(indexed);
    return 1;
}

int test_index_node_2d_both_axes() {
    Node* n1;
    nr_intp shape[2] = {5, 6};
    TEST_NEW_NODE_INT(n1, 30, 2, shape);
    
    // Create index nodes
    Node* idx0;
    nr_intp idx0_shape[1] = {2};
    idx0 = Node_NewEmpty(1, idx0_shape, NR_INT64);
    nr_int64* idx0_data = (nr_int64*)NODE_DATA(idx0);
    idx0_data[0] = 1;
    idx0_data[1] = 3;
    
    Node* idx1;
    nr_intp idx1_shape[1] = {2};
    idx1 = Node_NewEmpty(1, idx1_shape, NR_INT64);
    nr_int64* idx1_data = (nr_int64*)NODE_DATA(idx1);
    idx1_data[0] = 2;
    idx1_data[1] = 4;
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNode(&rs, idx0);
    NIndexRuleSet_AddNode(&rs, idx1);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        Node_Free(idx0);
        Node_Free(idx1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 2);
    // Should get elements [1,2] and [3,4]
    // [1,2] = 1*6+2 = 8
    // [3,4] = 3*6+4 = 22
    VERIFY_DATA_INT(indexed, 2, 8, 22);
    
    Node_Free(n1);
    Node_Free(idx0);
    Node_Free(idx1);
    Node_Free(indexed);
    return 1;
}

int test_index_node_broadcasting() {
    Node* n1;
    nr_intp shape[2] = {4, 5};
    TEST_NEW_NODE_INT(n1, 20, 2, shape);
    
    // Create index nodes with different shapes
    Node* idx0;
    nr_intp idx0_shape[1] = {2};
    idx0 = Node_NewEmpty(1, idx0_shape, NR_INT64);
    nr_int64* idx0_data = (nr_int64*)NODE_DATA(idx0);
    idx0_data[0] = 0;
    idx0_data[1] = 2;
    
    Node* idx1;
    nr_intp idx1_shape[2] = {2, 1};
    idx1 = Node_NewEmpty(2, idx1_shape, NR_INT64);
    nr_int64* idx1_data = (nr_int64*)NODE_DATA(idx1);
    idx1_data[0] = 1;
    idx1_data[1] = 3;
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNode(&rs, idx0);
    NIndexRuleSet_AddNode(&rs, idx1);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        Node_Free(idx0);
        Node_Free(idx1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 2, 2);
    
    Node_Free(n1);
    Node_Free(idx0);
    Node_Free(idx1);
    Node_Free(indexed);
    return 1;
}

// ============================================================================
// BOOLEAN INDEXING TESTS
// ============================================================================

int test_index_bool_1d() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    // Create boolean index
    Node* bool_idx;
    nr_intp bool_shape[1] = {10};
    bool_idx = Node_NewEmpty(1, bool_shape, NR_BOOL);
    nr_bool* bool_data = (nr_bool*)NODE_DATA(bool_idx);
    for (int i = 0; i < 10; i++) {
        bool_data[i] = (i % 2 == 0);  // True for even indices
    }
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNode(&rs, bool_idx);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        Node_Free(bool_idx);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 5);
    VERIFY_DATA_INT(indexed, 5, 0, 2, 4, 6, 8);
    
    Node_Free(n1);
    Node_Free(bool_idx);
    Node_Free(indexed);
    return 1;
}

int test_index_bool_2d() {
    Node* n1;
    nr_intp shape[2] = {4, 5};
    TEST_NEW_NODE_INT(n1, 20, 2, shape);
    
    // Create boolean index
    Node* bool_idx;
    nr_intp bool_shape[2] = {4, 5};
    bool_idx = Node_NewEmpty(2, bool_shape, NR_BOOL);
    nr_bool* bool_data = (nr_bool*)NODE_DATA(bool_idx);
    for (int i = 0; i < 20; i++) {
        bool_data[i] = (i % 3 == 0);  // True for every 3rd element
    }
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNode(&rs, bool_idx);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        Node_Free(bool_idx);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 7);  // 7 elements match the condition
    
    Node_Free(n1);
    Node_Free(bool_idx);
    Node_Free(indexed);
    return 1;
}

// ============================================================================
// MIXED FANCY AND REGULAR INDEXING TESTS
// ============================================================================

int test_index_mixed_node_slice() {
    Node* n1;
    nr_intp shape[3] = {4, 5, 6};
    TEST_NEW_NODE_INT(n1, 120, 3, shape);
    
    // Create index node for first dimension
    Node* idx;
    nr_intp idx_shape[1] = {2};
    idx = Node_NewEmpty(1, idx_shape, NR_INT64);
    nr_int64* idx_data = (nr_int64*)NODE_DATA(idx);
    idx_data[0] = 0;
    idx_data[1] = 2;
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNode(&rs, idx);
    NIndexRuleSet_AddSlice(&rs, 1, 4, 1);  // Slice second dimension
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        Node_Free(idx);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 3, 2, 3, 6);
    
    Node_Free(n1);
    Node_Free(idx);
    Node_Free(indexed);
    return 1;
}

int test_index_mixed_node_int() {
    Node* n1;
    nr_intp shape[3] = {4, 5, 6};
    TEST_NEW_NODE_INT(n1, 120, 3, shape);
    
    // Create index node
    Node* idx;
    nr_intp idx_shape[1] = {3};
    idx = Node_NewEmpty(1, idx_shape, NR_INT64);
    nr_int64* idx_data = (nr_int64*)NODE_DATA(idx);
    idx_data[0] = 0;
    idx_data[1] = 1;
    idx_data[2] = 3;
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNode(&rs, idx);
    NIndexRuleSet_AddInt(&rs, 2);  // Fix second dimension
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        Node_Free(idx);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 3, 6);
    
    Node_Free(n1);
    Node_Free(idx);
    Node_Free(indexed);
    return 1;
}

int test_index_mixed_slice_node_slice() {
    Node* n1;
    nr_intp shape[4] = {3, 4, 5, 6};
    TEST_NEW_NODE_INT(n1, 360, 4, shape);
    
    // Create index node for second dimension
    Node* idx;
    nr_intp idx_shape[1] = {2};
    idx = Node_NewEmpty(1, idx_shape, NR_INT64);
    nr_int64* idx_data = (nr_int64*)NODE_DATA(idx);
    idx_data[0] = 1;
    idx_data[1] = 3;
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 0, 2, 1);   // Slice first dimension
    NIndexRuleSet_AddNode(&rs, idx);         // Fancy index second
    NIndexRuleSet_AddSlice(&rs, 2, 5, 1);   // Slice third dimension
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        Node_Free(idx);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 4, 2, 2, 3, 6);
    
    Node_Free(n1);
    Node_Free(idx);
    Node_Free(indexed);
    return 1;
}

// ============================================================================
// EDGE CASES AND SPECIAL TESTS
// ============================================================================

int test_index_empty_slice() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 5, 5, 1);  // Empty slice
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }

    VERIFY_SHAPE(indexed, 1, 0);  // Empty result
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_full_slice() {
    Node* n1;
    nr_intp shape[2] = {4, 5};
    TEST_NEW_NODE_INT(n1, 20, 2, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddFullSlice(&rs);
    NIndexRuleSet_AddFullSlice(&rs);
    Node* indexed = Node_Index(n1, &rs);

    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 4, 5);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_range() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddRange(&rs, 3, 8);  // [3:8]
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }

    VERIFY_SHAPE(indexed, 1, 5);
    VERIFY_DATA_INT(indexed, 5, 3, 4, 5, 6, 7);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_too_many_indices() {
    Node* n1;
    nr_intp shape[2] = {3, 4};
    TEST_NEW_NODE_INT(n1, 12, 2, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 0);
    NIndexRuleSet_AddInt(&rs, 1);
    NIndexRuleSet_AddInt(&rs, 2);  // Too many indices
    Node* indexed = Node_Index(n1, &rs);
    
    if (indexed) {
        printf("Expected indexing to fail for too many indices.\n");
        Node_Free(n1);
        Node_Free(indexed);
        return 0;
    }
    
    Node_Free(n1);
    return 1;
}

int test_index_scalar_node() {
    Node* n1;
    nr_intp shape[1] = {1};
    TEST_NEW_NODE_INT(n1, 1, 1, shape);
    nr_int* data = (nr_int*)NODE_DATA(n1);
    data[0] = 42;
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 0);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 0, 0);  // Scalar
    nr_int value = *(nr_int*)NODE_DATA(indexed);
    if (value != 42) {
        printf("Expected value 42, got %lld\n", value);
        Node_Free(n1);
        Node_Free(indexed);
        return 0;
    }
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_high_dimensional() {
    Node* n1;
    nr_intp shape[5] = {2, 3, 4, 5, 6};
    TEST_NEW_NODE_INT(n1, 720, 5, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 1);
    NIndexRuleSet_AddSlice(&rs, 0, 2, 1);
    NIndexRuleSet_AddInt(&rs, 2);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 3, 2, 5, 6);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_contiguous_check() {
    Node* n1;
    nr_intp shape[2] = {5, 6};
    TEST_NEW_NODE_INT(n1, 30, 2, shape);
    
    // Simple slice should maintain view (no copy)
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 1, 4, 1);
    NIndexRuleSet_AddSlice(&rs, 0, 6, 1);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 2, 3, 6);
    
    // Data should be a view, not a copy (check pointer relationship)
    char* base_data = (char*)NODE_DATA(n1);
    char* indexed_data = (char*)NODE_DATA(indexed);
    
    // Indexed data should point somewhere within base data range
    if (indexed_data < base_data || indexed_data >= base_data + 30 * sizeof(nr_int)) {
        printf("Expected view, but got copy.\n");
        Node_Free(n1);
        Node_Free(indexed);
        return 0;
    }
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_copy_needed() {
    Node* n1;
    nr_intp shape[2] = {5, 6};
    TEST_NEW_NODE_INT(n1, 30, 2, shape);
    
    // Integer indexing requires copy
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddInt(&rs, 2);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 6);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_large_step() {
    Node* n1;
    nr_intp shape[1] = {100};
    TEST_NEW_NODE_INT(n1, 100, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSlice(&rs, 0, 100, 10);  // Every 10th element
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 10);
    VERIFY_DATA_INT(indexed, 10, 0, 10, 20, 30, 40, 50, 60, 70, 80, 90);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_negative_step_full() {
    Node* n1;
    nr_intp shape[1] = {10};
    TEST_NEW_NODE_INT(n1, 10, 1, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddSliceAdvanced(&rs, 0, 0, -1, 0, 0);  // Reverse the array
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 10);
    VERIFY_DATA_INT(indexed, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_complex_fancy() {
    Node* n1;
    nr_intp shape[3] = {5, 6, 7};
    TEST_NEW_NODE_INT(n1, 210, 3, shape);
    
    // Create two index nodes
    Node* idx0;
    nr_intp idx0_shape[2] = {2, 3};
    idx0 = Node_NewEmpty(2, idx0_shape, NR_INT64);
    nr_int64* idx0_data = (nr_int64*)NODE_DATA(idx0);
    idx0_data[0] = 0; idx0_data[1] = 1; idx0_data[2] = 2;
    idx0_data[3] = 2; idx0_data[4] = 3; idx0_data[5] = 4;
    
    Node* idx1;
    nr_intp idx1_shape[2] = {2, 3};
    idx1 = Node_NewEmpty(2, idx1_shape, NR_INT64);
    nr_int64* idx1_data = (nr_int64*)NODE_DATA(idx1);
    idx1_data[0] = 1; idx1_data[1] = 2; idx1_data[2] = 3;
    idx1_data[3] = 3; idx1_data[4] = 4; idx1_data[5] = 5;
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNode(&rs, idx0);
    NIndexRuleSet_AddNode(&rs, idx1);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        Node_Free(idx0);
        Node_Free(idx1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 3, 2, 3, 7);
    
    Node_Free(n1);
    Node_Free(idx0);
    Node_Free(idx1);
    Node_Free(indexed);
    return 1;
}

int test_index_newaxis_ellipsis_combination() {
    Node* n1;
    nr_intp shape[3] = {3, 4, 5};
    TEST_NEW_NODE_INT(n1, 60, 3, shape);
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNewAxis(&rs);
    NIndexRuleSet_AddEllipsis(&rs);
    NIndexRuleSet_AddNewAxis(&rs);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 5, 1, 3, 4, 5, 1);
    
    Node_Free(n1);
    Node_Free(indexed);
    return 1;
}

int test_index_single_element_fancy() {
    Node* n1;
    nr_intp shape[2] = {5, 5};
    TEST_NEW_NODE_INT(n1, 25, 2, shape);
    
    // Create single element index nodes
    Node* idx0;
    nr_intp idx0_shape[1] = {1};
    idx0 = Node_NewEmpty(1, idx0_shape, NR_INT64);
    nr_int64* idx0_data = (nr_int64*)NODE_DATA(idx0);
    idx0_data[0] = 2;
    
    Node* idx1;
    nr_intp idx1_shape[1] = {1};
    idx1 = Node_NewEmpty(1, idx1_shape, NR_INT64);
    nr_int64* idx1_data = (nr_int64*)NODE_DATA(idx1);
    idx1_data[0] = 3;
    
    NIndexRuleSet rs = NIndexRuleSet_New();
    NIndexRuleSet_AddNode(&rs, idx0);
    NIndexRuleSet_AddNode(&rs, idx1);
    Node* indexed = Node_Index(n1, &rs);
    
    if (!indexed) {
        printf("Indexing failed.\n");
        Node_Free(n1);
        Node_Free(idx0);
        Node_Free(idx1);
        return 0;
    }
    
    VERIFY_SHAPE(indexed, 1, 1);
    nr_int value = *(nr_int*)NODE_DATA(indexed);
    if (value != 13) {  // 2*5 + 3 = 13
        printf("Expected value 13, got %lld\n", value);
        Node_Free(n1);
        Node_Free(idx0);
        Node_Free(idx1);
        Node_Free(indexed);
        return 0;
    }
    
    Node_Free(n1);
    Node_Free(idx0);
    Node_Free(idx1);
    Node_Free(indexed);
    return 1;
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

void test_index() {
    TestFunc tests[] = {
        // Integer indexing tests
        test_index_int_1d_positive,
        test_index_int_1d_negative,
        test_index_int_2d_single,
        test_index_int_2d_both,
        test_index_int_3d,
        test_index_int_out_of_bounds,
        
        // Slice indexing tests
        test_index_slice_1d_basic,
        test_index_slice_1d_step,
        test_index_slice_1d_negative_step,
        test_index_slice_1d_negative_indices,
        test_index_slice_2d_first_dim,
        test_index_slice_2d_both_dims,
        test_index_slice_step_zero,
        
        // Mixed int and slice tests
        test_index_mixed_int_slice,
        test_index_mixed_slice_int,
        test_index_mixed_alternating,
        
        // NewAxis tests
        test_index_newaxis_front,
        test_index_newaxis_middle,
        test_index_newaxis_multiple,
        test_index_newaxis_with_int,
        
        // Ellipsis tests
        test_index_ellipsis_front,
        test_index_ellipsis_middle,
        test_index_ellipsis_back,
        test_index_ellipsis_multiple_error,
        
        // Fancy (node) indexing tests
        test_index_node_1d_simple,
        test_index_node_2d_single_axis,
        test_index_node_2d_both_axes,
        test_index_node_broadcasting,
        
        // Boolean indexing tests
        test_index_bool_1d,
        test_index_bool_2d,
        
        // Mixed fancy and regular indexing
        test_index_mixed_node_slice,
        test_index_mixed_node_int,
        test_index_mixed_slice_node_slice,
        
        // Edge cases and special tests
        test_index_empty_slice,
        test_index_full_slice,
        test_index_range,
        test_index_too_many_indices,
        test_index_scalar_node,
        test_index_high_dimensional,
        test_index_contiguous_check,
        test_index_copy_needed,
        test_index_large_step,
        test_index_negative_step_full,
        test_index_complex_fancy,
        test_index_newaxis_ellipsis_combination,
        test_index_single_element_fancy,
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    run_all_tests(tests, "Index Tests", num_tests);
}