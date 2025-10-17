#include "main.h"
#include "../src/node_core.h"
#include "../src/tc_methods.h"
#include "../src/free.h"

// Macro to generate type conversion test
// Tests converting from in_type to out_type
#define TC_TEST(in_type, out_type) \
    tc_test_##in_type##_to_##out_type()

// Helper macro to define a specific type conversion test function
#define DEFINE_TC_TEST(in_type, out_type, in_dtype, out_dtype, in_ctype, out_ctype) \
static int tc_test_##in_type##_to_##out_type() { \
    in_ctype data[4] = {0, 1, 2, 3}; \
    nr_intp shape[1] = {4}; \
    Node* src = Node_New(data, 1, 1, shape, in_dtype); \
    if (!src) return 0; \
    \
    Node* dst = Node_ToType(NULL, src, out_dtype); \
    if (!dst) { Node_Free(src); return 0; } \
    \
    if (NODE_DTYPE(dst) != out_dtype) { \
        Node_Free(dst); Node_Free(src); return 0; \
    } \
    \
    if (NODE_SHAPE(dst)[0] != 4) { \
        Node_Free(dst); Node_Free(src); return 0; \
    } \
    \
    out_ctype* result = (out_ctype*)NODE_DATA(dst); \
    if (result[0] != (out_ctype)0 || result[1] != (out_ctype)1 || \
        result[2] != (out_ctype)2 || result[3] != (out_ctype)3) { \
        Node_Free(dst); Node_Free(src); return 0; \
    } \
    \
    Node_Free(dst); \
    Node_Free(src); \
    return 1; \
}

// Define all type conversion tests
// bool conversions
DEFINE_TC_TEST(bool, bool, NR_BOOL, NR_BOOL, nr_bool, nr_bool)
DEFINE_TC_TEST(bool, int8, NR_BOOL, NR_INT8, nr_bool, nr_int8)
DEFINE_TC_TEST(bool, uint8, NR_BOOL, NR_UINT8, nr_bool, nr_uint8)
DEFINE_TC_TEST(bool, int16, NR_BOOL, NR_INT16, nr_bool, nr_int16)
DEFINE_TC_TEST(bool, uint16, NR_BOOL, NR_UINT16, nr_bool, nr_uint16)
DEFINE_TC_TEST(bool, int32, NR_BOOL, NR_INT32, nr_bool, nr_int32)
DEFINE_TC_TEST(bool, uint32, NR_BOOL, NR_UINT32, nr_bool, nr_uint32)
DEFINE_TC_TEST(bool, int64, NR_BOOL, NR_INT64, nr_bool, nr_int64)
DEFINE_TC_TEST(bool, uint64, NR_BOOL, NR_UINT64, nr_bool, nr_uint64)
DEFINE_TC_TEST(bool, float32, NR_BOOL, NR_FLOAT32, nr_bool, nr_float32)
DEFINE_TC_TEST(bool, float64, NR_BOOL, NR_FLOAT64, nr_bool, nr_float64)

// int8 conversions
DEFINE_TC_TEST(int8, bool, NR_INT8, NR_BOOL, nr_int8, nr_bool)
DEFINE_TC_TEST(int8, int8, NR_INT8, NR_INT8, nr_int8, nr_int8)
DEFINE_TC_TEST(int8, uint8, NR_INT8, NR_UINT8, nr_int8, nr_uint8)
DEFINE_TC_TEST(int8, int16, NR_INT8, NR_INT16, nr_int8, nr_int16)
DEFINE_TC_TEST(int8, uint16, NR_INT8, NR_UINT16, nr_int8, nr_uint16)
DEFINE_TC_TEST(int8, int32, NR_INT8, NR_INT32, nr_int8, nr_int32)
DEFINE_TC_TEST(int8, uint32, NR_INT8, NR_UINT32, nr_int8, nr_uint32)
DEFINE_TC_TEST(int8, int64, NR_INT8, NR_INT64, nr_int8, nr_int64)
DEFINE_TC_TEST(int8, uint64, NR_INT8, NR_UINT64, nr_int8, nr_uint64)
DEFINE_TC_TEST(int8, float32, NR_INT8, NR_FLOAT32, nr_int8, nr_float32)
DEFINE_TC_TEST(int8, float64, NR_INT8, NR_FLOAT64, nr_int8, nr_float64)

