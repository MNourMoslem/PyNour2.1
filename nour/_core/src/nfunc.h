#ifndef NOUR__CORE_SRC__NOUR_NR_FUNC_H
#define NOUR__CORE_SRC__NOUR_NR_FUNC_H

#include "nour/nour.h"

NR_PUBLIC int
NFunc_Call(const NFunc* nfunc, NFuncArgs* args);

NR_PUBLIC NFuncArgs*
NFuncArgs_New(int nin, int nout);

NR_PUBLIC void
_NFuncArgs_DECREF(NFuncArgs* args);

#define NFuncArgs_INCREF(args) ((args)->__ref_count++)
#define NFuncArgs_DECREF(args) _NFuncArgs_DECREF(args)

NR_PUBLIC NFuncFuncInfo*
_NFuncFuncInfo_New(const NFunc* nfunc, NFuncArgs* args, int out_idx);

NR_PUBLIC int
_NFuncFuncInfo_RegisterToNode(Node* node, const NFunc* nfunc, NFuncArgs* args, int out_idx);

NR_PUBLIC void
_NFuncFuncInfo_Free(NFuncFuncInfo* nfunc_info);

#endif // NOUR__CORE_SRC__NOUR_NR_FUNC_H