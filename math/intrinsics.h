DEBUG_OPTIMIZE_ON

//
// TODO - Convert all of these to platform-efficient intrinsic versions instead of using <math.h>
//

#include <math.h>

#include <immintrin.h>

TODO_MATH_CONSTEXPR f32
Abs(f32 value)
{
#if CRT_DISABLED
    f32 result = (value < 0) ? -value : value;
#else
    f32 result = fabsf(value);
#endif

    return result;
}

TODO_MATH_CONSTEXPR f64
Abs(f64 value)
{
#if CRT_DISABLED
    f64 result = (value < 0) ? -value : value;
#else
    f64 result = fabs(value);
#endif

    return result;
}

TODO_MATH_CONSTEXPR i32
Abs(i32 value)
{
#if CRT_DISABLED
    i32 result = (value < 0) ? -value : value;
#else
    i32 result = abs(value);
#endif

    return result;
}

TODO_MATH_CONSTEXPR i64
Abs(i64 value)
{
#if CRT_DISABLED
    i64 result = (value < 0) ? -value : value;
#else
    i64 result = llabs(value);
#endif

    return result;
}

template <class T>
constexpr T
SignOf(T value)
{
    // TODO - decide if I prefer template or overloads for Abs and SignOf and be
    //  consistent.
    // Templates
    //  +would let me use them on fixed math values
    //  -would let me use them on unsigned ints, which makes no sense
    T result = (value >= T(0)) ? T(1) : T(-1);
    return result;
}

TODO_MATH_CONSTEXPR f32
sqrt(f32 value)
{
    // @SSE
    f32 Result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(value)));
    return(Result);
}

#if 0

constexpr i64
i64_from_
function i64
RoundF64ToI64(f64 value)
{
    i64 result = (i64)round(value);
    return result;
}

function i32
FloorF32ToI32(f32 value)
{
    return (i32)floorf(value);
}

function i32
CeilF32ToI32(f32 value)
{
    return (i32)ceilf(value);
}

function u8
RoundF32ToU8(f32 value)
{
    u8 result = (u8)(value + 0.5f);
    return result;
}

function u8
NormalizedF32ToU8(f32 value)
{
    f32 scaledValue = value * 0xFF;
    u8 result = RoundF32ToU8(scaledValue);
    return result;
}
#endif

function f32
f32_round(f32 value)
{
    // @SSE 4.1
    f32 result = _mm_cvtss_f32(
                    _mm_round_ss(
                        _mm_setzero_ps(),
                        _mm_set_ss(value),
                        _MM_FROUND_TO_NEAREST_INT));
    return result;
}

function i32
i32_from_f32_round(f32 value)
{
    // @SSE 4.1
    i32 result = _mm_cvtss_si32(
                    _mm_round_ss(
                        _mm_setzero_ps(),
                        _mm_set_ss(value),
                        _MM_FROUND_TO_NEAREST_INT));
    return result;
}

function f32
f32_floor(f32 value)
{
    // @SSE 4.1
    f32 result = _mm_cvtss_f32(
                    _mm_floor_ss(
                        _mm_setzero_ps(),
                        _mm_set_ss(value)));
    return result;
}

function i32
i32_from_f32_floor(f32 value)
{
    // @SSE 4.1
    i32 result = _mm_cvtss_si32(
                    _mm_floor_ss(
                        _mm_setzero_ps(),
                        _mm_set_ss(value)));
    return result;
}

function f32
f32_ceil(f32 value)
{
    // @SSE 4.1
    f32 result = _mm_cvtss_f32(
                    _mm_ceil_ss(
                        _mm_setzero_ps(),
                        _mm_set_ss(value)));
    return result;
}

function i32
i32_from_f32_ceil(f32 value)
{
    // @SSE 4.1
    i32 result = _mm_cvtss_si32(
                    _mm_ceil_ss(
                        _mm_setzero_ps(),
                        _mm_set_ss(value)));
    return result;
}

#if !CRT_DISABLED
function f32
Sin(f32 radians)
{
    return sinf(radians);
}

function f32
Cos(f32 radians)
{
    return cosf(radians);
}

function f32
Tan(f32 radians)
{
    return tanf(radians);
}

function f32
Asin(f32 val)
{
    return asinf(val);
}

function f32
Acos(f32 val)
{
    return acosf(val);
}

function f32
Atan2(f32 y, f32 x)
{
    return atan2f(y, x);
}
#endif

#if 0
struct BitScanResult
{
    u32 index;
    bool found;
};

function BitScanResult
FindLeastSignificantBitSet(u32 value)
{
    BitScanResult result = {};
    
#if COMPILER_MSVC
    result.found = _BitScanForward((unsigned long *)&result.index, value);
#else
    for (int iBit = 0; iBit < 32; iBit++)
    {
        if (value & (1 << iBit))
        {
            result.found = true;
            result.index = iBit;
            return result;
        }
    }
#endif

    return result;
}

function u32
RotateLeft(u32 value, int shift)
{
#if COMPILER_MSVC
    u32 result = _rotl(value, shift);
#else
    u32 result =
        (shift > 0) ?
        (value << shift) | (value >> (32 - shift)) :
        (value >> -shift) | (value << (32 + shift));
#endif
    
    return result;
}

function u32
RotateRight(u32 value, int shift)
{
#if COMPILER_MSVC
    u32 result = _rotr(value, shift);
#else
    u32 result =
        (shift > 0) ?
        (value >> shift) | (value << (32 - shift)) :
        (value << -shift) | (value >> (32 + shift));
#endif
    
    return result;
}
#endif

// Credit: Hacker's Delight, Chapter 3 Section 2 - Rounding Up/Down to the Next Power of 2
// https://github.com/hcs0/Hackers-Delight/blob/master/clp2.c.txt
inline u32
u32_ceil_power_of_2(u32 x)
{
    x--;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x++;
    return x;
}

inline u64
u64_ceil_power_of_2(u64 x)
{
    x--;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x |= (x >> 32);
    x++;
    return x;
}

// https://github.com/hcs0/Hackers-Delight/blob/master/flp2.c.txt
inline u32
u32_floor_power_of_2(u32 x)
{
   x |= (x >> 1);
   x |= (x >> 2);
   x |= (x >> 4);
   x |= (x >> 8);
   x |= (x >> 16);
   x -= (x >> 1);
   return x;
}

inline u64
u64_floor_power_of_2(u64 x)
{
   x |= (x >> 1);
   x |= (x >> 2);
   x |= (x >> 4);
   x |= (x >> 8);
   x |= (x >> 16);
   x |= (x >> 32);
   x -= (x >> 1);
   return x;
}

#if COMPILER_MSVC
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanReverse64)
#endif

inline bool
bitscan_msb_index(u32 value, int* out)
{
#if COMPILER_MSVC
    unsigned long result;
    if (_BitScanReverse(&result, value))
    {
        *out = (int)result;
        return true;
    }

    return false;
#else
    ASSERT_TODO;
    return false;
#endif
}

inline bool
bitscan_msb_index(u64 value, int* out)
{
#if COMPILER_MSVC
    unsigned long result;
    if (_BitScanReverse64(&result, value))
    {
        *out = (int)result;
        return true;
    }

    return false;
#else
    ASSERT_TODO;
    return false;
#endif
}

DEBUG_OPTIMIZE_OFF
