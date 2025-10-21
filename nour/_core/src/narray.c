#include "narray.h"
#include "nerror.h"
#include "node_core.h"
#include <stdlib.h>
#include <string.h>

NR_PUBLIC void
NArray_CalcStrides(int ndim, const nr_intp* shape, nr_intp itemsize, nr_intp* strides){
    if (ndim == 0) return;
    
    strides[ndim - 1] = itemsize;
    for (int i = ndim - 2; i >= 0; i--){
        strides[i] = strides[i + 1] * shape[i + 1];
    }
}

NR_PUBLIC NArray*
NArray_New(void* data, int ndim, const nr_intp* shape, const nr_intp* strides, 
           NR_DTYPE dtype, int copy_data){
    if (ndim < 0 || ndim > NR_NODE_MAX_NDIM){
        NError_RaiseError(NError_ValueError, 
            "Invalid number of dimensions: %d. Must be between 0 and %d.", 
            ndim, NR_NODE_MAX_NDIM);
        return NULL;
    }

    if (ndim > 0 && !shape){
        NError_RaiseError(NError_ValueError, "Shape cannot be NULL for ndim > 0.");
        return NULL;
    }

    NArray* arr = (NArray*)malloc(sizeof(NArray));
    if (!arr){
        NError_RaiseMemoryError();
        return NULL;
    }

    arr->ndim = ndim;
    arr->dtype = dtype;
    arr->owns_data = 0;
    arr->owns_shape = 0;
    arr->owns_strides = 0;

    // Calculate size
    arr->size = 1;
    for (int i = 0; i < ndim; i++){
        if (shape[i] < 0){
            NError_RaiseError(NError_ValueError, 
                "Shape values must be non-negative. Got %lld at dimension %d.", 
                (long long)shape[i], i);
            free(arr);
            return NULL;
        }
        arr->size *= shape[i];
    }

    // Allocate and copy shape
    if (ndim > 0){
        arr->shape = (nr_intp*)malloc(ndim * sizeof(nr_intp));
        if (!arr->shape){
            free(arr);
            NError_RaiseMemoryError();
            return NULL;
        }
        memcpy(arr->shape, shape, ndim * sizeof(nr_intp));
        arr->owns_shape = 1;
    } else {
        arr->shape = NULL;
    }

    // Handle strides
    if (ndim > 0){
        arr->strides = (nr_intp*)malloc(ndim * sizeof(nr_intp));
        if (!arr->strides){
            free(arr->shape);
            free(arr);
            NError_RaiseMemoryError();
            return NULL;
        }

        if (strides){
            memcpy(arr->strides, strides, ndim * sizeof(nr_intp));
        } else {
            // Calculate default C-order strides
            NArray_CalcStrides(ndim, shape, NDtype_Size(dtype), arr->strides);
        }
        arr->owns_strides = 1;
    } else {
        arr->strides = NULL;
    }

    // Handle data
    nr_intp data_size = arr->size * NDtype_Size(dtype);
    if (copy_data && data){
        arr->data = malloc(data_size);
        if (!arr->data){
            if (arr->strides) free(arr->strides);
            if (arr->shape) free(arr->shape);
            free(arr);
            NError_RaiseMemoryError();
            return NULL;
        }
        memcpy(arr->data, data, data_size);
        arr->owns_data = 1;
    } else if (data){
        arr->data = data;
        arr->owns_data = 0;
    } else {
        // Allocate empty data
        arr->data = malloc(data_size);
        if (!arr->data){
            if (arr->strides) free(arr->strides);
            if (arr->shape) free(arr->shape);
            free(arr);
            NError_RaiseMemoryError();
            return NULL;
        }
        arr->owns_data = 1;
    }

    return arr;
}

NR_PUBLIC NArray*
NArray_NewEmpty(int ndim, const nr_intp* shape, NR_DTYPE dtype){
    return NArray_New(NULL, ndim, shape, NULL, dtype, 0);
}

NR_PUBLIC NArray*
NArray_FromNode(const Node* node, int copy_data){
    if (!node){
        NError_RaiseError(NError_ValueError, "Node cannot be NULL.");
        return NULL;
    }

    return NArray_New(node->data, node->ndim, node->shape, node->strides, 
                     NODE_DTYPE(node), copy_data);
}

NR_PUBLIC NArray*
NArray_FromIntArray(const nr_intp* indices, nr_intp size, int copy_data){
    if (!indices){
        NError_RaiseError(NError_ValueError, "Indices cannot be NULL.");
        return NULL;
    }

    if (size < 0){
        NError_RaiseError(NError_ValueError, "Size cannot be negative.");
        return NULL;
    }

    return NArray_New((void*)indices, 1, &size, NULL, NR_INT64, copy_data);
}

