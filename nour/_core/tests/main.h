#ifndef NOUR__CORE_TESTS_MAIN_H
#define NOUR__CORE_TESTS_MAIN_H

#include "../src/cnour.h"

#define TEST_NEW_NODE_INT(dst, size, ndim, shape) \
    nr_int __data[size]; \
    for (int i = 0; i < size; i++) { \
        __data[i] = i; \
    } \
    dst = Node_New(__data, 0, ndim, shape, NR_INT32); \

#define TEST_NEW_NODE_32_INT_FLAT(dst) \
    TEST_NEW_NODE_INT(dst, 32, 1, (nr_intp[]){32})

typedef int (*TestFunc)();
void run_all_tests(TestFunc* tests, const char* test_title, int num_tests);


void test_basic();
void test_index();


#endif // NOUR__CORE_TESTS_MAIN_H