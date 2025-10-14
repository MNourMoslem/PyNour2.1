#ifndef NR__CORE__INCLUDE__NOUR__NR_ERROR_H
#define NR__CORE__INCLUDE__NOUR__NR_ERROR_H

#define NERROR_MAX_STRING_LEN 256

#include "nr_config.h"
#include <stdarg.h>
#include <stdio.h>

/* 
    Enumeration of all possible error types in PyNour.
    Each error type corresponds to a specific category of errors
    that can occur during library operation.
*/
typedef enum {
    NError_NoError,          // No error has occurred
    NError_MemoryError,      // Memory allocation or management errors
    NError_TypeError,        // Type-related errors (invalid or mismatched types)
    NError_IndexError,       // Index out of bounds errors
    NError_ValueError,       // Invalid value errors
    NError_IOError,          // Input/output operation errors
    NError_ZeroDivisionError,// Division by zero errors
    NError_ImportError,      // Module or component import errors
    NError_AttributeError,   // Attribute access or modification errors
    NError_KeyError,         // Key-related errors in mapping operations
    NError_AssertionError,   // Assertion failure errors
    NError_RuntimeError,     // General runtime errors
    NError_OverflowError     // Numeric overflow errors
}NError_Type;

/*
    Structure containing error information.
    Stores both the type of error and additional context information
    as a formatted string.
*/
typedef struct
{
    NError_Type type;                            // Type of the error
    char context[NERROR_MAX_STRING_LEN];         // Error message and context
}NError;

/*
    Global error state variable.
    Thread-local storage ensures thread safety.
    Initialized to no error state.
*/
extern NError __NR_NERROR_GLOBAL_ERROR_VAR__;

#endif // NR__CORE__INCLUDE__NOUR__NR_ERROR_H