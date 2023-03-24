DEBUG_OPTIMIZE_ON

//
// TODO - Convert all of these to platform-efficient intrinsic versions instead of using <math.h>
//

#include <math.h>

#include <immintrin.h>

TODO_MATH_CONSTEXPR f32
Abs(f32 value)
{
    f32 result = fabsf(value);
    return result;
}

TODO_MATH_CONSTEXPR f64
Abs(f64 value)
{
    f64 result = fabs(value);
    return result;
}

TODO_MATH_CONSTEXPR i32
Abs(i32 value)
{
    i32 result = abs(value);
    return result;
}

TODO_MATH_CONSTEXPR i64
Abs(i64 value)
{
    i64 result = llabs(value);
    return result;
}

template <class T>
constexpr T
SignOf(T value)
{
    T result = (value >= T(0)) ? T(1) : T(-1);
    return result;
}

TODO_MATH_CONSTEXPR f32
Sqrt(f32 value)
{
    f32 result = sqrtf(value);
    return result;
}

#if 0

constexpr i64
i64_from_
inline i64
RoundF64ToI64(f64 value)
{
    i64 result = (i64)round(value);
    return result;
}

inline i32
FloorF32ToI32(f32 value)
{
    return (i32)floorf(value);
}

inline i32
CeilF32ToI32(f32 value)
{
    return (i32)ceilf(value);
}

inline u8
RoundF32ToU8(f32 value)
{
    u8 result = (u8)(value + 0.5f);
    return result;
}

inline u8
NormalizedF32ToU8(f32 value)
{
    f32 scaledValue = value * 0xFF;
    u8 result = RoundF32ToU8(scaledValue);
    return result;
}
#endif

TODO_MATH_CONSTEXPR i32
Round(f32 value)
{
    // i32 result = (i32)((value >= 0) ? (value + 0.5f) : (value - 0.5f));

    // Converts using CPU rounding mode
    i32 result = _mm_cvt_ss2si(_mm_set_ss(value));
    return result;
}

inline i32
Ceil(f32 value)
{
    return (i32)ceilf(value);
}

inline f32
Sin(f32 radians)
{
    return sinf(radians);
}

inline f32
Cos(f32 radians)
{
    return cosf(radians);
}

inline f32
Tan(f32 radians)
{
    return tanf(radians);
}

inline f32
Asin(f32 val)
{
    return asinf(val);
}

inline f32
Acos(f32 val)
{
    return acosf(val);
}

inline f32
Atan2(f32 y, f32 x)
{
    return atan2f(y, x);
}

struct BitScanResult
{
    u32 index;
    bool found;
};

#if 0
inline BitScanResult
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

inline u32
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

inline u32
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

DEBUG_OPTIMIZE_OFF
