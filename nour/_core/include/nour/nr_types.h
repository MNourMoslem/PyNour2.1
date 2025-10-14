#ifndef NR__CORE__INCLUDE__NR_TYPES_H
#define NR__CORE__INCLUDE__NR_TYPES_H

#include "./nr_config.h"

#include <stdlib.h>
#include <string.h>

/*Flag operation macros */
#define NR_SETFLG(x, flag) (x |= flag)    // Set flag
#define NR_RMVFLG(x, flag) (x &= ~flag)   // Remove flag
#define NR_CHKFLG(x, flag) ((x & flag) == flag)  // Check if flag is set
#define NR_CHKUNI(x, flag) ((x & flag) != 0)     // Check if any bit is set

/* Basic comparison macros */
#define NR_MAX(a, b) ((a > b) ? a : b)    // Maximum of two values
#define NR_MIN(a, b) ((a < b) ? a : b)    // Minimum of two values

/* Type size definitions */
#define NR_BOOL_SIZE 1
#define NR_INT8_SIZE 1
#define NR_UINT8_SIZE 1
#define NR_INT16_SIZE 2
#define NR_UINT16_SIZE 2
#define NR_INT32_SIZE 4
#define NR_UINT32_SIZE 4
#define NR_INT64_SIZE 8
#define NR_UINT64_SIZE 8
#define NR_FLOAT32_SIZE 4
#define NR_FLOAT64_SIZE 8

/* Basic type definitions */
typedef unsigned char nr_byte;
typedef char nr_char;
typedef unsigned char nr_uint8;
typedef char nr_int8;
typedef unsigned short nr_uint16;
typedef short nr_int16;
typedef unsigned int nr_uint32;
typedef int nr_int32;
typedef unsigned long long nr_uint64;
typedef long long nr_int64;
typedef float nr_float32;
typedef double nr_float64;

/* Common type aliases */
typedef nr_int32 nr_int;
typedef nr_float32 nr_float;
typedef nr_float64 nr_double;
typedef nr_int64 nr_long;
typedef intptr_t nr_intp;
typedef uintptr_t nr_uintp;
typedef size_t nr_size_t;

/* Type range definitions */
#define NR_MAX_INT8 127
#define NR_MIN_INT8 -128
#define NR_MAX_UINT8 255
#define NR_MIN_UINT8 0
#define NR_MAX_INT16 32767
#define NR_MIN_INT16 -32768
#define NR_MAX_UINT16 65535
#define NR_MIN_UINT16 0
#define NR_MAX_INT32 2147483647
#define NR_MIN_INT32 -2147483648
#define NR_MAX_UINT32 4294967295u
#define NR_MIN_UINT32 0
#define NR_MAX_INT64 9223372036854775807LL
#define NR_MIN_INT64 -9223372036854775808LL
#define NR_MAX_UINT64 18446744073709551615ULL
#define NR_MIN_UINT64 0

/* Boolean type definition */
typedef unsigned char nr_bool;
#define NR_FALSE 0
#define NR_TRUE 1

/* Common array initializers */
#define NR_32ONES  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}

#define NR_32NONES  {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, \
                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}

#define NR_32ZEROS {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

/* Range array initializers */
#define NR_32RANGE { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, \
                    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}

#define NR_32RANGE1 { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, \
                     17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}

/* Negative range array initializers */
#define NR_32NRANGE { 0,  -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -9, -10, -11, -12, -13, -14, -15, \
                     -16, -17, -18, -19, -20, -21, -22, -23, -24, -25, -26, -27, -28, -29, -30, -31}

#define NR_32NRANGE1 { -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -9, -10, -11, -12, -13, -14, -15, -16, \
                      -17, -18, -19, -20, -21, -22, -23, -24, -25, -26, -27, -28, -29, -30, -31, -32}

/* Array fill macro */
#define NR_32LIKE(n) {n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, \
                      n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n}


/* 64 element array initializers */
#define NR_64ONES  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
                    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}

#define NR_64NONES  {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, \
                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, \
                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, \
                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}

#define NR_64ZEROS {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

#define NR_64RANGE { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, \
                    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, \
                    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, \
                    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63}

#define NR_64RANGE1 { 1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, \
                     17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, \
                     33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, \
                     49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64}

#define NR_64NRANGE {  0,  -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -9, -10, -11, -12, -13, -14, -15, \
                     -16, -17, -18, -19, -20, -21, -22, -23, -24, -25, -26, -27, -28, -29, -30, -31, \
                     -32, -33, -34, -35, -36, -37, -38, -39, -40, -41, -42, -43, -44, -45, -46, -47, \
                     -48, -49, -50, -51, -52, -53, -54, -55, -56, -57, -58, -59, -60, -61, -62, -63}

#define NR_64NRANGE1 { -1,  -2,  -3,  -4,  -5,  -6,  -7,  -8,  -9, -10, -11, -12, -13, -14, -15, -16, \
                      -17, -18, -19, -20, -21, -22, -23, -24, -25, -26, -27, -28, -29, -30, -31, -32, \
                      -33, -34, -35, -36, -37, -38, -39, -40, -41, -42, -43, -44, -45, -46, -47, -48, \
                      -49, -50, -51, -52, -53, -54, -55, -56, -57, -58, -59, -60, -61, -62, -63, -64}

#define NR_64LIKE(n) {n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, \
                      n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, \
                      n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, \
                      n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n}

/*
    Calculates total number of items in array.
    Probably should be moved to another file in the future.
*/
NR_STATIC_INLINE nr_intp
NR_NItems(int ndim, const nr_intp* shape){
    nr_intp nitems = 1;
    for (int i = 0; i < ndim; i++){
        nitems *= shape[i];
    }
    return (nr_intp)nitems;
}

#endif // NR__CORE__INCLUDE__NR_TYPES_H