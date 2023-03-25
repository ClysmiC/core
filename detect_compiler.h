#ifndef COMPILER_MSVC
 #define COMPILER_MSVC 0
#endif

#ifndef COMPILER_CLANG
 #define COMPILER_CLANG 0
#endif

#if !COMPILER_MSVC && !COMPILER_CLANG
    #if    defined(_MSC_VER)
        #undef COMPILER_MSVC
        #define COMPILER_MSVC 1
    #elif  defined(__clang__)
        #undef COMPILER_CLANG
        #define COMPILER_CLANG 1
    #endif
    // TODO - Detect more compilers!
#endif
