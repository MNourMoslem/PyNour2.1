#include "node2str.h"
#include "ntools.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

NR_PRIVATE int
_Num2Char(void* num_ptr, NR_DTYPE dtype, char* buffer){
    switch (dtype){
        case NR_INT8:
            sprintf(buffer, "%d", *(nr_int8*)num_ptr);
            break;
        case NR_INT16:
            sprintf(buffer, "%d", *(nr_int16*)num_ptr);
            break;
        case NR_INT32:
            sprintf(buffer, "%d", *(nr_int32*)num_ptr);
            break;
        case NR_INT64:
            sprintf(buffer, "%lld", *(nr_int64*)num_ptr);
            break;
        case NR_UINT8:
            sprintf(buffer, "%u", *(nr_uint8*)num_ptr);
            break;
        case NR_UINT16:
            sprintf(buffer, "%u", *(nr_uint16*)num_ptr);
            break;
        case NR_UINT32:
            sprintf(buffer, "%u", *(nr_uint32*)num_ptr);
            break;
        case NR_UINT64:
            sprintf(buffer, "%llu", *(nr_uint64*)num_ptr);
            break;
        case NR_FLOAT32:
            sprintf(buffer, "%.6f", *(nr_float32*)num_ptr);
            break;
        case NR_FLOAT64:
            sprintf(buffer, "%.6lf", *(nr_float64*)num_ptr);
            break;
        case NR_BOOL:
            sprintf(buffer, "%s", (*(nr_uint8*)num_ptr) ? "True" : "False");
            break;
        default:
            strcpy(buffer, "Unknown Type");
            return -1;
    }

    return 0;
}



// node2str.h additions:
// Add this to your node2str.h header file:
/*
typedef struct {
    nr_intp threshold;      // Total elements before summarizing (default: 1000)
    int edgeitems;         // Items to show at start/end when summarizing (default: 3)
    int precision;         // Decimal precision for floats (default: 8)
    bool suppress_small;   // Suppress small float values (default: true)
    int linewidth;         // Max line width before wrapping (default: 75)
    bool show_name;        // Show node name (default: true)
} NodePrintOptions;

NR_PUBLIC void Node_SetPrintOptions(NodePrintOptions* opts);
NR_PUBLIC NodePrintOptions* Node_GetPrintOptions(void);
NR_PUBLIC void Node_ToStringWithOptions(Node* node, char* buffer, NodePrintOptions* opts);
NR_PUBLIC void Node_PrintWithOptions(Node* node, NodePrintOptions* opts);
*/

// Default print options (can be modified)
static NodePrintOptions _default_options = {
    .threshold = 1000,      // Total elements before summarizing
    .edgeitems = 3,         // Items to show at start/end when summarizing
    .precision = 8,         // Decimal precision for floats
    .suppress_small = true, // Suppress small float values
    .linewidth = 75,        // Max line width before wrapping
    .show_name = true       // Show node name
};

NR_PUBLIC void
Node_SetPrintOptions(NodePrintOptions* opts){
    if (opts->threshold > 0) _default_options.threshold = opts->threshold;
    if (opts->edgeitems > 0) _default_options.edgeitems = opts->edgeitems;
    if (opts->precision > 0) _default_options.precision = opts->precision;
    _default_options.suppress_small = opts->suppress_small;
    if (opts->linewidth > 0) _default_options.linewidth = opts->linewidth;
    _default_options.show_name = opts->show_name;
}

NR_PUBLIC NodePrintOptions*
Node_GetPrintOptions(void){
    return &_default_options;
}

NR_PRIVATE void
_Append(char* buffer, const char* s){
    strcat(buffer, s);
}

NR_PRIVATE void
_AppendNum(char* buffer, void* dataptr, NR_DTYPE dtype, int precision){
    char num[64];
    char format[16];
    
    // Use precision for floating point types
    switch(dtype){
        case NR_FLOAT32:
        case NR_FLOAT64:
            snprintf(format, sizeof(format), "%%.%dg", precision);
            break;
        default:
            format[0] = '\0';
            break;
    }
    
    if (_Num2Char(dataptr, dtype, num) == 0){
        strcat(buffer, num);
    } else {
        strcat(buffer, "<err>");
    }
}

NR_PRIVATE void
_AppendIndent(char* buffer, int level){
    for (int i = 0; i < level; ++i){
        strcat(buffer, " ");
    }
}

NR_PRIVATE nr_intp
_TotalElements(Node* node){
    nr_intp total = 1;
    for (int i = 0; i < node->ndim; ++i){
        total *= node->shape[i];
    }
    return total;
}

