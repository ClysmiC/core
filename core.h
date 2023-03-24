#pragma once

// --- Compiler / feature detection

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

#if DEBUG_BUILD
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
 #define ENABLE_ASSERT DEBUG_BUILD
#endif

// TODO - Better way to force breakpoint
#ifndef ForceBreakpoint
 #define ForceBreakpoint() (*(int*)0 = 0)
#endif

#if DEBUG_BUILD && ENABLE_ASSERT
 #define Assert(EXPRESSION) if (!(EXPRESSION)) { ForceBreakpoint(); }
#else
 #define Assert(EXPRESSION)
#endif

#define AssertFalseWarn Assert(false)
#define AssertFalse Assert(false)
#define AssertTodo Assert(false)
#define AssertWarn(EXPRESSION) Assert(EXPRESSION)
#define AssertElseTodo(EXPRESSION) Assert(EXPRESSION)


#if DEBUG_BUILD
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

#undef min          // Conflicts with U32/I32/etc. namespace constant values. core.h provides Min(..) and Max(..) macros.
#undef max          // ...

#ifdef _WINDOWS_
 #undef ZeroMemory  // Collides with our own implementation
 #undef CopyMemory  // ...
 #undef MoveMemory  // ...
#endif



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
static u64 constexpr min = 0;
static u64 constexpr max = 0xFF'FF'FF'FF'FF'FF'FF'FF;
}

namespace U32
{
static u32 constexpr min = 0;
static u32 constexpr max = 0xFF'FF'FF'FF;
}

namespace U16
{
static u16 constexpr min = 0;
static u16 constexpr max = 0xFF'FF;
}

namespace U8
{
static u8 constexpr min = 0;
static u8 constexpr max = 0xFF;
}

namespace I64
{
static i64 constexpr min = 0x80'00'00'00'00'00'00'00;
static i64 constexpr max = 0x7F'FF'FF'FF'FF'FF'FF'FF;
}

namespace I32
{
static i32 constexpr min = 0x80'00'00'00;
static i32 constexpr max = 0x7F'FF'FF'FF;
}

namespace I16
{
static i16 constexpr min = -32768; // HMM - Compiler complains about truncation if I put 0x80'00 ?
static i16 constexpr max = 0x7F'FF;
}

namespace I8
{
static i8 constexpr min = -128;    // HMM - Compiler complains about truncation if I put 0x80 ?
static i8 constexpr max = 0x7F;
}

namespace F32
{

static f32 constexpr halfPi = 1.57079632679f;
static f32 constexpr pi = 3.14159265359f;
static f32 constexpr threeHalvesPi = 4.71238898038f;
static f32 constexpr twoPi = 6.28318530718f;

static f32 constexpr toDeg = 57.2958f;
static f32 constexpr toRad = 0.0174533f;
    
static f32 constexpr sqrt2 = 1.41421356237f;
static f32 constexpr halfSqrt2 = 0.70710678118f;

static f32 constexpr goldenRatio = 1.61803398875f;

} // namespace F32



// --- hgen defines

// Tells hgen to generate a forward declaration
#define function

// Tells hgen to emit a default argument value in the generated forward declaration
#define OptionalArg(name, defaultValue) name
#define OptionalArg0(name) name



// --- Enum stuff

#define DefaultInvalid default: { AssertFalse; } break
#define DefaultNilInvalid case 0: default: { AssertFalse; } break
#define DefaultInvalidEnum(ENUM) case ENUM::ENUM_COUNT: default: { AssertFalse; } break
#define DefaultNilInvalidEnum(ENUM) case ENUM::NIL: case ENUM::ENUM_COUNT: default: { AssertFalse; } break

#define DefaultInvalidFallthrough default: { AssertFalse; }
#define DefaultInvalidEnumFallthrough(ENUM) case ENUM::ENUM_COUNT: default: { AssertFalse; }
#define DefaultNilInvalidEnumFallthrough(ENUM) case ENUM::NIL: case ENUM::ENUM_COUNT: default: { AssertFalse; }

