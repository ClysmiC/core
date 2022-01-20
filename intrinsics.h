DEBUG_OPTIMIZE_ON

//
// TODO - Convert all of these to platform-efficient versions instead of using <math.h>
//

#include <math.h>

#if COMPILER_MSVC
#include <intrin.h>
#endif

inline f32
Abs(f32 value)
{
    f32 result = fabsf(value);
    return result;
}

inline f64
Abs(f64 value)
{
    f64 result = fabs(value);
    return result;
}

inline s32
Abs(s32 value)
{
    s32 result = abs(value);
    return result;
}

inline s64
Abs(s64 value)
{
    s64 result = llabs(value);
    return result;
}

inline s32
SignOf(s32 value)
{
    s32 result = (value >= 0) ? 1 : -1;
    return result;
}

inline f32
Sqrt(f32 value)
{
    f32 result = sqrtf(value);
    return result;
}

inline s32
RoundF32ToS32(f32 value)
{
    s32 result = (s32)roundf(value);
    return result;
}

inline constexpr s32
RoundF32ToS32Constexpr(f32 value)
{
    s32 result = (s32)((value >= 0) ? (value + 0.5f) : (value - 0.5f));
    return result;
}

inline s64
RoundF64ToS64(f64 value)
{
    s64 result = (s64)round(value);
    return result;
}

inline s32
FloorF32ToS32(f32 value)
{
    return (s32)floorf(value);
}

inline s32
CeilF32ToS32(f32 value)
{
    return (s32)ceilf(value);
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

DEBUG_OPTIMIZE_OFF
