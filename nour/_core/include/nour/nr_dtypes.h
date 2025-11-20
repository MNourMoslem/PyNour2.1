#ifndef NR__CORE__INCLUDE__NR_DTYPES_H
#define NR__CORE__INCLUDE__NR_DTYPES_H

#include "nr_types.h"

/* DType Definitions */

#define NDTYPE_NONE 0
#define NDTYPE_BOOL 1
#define NDTYPE_INT 2
#define NDTYPE_FLOAT 4

/*
    Data Type Enumeration
    --------------------
    Defines all supported numeric types in PyNour.
    Each type corresponds to a specific numeric format.
*/
typedef enum{
    NR_NONE = -1,  // Invalid or unspecified type

    NR_BOOL = 0,     // Boolean type
    NR_INT8,         // 8-bit signed integer
    NR_UINT8,        // 8-bit unsigned integer
    NR_INT16,        // 16-bit signed integer
    NR_UINT16,       // 16-bit unsigned integer
    NR_INT32,        // 32-bit signed integer
    NR_UINT32,       // 32-bit unsigned integer
    NR_INT64,        // 64-bit signed integer
    NR_UINT64,       // 64-bit unsigned integer
    NR_FLOAT32,      // 32-bit floating point
    NR_FLOAT64,      // 64-bit floating point
    NR_NUM_NUMIRC_DT // Number of numeric data types
}NR_DTYPE;

/*
    Data Type Information Structure
    -----------------------------
    Stores metadata about a specific data type.
*/
typedef struct
{
    NR_DTYPE dtype;    // The data type identifier
    nr_intp size;    // Size of the type in bytes
}NDtype;

/*
    Data Type Size Array
    ------------------
    Stores the size in bytes for each data type.
    Used for quick size lookups.
*/
static const nr_intp __NR_NDTYPE_SIZES__[] = {
    NR_BOOL_SIZE,
    NR_INT8_SIZE,
    NR_UINT8_SIZE,
    NR_INT16_SIZE,
    NR_UINT16_SIZE,
    NR_INT32_SIZE,
    NR_UINT32_SIZE,
    NR_INT64_SIZE,
    NR_UINT64_SIZE,
    NR_FLOAT32_SIZE,
    NR_FLOAT64_SIZE,
};

/*
    Returns the size in bytes for a given data type.
    
    Parameters:
        dtype: The data type to get the size for
    
    Returns:
        Size of the data type in bytes
*/
NR_HEADER nr_intp
NDtype_Size(NR_DTYPE dtype){
    return __NR_NDTYPE_SIZES__[dtype];
}


/*
    Creates a new NDtype structure for a given data type.
    
    Parameters:
        dtype: The data type to create the structure for
    
    Returns:
        An NDtype structure with the specified data type and its size
*/
NR_HEADER NDtype
NDtype_New(NR_DTYPE dtype){
    NDtype dt;
    dt.dtype = dtype;
    dt.size = NDtype_Size(dtype);
    return dt;
}

/*
    Converts a data type to its string representation.
    
    Parameters:
        dtype: The data type to convert
        dst: Destination buffer for the string
*/
NR_HEADER void
NDtype_AsString(NR_DTYPE dtype, char dst[]){
    switch (dtype)
    {
    case NR_BOOL:
        strcpy(dst, "NR_BOOL");
        break;
    case NR_INT8:
        strcpy(dst, "NR_INT8");
        break;
    case NR_UINT8:
        strcpy(dst, "NR_UINT8");
        break;
    case NR_INT16:
        strcpy(dst, "NR_INT16");
        break;
    case NR_UINT16:
        strcpy(dst, "NR_UINT16");
        break;
    case NR_INT32:
        strcpy(dst, "NR_INT32");
        break;
    case NR_UINT32:
        strcpy(dst, "NR_UINT32");
        break;
    case NR_INT64:
        strcpy(dst, "NR_INT64");
        break;
    case NR_UINT64:
        strcpy(dst, "NR_UINT64");
        break;
    case NR_FLOAT32:
        strcpy(dst, "NR_FLOAT32");
        break;
    case NR_FLOAT64:
        strcpy(dst, "NR_FLOAT64");
        break;
    default:
        strcpy(dst, "UNKNOWN");
        break;
    }
}


