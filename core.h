#pragma once

// --- Compiler / feature detection

#include "detect_compiler.h"

#if BUILD_DEBUG
 #if COMPILER_MSVC
  #define DEBUG_OPTIMIZE_OFF __pragma(optimize("", off))
  #define DEBUG_OPTIMIZE_ON __pragma(optimize("", on))
 #else
 // TODO - Detect more compilers!
 #endif
#else
 #define DEBUG_OPTIMIZE_OFF
 #define DEBUG_OPTIMIZE_ON
#endif

// --- Asserts

#ifndef ENABLE_ASSERT
 #define ENABLE_ASSERT BUILD_DEBUG
#endif

//#define DO_WHILE0(STATEMENT) do { STATEMENT } while(0)

#if BUILD_DEBUG && ENABLE_ASSERT
// TODO - better implementation of this...
 #define ASSERT_TRACKER_COUNT 32768
 static bool g_assert_tracker_[ASSERT_TRACKER_COUNT] = {};

 #if COMPILER_MSVC
  #define FORCE_BREAKPOINT() __debugbreak()
 #else
  // TODO - Better way to force breakpoint
  #define FORCE_BREAKPOINT() (*(int*)0 = 0)
 #endif

 #define FORCE_BREAKPOINT_ONCE(counter) ((counter) >= 0 && (counter) < ASSERT_TRACKER_COUNT && !g_assert_tracker_[counter] ? (FORCE_BREAKPOINT(), g_assert_tracker_[counter] = true) : true)
 #define ASSERT(EXPRESSION) ((EXPRESSION) ? true : (FORCE_BREAKPOINT_ONCE(__COUNTER__), false))
#else
 #define ASSERT(EXPRESSION)
 #define FORCE_BREAKPOINT()
 #define FORCE_BREAKPOINT_ONCE()
#endif

#define ASSERT_FALSE_WARN ASSERT(false)
#define ASSERT_FALSE ASSERT(false)
#define ASSERT_TODO ASSERT(false)
#define ASSERT_WARN(EXPRESSION) ASSERT(EXPRESSION)


#if BUILD_DEBUG && ENABLE_ASSERT
  #define VERIFY(expression) ASSERT(expression)
  #define VERIFY_WARN(expression) ASSERT(expression)
#else
 #define VERIFY(expression) (expression)
 #define VERIFY_WARN(expression) (expression)
#endif

// TODO - more informative error...
#define STATIC_ASSERT(expr) static_assert(expr, "Static assert failed!")

// Expressive macros inside asserts
#define IMPLIES(p, q) (!(p) || (q))
#define IFF(p, q) ((bool)(p) == (bool)(q))


// TODO - get DEBUG_OPTIMIZE working.
//  probably need to tweak build.ps1 script, I think we need to have the script tell cl to optimize in debug builds,
//  then we use this flag to turn it off.
//  The idea behind this flag is that you can get optimized code in debug builds for core things like math.h
// DEBUG_OPTIMIZE_OFF



// --- Undefine conflicting platform macros. core.h should always be included *after* system headers, for this reason.

#undef min          // Conflicts with common variable names
#undef max          // ...


// --- Type definitions

#include <stdint.h>

using uint = unsigned int;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 =  uint64_t;
using uintptr = uintptr_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using intptr = intptr_t;

using f32 = float;
using f64 = double;

namespace U64
{
u64 constexpr MIN = 0;
u64 constexpr MAX = 0xFF'FF'FF'FF'FF'FF'FF'FF;
}

namespace U32
{
u32 constexpr MIN = 0;
u32 constexpr MAX = 0xFF'FF'FF'FF;
}

namespace U16
{
u16 constexpr MIN = 0;
u16 constexpr MAX = 0xFF'FF;
}

namespace U8
{
u8 constexpr MIN = 0;
u8 constexpr MAX = 0xFF;
}

namespace I64
{
i64 constexpr MIN = 0x80'00'00'00'00'00'00'00;
i64 constexpr MAX = 0x7F'FF'FF'FF'FF'FF'FF'FF;
}

namespace I32
{
i32 constexpr MIN = 0x80'00'00'00;
i32 constexpr MAX = 0x7F'FF'FF'FF;
}

namespace I16
{
i16 constexpr MIN = -32768;
i16 constexpr MAX = 32767;
}

namespace I8
{
i8 constexpr MIN = -128;
i8 constexpr MAX = 127;
}

namespace F32
{

f32 constexpr HALF_PI = 1.57079632679f;
f32 constexpr PI = 3.14159265359f;
f32 constexpr THREE_HALVES_PI = 4.71238898038f;
f32 constexpr TWO_PI = 6.28318530718f;

f32 constexpr TO_DEG = 57.2958f;
f32 constexpr TO_RAD = 0.0174533f;

f32 constexpr SQRT2 = 1.41421356237f;
f32 constexpr HALF_SQRT2 = 0.70710678118f;

f32 constexpr GOLDEN_RATIO = 1.61803398875f;

f32 constexpr MAX = 3.402823466e38f;

} // namespace F32



// --- hgen defines

// Tells hgen to generate a forward declaration
#define function static

#if COMPILER_MSVC
    #define DLLEXPORT extern "C" __declspec(dllexport)
#else
    #define DLLEXPORT extern "C"
#endif

// Tells hgen to emit a default argument value in the generated forward declaration
// TODO - rename these to OPTIONAL_ARG and OPTIONAL_ARG0
//      - need to update hgen keywords too...
#define OptionalArg(name, defaultValue) name
#define OptionalArg0(name) name

#include "enum.h"
#include "math/math.h"
#include "mem_util.h"
#include "mem_alloc.h"
#include "dict.h"
#include "string/string.h"
#include "string/string_parse.h"
#include "array/array.h"
#include "math/extra/geometry2d.h"
#include "io/io_visit.h"
#include "io/io_file.h"
#include "io/io_file.cpp"
#include "tree.h"
#include "string/utf.h"
#include "string/gap_buffer.h"
#include "util/util.h"  // HMM - should util be moved out of core?