NR_PUBLIC NArray*
NArray_FromBoolArray(const nr_bool* mask, nr_intp size, int copy_data){
    if (!mask){
        NError_RaiseError(NError_ValueError, "Mask cannot be NULL.");
        return NULL;
    }

    if (size < 0){
        NError_RaiseError(NError_ValueError, "Size cannot be negative.");
        return NULL;
    }

    return NArray_New((void*)mask, 1, &size, NULL, NR_BOOL, copy_data);
}

NR_PUBLIC void
NArray_Free(NArray* arr){
    if (!arr) return;

    if (arr->owns_data && arr->data){
        free(arr->data);
    }

    if (arr->owns_shape && arr->shape){
        free(arr->shape);
    }

    if (arr->owns_strides && arr->strides){
        free(arr->strides);
    }

    free(arr);
}

NR_PUBLIC Node*
NArray_ToNode(const NArray* arr, int copy_data){
    if (!arr){
        NError_RaiseError(NError_ValueError, "NArray cannot be NULL.");
        return NULL;
    }

    return Node_New(arr->data, copy_data, arr->ndim, arr->shape, arr->dtype);
}

NR_PUBLIC int
NArray_IsContiguous(const NArray* arr){
    if (!arr || arr->ndim == 0) return 1;

    nr_intp expected_stride = NDtype_Size(arr->dtype);
    for (int i = arr->ndim - 1; i >= 0; i--){
        if (arr->strides[i] != expected_stride){
            return 0;
        }
        expected_stride *= arr->shape[i];
    }

    return 1;
}

NR_PUBLIC void*
NArray_GetItem(const NArray* arr, const nr_intp* indices){
    if (!arr || !indices){
        NError_RaiseError(NError_ValueError, "NArray and indices cannot be NULL.");
        return NULL;
    }

    // Check bounds and calculate offset
    nr_intp offset = 0;
    for (int i = 0; i < arr->ndim; i++){
        nr_intp idx = indices[i];
        
        // Handle negative indices
        if (idx < 0){
            idx += arr->shape[i];
        }

        // Check bounds
        if (idx < 0 || idx >= arr->shape[i]){
            NError_RaiseError(NError_IndexError, 
                "Index %lld out of bounds for dimension %d with size %lld.",
                (long long)indices[i], i, (long long)arr->shape[i]);
            return NULL;
        }

        offset += idx * arr->strides[i];
    }

    return (char*)arr->data + offset;
}

NR_PUBLIC int
NArray_Copy(NArray* dst, const NArray* src){
    if (!dst || !src){
        NError_RaiseError(NError_ValueError, "Source and destination cannot be NULL.");
        return -1;
    }

    if (dst->ndim != src->ndim){
        NError_RaiseError(NError_ValueError, 
            "Dimension mismatch: destination has %d dims, source has %d dims.",
            dst->ndim, src->ndim);
        return -1;
    }

    for (int i = 0; i < src->ndim; i++){
        if (dst->shape[i] != src->shape[i]){
            NError_RaiseError(NError_ValueError, 
                "Shape mismatch at dimension %d: destination is %lld, source is %lld.",
                i, (long long)dst->shape[i], (long long)src->shape[i]);
            return -1;
        }
    }

    if (dst->dtype != src->dtype){
        NError_RaiseError(NError_TypeError, 
            "Data type mismatch: destination is type %d, source is type %d.",
            dst->dtype, src->dtype);
        return -1;
    }

    // If both are contiguous, do a simple memcpy
    if (NArray_IsContiguous(dst) && NArray_IsContiguous(src)){
        nr_intp data_size = src->size * NDtype_Size(src->dtype);
        memcpy(dst->data, src->data, data_size);
        return 0;
    }

    // Otherwise, copy element by element
    nr_intp itemsize = NDtype_Size(src->dtype);
    nr_intp coords[NR_NODE_MAX_NDIM] = {0};
    
    for (nr_intp i = 0; i < src->size; i++){
        void* src_ptr = NArray_GetItem(src, coords);
        void* dst_ptr = NArray_GetItem(dst, coords);
        
        if (!src_ptr || !dst_ptr){
            return -1;
        }
        
        memcpy(dst_ptr, src_ptr, itemsize);
        
        // Increment coordinates
        for (int j = src->ndim - 1; j >= 0; j--){
            coords[j]++;
            if (coords[j] < src->shape[j]){
                break;
            }
            coords[j] = 0;
        }
    }

    return 0;
}
