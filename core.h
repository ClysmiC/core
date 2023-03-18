#pragma once

// TODO
// - Templated "List" struct? Literally just a count and a pointer to a buffer. See OpenDDL/OpenGEX where I have
//   StructureList, PropertyList, (The StructureListReader, PropertyListReader stuff probably isn't generalizable,
//   but the List stuff probably is)


// NOTE - This allows the build file to specify which compiler it is using. In the case that none
//  is specified, we will try to auto-detect

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

#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <new> // apparently needed for placement new? TODO - Just get rid of all ctors then axe this

DEBUG_OPTIMIZE_OFF

// --- Undefine conflicting platform macros. core.h should always be included *after* system headers, for this reason.

#undef min          // Conflicts with U32/I32/etc. namespace constant values
#undef max          // ...
#undef ZeroMemory   // Collides with our own implementation
#undef CopyMemory   // ...
#undef MoveMemory   // ...
#undef CONST        // Collides with our namespace naming conventions
#undef OPTIONAL     // Collides with our hgen macro
#undef DELETE       // Collides with delete key in win32_keyboard.h

typedef unsigned int uint;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef uintptr_t uintptr;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef intptr_t intptr;

typedef float f32;
typedef double f64;

namespace U64
{
static constexpr u64 min = 0;
static constexpr u64 max = 0xFF'FF'FF'FF'FF'FF'FF'FF;
}

namespace U32
{
static constexpr u32 min = 0;
static constexpr u32 max = 0xFF'FF'FF'FF;
}

namespace U16
{
static constexpr u16 min = 0;
static constexpr u16 max = 0xFF'FF;
}

namespace U8
{
static constexpr u8 min = 0;
static constexpr u8 max = 0xFF;
}

namespace I64
{
static constexpr i64 min = 0x80'00'00'00'00'00'00'00;
static constexpr i64 max = 0x7F'FF'FF'FF'FF'FF'FF'FF;
}

namespace I32
{
static constexpr i32 min = 0x80'00'00'00;
static constexpr i32 max = 0x7F'FF'FF'FF;
}

namespace I16
{
static constexpr i16 min = -32768; // HMM - Compiler complains about truncation if I put 0x80'00 ?
static constexpr i16 max = 0x7F'FF;
}

namespace I8
{
static constexpr i8 min = -128;    // HMM - Compiler complains about truncation if I put 0x80 ?
static constexpr i8 max = 0x7F;
}

namespace F32
{

static constexpr f32 halfPi = 1.57079632679f;
static constexpr f32 pi = 3.14159265359f;
static constexpr f32 threeHalvesPi = 4.71238898038f;
static constexpr f32 twoPi = 6.28318530718f;

static constexpr f32 toDeg = 57.2958f;
static constexpr f32 toRad = 0.0174533f;
    
static constexpr f32 sqrt2 = 1.41421356237f;
static constexpr f32 halfSqrt2 = 0.70710678118f;

static constexpr f32 goldenRatio = 1.61803398875f;

static constexpr f32 max = FLT_MAX;
static constexpr f32 minPositive = FLT_MIN;

} // namespace F32

// --- hgen stuff

#define function
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

// HMM - do I want shift operators?
// HMM - IsFlagSet really checks if an entire bit pattern is set, by design. It is a slight misnomer though.
//          Really, the solution to this one is at the type level there should be a distinction between a flag and a flag set. | operator could always produce flag set... other operators might get fuzzy
#define DefineFlagOps(ENUM, INT_TYPE) \
    inline constexpr ENUM operator|(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs | (INT_TYPE)rhs); } \
    inline constexpr ENUM & operator|=(ENUM & lhs, ENUM rhs) { lhs = lhs | rhs; return lhs; } \
    inline constexpr ENUM operator&(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs & (INT_TYPE)rhs); } \
    inline constexpr ENUM & operator&=(ENUM & lhs, ENUM rhs) { lhs = lhs & rhs; return lhs; } \
    inline constexpr bool IsFlagSet(ENUM flags, ENUM query) { return (flags & query) == query; } \
    inline constexpr bool IsAnyFlagSet(ENUM flags, ENUM query) { return (INT_TYPE)(flags & query) != 0; } \
    inline constexpr ENUM operator~(ENUM e) { return (ENUM)~(INT_TYPE)e; } \
    StaticAssert(sizeof(ENUM) == sizeof(INT_TYPE))

