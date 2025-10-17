// Exhaustive nmath tests: every dtype x every operation, using macro-driven checks
#include "nour/nour.h"
#include "../src/nmath.h"
#include "../src/node_core.h"
#include "../src/free.h"
#include "main.h"

// Data length for vector tests
#define NM_N 8

// Helper to compare floats with tolerance
static int approx_equal_double(double a, double b, double tol){
    double d = a - b;
    if (d < 0) d = -d;
    return d <= tol;
}

// Shorthand tokens mapping
#define D_BOOL    NR_BOOL
#define D_INT8    NR_INT8
#define D_UINT8   NR_UINT8
#define D_INT16   NR_INT16
#define D_UINT16  NR_UINT16
#define D_INT32   NR_INT32
#define D_UINT32  NR_UINT32
#define D_INT64   NR_INT64
#define D_UINT64  NR_UINT64
#define D_FLOAT32 NR_FLOAT32
#define D_FLOAT64 NR_FLOAT64

#define T_BOOL    nr_bool
#define T_INT8    nr_int8
#define T_UINT8   nr_uint8
#define T_INT16   nr_int16
#define T_UINT16  nr_uint16
#define T_INT32   nr_int32
#define T_UINT32  nr_uint32
#define T_INT64   nr_int64
#define T_UINT64  nr_uint64
#define T_FLOAT32 nr_float32
#define T_FLOAT64 nr_float64

// Fill input arrays with safe values (no division/mod by zero). For bool, keep b=1.
#define FILL_DATA(ctype, a_arr, b_arr) do { \
    for (int i = 0; i < NM_N; ++i){ \
        a_arr[i] = (ctype)i; \
        b_arr[i] = (ctype)(i + 1); \
    } \
} while(0)

#define FILL_DATA_BOOL(a_arr, b_arr) do { \
    for (int i = 0; i < NM_N; ++i){ \
        a_arr[i] = (nr_bool)(i & 1); /* 0,1,0,1,... */ \
        b_arr[i] = (nr_bool)1;       /* avoid div/mod by zero */ \
    } \
} while(0)

