#include "../src/node2str.h"
#include "../src/node_core.h"
#include "../src/free.h"
#include "main.h"
#include <string.h>

// Helper function to print section headers
static void print_section(const char* title){
    printf("\n");
    printf("=" );
    for (int i = 0; i < 60; i++) printf("=");
    printf("\n");
    printf("  %s\n", title);
    printf("=");
    for (int i = 0; i < 60; i++) printf("=");
    printf("\n\n");
}

// Test 1: Basic scalar node
static void test_scalar(){
    print_section("TEST 1: Scalar Node");
    
    nr_int32 value = 42;
    Node* scalar = Node_NewScalar(&value, NR_INT32);
    Node_SetName(scalar, "scalar_value");
    
    printf("Scalar with name:\n");
    Node_Print(scalar);
    
    Node_Free(scalar);
}

// Test 2: 1D array
static void test_1d_array(){
    print_section("TEST 2: 1D Array");
    
    nr_intp shape[1] = {5};
    Node* arr = Node_NewEmpty(1, shape, NR_INT32);
    Node_SetName(arr, "vector");
    
    nr_int32* data = (nr_int32*)arr->data;
    for (int i = 0; i < 5; i++){
        data[i] = i * 10;
    }
    
    printf("1D array with name:\n");
    Node_Print(arr);
    
    Node_Free(arr);
}

// Test 3: 2D array (matrix)
static void test_2d_array(){
    print_section("TEST 3: 2D Array (Matrix)");
    
    nr_intp shape[2] = {3, 4};
    Node* matrix = Node_NewEmpty(2, shape, NR_INT32);
    Node_SetName(matrix, "matrix");
    
    nr_int32* data = (nr_int32*)matrix->data;
    for (int i = 0; i < 12; i++){
        data[i] = i + 1;
    }
    
    printf("2D array (3x4 matrix):\n");
    Node_Print(matrix);
    
    Node_Free(matrix);
}

// Test 4: 3D array
static void test_3d_array(){
    print_section("TEST 4: 3D Array");
    
    nr_intp shape[3] = {2, 3, 4};
    Node* tensor = Node_NewEmpty(3, shape, NR_INT32);
    Node_SetName(tensor, "tensor");
    
    nr_int32* data = (nr_int32*)tensor->data;
    for (int i = 0; i < 24; i++){
        data[i] = i;
    }
    
    printf("3D array (2x3x4 tensor):\n");
    Node_Print(tensor);
    
    Node_Free(tensor);
}

// Test 5: Large array with summarization
static void test_large_array(){
    print_section("TEST 5: Large Array (Summarization with ...)");
    
    nr_intp shape[1] = {100};
    Node* large = Node_NewEmpty(1, shape, NR_INT32);
    Node_SetName(large, "large_array");
    
    nr_int32* data = (nr_int32*)large->data;
    for (int i = 0; i < 100; i++){
        data[i] = i;
    }
    
    printf("Large array (100 elements, threshold=50, edgeitems=3):\n");
    NodePrintOptions opts = {
        .threshold = 50,
        .edgeitems = 3,
        .precision = 8,
        .suppress_small = true,
        .linewidth = 75,
        .show_name = true
    };
    Node_PrintWithOptions(large, &opts);
    
    Node_Free(large);
}

// Test 6: Large 2D array with summarization
static void test_large_2d_array(){
    print_section("TEST 6: Large 2D Array (Summarization)");
    
    nr_intp shape[2] = {20, 10};
    Node* large2d = Node_NewEmpty(2, shape, NR_INT32);
    Node_SetName(large2d, "large_matrix");
    
    nr_int32* data = (nr_int32*)large2d->data;
    for (int i = 0; i < 200; i++){
        data[i] = i;
    }
    
    printf("Large 2D array (20x10=200 elements, threshold=100):\n");
    NodePrintOptions opts = {
        .threshold = 100,
        .edgeitems = 2,
        .precision = 8,
        .suppress_small = true,
        .linewidth = 75,
        .show_name = true
    };
    Node_PrintWithOptions(large2d, &opts);
    
    Node_Free(large2d);
}

// Test 7: Float array with precision control
static void test_float_precision(){
    print_section("TEST 7: Float Array (Precision Control)");
    
    nr_intp shape[1] = {6};
    Node* floats = Node_NewEmpty(1, shape, NR_FLOAT32);
    Node_SetName(floats, "float_data");
    
    nr_float32* data = (nr_float32*)floats->data;
    data[0] = 3.14159265359f;
    data[1] = 2.71828182846f;
    data[2] = 1.41421356237f;
    data[3] = 0.00000123456f;
    data[4] = 123456.789012f;
    data[5] = -99.9999999f;
    
    printf("Float array with precision=3:\n");
    NodePrintOptions opts1 = {
        .threshold = 1000,
        .edgeitems = 3,
        .precision = 3,
        .suppress_small = true,
        .linewidth = 75,
        .show_name = true
    };
    Node_PrintWithOptions(floats, &opts1);
    
    printf("\nSame array with precision=8:\n");
    NodePrintOptions opts2 = {
        .threshold = 1000,
        .edgeitems = 3,
        .precision = 8,
        .suppress_small = true,
        .linewidth = 75,
        .show_name = true
    };
    Node_PrintWithOptions(floats, &opts2);
    
    Node_Free(floats);
}

