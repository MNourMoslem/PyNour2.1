#include "main.h"

int main(){
   test_allocation_test();
   test_shape_test();
   test_index_test();
   test_multislice_test();
   test_refcount_test();
   test_boolean_mask_test();
   test_tc_test();
   test_nmath_test();
   test_node2string_test();
   test_narray_test();

   return 0;
}