// Core test body for binary ops producing numeric outputs (Add/Sub/Mul/Div/Mod/Tdv)
#define DEFINE_NUMERIC_OP_TEST(func_name, in_tok, IN_DTYPE, IN_CTYPE, OUT_DTYPE, CHECK_EXPR, IS_FLOAT_OUT) \
static int test_##func_name##_##in_tok() { \
    IN_CTYPE a_data[NM_N]; \
    IN_CTYPE b_data[NM_N]; \
    if (IN_DTYPE == NR_BOOL){ \
        FILL_DATA_BOOL(a_data, b_data); \
    } else { \
        FILL_DATA(IN_CTYPE, a_data, b_data); \
    } \
    nr_intp shape[1] = { NM_N }; \
    Node* a = Node_New(a_data, 1, 1, shape, IN_DTYPE); \
    Node* b = Node_New(b_data, 1, 1, shape, IN_DTYPE); \
    if (!a || !b){ if (a) Node_Free(a); if (b) Node_Free(b); return 0; } \
    Node* out = NMath_##func_name(NULL, a, b); \
    if (!out){ Node_Free(a); Node_Free(b); return 0; } \
    /* dtype check */ \
    if (NODE_DTYPE(out) != OUT_DTYPE){ Node_Free(out); Node_Free(a); Node_Free(b); return 0; } \
    /* element-wise check */ \
    int ok = 1; \
    if (IS_FLOAT_OUT){ \
        /* float32/64 outputs -> compare with tolerance */ \
        double* od; \
        if (NODE_DTYPE(out) == NR_FLOAT32){ \
            nr_float32* p = (nr_float32*)NODE_DATA(out); \
            for (int i = 0; i < NM_N; ++i){ \
                double expv = (double)(CHECK_EXPR(IN_CTYPE, a_data[i], b_data[i])); \
                if (!approx_equal_double((double)p[i], expv, 1e-6)){ ok = 0; break; } \
            } \
        } else { \
            nr_float64* p = (nr_float64*)NODE_DATA(out); \
            for (int i = 0; i < NM_N; ++i){ \
                double expv = (double)(CHECK_EXPR(IN_CTYPE, a_data[i], b_data[i])); \
                if (!approx_equal_double((double)p[i], expv, 1e-9)){ ok = 0; break; } \
            } \
        } \
    } else { \
        /* integer/bool outputs -> exact compare using out dtype */ \
        switch (NODE_DTYPE(out)) { \
            case NR_BOOL: { nr_bool* p = (nr_bool*)NODE_DATA(out); \
                for (int i = 0; i < NM_N; ++i){ nr_bool ev = (nr_bool)(CHECK_EXPR(IN_CTYPE, a_data[i], b_data[i])); if (p[i] != ev){ ok = 0; break; } } \
                break; } \
            case NR_INT8: { nr_int8* p = (nr_int8*)NODE_DATA(out); \
                for (int i = 0; i < NM_N; ++i){ nr_int8 ev = (nr_int8)(CHECK_EXPR(IN_CTYPE, a_data[i], b_data[i])); if (p[i] != ev){ ok = 0; break; } } \
                break; } \
            case NR_UINT8: { nr_uint8* p = (nr_uint8*)NODE_DATA(out); \
                for (int i = 0; i < NM_N; ++i){ nr_uint8 ev = (nr_uint8)(CHECK_EXPR(IN_CTYPE, a_data[i], b_data[i])); if (p[i] != ev){ ok = 0; break; } } \
                break; } \
            case NR_INT16: { nr_int16* p = (nr_int16*)NODE_DATA(out); \
                for (int i = 0; i < NM_N; ++i){ nr_int16 ev = (nr_int16)(CHECK_EXPR(IN_CTYPE, a_data[i], b_data[i])); if (p[i] != ev){ ok = 0; break; } } \
                break; } \
            case NR_UINT16: { nr_uint16* p = (nr_uint16*)NODE_DATA(out); \
                for (int i = 0; i < NM_N; ++i){ nr_uint16 ev = (nr_uint16)(CHECK_EXPR(IN_CTYPE, a_data[i], b_data[i])); if (p[i] != ev){ ok = 0; break; } } \
                break; } \
            case NR_INT32: { nr_int32* p = (nr_int32*)NODE_DATA(out); \
                for (int i = 0; i < NM_N; ++i){ nr_int32 ev = (nr_int32)(CHECK_EXPR(IN_CTYPE, a_data[i], b_data[i])); if (p[i] != ev){ ok = 0; break; } } \
                break; } \
            case NR_UINT32: { nr_uint32* p = (nr_uint32*)NODE_DATA(out); \
                for (int i = 0; i < NM_N; ++i){ nr_uint32 ev = (nr_uint32)(CHECK_EXPR(IN_CTYPE, a_data[i], b_data[i])); if (p[i] != ev){ ok = 0; break; } } \
                break; } \
            case NR_INT64: { nr_int64* p = (nr_int64*)NODE_DATA(out); \
                for (int i = 0; i < NM_N; ++i){ nr_int64 ev = (nr_int64)(CHECK_EXPR(IN_CTYPE, a_data[i], b_data[i])); if (p[i] != ev){ ok = 0; break; } } \
                break; } \
            case NR_UINT64: { nr_uint64* p = (nr_uint64*)NODE_DATA(out); \
                for (int i = 0; i < NM_N; ++i){ nr_uint64 ev = (nr_uint64)(CHECK_EXPR(IN_CTYPE, a_data[i], b_data[i])); if (p[i] != ev){ ok = 0; break; } } \
                break; } \
            default: ok = 0; \
        } \
    } \
    Node_Free(out); Node_Free(a); Node_Free(b); \
    return ok; \
}

// Expression helpers: use a,b with provided input ctype
#define EXPR_ADD(T, a, b) ((T)(a) + (T)(b))
#define EXPR_SUB(T, a, b) ((T)(a) - (T)(b))
#define EXPR_MUL(T, a, b) ((T)(a) * (T)(b))
// Division: compute as double to match float outputs
#define EXPR_DIV(T, a, b) (((double)(a)) / ((double)(b)))
// Mod/TrueDiv (integer division): cast inputs to signed 64 first; our data non-negative so safe for unsigned
#define EXPR_MOD(T, a, b) ((nr_int64)((nr_int64)(a) % (nr_int64)(b)))
#define EXPR_TDV(T, a, b) ((nr_int64)((nr_int64)(a) / (nr_int64)(b)))

