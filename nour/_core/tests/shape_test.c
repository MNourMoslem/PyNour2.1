#include "main.h"
#include "../src/node_core.h"
#include "../src/shape.h"
#include "../src/free.h"

NR_STATIC int test_reshape(){
    int data[6] = {1,2,3,4,5,6};
    nr_intp shape[2] = {2,3};

    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    nr_long new_shp[2] = {3,2};
    if (Node_Reshape(node, new_shp, 2) != 0){
        Node_Free(node);
        return 0;
    }

    // Check shape and total items
    if (NODE_NDIM(node) != 2 || NODE_SHAPE(node)[0] != 3 || NODE_SHAPE(node)[1] != 2){
        Node_Free(node);
        return 0;
    }

    if (Node_NItems(node) != 6){
        Node_Free(node);
        return 0;
    }

    Node_Free(node);
    return 1;
}

NR_STATIC int test_reshape_3d(){
    int data[24] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
    nr_intp shape[3] = {2,3,4};

    Node* node = Node_New(data, 1, 3, shape, NR_INT32);
    if (!node) return 0;

    // Reshape to different 3D shape
    nr_long new_shp[3] = {4,2,3};
    if (Node_Reshape(node, new_shp, 3) != 0){
        Node_Free(node);
        return 0;
    }

    // Check shape and total items preserved
    if (NODE_NDIM(node) != 3 || 
        NODE_SHAPE(node)[0] != 4 || 
        NODE_SHAPE(node)[1] != 2 || 
        NODE_SHAPE(node)[2] != 3 ||
        Node_NItems(node) != 24){
        Node_Free(node);
        return 0;
    }

    // Try invalid reshape (wrong total size)
    nr_long bad_shp[2] = {5,5};
    if (Node_Reshape(node, bad_shp, 2) == 0){ // Should fail
        Node_Free(node);
        return 0;
    }

    Node_Free(node);
    return 1;
}

NR_STATIC int test_reshape_to_1d(){
    float data[6] = {1.0f,2.0f,3.0f,4.0f,5.0f,6.0f};
    nr_intp shape[3] = {1,2,3};

    Node* node = Node_New(data, 1, 3, shape, NR_FLOAT32);
    if (!node) return 0;

    // Flatten to 1D
    nr_long new_shp[1] = {6};
    if (Node_Reshape(node, new_shp, 1) != 0){
        Node_Free(node);
        return 0;
    }

    if (NODE_NDIM(node) != 1 || NODE_SHAPE(node)[0] != 6){
        Node_Free(node);
        return 0;
    }

    Node_Free(node);
    return 1;
}

int test_squeeze(){
    int data[6] = {1,2,3,4,5,6};
    nr_intp shape[3] = {1,2,3};

    Node* node = Node_New(data, 1, 3, shape, NR_INT32);
    if (!node) return 0;

    if (Node_Squeeze(node) != 0){
        Node_Free(node);
        return 0;
    }

    // After squeeze, ndim should be 2 and first dim removed
    if (NODE_NDIM(node) != 2 || 
        NODE_SHAPE(node)[0] != 2 || 
        NODE_SHAPE(node)[1] != 3) {
        Node_Free(node);
        return 0;
    }

    Node_Free(node);
    return 1;
}

int test_squeeze_multiple(){
    int data[6] = {1,2,3,4,5,6};
    nr_intp shape[5] = {1,1,2,3,1};

    Node* node = Node_New(data, 1, 5, shape, NR_INT32);
    if (!node) return 0;

    if (Node_Squeeze(node) != 0){
        Node_Free(node);
        return 0;
    }

    // After squeeze, ndim should be 2 and dims of size 1 removed
    if (NODE_NDIM(node) != 2 || 
        NODE_SHAPE(node)[0] != 2 || 
        NODE_SHAPE(node)[1] != 3) {
        Node_Free(node);
        return 0;
    }

    Node_Free(node);
    return 1;
}

int test_squeeze_all_ones(){
    int data[1] = {42};
    nr_intp shape[3] = {1,1,1};

    Node* node = Node_New(data, 1, 3, shape, NR_INT32);
    if (!node) return 0;

    if (Node_Squeeze(node) != 0){
        Node_Free(node);
        return 0;
    }

    // After squeezing all ones, should be 0-dim scalar
    if (NODE_NDIM(node) != 0 || Node_NItems(node) != 1){
        Node_Free(node);
        return 0;
    }

    Node_Free(node);
    return 1;
}

