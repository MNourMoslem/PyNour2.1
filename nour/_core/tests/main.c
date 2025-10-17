#include "main.h"

int main(){
   test_allocation_test();
   test_shape_test();
   test_slice_test();
   test_multislice_test();
   test_refcount_test();
   test_boolean_mask_test();
   test_nmath_test();

   return 0;
}