// Comparison ops -> boolean outputs
#define DEFINE_COMPARE_OP_TEST(func_name, in_tok, IN_DTYPE, IN_CTYPE, CMP_EXPR) \
static int test_##func_name##_##in_tok() { \
    IN_CTYPE a_data[NM_N]; \
    IN_CTYPE b_data[NM_N]; \
    if (IN_DTYPE == NR_BOOL){ FILL_DATA_BOOL(a_data, b_data); } else { FILL_DATA(IN_CTYPE, a_data, b_data); } \
    nr_intp shape[1] = { NM_N }; \
    Node* a = Node_New(a_data, 1, 1, shape, IN_DTYPE); \
    Node* b = Node_New(b_data, 1, 1, shape, IN_DTYPE); \
    if (!a || !b){ if (a) Node_Free(a); if (b) Node_Free(b); return 0; } \
    Node* out = NMath_##func_name(NULL, a, b); \
    if (!out){ Node_Free(a); Node_Free(b); return 0; } \
    if (NODE_DTYPE(out) != NR_BOOL){ Node_Free(out); Node_Free(a); Node_Free(b); return 0; } \
    nr_bool* p = (nr_bool*)NODE_DATA(out); \
    for (int i = 0; i < NM_N; ++i){ \
        nr_bool expv = (nr_bool)(CMP_EXPR(a_data[i], b_data[i]) ? 1 : 0); \
        if (p[i] != expv){ Node_Free(out); Node_Free(a); Node_Free(b); return 0; } \
    } \
    Node_Free(out); Node_Free(a); Node_Free(b); \
    return 1; \
}

#define CMP_BG(a,b)  ((a) >  (b))
#define CMP_BGE(a,b) ((a) >= (b))
#define CMP_LS(a,b)  ((a) <  (b))
#define CMP_LSE(a,b) ((a) <= (b))
#define CMP_EQ(a,b)  ((a) == (b))
#define CMP_NEQ(a,b) ((a) != (b))

// Shorthand to instantiate tests for a given op across all dtypes
#define ALL_DT_NUMERIC_OP(opname, OUTTYPE_SELECTOR, EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR) \
    DEFINE_NUMERIC_OP_TEST(opname, bool,    D_BOOL,    T_BOOL,    OUTTYPE_SELECTOR(D_BOOL),    EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR(D_BOOL)) \
    DEFINE_NUMERIC_OP_TEST(opname, int8,    D_INT8,    T_INT8,    OUTTYPE_SELECTOR(D_INT8),    EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR(D_INT8)) \
    DEFINE_NUMERIC_OP_TEST(opname, uint8,   D_UINT8,   T_UINT8,   OUTTYPE_SELECTOR(D_UINT8),   EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR(D_UINT8)) \
    DEFINE_NUMERIC_OP_TEST(opname, int16,   D_INT16,   T_INT16,   OUTTYPE_SELECTOR(D_INT16),   EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR(D_INT16)) \
    DEFINE_NUMERIC_OP_TEST(opname, uint16,  D_UINT16,  T_UINT16,  OUTTYPE_SELECTOR(D_UINT16),  EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR(D_UINT16)) \
    DEFINE_NUMERIC_OP_TEST(opname, int32,   D_INT32,   T_INT32,   OUTTYPE_SELECTOR(D_INT32),   EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR(D_INT32)) \
    DEFINE_NUMERIC_OP_TEST(opname, uint32,  D_UINT32,  T_UINT32,  OUTTYPE_SELECTOR(D_UINT32),  EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR(D_UINT32)) \
    DEFINE_NUMERIC_OP_TEST(opname, int64,   D_INT64,   T_INT64,   OUTTYPE_SELECTOR(D_INT64),   EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR(D_INT64)) \
    DEFINE_NUMERIC_OP_TEST(opname, uint64,  D_UINT64,  T_UINT64,  OUTTYPE_SELECTOR(D_UINT64),  EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR(D_UINT64)) \
    DEFINE_NUMERIC_OP_TEST(opname, float32, D_FLOAT32, T_FLOAT32, OUTTYPE_SELECTOR(D_FLOAT32), EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR(D_FLOAT32)) \
    DEFINE_NUMERIC_OP_TEST(opname, float64, D_FLOAT64, T_FLOAT64, OUTTYPE_SELECTOR(D_FLOAT64), EXPR_MACRO, FLOAT_OUT_FLAG_SELECTOR(D_FLOAT64))

#define ALL_DT_COMPARE_OP(opname, CMP_MACRO) \
    DEFINE_COMPARE_OP_TEST(opname, bool,    D_BOOL,    T_BOOL,    CMP_MACRO) \
    DEFINE_COMPARE_OP_TEST(opname, int8,    D_INT8,    T_INT8,    CMP_MACRO) \
    DEFINE_COMPARE_OP_TEST(opname, uint8,   D_UINT8,   T_UINT8,   CMP_MACRO) \
    DEFINE_COMPARE_OP_TEST(opname, int16,   D_INT16,   T_INT16,   CMP_MACRO) \
    DEFINE_COMPARE_OP_TEST(opname, uint16,  D_UINT16,  T_UINT16,  CMP_MACRO) \
    DEFINE_COMPARE_OP_TEST(opname, int32,   D_INT32,   T_INT32,   CMP_MACRO) \
    DEFINE_COMPARE_OP_TEST(opname, uint32,  D_UINT32,  T_UINT32,  CMP_MACRO) \
    DEFINE_COMPARE_OP_TEST(opname, int64,   D_INT64,   T_INT64,   CMP_MACRO) \
    DEFINE_COMPARE_OP_TEST(opname, uint64,  D_UINT64,  T_UINT64,  CMP_MACRO) \
    DEFINE_COMPARE_OP_TEST(opname, float32, D_FLOAT32, T_FLOAT32, CMP_MACRO) \
    DEFINE_COMPARE_OP_TEST(opname, float64, D_FLOAT64, T_FLOAT64, CMP_MACRO)

