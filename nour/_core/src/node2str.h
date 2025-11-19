#ifndef NOUR__CORE_SRC_NOUR_NODE2STR_H
#define NOUR__CORE_SRC_NOUR_NODE2STR_H

#include "nour/nour.h"
#include <stdbool.h>

typedef struct
{
    int threshold;      // Total elements before summarizing (default: 1000)
    int edgeitems;      // Items to show at start/end when summarizing (default: 3)
    int precision;      // Decimal precision for floats (default: 8)
    bool suppress_small;// Suppress small float values (default: true)
    int linewidth;     // Max line width before wrapping (default: 75)
    bool show_name;    // Show node name (default: true)
} NodePrintOptions;


NR_PUBLIC void
Node_ToString(Node* node, char* buffer);

NR_PUBLIC void
Node_Print(Node* node);

NR_PUBLIC void
Node_SetPrintOptions(NodePrintOptions* opts);

NR_PUBLIC NodePrintOptions*
Node_GetPrintOptions(void);

NR_PUBLIC void
Node_ToStringWithOptions(Node* node, char* buffer, NodePrintOptions* opts);

NR_PUBLIC void
Node_PrintWithOptions(Node* node, NodePrintOptions* opts);

NR_PUBLIC void
Node_InfoToString(Node* node, char* buffer);

NR_PUBLIC void
Node_PrintInfo(Node* node);

#endif // NOUR__CORE_SRC_NOUR_NODE2STR_H