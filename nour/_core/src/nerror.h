#ifndef NOUDE__CORE_SRC_NOUR_NERROR_H
#define NOUDE__CORE_SRC_NOUR_NERROR_H

#include "nour/nr_error.h"

NR_PUBLIC int 
NError_IsError();

NR_PUBLIC void* 
NError_RaiseError(NError_Type type, const char *format, ...);

NR_PUBLIC void 
NError_Print();

NR_PUBLIC void
NError_Clear();

NR_PUBLIC void* 
NError_RaiseErrorNoContext(NError_Type type);

// Convenience functions remain in header
NR_HEADER void* NError_RaiseMemoryError() {
    return NError_RaiseErrorNoContext(NError_MemoryError);
}

NR_HEADER void* NError_RaiseTypeError() {
    return NError_RaiseErrorNoContext(NError_TypeError);
}

NR_HEADER void* NError_RaiseIndexError() {
    return NError_RaiseErrorNoContext(NError_IndexError);
}

NR_HEADER void* NError_RaiseValueError() {
    return NError_RaiseErrorNoContext(NError_ValueError);
}

NR_HEADER void* NError_RaiseIOError() {
    return NError_RaiseErrorNoContext(NError_IOError);
}

NR_HEADER void* NError_RaiseZeroDivisionError() {
    return NError_RaiseErrorNoContext(NError_ZeroDivisionError);
}

NR_HEADER void* NError_RaiseImportError() {
    return NError_RaiseErrorNoContext(NError_ImportError);
}

NR_HEADER void* NError_RaiseAttributeError() {
    return NError_RaiseErrorNoContext(NError_AttributeError);
}

NR_HEADER void* NError_RaiseKeyError() {
    return NError_RaiseErrorNoContext(NError_KeyError);
}

NR_HEADER void* NError_RaiseAssertionError() {
    return NError_RaiseErrorNoContext(NError_AssertionError);
}

NR_HEADER void* NError_RaiseRuntimeError() {
    return NError_RaiseErrorNoContext(NError_RuntimeError);
}

NR_HEADER void* NError_RaiseOverflowError() {
    return NError_RaiseErrorNoContext(NError_OverflowError);
}

#endif