// uint8 conversions
DEFINE_TC_TEST(uint8, bool, NR_UINT8, NR_BOOL, nr_uint8, nr_bool)
DEFINE_TC_TEST(uint8, int8, NR_UINT8, NR_INT8, nr_uint8, nr_int8)
DEFINE_TC_TEST(uint8, uint8, NR_UINT8, NR_UINT8, nr_uint8, nr_uint8)
DEFINE_TC_TEST(uint8, int16, NR_UINT8, NR_INT16, nr_uint8, nr_int16)
DEFINE_TC_TEST(uint8, uint16, NR_UINT8, NR_UINT16, nr_uint8, nr_uint16)
DEFINE_TC_TEST(uint8, int32, NR_UINT8, NR_INT32, nr_uint8, nr_int32)
DEFINE_TC_TEST(uint8, uint32, NR_UINT8, NR_UINT32, nr_uint8, nr_uint32)
DEFINE_TC_TEST(uint8, int64, NR_UINT8, NR_INT64, nr_uint8, nr_int64)
DEFINE_TC_TEST(uint8, uint64, NR_UINT8, NR_UINT64, nr_uint8, nr_uint64)
DEFINE_TC_TEST(uint8, float32, NR_UINT8, NR_FLOAT32, nr_uint8, nr_float32)
DEFINE_TC_TEST(uint8, float64, NR_UINT8, NR_FLOAT64, nr_uint8, nr_float64)

// int16 conversions
DEFINE_TC_TEST(int16, bool, NR_INT16, NR_BOOL, nr_int16, nr_bool)
DEFINE_TC_TEST(int16, int8, NR_INT16, NR_INT8, nr_int16, nr_int8)
DEFINE_TC_TEST(int16, uint8, NR_INT16, NR_UINT8, nr_int16, nr_uint8)
DEFINE_TC_TEST(int16, int16, NR_INT16, NR_INT16, nr_int16, nr_int16)
DEFINE_TC_TEST(int16, uint16, NR_INT16, NR_UINT16, nr_int16, nr_uint16)
DEFINE_TC_TEST(int16, int32, NR_INT16, NR_INT32, nr_int16, nr_int32)
DEFINE_TC_TEST(int16, uint32, NR_INT16, NR_UINT32, nr_int16, nr_uint32)
DEFINE_TC_TEST(int16, int64, NR_INT16, NR_INT64, nr_int16, nr_int64)
DEFINE_TC_TEST(int16, uint64, NR_INT16, NR_UINT64, nr_int16, nr_uint64)
DEFINE_TC_TEST(int16, float32, NR_INT16, NR_FLOAT32, nr_int16, nr_float32)
DEFINE_TC_TEST(int16, float64, NR_INT16, NR_FLOAT64, nr_int16, nr_float64)

// uint16 conversions
DEFINE_TC_TEST(uint16, bool, NR_UINT16, NR_BOOL, nr_uint16, nr_bool)
DEFINE_TC_TEST(uint16, int8, NR_UINT16, NR_INT8, nr_uint16, nr_int8)
DEFINE_TC_TEST(uint16, uint8, NR_UINT16, NR_UINT8, nr_uint16, nr_uint8)
DEFINE_TC_TEST(uint16, int16, NR_UINT16, NR_INT16, nr_uint16, nr_int16)
DEFINE_TC_TEST(uint16, uint16, NR_UINT16, NR_UINT16, nr_uint16, nr_uint16)
DEFINE_TC_TEST(uint16, int32, NR_UINT16, NR_INT32, nr_uint16, nr_int32)
DEFINE_TC_TEST(uint16, uint32, NR_UINT16, NR_UINT32, nr_uint16, nr_uint32)
DEFINE_TC_TEST(uint16, int64, NR_UINT16, NR_INT64, nr_uint16, nr_int64)
DEFINE_TC_TEST(uint16, uint64, NR_UINT16, NR_UINT64, nr_uint16, nr_uint64)
DEFINE_TC_TEST(uint16, float32, NR_UINT16, NR_FLOAT32, nr_uint16, nr_float32)
DEFINE_TC_TEST(uint16, float64, NR_UINT16, NR_FLOAT64, nr_uint16, nr_float64)

