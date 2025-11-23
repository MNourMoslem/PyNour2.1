#ifndef NOUR__CORE_SRC_NOUR_INDEX_H
#define NOUR__CORE_SRC_NOUR_INDEX_H

#include "nour/nour.h"

#define NINDEXRULESET_MAX_RULES NR_NODE_MAX_NDIM

typedef enum {
    NIndexRuleType_Int,
    NIndexRuleType_Slice,
    NIndexRuleType_Node,
    NIndexRuleType_Ellipsis,
    NIndexRuleType_NewAxis,
} NIndexRuleType;

typedef struct
{
    nr_intp start;
    nr_intp stop;
    nr_intp step;

    nr_bool has_start;
    nr_bool has_stop;
} NIndexSlice;

typedef struct {
    nr_intp index;
} NIndexInt;

typedef struct {
    Node* node;
} NIndexNode;

typedef union
{
    NIndexInt int_data;
    NIndexSlice slice_data;
    NIndexNode node_data;
} NIndexData;


typedef struct {
    NIndexRuleType type;
    NIndexData data;
} NIndexRule;


typedef struct {
    NIndexRule rules[NINDEXRULESET_MAX_RULES];
    nr_intp num_rules;
} NIndexRuleSet;

NR_PUBLIC NIndexRuleSet
NIndexRuleSet_New();

NR_PUBLIC void
NIndexRuleSet_Init(NIndexRuleSet* rs);

NR_PUBLIC int
NIndexRuleSet_AddInt(NIndexRuleSet* rs, nr_intp index);

NR_PUBLIC int
NIndexRuleSet_AddSlice(NIndexRuleSet* rs, nr_intp start, nr_intp stop, nr_intp step);

NR_PUBLIC int
NIndexRuleSet_AddSliceAdvanced(NIndexRuleSet* rs, nr_intp start, nr_intp stop, nr_intp step,
                                 nr_bool has_start, nr_bool has_stop);

NR_PUBLIC int
NIndexRuleSet_AddNewAxis(NIndexRuleSet* rs);

NR_PUBLIC int
NIndexRuleSet_AddEllipsis(NIndexRuleSet* rs);

NR_PUBLIC int
NIndexRuleSet_AddNode(NIndexRuleSet* rs, Node* index_node);

NR_PUBLIC NIndexRuleSet
NIndexRuleSet_NewFromString(const char* index_string);

NR_PUBLIC void
NIndexRuleSet_Cleanup(NIndexRuleSet* rs);

// Convenience functions for common indexing patterns
NR_PUBLIC int
NIndexRuleSet_AddFullSlice(NIndexRuleSet* rs);

NR_PUBLIC int
NIndexRuleSet_AddRange(NIndexRuleSet* rs, nr_intp start, nr_intp stop);

// Core get/set operations
NR_PUBLIC Node* Node_Get(Node* base_node, NIndexRuleSet* rs);
NR_PUBLIC Node* Node_RiskyGet(Node* base_node, NIndexRuleSet* rs);
NR_PUBLIC int Node_Set(Node* base_node, NIndexRuleSet* rs, Node* value);
NR_PUBLIC int Node_RiskySet(Node* base_node, NIndexRuleSet* rs, Node* value);

// Macros to access structs members

#define NIndexSlice_START(slice) ((slice).start)
#define NIndexSlice_STOP(slice)  ((slice).stop)
#define NIndexSlice_STEP(slice)  ((slice).step)
#define NIndexSlice_HAS_START(slice)  ((slice).has_start)
#define NIndexSlice_HAS_STOP(slice)   ((slice).has_stop)

#define NIndexInt_INDEX(int_data) ((int_data).index)

#define NIndexNode_NODE(node_data) ((node_data).node)

#define NIndexData_AS_INT(data)   ((data).int_data)
#define NIndexData_AS_SLICE(data) ((data).slice_data)
#define NIndexData_AS_NODE(data)  ((data).node_data)

#define NIndexRule_TYPE(rulep)      ((rulep)->type)
#define NIndexRule_DATA(rulep)      ((rulep)->data)
#define NIndexRule_DATA_AS_INT(rulep)   NIndexData_AS_INT(NIndexRule_DATA(rulep))
#define NIndexRule_DATA_AS_SLICE(rulep) NIndexData_AS_SLICE(NIndexRule_DATA(rulep))
#define NIndexRule_DATA_AS_NODE(rulep)  NIndexData_AS_NODE(NIndexRule_DATA(rulep))

#define NIndexRuleSet_RULES(rsp)      ((rsp)->rules)
#define NIndexRuleSet_RULE(rsp, i)   ((rsp)->rules[(i)])
#define NIndexRuleSet_NUM_RULES(rsp)    ((rsp)->num_rules)

#define NIndexLooper_BASE_ARRAY(looperp)   ((looperp)->base_array)
#define NIndexLooper_NODE_IN_INDICES(looperp) ((looperp)->node_in_indices)
#define NIndexLooper_COPY(looperp) ((looperp)->copy)
#define NIndexLooper_TARGET_SHAPE(looperp) ((looperp)->target_shape)
#define NIndexLooper_TARGET_STRIDES(looperp) ((looperp)->target_strides)
#define NIndexLooper_TARGET_NDIM(looperp) ((looperp)->target_ndim)


#endif 