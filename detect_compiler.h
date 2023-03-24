#ifndef COMPILER_MSVC
 #define COMPILER_MSVC 0
#endif

#ifndef COMPILER_LLVM
 #define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
 #if _MSC_VER
  #undef COMPILER_MSVC
  #define COMPILER_MSVC 1
  #else
  // TODO - Detect more compilers!
  #endif
#endif