// int32 conversions
DEFINE_TC_TEST(int32, bool, NR_INT32, NR_BOOL, nr_int32, nr_bool)
DEFINE_TC_TEST(int32, int8, NR_INT32, NR_INT8, nr_int32, nr_int8)
DEFINE_TC_TEST(int32, uint8, NR_INT32, NR_UINT8, nr_int32, nr_uint8)
DEFINE_TC_TEST(int32, int16, NR_INT32, NR_INT16, nr_int32, nr_int16)
DEFINE_TC_TEST(int32, uint16, NR_INT32, NR_UINT16, nr_int32, nr_uint16)
DEFINE_TC_TEST(int32, int32, NR_INT32, NR_INT32, nr_int32, nr_int32)
DEFINE_TC_TEST(int32, uint32, NR_INT32, NR_UINT32, nr_int32, nr_uint32)
DEFINE_TC_TEST(int32, int64, NR_INT32, NR_INT64, nr_int32, nr_int64)
DEFINE_TC_TEST(int32, uint64, NR_INT32, NR_UINT64, nr_int32, nr_uint64)
DEFINE_TC_TEST(int32, float32, NR_INT32, NR_FLOAT32, nr_int32, nr_float32)
DEFINE_TC_TEST(int32, float64, NR_INT32, NR_FLOAT64, nr_int32, nr_float64)

// uint32 conversions
DEFINE_TC_TEST(uint32, bool, NR_UINT32, NR_BOOL, nr_uint32, nr_bool)
DEFINE_TC_TEST(uint32, int8, NR_UINT32, NR_INT8, nr_uint32, nr_int8)
DEFINE_TC_TEST(uint32, uint8, NR_UINT32, NR_UINT8, nr_uint32, nr_uint8)
DEFINE_TC_TEST(uint32, int16, NR_UINT32, NR_INT16, nr_uint32, nr_int16)
DEFINE_TC_TEST(uint32, uint16, NR_UINT32, NR_UINT16, nr_uint32, nr_uint16)
DEFINE_TC_TEST(uint32, int32, NR_UINT32, NR_INT32, nr_uint32, nr_int32)
DEFINE_TC_TEST(uint32, uint32, NR_UINT32, NR_UINT32, nr_uint32, nr_uint32)
DEFINE_TC_TEST(uint32, int64, NR_UINT32, NR_INT64, nr_uint32, nr_int64)
DEFINE_TC_TEST(uint32, uint64, NR_UINT32, NR_UINT64, nr_uint32, nr_uint64)
DEFINE_TC_TEST(uint32, float32, NR_UINT32, NR_FLOAT32, nr_uint32, nr_float32)
DEFINE_TC_TEST(uint32, float64, NR_UINT32, NR_FLOAT64, nr_uint32, nr_float64)

// int64 conversions
DEFINE_TC_TEST(int64, bool, NR_INT64, NR_BOOL, nr_int64, nr_bool)
DEFINE_TC_TEST(int64, int8, NR_INT64, NR_INT8, nr_int64, nr_int8)
DEFINE_TC_TEST(int64, uint8, NR_INT64, NR_UINT8, nr_int64, nr_uint8)
DEFINE_TC_TEST(int64, int16, NR_INT64, NR_INT16, nr_int64, nr_int16)
DEFINE_TC_TEST(int64, uint16, NR_INT64, NR_UINT16, nr_int64, nr_uint16)
DEFINE_TC_TEST(int64, int32, NR_INT64, NR_INT32, nr_int64, nr_int32)
DEFINE_TC_TEST(int64, uint32, NR_INT64, NR_UINT32, nr_int64, nr_uint32)
DEFINE_TC_TEST(int64, int64, NR_INT64, NR_INT64, nr_int64, nr_int64)
DEFINE_TC_TEST(int64, uint64, NR_INT64, NR_UINT64, nr_int64, nr_uint64)
DEFINE_TC_TEST(int64, float32, NR_INT64, NR_FLOAT32, nr_int64, nr_float32)
DEFINE_TC_TEST(int64, float64, NR_INT64, NR_FLOAT64, nr_int64, nr_float64)