// Outtype selectors
#define OUT_SAME(dt)             (dt)
#define OUT_FLOAT_FOR_DIV(dt)    (((dt)==NR_FLOAT32)?NR_FLOAT32:(((dt)==NR_FLOAT64)?NR_FLOAT64:NR_FLOAT64))
#define OUT_INT_FOR_INTOPS(dt)   (((dt)<=NR_UINT64)?(dt):NR_INT64)
#define OUT_BOOL(dt)             (NR_BOOL)

// Float-out flags
#define IS_FLOAT_FALSE(dt) 0
#define IS_FLOAT_TRUE(dt)  1
#define IS_FLOAT_FOR_DIV(dt) (((dt)==NR_FLOAT32)||((dt)==NR_FLOAT64))

// Instantiate tests per op across all dtypes
ALL_DT_NUMERIC_OP(Add, OUT_SAME, EXPR_ADD, IS_FLOAT_FALSE)
ALL_DT_NUMERIC_OP(Sub, OUT_SAME, EXPR_SUB, IS_FLOAT_FALSE)
ALL_DT_NUMERIC_OP(Mul, OUT_SAME, EXPR_MUL, IS_FLOAT_FALSE)
ALL_DT_NUMERIC_OP(Div, OUT_FLOAT_FOR_DIV, EXPR_DIV, IS_FLOAT_FOR_DIV)
ALL_DT_NUMERIC_OP(Mod, OUT_INT_FOR_INTOPS, EXPR_MOD, IS_FLOAT_FALSE)
ALL_DT_NUMERIC_OP(Tdv, OUT_INT_FOR_INTOPS, EXPR_TDV, IS_FLOAT_FALSE)

ALL_DT_COMPARE_OP(Bg,  CMP_BG)
ALL_DT_COMPARE_OP(Bge, CMP_BGE)
ALL_DT_COMPARE_OP(Ls,  CMP_LS)
ALL_DT_COMPARE_OP(Lse, CMP_LSE)
ALL_DT_COMPARE_OP(Eq,  CMP_EQ)
ALL_DT_COMPARE_OP(Neq, CMP_NEQ)

// Group runners: each returns conjunction of all dtype tests for that op
#define RUN_ALL_DT(opname) ( \
    test_##opname##_bool() && \
    test_##opname##_int8() && \
    test_##opname##_uint8() && \
    test_##opname##_int16() && \
    test_##opname##_uint16() && \
    test_##opname##_int32() && \
    test_##opname##_uint32() && \
    test_##opname##_int64() && \
    test_##opname##_uint64() && \
    test_##opname##_float32() && \
    test_##opname##_float64() )

static int test_op_add(){ return RUN_ALL_DT(Add); }
static int test_op_sub(){ return RUN_ALL_DT(Sub); }
static int test_op_mul(){ return RUN_ALL_DT(Mul); }
static int test_op_div(){ return RUN_ALL_DT(Div); }
static int test_op_mod(){ return RUN_ALL_DT(Mod); }
static int test_op_tdv(){ return RUN_ALL_DT(Tdv); }
static int test_op_bg(){  return RUN_ALL_DT(Bg);  }
static int test_op_bge(){ return RUN_ALL_DT(Bge); }
static int test_op_ls(){  return RUN_ALL_DT(Ls);  }
static int test_op_lse(){ return RUN_ALL_DT(Lse); }
static int test_op_eq(){  return RUN_ALL_DT(Eq);  }
static int test_op_neq(){ return RUN_ALL_DT(Neq); }

void test_nmath_test(){
    run_test("NMath Ops x DTypes", 12, (test_callback_t[]){
        test_op_add,
        test_op_sub,
        test_op_mul,
        test_op_div,
        test_op_mod,
        test_op_tdv,
        test_op_bg,
        test_op_bge,
        test_op_ls,
        test_op_lse,
        test_op_eq,
        test_op_neq
    });
}