#define DefineIdOps(ENUM, INT_TYPE) \
    inline constexpr bool operator==(ENUM lhs, INT_TYPE rhs) { return (INT_TYPE)lhs == rhs; } \
    inline constexpr bool operator==(INT_TYPE lhs, ENUM rhs) { return rhs == lhs; } \
    inline constexpr ENUM& operator++(ENUM & value) { value = (ENUM)((INT_TYPE)value + 1); return value; } \
    inline constexpr ENUM& operator++(ENUM & value, int) { value = (ENUM)((INT_TYPE)value + 1); return value; } \
    inline constexpr ENUM& operator--(ENUM & value) { value = (ENUM)((INT_TYPE)value - 1); return value; } \
    inline constexpr ENUM& operator--(ENUM & value, int) { value = (ENUM)((INT_TYPE)value - 1); return value; } \
    inline constexpr ENUM operator+(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs + (INT_TYPE)rhs); } \
    inline constexpr ENUM operator+(ENUM lhs, int rhs) { return (ENUM)((INT_TYPE)lhs + rhs); } \
    inline constexpr ENUM& operator+=(ENUM & lhs, ENUM rhs) { lhs = lhs + rhs; return lhs; } \
    inline constexpr ENUM& operator+=(ENUM & lhs, int rhs) { lhs = lhs + rhs; return lhs; } \
    inline constexpr ENUM operator-(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs - (INT_TYPE)rhs); } \
    inline constexpr ENUM operator-(ENUM lhs, int rhs) { return (ENUM)((INT_TYPE)lhs - rhs); } \
    inline constexpr ENUM& operator-=(ENUM & lhs, ENUM rhs) { lhs = lhs - rhs; return lhs; } \
    inline constexpr ENUM& operator-=(ENUM & lhs, int rhs) { lhs = lhs - rhs; return lhs; } \
    StaticAssert(sizeof(ENUM) == sizeof(INT_TYPE))

#define DefineEnumOps(ENUM, INT_TYPE) \
    DefineIdOps(ENUM, INT_TYPE); \
    inline constexpr bool IsEnumValid(ENUM value) { return value > ENUM::NIL && value < ENUM::ENUM_COUNT; } \
    StaticAssert(sizeof(ENUM) == sizeof(INT_TYPE))

#define DefineEnumOpsAllowNil(ENUM, INT_TYPE) \
    DefineIdOps(ENUM, INT_TYPE); \
    inline constexpr bool IsEnumValid(ENUM value) { return value >= ENUM::NIL && value < ENUM::ENUM_COUNT; } \
    StaticAssert(sizeof(ENUM) == sizeof(INT_TYPE))

#define ForEnum(ENUM, it) for (ENUM it = (ENUM)1; it < ENUM::ENUM_COUNT; it++)
#define ForEnumAllowNil(ENUM, it) for (ENUM it = ENUM::NIL; it < ENUM::ENUM_COUNT; it++)

// TODO - Should these always use 64 bit?
#define ArrayLen(array) (sizeof(array) / sizeof((array)[0]))
#define Kilobytes(value) ((value) * 1024LL)
#define Megabytes(value) (Kilobytes(value) * 1024LL)
#define Gigabytes(value) (Megabytes(value) * 1024LL)
#define Terabytes(value) (Gigabytes(value) * 1024LL)

#define Implies(p, q) (!(p) || (q))
#define Iff(p, q) ((bool)(p) == (bool)(q))

#define Min(a, b) ((a) <  (b)) ? (a) : (b)
#define Max(a, b) ((a) >  (b)) ? (a) : (b)

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

#define IncrementIfZero(value) do { (value) = (decltype(value))((value) + !bool(value)); } while(0)
#define DecrementIfNonZero(value) do { (value) = (decltype(value))((value) - bool(value)); } while(0)

#include "intrinsics.h"
#include "math.h"
#include "mem_util.h"
#include "mem_alloc.h"
#include "string.h"
#include "array.h"
#include "dict.h"
#include "tree.h"
#include "utf.h"
#include "util/util.h"  // HMM - should util be moved out of core?