// uint64 conversions
DEFINE_TC_TEST(uint64, bool, NR_UINT64, NR_BOOL, nr_uint64, nr_bool)
DEFINE_TC_TEST(uint64, int8, NR_UINT64, NR_INT8, nr_uint64, nr_int8)
DEFINE_TC_TEST(uint64, uint8, NR_UINT64, NR_UINT8, nr_uint64, nr_uint8)
DEFINE_TC_TEST(uint64, int16, NR_UINT64, NR_INT16, nr_uint64, nr_int16)
DEFINE_TC_TEST(uint64, uint16, NR_UINT64, NR_UINT16, nr_uint64, nr_uint16)
DEFINE_TC_TEST(uint64, int32, NR_UINT64, NR_INT32, nr_uint64, nr_int32)
DEFINE_TC_TEST(uint64, uint32, NR_UINT64, NR_UINT32, nr_uint64, nr_uint32)
DEFINE_TC_TEST(uint64, int64, NR_UINT64, NR_INT64, nr_uint64, nr_int64)
DEFINE_TC_TEST(uint64, uint64, NR_UINT64, NR_UINT64, nr_uint64, nr_uint64)
DEFINE_TC_TEST(uint64, float32, NR_UINT64, NR_FLOAT32, nr_uint64, nr_float32)
DEFINE_TC_TEST(uint64, float64, NR_UINT64, NR_FLOAT64, nr_uint64, nr_float64)

// float32 conversions
DEFINE_TC_TEST(float32, bool, NR_FLOAT32, NR_BOOL, nr_float32, nr_bool)
DEFINE_TC_TEST(float32, int8, NR_FLOAT32, NR_INT8, nr_float32, nr_int8)
DEFINE_TC_TEST(float32, uint8, NR_FLOAT32, NR_UINT8, nr_float32, nr_uint8)
DEFINE_TC_TEST(float32, int16, NR_FLOAT32, NR_INT16, nr_float32, nr_int16)
DEFINE_TC_TEST(float32, uint16, NR_FLOAT32, NR_UINT16, nr_float32, nr_uint16)
DEFINE_TC_TEST(float32, int32, NR_FLOAT32, NR_INT32, nr_float32, nr_int32)
DEFINE_TC_TEST(float32, uint32, NR_FLOAT32, NR_UINT32, nr_float32, nr_uint32)
DEFINE_TC_TEST(float32, int64, NR_FLOAT32, NR_INT64, nr_float32, nr_int64)
DEFINE_TC_TEST(float32, uint64, NR_FLOAT32, NR_UINT64, nr_float32, nr_uint64)
DEFINE_TC_TEST(float32, float32, NR_FLOAT32, NR_FLOAT32, nr_float32, nr_float32)
DEFINE_TC_TEST(float32, float64, NR_FLOAT32, NR_FLOAT64, nr_float32, nr_float64)

