#ifndef NR__CORE__INCLUDE__NR_CONFIG_H
#define NR__CORE__INCLUDE__NR_CONFIG_H

/* Maximum number of dimensions supported for arrays */
#define NR_NODE_MAX_NDIM 32

/*
    Compiler-Specific Definitions
    ----------------------------
    Detects the compiler being used and sets appropriate
    function attributes and optimizations.
*/
#if defined(__GNUC__) || defined(__clang__)  // GCC and Clang
    #define NR_INLINE __inline__
    #define NR_NOINLINE __attribute__((noinline))
    #define NR_FINLINE __attribute__((always_inline)) __inline__
    #define NR_TLS __thread
#elif defined(_MSC_VER)  // Microsoft Visual Studio Compiler
    #define NR_INLINE __inline
    #define NR_NOINLINE __declspec(noinline)
    #define NR_FINLINE __forceinline
    #define NR_TLS __declspec(thread)
#else  // Other compilers
    #define NR_INLINE inline
    #define NR_NOINLINE
    #define NR_FINLINE inline
    #define NR_TLS
#endif

/*
    Platform-Specific Definitions
    ---------------------------
    Detects the operating system and sets appropriate
    platform-specific configurations.
*/
#if defined(_WIN32) || defined(_WIN64)  // Windows
    #define NR_WINDOWS 1
    #define NR_UNIX 0
#elif defined(__unix__) || defined(__APPLE__)  // Unix-like OS (Linux, MacOS)
    #define NR_WINDOWS 0
    #define NR_UNIX 1
#else
    #define NR_WINDOWS 0
    #define NR_UNIX 0
#endif

/*
    Function Visibility and Linkage Controls
    --------------------------------------
    Defines macros for controlling function visibility
    and linkage across the library.
*/

#define NR_STATIC static
#define NR_STATIC_INLINE NR_STATIC NR_INLINE
#define NR_STATIC_FINLINE NR_STATIC NR_FINLINE

#define NR_HEADER NR_STATIC_INLINE
#define NR_PUBLIC

/* Private/internal function visibility (translation-unit local) */
#ifndef NR_PRIVATE
#define NR_PRIVATE static
#endif

/* Null pointer definition */
#define NR_NULL ((void*)0)

/*
    Unused Variable Handling
    ----------------------
    Provides compiler-specific ways to mark variables
    as intentionally unused to prevent warnings.
*/
#if defined(__GNUC__) || defined(__ICC) || defined(__clang__)
    #define NR_UNUSED(x) __attribute__((unused)) x
#else
    #define NR_UNUSED(x) x
#endif


#endif // NR__CORE__INCLUDE__NR_CONFIG_H