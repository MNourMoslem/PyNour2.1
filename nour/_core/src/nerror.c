#include "nerror.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

NError __NR_NERROR_GLOBAL_ERROR_VAR__ = {
    .type = NError_NoError,
    .context = ""
};

NR_PRIVATE void
_NError_TypeAsString(NError_Type type, char* trg_str) {
    switch (type) {
        case NError_NoError:
            strcpy(trg_str, "NoError");
            break;
        case NError_MemoryError:
            strcpy(trg_str, "MemoryError");
            break;
        case NError_TypeError:
            strcpy(trg_str, "TypeError");
            break;
        case NError_IndexError:
            strcpy(trg_str, "IndexError");
            break;
        case NError_ValueError:
            strcpy(trg_str, "ValueError");
            break;
        case NError_IOError:
            strcpy(trg_str, "IOError");
            break;
        case NError_ZeroDivisionError:
            strcpy(trg_str, "ZeroDivisionError");
            break;
        case NError_ImportError:
            strcpy(trg_str, "ImportError");
            break;
        case NError_AttributeError:
            strcpy(trg_str, "AttributeError");
            break;
        case NError_KeyError:
            strcpy(trg_str, "KeyError");
            break;
        case NError_AssertionError:
            strcpy(trg_str, "AssertionError");
            break;
        case NError_RuntimeError:
            strcpy(trg_str, "RuntimeError");
            break;
        case NError_OverflowError:
            strcpy(trg_str, "OverflowError");
            break;
        default:
            strcpy(trg_str, "UnknownError");
            break;
    }
}

NR_PUBLIC int 
NError_IsError() {
    return __NR_NERROR_GLOBAL_ERROR_VAR__.type != NError_NoError;
}

NR_PUBLIC void* 
NError_RaiseError(NError_Type type, const char *format, ...) {
    __NR_NERROR_GLOBAL_ERROR_VAR__.type = type;

    va_list args;
    va_start(args, format);
    vsnprintf(__NR_NERROR_GLOBAL_ERROR_VAR__.context, 
             NERROR_MAX_STRING_LEN, format, args);
    va_end(args);

    return NULL;
}

NR_PUBLIC void 
NError_Print() {
    char type_str[30];
    _NError_TypeAsString(__NR_NERROR_GLOBAL_ERROR_VAR__.type, type_str);
    printf("%s: %s\n", type_str, __NR_NERROR_GLOBAL_ERROR_VAR__.context);
}

NR_PUBLIC void* 
NError_RaiseErrorNoContext(NError_Type type) {
    __NR_NERROR_GLOBAL_ERROR_VAR__.type = type;
    __NR_NERROR_GLOBAL_ERROR_VAR__.context[0] = '\0';
    return NULL;
}