int test_transpose(){
    int data[6] = {1,2,3,4,5,6};
    nr_intp shape[2] = {2,3};

    Node* node = Node_New(data, 1, 2, shape, NR_INT32);
    if (!node) return 0;

    if (Node_Transpose(node) != 0){
        Node_Free(node);
        return 0;
    }

    // Expect shape to be reversed
    if (NODE_NDIM(node) != 2 || NODE_SHAPE(node)[0] != 3 || NODE_SHAPE(node)[1] != 2){
        Node_Free(node);
        return 0;
    }

    Node_Free(node);
    return 1;
}

int test_transpose_3d(){
    int data[24] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
    nr_intp shape[3] = {2,3,4};

    Node* node = Node_New(data, 1, 3, shape, NR_INT32);
    if (!node) return 0;

    if (Node_Transpose(node) != 0){
        Node_Free(node);
        return 0;
    }

    // Expect shape to be reversed for 3D
    if (NODE_NDIM(node) != 3 || 
        NODE_SHAPE(node)[0] != 4 || 
        NODE_SHAPE(node)[1] != 3 || 
        NODE_SHAPE(node)[2] != 2){
        Node_Free(node);
        return 0;
    }

    Node_Free(node);
    return 1;
}

int test_swap_axes(){
    float data[12] = {1,2,3,4,5,6,7,8,9,10,11,12};
    nr_intp shape[3] = {2,2,3};

    Node* node = Node_New(data, 1, 3, shape, NR_FLOAT32);
    if (!node) return 0;

    // Swap first and last axes
    if (Node_SwapAxes(node, 0, 2) != 0){
        Node_Free(node);
        return 0;
    }

    if (NODE_NDIM(node) != 3 || 
        NODE_SHAPE(node)[0] != 3 || 
        NODE_SHAPE(node)[1] != 2 || 
        NODE_SHAPE(node)[2] != 2){
        Node_Free(node);
        return 0;
    }

    // Test invalid axis
    if (Node_SwapAxes(node, 0, 3) == 0){ // Should fail
        Node_Free(node);
        return 0;
    }

    Node_Free(node);
    return 1;
}

int test_swap_axes_multi(){
    int data[24] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
    nr_intp shape[4] = {2,2,2,3};

    Node* node = Node_New(data, 1, 4, shape, NR_INT32);
    if (!node) return 0;

    // Swap multiple pairs of axes
    int axes1[] = {0, 1, 2};
    int axes2[] = {3, 2, 1};
    if (Node_SwapAxesMulti(node, axes1, axes2, 3) != 0){
        Node_Free(node);
        return 0;
    }

    if (NODE_NDIM(node) != 4 || 
        NODE_SHAPE(node)[0] != 3 || 
        NODE_SHAPE(node)[1] != 2 || 
        NODE_SHAPE(node)[2] != 2 ||
        NODE_SHAPE(node)[3] != 2){
        Node_Free(node);
        return 0;
    }

    Node_Free(node);
    return 1;
}

int test_matrix_transpose(){
    float data[6] = {1,2,3,4,5,6};
    nr_intp shape[2] = {2,3};

    Node* node = Node_New(data, 1, 2, shape, NR_FLOAT32);
    if (!node) return 0;

    if (Node_MatrixTranspose(node) != 0){
        Node_Free(node);
        return 0;
    }

    // Check dimensions swapped
    if (NODE_NDIM(node) != 2 || 
        NODE_SHAPE(node)[0] != 3 || 
        NODE_SHAPE(node)[1] != 2){
        Node_Free(node);
        return 0;
    }

    // Test non-2D matrix transpose (should fail)
    nr_intp shape3d[3] = {2,2,2};
    Node* node3d = Node_New(data, 1, 3, shape3d, NR_FLOAT32);
    if (!node3d){
        Node_Free(node);
        return 0;
    }

    if (Node_MatrixTranspose(node3d) == 0){ // Should fail for 3D
        Node_Free(node);
        Node_Free(node3d);
        return 0;
    }

    Node_Free(node);
    Node_Free(node3d);
    return 1;
}

void test_shape_test(){
    run_test("Shape Test", 11, (test_callback_t[]){
        test_reshape,
        test_reshape_3d,
        test_reshape_to_1d,
        test_squeeze,
        test_squeeze_multiple,
        test_squeeze_all_ones,
        test_transpose,
        test_transpose_3d,
        test_swap_axes,
        test_swap_axes_multi,
        test_matrix_transpose
    });
}
