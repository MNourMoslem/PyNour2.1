#ifndef NR__CORE__TESTS__MAIN_H
#define NR__CORE__TESTS__MAIN_H

#include "nour/nour.h"
#include "commen.h"
#include <stdio.h>

typedef int (*test_callback_t)();

static void run_test(const char* test_name, int num_of_tests, test_callback_t* test_cb){
    printf("Running test: %s (%d sub-tests)\n", test_name, num_of_tests);
    int total_passed = 0;
    for (int i = 0; i < num_of_tests; i++){
        int status = test_cb[i]();
        char* result = status ? "SUCCESS" : "FAILURE";
        printf("  Sub-test %d: %s\n", i+1, result);
        if (status) total_passed++;
    }
    printf("Test %s passed (%d/%d).\n", test_name, total_passed, num_of_tests);
}

void test_allocation_test();

#endif // NR__CORE__TESTS__MAIN_H