//          Really, the solution to this one is at the type level there should be a distinction between a flag and a flag set. | operator could always produce flag set... other operators might get fuzzy
// HMM - do I want shift operators?
#define DefineFlagOps(ENUM, INT_TYPE) \
    constexpr ENUM operator|(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs | (INT_TYPE)rhs); } \
    constexpr ENUM & operator|=(ENUM & lhs, ENUM rhs) { lhs = lhs | rhs; return lhs; } \
    constexpr ENUM operator&(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs & (INT_TYPE)rhs); } \
    constexpr ENUM & operator&=(ENUM & lhs, ENUM rhs) { lhs = lhs & rhs; return lhs; } \
    constexpr bool IsFlagSet(ENUM flags, ENUM query) { return (flags & query) == query; } \
    constexpr bool IsAnyFlagSet(ENUM flags, ENUM query) { return (INT_TYPE)(flags & query) != 0; } \
    constexpr ENUM operator~(ENUM e) { return (ENUM)~(INT_TYPE)e; } \
    StaticAssert(sizeof(ENUM) == sizeof(INT_TYPE))

    // ID's often increment/decrement by an integer
#define DefineIdOps(ENUM, INT_TYPE) \
    constexpr bool operator==(ENUM lhs, INT_TYPE rhs) { return (INT_TYPE)lhs == rhs; } \
    constexpr bool operator==(INT_TYPE lhs, ENUM rhs) { return rhs == lhs; } \
    constexpr ENUM& operator++(ENUM & value) { value = (ENUM)((INT_TYPE)value + 1); return value; } \
    constexpr ENUM& operator++(ENUM & value, int) { value = (ENUM)((INT_TYPE)value + 1); return value; } \
    constexpr ENUM& operator--(ENUM & value) { value = (ENUM)((INT_TYPE)value - 1); return value; } \
    constexpr ENUM& operator--(ENUM & value, int) { value = (ENUM)((INT_TYPE)value - 1); return value; } \
    constexpr ENUM operator+(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs + (INT_TYPE)rhs); } \
    constexpr ENUM operator+(ENUM lhs, INT_TYPE rhs) { return (ENUM)((INT_TYPE)lhs + rhs); } \
    constexpr ENUM operator+(INT_TYPE lhs, ENUM rhs) { return (ENUM)(lhs + (INT_TYPE)rhs); } \
    constexpr ENUM& operator+=(ENUM & lhs, ENUM rhs) { lhs = lhs + rhs; return lhs; } \
    constexpr ENUM& operator+=(ENUM & lhs, INT_TYPE rhs) { lhs = lhs + rhs; return lhs; } \
    constexpr ENUM operator-(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs - (INT_TYPE)rhs); } \
    constexpr ENUM operator-(ENUM lhs, INT_TYPE rhs) { return (ENUM)((INT_TYPE)lhs - rhs); } \
    constexpr ENUM& operator-=(ENUM & lhs, ENUM rhs) { lhs = lhs - rhs; return lhs; } \
    constexpr ENUM& operator-=(ENUM & lhs, INT_TYPE rhs) { lhs = lhs - rhs; return lhs; } \
    StaticAssert(sizeof(ENUM) == sizeof(INT_TYPE))

#define DefineEnumOps(ENUM, INT_TYPE) \
    DefineIdOps(ENUM, INT_TYPE); \
    constexpr bool IsEnumValid(ENUM value) { return value > ENUM::NIL && value < ENUM::ENUM_COUNT; } \
    StaticAssert(sizeof(ENUM) == sizeof(INT_TYPE))

#define DefineEnumOpsAllowNil(ENUM, INT_TYPE) \
    DefineIdOps(ENUM, INT_TYPE); \
    constexpr bool IsEnumValid(ENUM value) { return value >= ENUM::NIL && value < ENUM::ENUM_COUNT; } \
    StaticAssert(sizeof(ENUM) == sizeof(INT_TYPE))

#define ForEnum(ENUM, it) for (ENUM it = (ENUM)1; it < ENUM::ENUM_COUNT; it++)
#define ForEnumAllowNil(ENUM, it) for (ENUM it = ENUM::NIL; it < ENUM::ENUM_COUNT; it++)



// --- Defer macro
// Courtesy of https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/

template <typename F>
    struct defer_ {
        F f;
    defer_(F f) : f(f) {}
        ~defer_() { f(); }
    };

template <typename F>
defer_<F> defer_func_(F f) {
    return defer_<F>(f);
}
#define Defer__1(x, y) x##y
#define Defer__2(x, y) Defer__1(x, y)
#define Defer__3(x)      Defer__2(x, __COUNTER__)
#define Defer(code)      auto Defer__3(_defer_) = defer_func_([&](){code;})



// --- Include remaining core features

#include "math.h"
#include "mem_util.h"
#include "mem_alloc.h"
#include "string.h"
#include "array.h"
#include "dict.h"
#include "tree.h"
#include "utf.h"
#include "util/util.h"  // HMM - should util be moved out of core?
