/*
    PyNour Function Interface
    ========================

    Defines the function interface for PyNour operations.
    Provides structures and types for handling function
    arguments and execution.

    Key Components:
    --------------
    1. Function argument structure (NFuncArgs)
    2. Function pointer type definition (NFunc)
*/

#ifndef NOUR__CORE_INCLUDE__NOUR_NR_FUNC_H
#define NOUR__CORE_INCLUDE__NOUR_NR_FUNC_H

#include "./nr_node.h"

/* Structure for passing arguments to PyNour functions */
typedef struct
{
    Node** nodes;     // Array of input nodes
    Node* out;        // Output node
    NR_DTYPE outtype; // Output data type
    int n_nodes;      // Number of input nodes
    int dim;          // Dimension parameter
    void* extra;      // Additional parameters
}NFuncArgs;

/* Function pointer type for PyNour operations */
typedef int (*NFunc) (NFuncArgs*);

#endif