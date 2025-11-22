#include "main.h"
#include <stdio.h>

#define print_separator \
    printf("===============================\n");

#define print_empty_line \
    printf("\n");

void
print_title(const char* title) {
    print_separator;
    printf("=== %s ===\n", title);
    print_separator;
}

void print_result(int num, int result) {
    printf("Test %d: %s\n", num, result ? "PASSED" : "FAILED");
}

void run_all_tests(TestFunc* tests, const char* test_title, int num_tests) {
    print_empty_line;
    print_title(test_title);

    int passed = 0;
    for (int i = 0; i < num_tests; i++) {
        NError_Clear();
        int result = tests[i]();
        print_result(i + 1, result);
        if (!result){
            NError_Print();
        } else {
            passed++;
        }
    }
    print_separator;
 
    printf("\n%s Tests Passed: %d/%d\n", test_title, passed, num_tests);
}

int main() {
    test_basic();
    test_index();
    test_reduce();
    test_cumulative();
    // Add calls to other test suites here as needed
    return 0;
}