/*
    Gets the basic type category for a given data type.
    
    Parameters:
        dtype: The data type to check
    
    Returns:
        NDTYPE_BOOL for boolean types
        NDTYPE_INT for integer types
        NDTYPE_FLOAT for floating-point types
        -1 for unknown types
*/
NR_HEADER int
NDtype_GetDtypeType(NR_DTYPE dtype){
    switch (dtype)
    {
    case NR_BOOL:
        return NDTYPE_BOOL;
    case NR_INT8:
    case NR_UINT8:
    case NR_INT16:
    case NR_UINT16:
    case NR_INT32:
    case NR_UINT32:
    case NR_INT64:
    case NR_UINT64:
        return NDTYPE_INT;
    case NR_FLOAT32:
    case NR_FLOAT64:
        return NDTYPE_FLOAT;
    default:
        return NDTYPE_NONE;
    }
}


/*
    Converts a data type to its variable type string representation.
    
    Parameters:
        dtype: The data type to convert
        dst: Destination buffer for the string
*/
NR_HEADER void
NDtype_AsStringVarType(NR_DTYPE dtype, char dst[]){
    switch (dtype)
    {
    case NR_BOOL:
        strcpy(dst, "nr_bool");
        break;
    case NR_INT8:
        strcpy(dst, "nr_int8");
        break;
    case NR_UINT8:
        strcpy(dst, "nr_uint8");
        break;
    case NR_INT16:
        strcpy(dst, "nr_int16");
        break;
    case NR_UINT16:
        strcpy(dst, "nr_uint16");
        break;
    case NR_INT32:
        strcpy(dst, "nr_int32");
        break;
    case NR_UINT32:
        strcpy(dst, "nr_uint32");
        break;
    case NR_INT64:
        strcpy(dst, "nr_int64");
        break;
    case NR_UINT64:
        strcpy(dst, "nr_uint64");
        break;
    case NR_FLOAT32:
        strcpy(dst, "nr_float32");
        break;
    case NR_FLOAT64:
        strcpy(dst, "nr_float64");
        break;
    default:
        strcpy(dst, "UNKNOWN");
        break;
    }
}

/*
    Converts a data type to its basic type string representation.
    
    Parameters:
        dtype: The data type to convert
        dst: Destination buffer for the string
*/
NR_HEADER void
NDtype_AsStringOnlyType(NR_DTYPE dtype, char dst[]){
    switch (dtype)
    {
    case NR_BOOL:
        strcpy(dst, "bool");
        break;
    case NR_INT8:
        strcpy(dst, "int8");
        break;
    case NR_UINT8:
        strcpy(dst, "uint8");
        break;
    case NR_INT16:
        strcpy(dst, "int16");
        break;
    case NR_UINT16:
        strcpy(dst, "uint16");
        break;
    case NR_INT32:
        strcpy(dst, "int32");
        break;
    case NR_UINT32:
        strcpy(dst, "uint32");
        break;
    case NR_INT64:
        strcpy(dst, "int64");
        break;
    case NR_UINT64:
        strcpy(dst, "uint64");
        break;
    case NR_FLOAT32:
        strcpy(dst, "float32");
        break;
    case NR_FLOAT64:
        strcpy(dst, "float64");
        break;
    default:
        strcpy(dst, "UNKNOWN");
        break;
    }
}

#define Node_IsBool(node) (NODE_DTYPE(node) == NR_BOOL)
#define Node_IsInt(node) (NDtype_GetDtypeType(NODE_DTYPE(node)) == NDTYPE_INT)
#define Node_IsFloat(node) (NDtype_GetDtypeType(NODE_DTYPE(node)) == NDTYPE_FLOAT)

#define NDtype_IsValid(dtype) ((dtype) >= NR_BOOL && (dtype) < NR_NUM_NUMIRC_DT)

#endif // NR__CORE__INCLUDE__NR_DTYPES_H