// float64 conversions
DEFINE_TC_TEST(float64, bool, NR_FLOAT64, NR_BOOL, nr_float64, nr_bool)
DEFINE_TC_TEST(float64, int8, NR_FLOAT64, NR_INT8, nr_float64, nr_int8)
DEFINE_TC_TEST(float64, uint8, NR_FLOAT64, NR_UINT8, nr_float64, nr_uint8)
DEFINE_TC_TEST(float64, int16, NR_FLOAT64, NR_INT16, nr_float64, nr_int16)
DEFINE_TC_TEST(float64, uint16, NR_FLOAT64, NR_UINT16, nr_float64, nr_uint16)
DEFINE_TC_TEST(float64, int32, NR_FLOAT64, NR_INT32, nr_float64, nr_int32)
DEFINE_TC_TEST(float64, uint32, NR_FLOAT64, NR_UINT32, nr_float64, nr_uint32)
DEFINE_TC_TEST(float64, int64, NR_FLOAT64, NR_INT64, nr_float64, nr_int64)
DEFINE_TC_TEST(float64, uint64, NR_FLOAT64, NR_UINT64, nr_float64, nr_uint64)
DEFINE_TC_TEST(float64, float32, NR_FLOAT64, NR_FLOAT32, nr_float64, nr_float32)
DEFINE_TC_TEST(float64, float64, NR_FLOAT64, NR_FLOAT64, nr_float64, nr_float64)

// Macro to combine all conversions to a specific output type
#define FULL_TC_TO_BOOL \
    TC_TEST(bool, bool) && \
    TC_TEST(int8, bool) && \
    TC_TEST(uint8, bool) && \
    TC_TEST(int16, bool) && \
    TC_TEST(uint16, bool) && \
    TC_TEST(int32, bool) && \
    TC_TEST(uint32, bool) && \
    TC_TEST(int64, bool) && \
    TC_TEST(uint64, bool) && \
    TC_TEST(float32, bool) && \
    TC_TEST(float64, bool)

#define FULL_TC_TO_INT8 \
    TC_TEST(bool, int8) && \
    TC_TEST(int8, int8) && \
    TC_TEST(uint8, int8) && \
    TC_TEST(int16, int8) && \
    TC_TEST(uint16, int8) && \
    TC_TEST(int32, int8) && \
    TC_TEST(uint32, int8) && \
    TC_TEST(int64, int8) && \
    TC_TEST(uint64, int8) && \
    TC_TEST(float32, int8) && \
    TC_TEST(float64, int8)

#define FULL_TC_TO_UINT8 \
    TC_TEST(bool, uint8) && \
    TC_TEST(int8, uint8) && \
    TC_TEST(uint8, uint8) && \
    TC_TEST(int16, uint8) && \
    TC_TEST(uint16, uint8) && \
    TC_TEST(int32, uint8) && \
    TC_TEST(uint32, uint8) && \
    TC_TEST(int64, uint8) && \
    TC_TEST(uint64, uint8) && \
    TC_TEST(float32, uint8) && \
    TC_TEST(float64, uint8)

#define FULL_TC_TO_INT16 \
    TC_TEST(bool, int16) && \
    TC_TEST(int8, int16) && \
    TC_TEST(uint8, int16) && \
    TC_TEST(int16, int16) && \
    TC_TEST(uint16, int16) && \
    TC_TEST(int32, int16) && \
    TC_TEST(uint32, int16) && \
    TC_TEST(int64, int16) && \
    TC_TEST(uint64, int16) && \
    TC_TEST(float32, int16) && \
    TC_TEST(float64, int16)

#define FULL_TC_TO_UINT16 \
    TC_TEST(bool, uint16) && \
    TC_TEST(int8, uint16) && \
    TC_TEST(uint8, uint16) && \
    TC_TEST(int16, uint16) && \
    TC_TEST(uint16, uint16) && \
    TC_TEST(int32, uint16) && \
    TC_TEST(uint32, uint16) && \
    TC_TEST(int64, uint16) && \
    TC_TEST(uint64, uint16) && \
    TC_TEST(float32, uint16) && \
    TC_TEST(float64, uint16)

#define FULL_TC_TO_INT32 \
    TC_TEST(bool, int32) && \
    TC_TEST(int8, int32) && \
    TC_TEST(uint8, int32) && \
    TC_TEST(int16, int32) && \
    TC_TEST(uint16, int32) && \
    TC_TEST(int32, int32) && \
    TC_TEST(uint32, int32) && \
    TC_TEST(int64, int32) && \
    TC_TEST(uint64, int32) && \
    TC_TEST(float32, int32) && \
    TC_TEST(float64, int32)

