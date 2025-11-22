#ifndef NOUR__CORE_SRC_NOUR___TC_METHODS_H
#define NOUR__CORE_SRC_NOUR___TC_METHODS_H

#include "nour/nour.h"

extern Node2NodeFunc __NODE_TC_METHODS_ARRAY__[];

NR_PUBLIC Node*
Node_ToType(Node* dst, const Node* src, NR_DTYPE dtype);

/* Scalar extraction helpers.
 * Each function returns the requested C type converted from the node's single value.
 * Requirements: node must be a scalar (ndim==0) OR have exactly one item (total size == 1).
 * On failure (NULL node, size !=1, unsupported dtype) raises NError and returns 0.
 */
NR_PUBLIC nr_bool   Node_AsBool   (const Node* node);
NR_PUBLIC nr_int8   Node_AsByte   (const Node* node);
NR_PUBLIC nr_uint8  Node_AsUByte  (const Node* node);
NR_PUBLIC nr_int16  Node_AsShort  (const Node* node);
NR_PUBLIC nr_uint16 Node_AsUShort (const Node* node);
NR_PUBLIC nr_int32  Node_AsInt    (const Node* node);
NR_PUBLIC nr_uint32 Node_AsUInt   (const Node* node);
NR_PUBLIC nr_int64  Node_AsLong   (const Node* node);
NR_PUBLIC nr_uint64 Node_AsULong  (const Node* node);
NR_PUBLIC nr_float32 Node_AsFloat (const Node* node);
NR_PUBLIC nr_float64 Node_AsDouble(const Node* node);
#endif