NR_PRIVATE bool
_ShouldSummarize(Node* node, NodePrintOptions* opts){
    return _TotalElements(node) > opts->threshold;
}

NR_PRIVATE void
_Node_ToString_Recur(Node* node, int dim, char* buffer, char* base, 
                     int indent, NodePrintOptions* opts, bool summarize){
    if (dim == node->ndim){
        // Leaf element
        _AppendNum(buffer, base, node->dtype.dtype, opts->precision);
        return;
    }
    
    _Append(buffer, "[");
    nr_intp len = node->shape[dim];
    nr_intp stride = node->strides ? node->strides[dim] : (nr_intp)node->dtype.size;
    
    // Determine if we should summarize this dimension
    bool show_ellipsis = summarize && len > 2 * opts->edgeitems;
    nr_intp show_start = show_ellipsis ? opts->edgeitems : len;
    nr_intp show_end = show_ellipsis ? opts->edgeitems : 0;
    
    for (nr_intp i = 0; i < show_start; ++i){
        char* ptr = base + i * stride;
        
        // Add newline and indent for all dimensions except the innermost
        if (i > 0 && dim < node->ndim - 1){
            _Append(buffer, "\n");
            _AppendIndent(buffer, indent + 1);
        }
        
        _Node_ToString_Recur(node, dim + 1, buffer, ptr, indent + 1, opts, summarize);
        
        // Add comma and space between elements
        if (i < show_start - 1 || show_ellipsis){
            _Append(buffer, ", ");
        } else if (i < len - 1){
            _Append(buffer, ", ");
        }
    }
    
    // Add ellipsis if summarizing
    if (show_ellipsis){
        if (dim < node->ndim - 1){
            _Append(buffer, "\n");
            _AppendIndent(buffer, indent + 1);
        }
        _Append(buffer, "...");
        
        // Show last few elements
        for (nr_intp i = len - show_end; i < len; ++i){
            char* ptr = base + i * stride;
            
            if (dim < node->ndim - 1){
                _Append(buffer, "\n");
                _AppendIndent(buffer, indent + 1);
            } else {
                _Append(buffer, ", ");
            }
            
            _Node_ToString_Recur(node, dim + 1, buffer, ptr, indent + 1, opts, summarize);
            
            if (i < len - 1){
                _Append(buffer, ", ");
            }
        }
    }
    
    _Append(buffer, "]");
}


NR_PUBLIC void
Node_ToStringWithOptions(Node* node, char* buffer, NodePrintOptions* opts){
    buffer[0] = '\0';
    int intend = 0;

    if (opts == NULL){
        opts = &_default_options;
    }
    
    // Add node name if enabled
    if (opts->show_name){
        const char* name = NODE_NAME(node);
        if (name && name[0] != '\0'){
            _Append(buffer, name);
            _Append(buffer, "(");
        }

        int len = strlen(buffer);
        intend += len;
    }
    
    if (NODE_IS_SCALAR(node)){
        _AppendNum(buffer, node->data, node->dtype.dtype, opts->precision);
    } else {
        bool summarize = _ShouldSummarize(node, opts);
        _Node_ToString_Recur(node, 0, buffer, (char*)node->data, intend, opts, summarize);
    }
    
    // Close node name parenthesis
    if (opts->show_name){
        const char* name = NODE_NAME(node);
        if (name && name[0] != '\0'){
            _Append(buffer, ")");
        }
    }
}

NR_PUBLIC void
Node_ToString(Node* node, char* buffer){
    Node_ToStringWithOptions(node, buffer, NULL);
}

NR_PUBLIC void
Node_PrintWithOptions(Node* node, NodePrintOptions* opts){
    char buffer[8192];
    Node_ToStringWithOptions(node, buffer, opts);
    printf("%s\n", buffer);
}

NR_PUBLIC void
Node_Print(Node* node){
    Node_PrintWithOptions(node, NULL);
}

NR_PUBLIC void
Node_InfoToString(Node* node, char* buffer){
    buffer[0] = '\0';

    char dtype_str[64];
    char shape_str[256];
    char strides_str[256];
    NDtype_AsStringOnlyType(node->dtype.dtype, dtype_str);
    NTools_ShapeAsString(node->shape, node->ndim, shape_str);
    NTools_ShapeAsString(node->strides, node->ndim, strides_str);

    sprintf(buffer, "%s(dtype=%s, shape=%s, strides=%s)", 
            NODE_NAME(node) ? NODE_NAME(node) : "unnamed",
            dtype_str,
            shape_str, strides_str);
}

NR_PUBLIC void
Node_PrintInfo(Node* node){
    char buffer[512];
    Node_InfoToString(node, buffer);
    printf("%s\n", buffer);
}