#define FULL_TC_TO_UINT32 \
    TC_TEST(bool, uint32) && \
    TC_TEST(int8, uint32) && \
    TC_TEST(uint8, uint32) && \
    TC_TEST(int16, uint32) && \
    TC_TEST(uint16, uint32) && \
    TC_TEST(int32, uint32) && \
    TC_TEST(uint32, uint32) && \
    TC_TEST(int64, uint32) && \
    TC_TEST(uint64, uint32) && \
    TC_TEST(float32, uint32) && \
    TC_TEST(float64, uint32)

#define FULL_TC_TO_INT64 \
    TC_TEST(bool, int64) && \
    TC_TEST(int8, int64) && \
    TC_TEST(uint8, int64) && \
    TC_TEST(int16, int64) && \
    TC_TEST(uint16, int64) && \
    TC_TEST(int32, int64) && \
    TC_TEST(uint32, int64) && \
    TC_TEST(int64, int64) && \
    TC_TEST(uint64, int64) && \
    TC_TEST(float32, int64) && \
    TC_TEST(float64, int64)

#define FULL_TC_TO_UINT64 \
    TC_TEST(bool, uint64) && \
    TC_TEST(int8, uint64) && \
    TC_TEST(uint8, uint64) && \
    TC_TEST(int16, uint64) && \
    TC_TEST(uint16, uint64) && \
    TC_TEST(int32, uint64) && \
    TC_TEST(uint32, uint64) && \
    TC_TEST(int64, uint64) && \
    TC_TEST(uint64, uint64) && \
    TC_TEST(float32, uint64) && \
    TC_TEST(float64, uint64)

#define FULL_TC_TO_FLOAT32 \
    TC_TEST(bool, float32) && \
    TC_TEST(int8, float32) && \
    TC_TEST(uint8, float32) && \
    TC_TEST(int16, float32) && \
    TC_TEST(uint16, float32) && \
    TC_TEST(int32, float32) && \
    TC_TEST(uint32, float32) && \
    TC_TEST(int64, float32) && \
    TC_TEST(uint64, float32) && \
    TC_TEST(float32, float32) && \
    TC_TEST(float64, float32)

#define FULL_TC_TO_FLOAT64 \
    TC_TEST(bool, float64) && \
    TC_TEST(int8, float64) && \
    TC_TEST(uint8, float64) && \
    TC_TEST(int16, float64) && \
    TC_TEST(uint16, float64) && \
    TC_TEST(int32, float64) && \
    TC_TEST(uint32, float64) && \
    TC_TEST(int64, float64) && \
    TC_TEST(uint64, float64) && \
    TC_TEST(float32, float64) && \
    TC_TEST(float64, float64)

// Test functions for each output type
int test_tc_bool(){
    return FULL_TC_TO_BOOL;
}

int test_tc_int8(){
    return FULL_TC_TO_INT8;
}

int test_tc_uint8(){
    return FULL_TC_TO_UINT8;
}

int test_tc_int16(){
    return FULL_TC_TO_INT16;
}

int test_tc_uint16(){
    return FULL_TC_TO_UINT16;
}

int test_tc_int32(){
    return FULL_TC_TO_INT32;
}

int test_tc_uint32(){
    return FULL_TC_TO_UINT32;
}

int test_tc_int64(){
    return FULL_TC_TO_INT64;
}

int test_tc_uint64(){
    return FULL_TC_TO_UINT64;
}

int test_tc_float32(){
    return FULL_TC_TO_FLOAT32;
}

int test_tc_float64(){
    return FULL_TC_TO_FLOAT64;
}

void test_tc_test(){
    run_test("Type Conversion Test", 11, (test_callback_t[]){
        test_tc_bool,
        test_tc_int8,
        test_tc_uint8,
        test_tc_int16,
        test_tc_uint16,
        test_tc_int32,
        test_tc_uint32,
        test_tc_int64,
        test_tc_uint64,
        test_tc_float32,
        test_tc_float64
    });
}