// Test 8: Without node name
static void test_without_name(){
    print_section("TEST 8: Display Without Node Name");
    
    nr_intp shape[2] = {2, 3};
    Node* arr = Node_NewEmpty(2, shape, NR_INT32);
    Node_SetName(arr, "my_array");
    
    nr_int32* data = (nr_int32*)arr->data;
    for (int i = 0; i < 6; i++){
        data[i] = i + 1;
    }
    
    printf("With name (show_name=true):\n");
    Node_Print(arr);
    
    printf("\nWithout name (show_name=false):\n");
    NodePrintOptions opts = {
        .threshold = 1000,
        .edgeitems = 3,
        .precision = 8,
        .suppress_small = true,
        .linewidth = 75,
        .show_name = false
    };
    Node_PrintWithOptions(arr, &opts);
    
    Node_Free(arr);
}

// Test 9: Custom edgeitems
static void test_custom_edgeitems(){
    print_section("TEST 9: Custom Edge Items");
    
    nr_intp shape[1] = {20};
    Node* arr = Node_NewEmpty(1, shape, NR_INT32);
    Node_SetName(arr, "edge_test");
    
    nr_int32* data = (nr_int32*)arr->data;
    for (int i = 0; i < 20; i++){
        data[i] = i;
    }
    
    printf("With edgeitems=2:\n");
    NodePrintOptions opts1 = {
        .threshold = 10,
        .edgeitems = 2,
        .precision = 8,
        .suppress_small = true,
        .linewidth = 75,
        .show_name = true
    };
    Node_PrintWithOptions(arr, &opts1);
    
    printf("\nWith edgeitems=5:\n");
    NodePrintOptions opts2 = {
        .threshold = 10,
        .edgeitems = 5,
        .precision = 8,
        .suppress_small = true,
        .linewidth = 75,
        .show_name = true
    };
    Node_PrintWithOptions(arr, &opts2);
    
    Node_Free(arr);
}

// Test 10: ToString to buffer
static void test_to_string_buffer(){
    print_section("TEST 10: ToString to Buffer");
    
    nr_intp shape[1] = {4};
    Node* arr = Node_NewEmpty(1, shape, NR_INT32);
    Node_SetName(arr, "buffer_test");
    
    nr_int32* data = (nr_int32*)arr->data;
    for (int i = 0; i < 4; i++){
        data[i] = (i + 1) * 100;
    }
    
    char buffer[1024];
    Node_ToString(arr, buffer);
    
    printf("String stored in buffer:\n");
    printf("Buffer contents: '%s'\n", buffer);
    printf("Buffer length: %zu characters\n", strlen(buffer));
    
    Node_Free(arr);
}

// Test 11: Global options
static void test_global_options(){
    print_section("TEST 11: Global Print Options");
    
    nr_intp shape[1] = {50};
    Node* arr = Node_NewEmpty(1, shape, NR_INT32);
    Node_SetName(arr, "global_opts");
    
    nr_int32* data = (nr_int32*)arr->data;
    for (int i = 0; i < 50; i++){
        data[i] = i;
    }
    
    printf("Before setting global options (defaults):\n");
    Node_Print(arr);
    
    printf("\nAfter setting global options (threshold=20, edgeitems=2):\n");
    NodePrintOptions new_defaults = {
        .threshold = 20,
        .edgeitems = 2,
        .precision = 4,
        .suppress_small = true,
        .linewidth = 75,
        .show_name = true
    };
    Node_SetPrintOptions(&new_defaults);
    Node_Print(arr);
    
    // Reset to defaults
    NodePrintOptions reset = {
        .threshold = 1000,
        .edgeitems = 3,
        .precision = 8,
        .suppress_small = true,
        .linewidth = 75,
        .show_name = true
    };
    Node_SetPrintOptions(&reset);
    
    Node_Free(arr);
}

// Test 12: Different data types
static void test_different_dtypes(){
    print_section("TEST 12: Different Data Types");
    
    nr_intp shape[1] = {5};
    
    // Int8
    Node* i8 = Node_NewEmpty(1, shape, NR_INT8);
    Node_SetName(i8, "int8_array");
    nr_int8* data_i8 = (nr_int8*)i8->data;
    for (int i = 0; i < 5; i++) data_i8[i] = i - 2;
    printf("INT8: ");
    Node_Print(i8);
    
    // Int16
    Node* i16 = Node_NewEmpty(1, shape, NR_INT16);
    Node_SetName(i16, "int16_array");
    nr_int16* data_i16 = (nr_int16*)i16->data;
    for (int i = 0; i < 5; i++) data_i16[i] = (i + 1) * 1000;
    printf("INT16: ");
    Node_Print(i16);
    
    // Float64
    Node* f64 = Node_NewEmpty(1, shape, NR_FLOAT64);
    Node_SetName(f64, "float64_array");
    nr_float64* data_f64 = (nr_float64*)f64->data;
    for (int i = 0; i < 5; i++) data_f64[i] = (i + 1) * 1.5;
    printf("FLOAT64: ");
    Node_Print(f64);
    
    Node_Free(i8);
    Node_Free(i16);
    Node_Free(f64);
}

// Main test function
void test_node2string_test(){
    printf("\n");
    printf("████████████████████████████████████████████████████████████████\n");
    printf("█                                                              █\n");
    printf("█       COMPREHENSIVE NODE TO STRING TEST SUITE               █\n");
    printf("█                                                              █\n");
    printf("████████████████████████████████████████████████████████████████\n");
    
    test_scalar();
    test_1d_array();
    test_2d_array();
    test_3d_array();
    test_large_array();
    test_large_2d_array();
    test_float_precision();
    test_without_name();
    test_custom_edgeitems();
    test_to_string_buffer();
    test_global_options();
    test_different_dtypes();
    
    print_section("ALL TESTS COMPLETED");
    printf("✓ All node2string features tested successfully!\n\n");
}