#ifndef NR__CORE__TESTS__MAIN_H
#define NR__CORE__TESTS__MAIN_H

#include "nour/nour.h"
#include "commen.h"
#include "../src/nerror.h"
#include <stdio.h>
#include <string.h>

typedef int (*test_callback_t)();

static void run_test(const char* test_name, int num_of_tests, test_callback_t* test_cb){
    printf("Running test: %s (%d sub-tests)\n", test_name, num_of_tests);
    int total_passed = 0;
    for (int i = 0; i < num_of_tests; i++){
        NError_Clear();
        int status = test_cb[i]();
        int error_status = NError_IsError();
        /* Count pass/fail strictly based on the test return status. */
        char result_buf[NERROR_MAX_STRING_LEN];
        if (status) {
            snprintf(result_buf, sizeof(result_buf), "SUCCESS");
        } else {
            snprintf(result_buf, sizeof(result_buf), "FAILURE");
        }
        /* If an error occurred, append its context but don't override pass counting. */
        if (error_status) {
            size_t cur = strlen(result_buf);
            if (cur + 9 < sizeof(result_buf)) { /* room for " ERROR: " */
                strncat(result_buf, " ERROR: ", sizeof(result_buf) - cur - 1);
                cur = strlen(result_buf);
                strncat(result_buf, NERROR_CONTEXT, sizeof(result_buf) - cur - 1);
            }
        }
        printf("  Sub-test %d: %s\n", i+1, result_buf);
        if (status) total_passed++;
    }
    printf("Test %s passed (%d/%d).\n", test_name, total_passed, num_of_tests);
}

void test_allocation_test();
void test_shape_test();
void test_index_test();
void test_multislice_test();
void test_refcount_test();
void test_boolean_mask_test();
void test_tc_test();
void test_nmath_test();
void test_node2string_test();

#endif // NR__CORE__TESTS__MAIN_H