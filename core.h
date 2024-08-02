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

// TODO - Better way to force breakpoint
#if COMPILER_MSVC
 #define ForceBreakpoint() __debugbreak()
#else
 #define ForceBreakpoint() (*(int*)0 = 0)
#endif

#define DoWhile0(STATEMENT) do { STATEMENT } while(0)

#if BUILD_DEBUG && ENABLE_ASSERT
 #define Assert(EXPRESSION) DoWhile0(if (!(EXPRESSION)) { ForceBreakpoint(); })
#else
 #define Assert(EXPRESSION)
#endif

#define AssertFalseWarn Assert(false)
#define AssertFalse Assert(false)
#define AssertTodo Assert(false)
#define AssertWarn(EXPRESSION) Assert(EXPRESSION)
#define AssertElseTodo(EXPRESSION) Assert(EXPRESSION)


#if BUILD_DEBUG
 #define Verify(expression) Assert(expression)
 #define VerifyWarn(expression) Assert(expression)
#else
 #define Verify(expression) expression
 #define VerifyWarn(expression) expression
#endif

#define StaticAssert(expr) static_assert(expr, "Static assert failed!")
#define StaticAssertTodo StaticAssert(false)

// Expressive macros inside asserts
#define Implies(p, q) (!(p) || (q))
#define Iff(p, q) ((bool)(p) == (bool)(q))


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
static u64 constexpr MIN = 0;
static u64 constexpr MAX = 0xFF'FF'FF'FF'FF'FF'FF'FF;
}

namespace U32
{
static u32 constexpr MIN = 0;
static u32 constexpr MAX = 0xFF'FF'FF'FF;
}

namespace U16
{
static u16 constexpr MIN = 0;
static u16 constexpr MAX = 0xFF'FF;
}

namespace U8
{
static u8 constexpr MIN = 0;
static u8 constexpr MAX = 0xFF;
}

namespace I64
{
static i64 constexpr MIN = 0x80'00'00'00'00'00'00'00;
static i64 constexpr MAX = 0x7F'FF'FF'FF'FF'FF'FF'FF;
}

namespace I32
{
static i32 constexpr MIN = 0x80'00'00'00;
static i32 constexpr MAX = 0x7F'FF'FF'FF;
}

namespace I16
{
static i16 constexpr MIN = -32768; // HMM - Compiler complains about truncation if I put 0x80'00 ?
static i16 constexpr MAX = 0x7F'FF;
}

namespace I8
{
static i8 constexpr MIN = -128;    // HMM - Compiler complains about truncation if I put 0x80 ?
static i8 constexpr MAX = 0x7F;
}

namespace F32
{

static f32 constexpr HALF_PI = 1.57079632679f;
static f32 constexpr PI = 3.14159265359f;
static f32 constexpr THREE_HALVES_PI = 4.71238898038f;
static f32 constexpr TWO_PI = 6.28318530718f;

static f32 constexpr TO_DEG = 57.2958f;
static f32 constexpr TO_RAD = 0.0174533f;

static f32 constexpr SQRT2 = 1.41421356237f;
static f32 constexpr HALF_SQRT2 = 0.70710678118f;

static f32 constexpr GOLDEN_RATIO = 1.61803398875f;

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
#define OptionalArg(name, defaultValue) name
#define OptionalArg0(name) name



#include "enum.h"
#include "math/math.h"
#include "mem_util.h"
#include "mem_alloc.h"
#include "string/string.h"
#include "array/array.h"
#include "io_visit.h"
#include "dict.h"
#include "tree.h"
#include "string/utf.h"
#include "string/gap_buffer.h"
#include "util/util.h"  // HMM - should util be moved out of core?
