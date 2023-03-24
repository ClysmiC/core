#pragma once

#define Min(a, b) ((a) <  (b)) ? (a) : (b)
#define Max(a, b) ((a) >  (b)) ? (a) : (b)

#define IncrementIfZero(value) do { (value) = (decltype(value))((value) + !bool(value)); } while(0)
#define DecrementIfNonZero(value) do { (value) = (decltype(value))((value) - bool(value)); } while(0)

inline bool
f32_eq_approx(
    f32 lhs,
    f32 rhs,
    f32 epsilon=0.001f)
{
    f32 delta = Abs(lhs - rhs);
    bool result = (delta <= epsilon);
    return result;
}

template <class T>
constexpr auto
Lerp(T a, T b, f32 u)
{
    auto result = ((1 - u) * a) + (u * b);
    return result;
}

template <class T>
constexpr T
Clamp(T value, T min, T max)
{
    T result = value;
    result = Max(result, min);
    result = Min(result, max);
    return result;
}

template <class T>
inline T
Clamp01(T value)
{
    T result = Clamp(value, 0, 1);
    return result;
}

inline f32
DivideSafeN(f32 numerator, f32 denominator, f32 n)
{
    f32 result = n;
    if (denominator != 0.0f)
    {
        result = numerator / denominator;
    }
    return result;
}

inline f32
DivideSafe1(f32 numerator, f32 denominator)
{
    f32 result = DivideSafeN(numerator, denominator, 1);
    return result;
}

inline f32
DivideSafe0(f32 numerator, f32 denominator)
{
    f32 result = DivideSafeN(numerator, denominator, 0);
    return result;
}

// --- Vectors

enum class Axis : u8
{
    NIL = 0,
    
    X,
    Y,
    Z,

    ENUM_COUNT,
};

enum class SignedAxis : u8
{
    NIL = 0,
    
    XPlus,
    XMinus,
    YPlus,
    YMinus,
    ZPlus,
    ZMinus,

    ENUM_COUNT
};

union Vec2
{
    struct
    {
        f32 x, y;
    };

    struct
    {
        f32 u, v;
    };

    f32 e[2];

    Vec2() = default;
    Vec2(f32 x, f32 y)
    {
        this->x = x;
        this->y = y;
    }
    Vec2(i32 x, i32 y)
    {
        this->x = (f32)x;
        this->y = (f32)y;
    }
    Vec2(u32 x, u32 y)
    {
        this->x = (f32)x;
        this->y = (f32)y;
    }

    // NOTE - Implemented in math_2.h
    // explicit Vec2(union Vec2x);
    explicit Vec2(union Vec2i);
};

union Vec3
{
    struct
    {
        f32 x, y, z;
    };

    struct
    {
        f32 u, v, w;
    };

    struct
    {
        f32 r, g, b;
    };

    struct
    {
        Vec2 xy;
        f32 pad0_;
    };

    struct
    {
        f32 pad1_;
        Vec2 yz;
    };

    struct
    {
        Vec2 uv;
        f32 pad2_;
    };

    struct
    {
        f32 pad3_;
        Vec2 vw;
    };
    
    f32 e[3];

    Vec3() = default;
    Vec3(f32 x, f32 y, f32 z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    Vec3(Vec2 xy, f32 z)
    {
        this->x = xy.x;
        this->y = xy.y;
        this->z = z;
    }
    Vec3(i32 x, i32 y, i32 z)
    {
        this->x = (f32)x;
        this->y = (f32)y;
        this->z = (f32)z;
    }
    Vec3(u32 x, u32 y, u32 z)
    {
        this->x = (f32)x;
        this->y = (f32)y;
        this->z = (f32)z;
    }
    
    // NOTE - Implemented in math_2.h
    // explicit Vec3(union Vec3x);
    // Vec3(Vec2x xy, struct fix64 z);
    explicit Vec3(union Vec3i);
};

union Vec4
{
    struct
    {
        f32 x, y, z, w;
    };

    struct
    {
        f32 r, g, b, a;
    };

    struct
    {
        Vec3 rgb;
    };

    struct
    {
        Vec2 xy;
        f32 pad0_;
        f32 pad1_;
    };

    struct
    {
        f32 pad2_;
        Vec2 yz;
        f32 pad3_;
    };

    struct
    {
        f32 pad4_;
        f32 pad5_;
        Vec2 zw;
    };
    
    struct
    {
        Vec3 xyz;
        f32 pad6_;
    };

    struct
    {
        f32 pad7_;
        Vec3 yzw;
    };

    f32 e[4];

    Vec4() = default;
    Vec4(f32 x, f32 y, f32 z, f32 w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
    Vec4(i32 x, i32 y, i32 z, i32 w)
    {
        this->x = (f32)x;
        this->y = (f32)y;
        this->z = (f32)z;
        this->w = (f32)w;
    }
    Vec4(u32 x, u32 y, u32 z, u32 w)
    {
        this->x = (f32)x;
        this->y = (f32)y;
        this->z = (f32)z;
        this->w = (f32)w;
    }
    Vec4(Vec2 xy, f32 z, f32 w)
    {
        this->x = xy.x;
        this->y = xy.y;
        this->z = z;
        this->w = w;
    }
    Vec4(Vec3 xyz, f32 w)
    {
        this->x = xyz.x;
        this->y = xyz.y;
        this->z = xyz.z;
        this->w = w;
    }

    // NOTE - Implemented in math_2.h
    // explicit Vec4(union Vec4x);
    explicit Vec4(union Vec4i);
};

// Vec2

inline Vec2
Vec2Fill(f32 scalar)
{
    Vec2 result(scalar, scalar);
    return result;
}

inline Vec2
Vec2Min(Vec2 v0, Vec2 v1)
{
    Vec2 result(Min(v0.x, v1.x), Min(v0.y, v1.y));
    return result;
}

inline Vec2
Vec2Min(Vec2 * candidates, int count)
{
    Vec2 result =  {};

    if (count > 0)
    {
        result = candidates[0];
    }

    for (int iCandidate = 1; iCandidate < count; iCandidate++)
    {
        result = Vec2Min(result, candidates[iCandidate]);
    }

    return result;
}

inline Vec2
Vec2Max(Vec2 v0, Vec2 v1)
{
    Vec2 result(Max(v0.x, v1.x), Max(v0.y, v1.y));
    return result;
}

inline Vec2
Vec2Max(Vec2 * candidates, int count)
{
    Vec2 result =  {};

    if (count > 0)
    {
        result = candidates[0];
    }

    for (int iCandidate = 1; iCandidate < count; iCandidate++)
    {
        result = Vec2Max(result, candidates[iCandidate]);
    }

    return result;
}

inline Vec2
Vec2Abs(Vec2 v)
{
    Vec2 result(Abs(v.x), Abs(v.y));
    return result;
}

inline Vec2
operator-(Vec2 rhs)
{
    Vec2 result;
    result.x = -rhs.x;
    result.y = -rhs.y;
    return result;
}

inline Vec2
operator+(Vec2 lhs, Vec2 rhs)
{
    Vec2 result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    return result;
}

inline Vec2 &
operator+=(Vec2 & lhs, Vec2 rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec2
operator-(Vec2 lhs, Vec2 rhs)
{
    Vec2 result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    return result;
}

inline Vec2 &
operator-=(Vec2 & lhs, Vec2 rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec2
operator*(Vec2 lhs, f32 rhs)
{
    Vec2 result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    return result;
}

inline Vec2
operator*(f32 lhs, Vec2 rhs)
{
    Vec2 result = rhs * lhs;
    return result;
}

inline Vec2 &
operator*=(Vec2 & lhs, f32 rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec2
operator/(Vec2 lhs, f32 rhs)
{
    Vec2 result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    return result;
}

inline Vec2 &
operator/=(Vec2 & lhs, f32 rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

inline f32
Dot(Vec2 v0, Vec2 v1)
{
    f32 result = v0.x * v1.x + v0.y * v1.y;
    return result;
}

inline Vec2
Perp(Vec2 v)
{
    Vec2 result;
    result.x = -v.y;
    result.y = v.x;
    return result;
}

inline f32
LengthSq(Vec2 v)
{
    f32 result = Dot(v, v);
    return result;
}

inline f32
Length(Vec2 v)
{
    f32 result = LengthSq(v);
    result = Sqrt(result);
    return result;
}

inline Vec2
Lerp(Vec2 a, Vec2 b, f32 t)
{
    Vec2 result = ((1 - t) * a) + (t * b);
    return result;
}

inline Vec2
Clamp(Vec2 v, Vec2 min, Vec2 max)
{
    Vec2 result;
    result.x = Clamp(v.x, min.x, max.x);
    result.y = Clamp(v.y, min.y, max.y);
    return result;
}

inline Vec2
Clamp01(Vec2 v)
{
    Vec2 result = Clamp(v, Vec2Fill(0), Vec2Fill(1));
    return result;
}

inline Vec2
NormalizeUnsafe(Vec2 v)
{
    Vec2 result = v / Length(v);
    return result;
}

inline Vec2
NormalizeSafeOr(Vec2 v, Vec2 fallback)
{
    Vec2 result = fallback;
    
    f32 length = Length(v);
    if (length != 0)
    {
        result = v / length;
    }
    return result;
}

inline Vec2
NormalizeSafe0(Vec2 v)
{
    Vec2 result = NormalizeSafeOr(v, Vec2Fill(0));
    return result;
}

inline Vec2
NormalizeSafeXAxis(Vec2 v)
{
    Vec2 result = NormalizeSafeOr(v, Vec2(1, 0));
    return result;
}

inline Vec2
NormalizeSafeYAxis(Vec2 v)
{
    Vec2 result = NormalizeSafeOr(v, Vec2(0, 1));
    return result;
}

inline bool
IsNormalized(Vec2 v, f32 epsilon=0.001f)
{
    bool result = f32_eq_approx(Length(v), 1.0f, epsilon);
    return result;
}

inline bool
IsZero(Vec2 v)
{
    // TODO - Epsilon version?
    bool result = (v.x == 0.0f && v.y == 0.0f);
    return result;
}

inline bool
AreOrthogonal(Vec2 v0, Vec2 v1)
{
    f32 dot = Dot(v0, v1);
    bool result = f32_eq_approx(dot, 0);
    return result;
}

inline Vec2
Project(Vec2 projectee, Vec2 onto)
{
    Vec2 result = onto * Dot(projectee, onto) / LengthSq(onto);
    return result;
}

inline Vec2
ProjectOntoNormalizedAxis(Vec2 projectee, Vec2 onto)
{
    Vec2 result = onto * Dot(projectee, onto);
    return result;
}

inline Vec2
Reflect(Vec2 reflectee, Vec2 reflectionAxis)
{
    Vec2 result = reflectee - 2.0f * Project(reflectee, reflectionAxis);
    return result;
}

inline Vec2
ReflectAcrossNormalizedAxis(Vec2 reflectee, Vec2 normalizedReflectionAxis)
{
    Vec2 result = reflectee - 2.0f * Dot(reflectee, normalizedReflectionAxis) * normalizedReflectionAxis;
    return result;
}

inline Vec2
Hadamard(Vec2 v0, Vec2 v1)
{
    Vec2 result;
    result.x = v0.x * v1.x;
    result.y = v0.y * v1.y;
    return result;
}

inline Vec2
HadamardDivideSafe0(Vec2 v0, Vec2 v1)
{
    Vec2 result;
    result.x = DivideSafe0(v0.x, v1.x);
    result.y = DivideSafe0(v0.y, v1.y);
    return result;
}

// Vec3

inline Vec3
Vec3Fill(f32 scalar)
{
    Vec3 result(scalar, scalar, scalar);
    return result;
}

inline Vec3
Vec3Min(Vec3 v0, Vec3 v1)
{
    Vec3 result(Min(v0.x, v1.x), Min(v0.y, v1.y), Min(v0.z, v1.z));
    return result;
}

inline Vec3
Vec3Min(Vec3 * candidates, int count)
{
    Vec3 result =  {};

    if (count > 0)
    {
        result = candidates[0];
    }

    for (int iCandidate = 1; iCandidate < count; iCandidate++)
    {
        result = Vec3Min(result, candidates[iCandidate]);
    }

    return result;
}

inline Vec3
Vec3Max(Vec3 v0, Vec3 v1)
{
    Vec3 result(Max(v0.x, v1.x), Max(v0.y, v1.y), Max(v0.z, v1.z));
    return result;
}

inline Vec3
Vec3Max(Vec3 * candidates, int count)
{
    Vec3 result =  {};

    if (count > 0)
    {
        result = candidates[0];
    }

    for (int iCandidate = 1; iCandidate < count; iCandidate++)
    {
        result = Vec3Max(result, candidates[iCandidate]);
    }

    return result;
}

inline Vec3
operator-(Vec3 rhs)
{
    Vec3 result;
    result.x = -rhs.x;
    result.y = -rhs.y;
    result.z = -rhs.z;
    return result;
}

inline Vec3
operator+(Vec3 lhs, Vec3 rhs)
{
    Vec3 result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    return result;
}

inline Vec3 &
operator+=(Vec3 & lhs, Vec3 rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec3
operator-(Vec3 lhs, Vec3 rhs)
{
    Vec3 result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    return result;
}

inline Vec3 &
operator-=(Vec3 & lhs, Vec3 rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec3
operator*(Vec3 lhs, f32 rhs)
{
    Vec3 result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    result.z = lhs.z * rhs;
    return result;
}

inline Vec3
operator*(f32 lhs, Vec3 rhs)
{
    Vec3 result = rhs * lhs;
    return result;
}

inline Vec3 &
operator*=(Vec3 & lhs, f32 rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec3
operator/(Vec3 lhs, f32 rhs)
{
    Vec3 result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    result.z = lhs.z / rhs;
    return result;
}

inline Vec3 &
operator/=(Vec3 & lhs, f32 rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

inline Vec3
Vec3Rgb(u32 rgb)
{
    u32 r = (rgb >> 16) & 0xFF;
    u32 g = (rgb >> 8) & 0xFF;
    u32 b = (rgb >> 0) & 0xFF;
    Vec3 result = Vec3(r, g, b) / 255.0f;
    return result;
}

inline Vec3
Vec3Rgb(f32 r, f32 g, f32 b)
{
    Vec3 result = Vec3(r, g, b) / 255.0f;
    return result;
}

inline f32
Dot(Vec3 v0, Vec3 v1)
{
    f32 result = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
    return result;
}

inline Vec3
Cross(Vec3 v0, Vec3 v1)
{
    Vec3 result;
    result.x = v0.y * v1.z - v0.z * v1.y;
    result.y = v0.z * v1.x - v0.x * v1.z;
    result.z = v0.x * v1.y - v0.y * v1.x;
    return result;
}

inline f32
LengthSq(Vec3 v)
{
    f32 result = Dot(v, v);
    return result;
}

inline f32
Length(Vec3 v)
{
    f32 result = LengthSq(v);
    result = Sqrt(result);
    return result;
}

inline Vec3
Lerp(Vec3 a, Vec3 b, f32 t)
{
    Vec3 result = ((1 - t) * a) + (t * b);
    return result;
}

inline Vec3
Clamp(Vec3 v, f32 min, f32 max)
{
    Vec3 result;
    result.x = Clamp(v.x, min, max);
    result.y = Clamp(v.y, min, max);
    result.z = Clamp(v.z, min, max);
    return result;
}

inline Vec3
Clamp01(Vec3 v)
{
    Vec3 result = Clamp(v, 0, 1);
    return result;
}

inline Vec3
NormalizeUnsafe(Vec3 v)
{
    Vec3 result = v / Length(v);
    return result;
}

inline Vec3
NormalizeSafeOr(Vec3 v, Vec3 fallback)
{
    Vec3 result = fallback;
    
    f32 length = Length(v);
    if (length != 0.0f)
    {
        result = v / length;
    }
    
    return result;
}

inline Vec3
NormalizeSafe0(Vec3 v)
{
    Vec3 result = NormalizeSafeOr(v, Vec3Fill(0));
    return result;
}

inline Vec3
NormalizeSafeXAxis(Vec3 v)
{
    Vec3 result = NormalizeSafeOr(v, Vec3(1, 0, 0));
    return result;
}

inline Vec3
NormalizeSafeYAxis(Vec3 v)
{
    Vec3 result = NormalizeSafeOr(v, Vec3(0, 1, 0));
    return result;
}

inline Vec3
NormalizeSafeZAxis(Vec3 v)
{
    Vec3 result = NormalizeSafeOr(v, Vec3(0, 0, 1));
    return result;
}

inline bool
IsNormalized(Vec3 v, f32 epsilon=0.001f)
{
    bool result = f32_eq_approx(Length(v), 1.0f, epsilon);
    return result;
}

inline bool
IsZero(Vec3 v)
{
    // TODO - Epsilon version?
    bool result = (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f);
    return result;
}

inline bool
AreOrthogonal(Vec3 v0, Vec3 v1)
{
    f32 dot = Dot(v0, v1);
    bool result = f32_eq_approx(dot, 0);
    return result;
}

inline Vec3
Project(Vec3 projectee, Vec3 onto)
{
    Vec3 result = onto * Dot(projectee, onto) / LengthSq(onto);
    return result;
}

inline Vec3
ProjectOntoNormalizedAxis(Vec3 projectee, Vec3 onto)
{
    Vec3 result = onto * Dot(projectee, onto);
    return result;
}

inline Vec3
Reflect(Vec3 reflectee, Vec3 reflectionAxis)
{
    Vec3 result = reflectee - 2.0f * Project(reflectee, reflectionAxis);
    return result;
}

inline Vec3
ReflectAcrossNormalizedAxis(Vec3 reflectee, Vec3 normalizedReflectionAxis)
{
    Vec3 result = reflectee - 2.0f * Dot(reflectee, normalizedReflectionAxis) * normalizedReflectionAxis;
    return result;
}

inline Vec3
Hadamard(Vec3 v0, Vec3 v1)
{
    Vec3 result;
    result.x = v0.x * v1.x;
    result.y = v0.y * v1.y;
    result.z = v0.z * v1.z;
    return result;
}

inline Vec3
HadamardDivideSafe0(Vec3 v0, Vec3 v1)
{
    Vec3 result;
    result.x = DivideSafe0(v0.x, v1.x);
    result.y = DivideSafe0(v0.y, v1.y);
    result.z = DivideSafe0(v0.z, v1.z);
    return result;
}

// Vec4

inline Vec4
Vec4Fill(f32 scalar)
{
    Vec4 result(scalar, scalar, scalar, scalar);
    return result;
}

inline Vec4
operator-(Vec4 rhs)
{
    Vec4 result;
    result.x = -rhs.x;
    result.y = -rhs.y;
    result.z = -rhs.z;
    result.w = -rhs.w;
    return result;
}

inline Vec4
operator+(Vec4 lhs, Vec4 rhs)
{
    Vec4 result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    result.w = lhs.w + rhs.w;
    return result;
}

inline Vec4 &
operator+=(Vec4 & lhs, Vec4 rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec4
operator-(Vec4 lhs, Vec4 rhs)
{
    Vec4 result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    result.w = lhs.w - rhs.w;
    return result;
}

inline Vec4 &
operator-=(Vec4 & lhs, Vec4 rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec4
operator*(Vec4 lhs, f32 rhs)
{
    Vec4 result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    result.z = lhs.z * rhs;
    result.w = lhs.w * rhs;
    return result;
}

inline Vec4
operator*(f32 lhs, Vec4 rhs)
{
    Vec4 result = rhs * lhs;
    return result;
}

inline Vec4 &
operator*=(Vec4 & lhs, f32 rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec4
operator/(Vec4 lhs, f32 rhs)
{
    Vec4 result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    result.z = lhs.z / rhs;
    result.w = lhs.w / rhs;
    return result;
}

inline Vec4 &
operator/=(Vec4 & lhs, f32 rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

inline Vec4
Vec4Rgba(u32 rgba)
{
    u32 r = (rgba >> 24) & 0xFF;
    u32 g = (rgba >> 16) & 0xFF;
    u32 b = (rgba >> 8) & 0xFF;
    u32 a = (rgba >> 0) & 0xFF;
    Vec4 result = Vec4(r, g, b, a) / 255.0f;
    return result;
}

inline Vec4
Vec4Rgba(f32 r, f32 g, f32 b, f32 a)
{
    Vec4 result = Vec4(r, g, b, a) / 255.0f;
    return result;
}

inline Vec4
Vec4Rgb(u32 rgb)
{
    u32 r = (rgb >> 16) & 0xFF;
    u32 g = (rgb >> 8) & 0xFF;
    u32 b = (rgb >> 0) & 0xFF;
    Vec4 result = Vec4(r, g, b, 255u) / 255.0f;
    return result;
}

inline Vec4
Vec4Rgb(f32 r, f32 g, f32 b)
{
    Vec4 result = Vec4(r, g, b, 255.0f) / 255.0f;
    return result;
}

inline f32
Dot(Vec4 v0, Vec4 v1)
{
    f32 result = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
    return result;
}

inline f32
LengthSq(Vec4 v)
{
    f32 result = Dot(v, v);
    return result;
}

inline f32
Length(Vec4 v)
{
    f32 result = LengthSq(v);
    result = Sqrt(result);
    return result;
}

inline Vec4
Lerp(Vec4 a, Vec4 b, f32 t)
{
    Vec4 result = ((1 - t) * a) + (t * b);
    return result;
}

inline Vec4
Clamp(Vec4 v, f32 min, f32 max)
{
    Vec4 result;
    result.x = Clamp(v.x, min, max);
    result.y = Clamp(v.y, min, max);
    result.z = Clamp(v.z, min, max);
    result.w = Clamp(v.w, min, max);
    return result;
}

inline Vec4
Clamp01(Vec4 v)
{
    Vec4 result = Clamp(v, 0, 1);
    return result;
}

inline Vec4
NormalizeUnsafe(Vec4 v)
{
    Vec4 result = v / Length(v);
    return result;
}

inline Vec4
NormalizeSafeOr(Vec4 v, Vec4 fallback)
{
    Vec4 result = fallback;
    
    f32 length = Length(v);
    if (length != 0.0f)
    {
        result = v / length;
    }
    
    return result;
}

inline Vec4
NormalizeSafe0(Vec4 v)
{
    Vec4 result = NormalizeSafeOr(v, Vec4Fill(0));
    return result;
}

inline Vec4
NormalizeSafeXAxis(Vec4 v)
{
    Vec4 result = NormalizeSafeOr(v, Vec4(1, 0, 0, 0));
    return result;
}

inline Vec4
NormalizeSafeYAxis(Vec4 v)
{
    Vec4 result = NormalizeSafeOr(v, Vec4(0, 1, 0, 0));
    return result;
}

inline Vec4
NormalizeSafeZAxis(Vec4 v)
{
    Vec4 result = NormalizeSafeOr(v, Vec4(0, 0, 1, 0));
    return result;
}

inline Vec4
NormalizeSafeWAxis(Vec4 v)
{
    Vec4 result = NormalizeSafeOr(v, Vec4(0, 0, 0, 1));
    return result;
}

inline bool
IsNormalized(Vec4 v, f32 epsilon=0.001f)
{
    bool result = f32_eq_approx(Length(v), 1.0f, epsilon);
    return result;
}

inline bool
IsZero(Vec4 v)
{
    // TODO - Epsilon version?
    bool result = (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f && v.w == 0.0f);
    return result;
}

inline bool
AreOrthogonal(Vec4 v0, Vec4 v1)
{
    f32 dot = Dot(v0, v1);
    bool result = f32_eq_approx(dot, 0);
    return result;
}

inline Vec4
Hadamard(Vec4 v0, Vec4 v1)
{
    Vec4 result;
    result.x = v0.x * v1.x;
    result.y = v0.y * v1.y;
    result.z = v0.z * v1.z;
    result.w = v0.w * v1.w;
    return result;
}

inline Vec4
HadamardDivideSafe0(Vec4 v0, Vec4 v1)
{
    Vec4 result;
    result.x = DivideSafe0(v0.x, v1.x);
    result.y = DivideSafe0(v0.y, v1.y);
    result.z = DivideSafe0(v0.z, v1.z);
    result.w = DivideSafe0(v0.w, v1.w);
    return result;
}

// --- "integer" vectors
// HMM - Should these use explicitly sized ints?

union Vec2i
{
    struct
    {
        int x, y;
    };

    int e[2];

    Vec2i() = default;
    Vec2i(int x, int y)
    {
        this->x = x;
        this->y = y;
    }
};

inline Vec2i
Vec2iFill(int scalar)
{
    Vec2i result(scalar, scalar);
    return result;
}

inline Vec2i
Vec2iMin(Vec2i v0, Vec2i v1)
{
    Vec2i result(Min(v0.x, v1.x), Min(v0.y, v1.y));
    return result;
}

inline Vec2i
Vec2iMin(Vec2i * candidates, int count)
{
    Vec2i result =  {};

    if (count > 0)
    {
        result = candidates[0];
    }

    for (int iCandidate = 1; iCandidate < count; iCandidate++)
    {
        result = Vec2iMin(result, candidates[iCandidate]);
    }

    return result;
}

inline Vec2i
Vec2iMax(Vec2i v0, Vec2i v1)
{
    Vec2i result(Max(v0.x, v1.x), Max(v0.y, v1.y));
    return result;
}

inline Vec2i
Vec2iMax(Vec2i * candidates, int count)
{
    Vec2i result =  {};

    if (count > 0)
    {
        result = candidates[0];
    }

    for (int iCandidate = 1; iCandidate < count; iCandidate++)
    {
        result = Vec2iMax(result, candidates[iCandidate]);
    }

    return result;
}

inline Vec2i
Vec2iFloor(Vec2 v)
{
    Vec2i result = Vec2i((int)v.x, (int)v.y);
    return result;
}

inline Vec2i
Vec2iCeil(Vec2 v)
{
    Vec2i result = Vec2i(Ceil(v.x), Ceil(v.y));
    return result;
}

inline Vec2i
operator+(Vec2i lhs, Vec2i rhs)
{
    Vec2i result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    return result;
}

inline Vec2i &
operator+=(Vec2i & lhs, Vec2i rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec2i
operator-(Vec2i lhs, Vec2i rhs)
{
    Vec2i result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    return result;
}

inline Vec2i &
operator-=(Vec2i & lhs, Vec2i rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec2i
operator*(Vec2i lhs, int rhs)
{
    Vec2i result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    return result;
}

inline Vec2i
operator*(int lhs, Vec2i rhs)
{
    Vec2i result = rhs * lhs;
    return result;
}

inline Vec2i &
operator*=(Vec2i & lhs, int rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec2i
operator/(Vec2i lhs, int rhs)
{
    Vec2i result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    return result;
}

inline Vec2i &
operator/=(Vec2i & lhs, int rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

inline Vec2i
Clamp(Vec2i v, Vec2i min, Vec2i max)
{
    Vec2i result;
    result.x = Clamp(v.x, min.x, max.x);
    result.y = Clamp(v.y, min.y, max.y);
    return result;
}

union Vec2u
{
    struct
    {
        uint x, y;
    };

    uint e[2];

    Vec2u() = default;
    Vec2u(uint x, uint y)
    {
        this->x = x;
        this->y = y;
    }
};

inline Vec2u
operator+(Vec2u lhs, Vec2u rhs)
{
    Vec2u result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    return result;
}

inline Vec2u &
operator+=(Vec2u & lhs, Vec2u rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec2u
operator-(Vec2u lhs, Vec2u rhs)
{
    Vec2u result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    return result;
}

inline Vec2u &
operator-=(Vec2u & lhs, Vec2u rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec2u
operator*(Vec2u lhs, uint rhs)
{
    Vec2u result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    return result;
}

inline Vec2u
operator*(uint lhs, Vec2u rhs)
{
    Vec2u result = rhs * lhs;
    return result;
}

inline Vec2u &
operator*=(Vec2u & lhs, uint rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec2u
operator/(Vec2u lhs, uint rhs)
{
    Vec2u result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    return result;
}

inline Vec2u &
operator/=(Vec2u & lhs, uint rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

union Vec3i
{
    struct
    {
        int x, y, z;
    };

    int e[3];

    Vec3i() = default;
    Vec3i(int x, int y, int z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

inline Vec3i
operator+(Vec3i lhs, Vec3i rhs)
{
    Vec3i result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    return result;
}

inline Vec3i &
operator+=(Vec3i & lhs, Vec3i rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec3i
operator-(Vec3i lhs, Vec3i rhs)
{
    Vec3i result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    return result;
}

inline Vec3i &
operator-=(Vec3i & lhs, Vec3i rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec3i
operator*(Vec3i lhs, int rhs)
{
    Vec3i result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    result.z = lhs.z * rhs;
    return result;
}

inline Vec3i
operator*(int lhs, Vec3i rhs)
{
    Vec3i result = rhs * lhs;
    return result;
}

inline Vec3i &
operator*=(Vec3i & lhs, int rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec3i
operator/(Vec3i lhs, int rhs)
{
    Vec3i result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    result.z = lhs.z / rhs;
    return result;
}

inline Vec3i &
operator/=(Vec3i & lhs, int rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

union Vec3u
{
    struct
    {
        uint x, y, z;
    };

    uint e[3];

    Vec3u() = default;
    Vec3u(uint x, uint y, uint z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

inline Vec3u
operator+(Vec3u lhs, Vec3u rhs)
{
    Vec3u result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    return result;
}

inline Vec3u &
operator+=(Vec3u & lhs, Vec3u rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec3u
operator-(Vec3u lhs, Vec3u rhs)
{
    Vec3u result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    return result;
}

inline Vec3u &
operator-=(Vec3u & lhs, Vec3u rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec3u
operator*(Vec3u lhs, uint rhs)
{
    Vec3u result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    result.z = lhs.z * rhs;
    return result;
}

inline Vec3u
operator*(uint lhs, Vec3u rhs)
{
    Vec3u result = rhs * lhs;
    return result;
}

inline Vec3u &
operator*=(Vec3u & lhs, uint rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec3u
operator/(Vec3u lhs, uint rhs)
{
    Vec3u result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    result.z = lhs.z / rhs;
    return result;
}

inline Vec3u &
operator/=(Vec3u & lhs, uint rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

union Vec4i
{
    struct
    {
        int x, y, z, w;
    };

    int e[4];

    Vec4i() = default;
    Vec4i(int x, int y, int z, int w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
};

inline Vec4i
operator+(Vec4i lhs, Vec4i rhs)
{
    Vec4i result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    result.w = lhs.w + rhs.w;
    return result;
}

inline Vec4i &
operator+=(Vec4i & lhs, Vec4i rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec4i
operator-(Vec4i lhs, Vec4i rhs)
{
    Vec4i result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    result.w = lhs.w - rhs.w;
    return result;
}

inline Vec4i &
operator-=(Vec4i & lhs, Vec4i rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec4i
operator*(Vec4i lhs, int rhs)
{
    Vec4i result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    result.z = lhs.z * rhs;
    result.w = lhs.w * rhs;
    return result;
}

inline Vec4i
operator*(int lhs, Vec4i rhs)
{
    Vec4i result = rhs * lhs;
    return result;
}

inline Vec4i &
operator*=(Vec4i & lhs, int rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec4i
operator/(Vec4i lhs, int rhs)
{
    Vec4i result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    result.z = lhs.z / rhs;
    result.w = lhs.w / rhs;
    return result;
}

inline Vec4i &
operator/=(Vec4i & lhs, int rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

union Vec4u
{
    struct
    {
        uint x, y, z, w;
    };

    uint e[4];

    Vec4u() = default;
    Vec4u(uint x, uint y, uint z, uint w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
};

inline Vec4u
operator+(Vec4u lhs, Vec4u rhs)
{
    Vec4u result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    result.w = lhs.w + rhs.w;
    return result;
}

inline Vec4u &
operator+=(Vec4u & lhs, Vec4u rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec4u
operator-(Vec4u lhs, Vec4u rhs)
{
    Vec4u result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    result.w = lhs.w - rhs.w;
    return result;
}

inline Vec4u &
operator-=(Vec4u & lhs, Vec4u rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec4u
operator*(Vec4u lhs, uint rhs)
{
    Vec4u result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    result.z = lhs.z * rhs;
    result.w = lhs.w * rhs;
    return result;
}

inline Vec4u
operator*(uint lhs, Vec4u rhs)
{
    Vec4u result = rhs * lhs;
    return result;
}

inline Vec4u &
operator*=(Vec4u & lhs, uint rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec4u
operator/(Vec4u lhs, uint rhs)
{
    Vec4u result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    result.z = lhs.z / rhs;
    result.w = lhs.w / rhs;
    return result;
}

inline Vec4u &
operator/=(Vec4u & lhs, uint rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

// --- Ray

struct Ray2
{
    Vec2 p0;
    Vec2 dir;

    Ray2() = default;
    Ray2(Vec2 p0, Vec2 dir)
    {
        this->p0 = p0;
        this->dir = dir;
    }
};

Vec2 PosAtT(Ray2 ray, f32 t)
{
    Vec2 result = ray.p0 + ray.dir * t;
    return result;
}

struct Ray3
{
    Vec3 p0;
    Vec3 dir;

    Ray3() = default;
    Ray3(Vec3 p0, Vec3 dir)
    {
        this->p0 = p0;
        this->dir = dir;
    }
    explicit Ray3(Ray2 ray)
    {
        this->p0 = Vec3(ray.p0, 0);
        this->dir = Vec3(ray.dir, 0);
    }
};

Vec3 PosAtT(Ray3 ray, f32 t)
{
    Vec3 result = ray.p0 + ray.dir * t;
    return result;
}

// Mat3

struct Mat3
{
    f32 e[3][3];
    
    f32 *
    operator[](int i)
    {
        // NOTE - Only overloads the first index to point to the correct array, then
        // the second index is handled by built-in c++
    
        f32 * result = e[i];
        return result;
    }
};

inline Mat3
Mat3Identity()
{
    Mat3 result = {{
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 }
        }};
    
    return result;
}

 Mat3
Mat3Translate(Vec2 t)
{
    Mat3 result = {{
            { 1, 0, t.x },
            { 0, 1, t.y },
            { 0, 0, 1 }
        }};
    return result;
}

 Mat3
Mat3Scale(f32 s)
{
    Mat3 result = {{
            { s, 0, 0 },
            { 0, s, 0 },
            { 0, 0, 1 }
        }};

    return result;
}

 Mat3
Mat3Scale(Vec2 s)
{
    Mat3 result = {{
            { s.x, 0, 0 },
            { 0, s.y, 0 },
            { 0, 0, 1 }
        }};

    return result;
}

 Mat3
operator*(Mat3 & lhs, Mat3 & rhs)
{
    // TODO - simd-ize?
    // https://tavianator.com/2016/matrix_multiply.html
    
    Mat3 result = {};
    for(int down = 0; down < 3; down++)
    {
        for(int across = 0; across < 3; across++)
        {
            for(int i = 0; i < 3; i++)
            {
                result[down][across] += (lhs[down][i] * rhs[i][across]);
            }
        }
    }
    
    return result;
}

inline f32 *
RowMajorPtr(Mat3 * mat)
{
    return mat->e[0];
}

// Mat4

struct Mat4
{
    f32 e[4][4];

    const f32 *
    operator[](int i)
    {
        // NOTE - Only overloads the first index to point to the correct array, then
        // the second index is handled by built-in c++
    
        f32 * result = e[i];
        return result;
    }
};

 Mat4
operator*(const Mat4 & lhs, const Mat4 & rhs)
{
    // TODO - simd-ize?
    // https://tavianator.com/2016/matrix_multiply.html
    
    Mat4 result = {};
    for(int down = 0; down < 4; down++)
    {
        for(int across = 0; across < 4; across++)
        {
            for(int i = 0; i < 4; i++)
            {
                result.e[down][across] += (lhs.e[down][i] * rhs.e[i][across]);
            }
        }
    }
    
    return result;
}

inline Vec4
operator*(const Mat4 & m, const Vec4 & v)
{
    Vec4 result;
    result.x = m.e[0][0] * v.x   +   m.e[0][1] * v.y   +   m.e[0][2] * v.z   +   m.e[0][3] * v.w;
    result.y = m.e[1][0] * v.x   +   m.e[1][1] * v.y   +   m.e[1][2] * v.z   +   m.e[1][3] * v.w;
    result.z = m.e[2][0] * v.x   +   m.e[2][1] * v.y   +   m.e[2][2] * v.z   +   m.e[2][3] * v.w;
    result.w = m.e[3][0] * v.x   +   m.e[3][1] * v.y   +   m.e[3][2] * v.z   +   m.e[3][3] * v.w;
    return result;
}

inline Mat4
Mat4Identity()
{
    Mat4 result = {{
            { 1, 0, 0, 0 },
            { 0, 1, 0, 0 },
            { 0, 0, 1, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

 Mat4
Mat4Translate(Vec3 t)
{
    Mat4 result = {{
            { 1, 0, 0, t.x },
            { 0, 1, 0, t.y },
            { 0, 0, 1, t.z },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

 Mat4
Mat4Scale(f32 s)
{
    Mat4 result = {{
            { s, 0, 0, 0 },
            { 0, s, 0, 0 },
            { 0, 0, s, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

 Mat4
Mat4Scale(Vec3 s)
{
    Mat4 result = {{
            { s.x, 0, 0, 0 },
            { 0, s.y, 0, 0 },
            { 0, 0, s.z, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

 Mat4
Mat4RotateX(f32 radians)
{
    f32 c = Cos(radians);
    f32 s = Sin(radians);

    Mat4 result = {{
            { 1, 0, 0, 0 },
            { 0, c,-s, 0 },
            { 0, s, c, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

 Mat4
Mat4RotateY(f32 radians)
{
    f32 c = Cos(radians);
    f32 s = Sin(radians);

    Mat4 result = {{
            { c, 0, s, 0 },
            { 0, 1, 0, 0 },
            {-s, 0, c, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

 Mat4
Mat4RotateZ(f32 radians)
{
    f32 c = Cos(radians);
    f32 s = Sin(radians);

    Mat4 result = {{
            { c,-s, 0, 0 },
            { s, c, 0, 0 },
            { 0, 0, 1, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

// NOTE - For this game, the coordinate systems are as follows

// World Space
//  X is to the "east"
//  Y is to the "north"
//  Z is "up"

// Camera Space
//  X is to the "right"
//  Y is "up"
//  Z is "backwards"

inline Mat4
Mat4Ortho(f32 height, f32 aspectRatio, f32 farDist)
{
    // See FGED Vol 2 - Chapter 6.3.4
    
    f32 width = height * aspectRatio;

    // The far distance is positive for user convenience, but since Z- is forward
    //  in camera space, the actual Z value is negated.
    f32 farZCamera = -farDist;

#if 1
    // Used for Opengl where NDC has Z [-1 to 1]
    f32 k0 = 2.0f / farZCamera;
    f32 k1 = -1;
#else
    // Used (untested) for DirectX where NDC has Z [0 to 1]
    f32 k0 = 1.0f / farZCamera;
    f32 k1 = 0;
#endif
    
    // NOTE - This matrix is simpler than the general form, because it places
    //  the near plane at the camera and centers the camera w/r/t width and height.
    //  This allows many terms to cancel.
    
    Mat4 result = {{
            { 2.0f / width, 0, 0, 0 },
            { 0, 2.0f / height, 0, 0 },
            { 0, 0, k0, k1 },
            { 0, 0, 0, 1 }
        }};

    return result;
}

inline Mat4
Mat4Perspective(f32 fovVertical, f32 aspectRatio, f32 nearDist, f32 farDist)
{
    // NOTE - FOV is in radians
    
    // See FGED Vol 2 - Chapter 6.3.1
    
    // NOTE - The image is not formed on the near plane, rather it is formed on the projection plane at
    //  distance g from the camera. g's value is chosen so that the projection plane's interesection with
    //  the frustum has a vertical distance of 2 (range [-1, 1]). Because the camera points in the negative
    //  z direction, g has a negative value.

    // Projection plane (in camera space)
    // ____________________________
    // |           +1              |
    // |                           |
    // |-s                         |+s
    // |___________-1______________|

    f32 g = 1.0f / Tan(0.5f * fovVertical);
    f32 s = aspectRatio;
    
    // NOTE - The distances are positive for user convenience, but since Z- is forward
    //  in camera space, the actual Z values should be negated. However, since Z+ is
    //  forward in clip space, we would negate them again. So ultimately, the convenient
    //  positive values that get passed in are the correct ones!
    
    f32 n = nearDist;
    f32 f = farDist;

#if 1
    // Used for Opengl where NDC has Z [-1 to 1]
    // NOTE - Derived by solving the following system of equations:
    // -a + b / n = -1
    // -a + b / f = 1
    
    f32 k = 2 * f / (n - f);
    f32 a = k + 1;
    f32 b = n * k;
#else
    // Used (untested) for DirectX where NDC has Z [0 to 1]
    // NOTE - Derived by solving the following system of equations:
    // -a + b / n = 0
    // -a + b / f = 1
    
    f32 k = f / (n - f);
    f32 a = k;
    f32 b = n * k;
#endif

    // NOTE - The -1 in the final row ensures that the projected w values is positive for objects in
    //  front of the camera (they have negative z values in camera space, so -1 * zCam is positive)
    
    Mat4 result = {{
            { g / s, 0, 0, 0 },
            { 0, g, 0, 0 },
            { 0, 0, a, b },
            { 0, 0, -1, 0 }
        }};
    
    return result;
}

 Mat4
Mat4LookAtDir(Vec3 pos, Vec3 dir, Vec3 up)
{
    dir = NormalizeSafeYAxis(dir);
    up = NormalizeSafeOr(up, dir + Vec3(0, 0, 1));

    Vec3 right = Cross(dir, up);
    up = Cross(right, dir);

    // NOTE - rotation is transposed from the matrix that would move item into
    //  the space defined by the camera, and translation is negated. This is due
    //  to the fact that the view matrix doesn't move the camera, but moves everything
    //  else in the opposite fashion.

    // NOTE - dir is negated because in camera space, dir corresponds to our negative z axis

    // NOTE - The not
    
    Mat4 rotation = {{
            { right.x, right.y, right.z, 0 },
            { up.x, up.y, up.z, 0 },
            { -dir.x, -dir.y, -dir.z, 0 },
            { 0, 0, 0, 1 }
        }};

    Mat4 translation = Mat4Translate(-pos);

    Mat4 result = rotation * translation;
    return result;
}

 Mat4
Mat4LookAtTarget(Vec3 pos, Vec3 target, Vec3 up)
{
    Vec3 dir = target - pos;
    Mat4 result = Mat4LookAtDir(pos, dir, up);
    return result;
}

inline const f32 *
RowMajorPtr(const Mat4 * mat)
{
    return mat->e[0];
}

// Rect2

struct Rect2
{
    Vec2 min;
    Vec2 max;

    Rect2() = default;
    
    // NOTE - Implemented in math_2.h
    // explicit Rect2(struct Rect2x);
    explicit Rect2(struct Rect2i);
};

inline Rect2
RectFromMinAndMax(Vec2 min, Vec2 max)
{
    Rect2 result;
    result.min = min;
    result.max = max;
    return result;
}

inline Rect2
RectFromPoints(Vec2 point0, Vec2 point1)
{
    Rect2 result;
    result.min = Vec2Min(point0, point1);
    result.max = Vec2Max(point0, point1);
    return result;
}

inline Rect2
RectFromCenterHalfDim(Vec2 center, Vec2 halfDim)
{
    Rect2 result;
    result.min = center - halfDim;
    result.max = center + halfDim;
    return result;
}

inline Rect2
RectFromCenterHalfDim(Vec2 center, f32 halfDim)
{
    return RectFromCenterHalfDim(center, Vec2Fill(halfDim));
}

inline Rect2
RectFromCenterDim(Vec2 center, Vec2 dim)
{
    Rect2 result = RectFromCenterHalfDim(center, dim * 0.5f);
    return result;
}

inline Rect2
RectFromCenterDim(Vec2 center, f32 dim)
{
    return RectFromCenterDim(center, Vec2Fill(dim));
}

inline Rect2
RectFromMinDim(Vec2 min, Vec2 dim)
{
    Rect2 result;
    result.min = min;
    result.max = min + dim;
    return result;
}

inline Rect2
RectFromMinDim(Vec2 min, f32 dim)
{
    Rect2 result;
    result.min = min;
    result.max = min + Vec2Fill(dim);
    return result;
}

inline Rect2
RectFromMaxDim(Vec2 max, Vec2 dim)
{
    Rect2 result;
    result.min = max - dim;
    result.max = max;
    return result;
}

inline Rect2
RectFromMaxDim(Vec2 max, f32 dim)
{
    Rect2 result;
    result.min = max - Vec2Fill(dim);
    result.max = max;
    return result;
}

inline Rect2
RectFromRectAndMargin(Rect2 original, f32 margin)
{
    Rect2 result = original;
    result.min -= Vec2Fill(margin);
    result.max += Vec2Fill(margin);
    return result;
}

inline Rect2
RectFromRectAndMargin(Rect2 original, Vec2 margin)
{
    Rect2 result = original;
    result.min -= margin;
    result.max += margin;
    return result;
}

inline Rect2
RectFromRectAndOffset(Rect2 original, f32 offset)
{
    Rect2 result;
    result.min = original.min + Vec2Fill(offset);
    result.max = original.max + Vec2Fill(offset);
    return result;
}

inline Rect2
RectFromRectAndOffset(Rect2 original, Vec2 offset)
{
    Rect2 result;
    result.min = original.min + offset;
    result.max = original.max + offset;
    return result;
}

inline Vec2
GetCenter(Rect2 rect)
{
    Vec2 result = 0.5f * (rect.min + rect.max);
    return result;
}

inline Vec2
GetDim(Rect2 rect)
{
    Vec2 result = rect.max - rect.min;
    return result;
}

inline bool
IsDimZeroOrNegative(Rect2 rect)
{
    bool result = (rect.max.x <= rect.min.x) || (rect.max.y <= rect.min.y);
    return result;
}

inline bool
TestPointInRect(Rect2 rect, Vec2 testPoint)
{
    bool result =
        testPoint.x >= rect.min.x &&
        testPoint.y >= rect.min.y &&
        testPoint.x < rect.max.x &&
        testPoint.y < rect.max.y;

    return result;
}

inline bool
TestRectOverlapsRect(Rect2 rect0, Rect2 rect1)
{
    bool result = !((rect0.min.x >= rect1.max.x) ||
                    (rect0.max.x <= rect1.min.x) ||
                    (rect0.min.y >= rect1.max.y) ||
                    (rect0.max.y <= rect1.min.y));

    return result;
}
    
// Rect3

struct Rect3
{
    Vec3 min;
    Vec3 max;

    Rect3() = default;

    // NOTE - Implemented in math_2.h
    // explicit Rect3(struct Rect3x);
};

inline Rect3
RectFromMinAndMax(Vec3 min, Vec3 max)
{
    Rect3 result;
    result.min = min;
    result.max = max;
    return result;
}

inline Rect3
RectFromPoints(Vec3 point0, Vec3 point1)
{
    Rect3 result;
    result.min = Vec3Min(point0, point1);
    result.max = Vec3Max(point0, point1);
    return result;
}

inline Rect3
RectFromCenterHalfDim(Vec3 center, Vec3 halfDim)
{
    Rect3 result;
    result.min = center - halfDim;
    result.max = center + halfDim;
    return result;
}

inline Rect3
RectFromCenterDim(Vec3 center, Vec3 dim)
{
    Rect3 result = RectFromCenterHalfDim(center, dim * 0.5f);
    return result;
}

inline Rect3
RectFromCenterDim(Vec3 center, f32 dim)
{
    return RectFromCenterDim(center, Vec3Fill(dim));
}

inline Rect3
RectFromMinDim(Vec3 min, Vec3 dim)
{
    Rect3 result;
    result.min = min;
    result.max = min + dim;
    return result;
}

inline Rect3
RectFromMinDim(Vec3 min, f32 dim)
{
    Rect3 result;
    result.min = min;
    result.max = min + Vec3Fill(dim);
    return result;
}

inline Rect3
RectFromMaxDim(Vec3 max, Vec3 dim)
{
    Rect3 result;
    result.min = max - dim;
    result.max = max;
    return result;
}

inline Rect3
RectFromMaxDim(Vec3 max, f32 dim)
{
    Rect3 result;
    result.min = max - Vec3Fill(dim);
    result.max = max;
    return result;
}

inline Rect3
RectFromRectAndMargin(Rect3 original, f32 margin)
{
    Rect3 result = original;
    result.min -= Vec3Fill(margin);
    result.max += Vec3Fill(margin);
    return result;
}

inline Rect3
RectFromRectAndMargin(Rect3 original, Vec3 margin)
{
    Rect3 result = original;
    result.min -= margin;
    result.max += margin;
    return result;
}

inline Rect3
RectFromRectAndOffset(Rect3 original, f32 offset)
{
    Rect3 result;
    result.min = original.min + Vec3Fill(offset);
    result.max = original.max + Vec3Fill(offset);
    return result;
}

inline Rect3
RectFromRectAndOffset(Rect3 original, Vec3 offset)
{
    Rect3 result;
    result.min = original.min + offset;
    result.max = original.max + offset;
    return result;
}

inline Vec3
GetCenter(Rect3 rect)
{
    Vec3 result = 0.5f * (rect.min + rect.max);
    return result;
}

inline Vec3
GetDim(Rect3 rect)
{
    Vec3 result = rect.max - rect.min;
    return result;
}

inline bool
IsDimZeroOrNegative(Rect3 rect)
{
    bool result = (rect.max.x <= rect.min.x) || (rect.max.y <= rect.min.y) || (rect.max.x <= rect.min.x);
    return result;
}

inline bool
TestPointInRect(Rect3 rect, Vec3 testPoint)
{
    bool result =
        testPoint.x >= rect.min.x &&
        testPoint.y >= rect.min.y &&
        testPoint.z >= rect.min.z &&
        testPoint.x < rect.max.x &&
        testPoint.y < rect.max.y &&
        testPoint.z < rect.max.z;

    return result;
}

inline bool
TestRectOverlapsRect(Rect3 rect0, Rect3 rect1)
{
    bool result = !((rect0.min.x >= rect1.max.x) ||
                    (rect0.max.x <= rect1.min.x) ||
                    (rect0.min.y >= rect1.max.y) ||
                    (rect0.max.y <= rect1.min.y) ||
                    (rect0.min.z >= rect1.max.z) ||
                    (rect0.max.z <= rect1.min.z));
                   
    return result;
}

// Rect2i

struct Rect2i
{
    // NOTE - min/max are a half-open interval [min, max)
    Vec2i min;
    Vec2i max;
};

inline Rect2i
Rect2i_MinAndMax(Vec2i min, Vec2i max)
{
    Rect2i result;
    result.min = min;
    result.max = max;
    return result;
}

inline Rect2i
Rect2i_MinDim(Vec2i min, Vec2i dim)
{
    Rect2i result;
    result.min = min;
    result.max = min + dim;
    return result;
}

inline Vec2i
GetDim(Rect2i rect)
{
    Vec2i result = rect.max - rect.min;
    return result;
}

inline bool
IsDimZeroOrNegative(Rect2i rect)
{
    bool result = rect.min.x >= rect.max.x || rect.min.y >= rect.max.y;
    return result;
}

inline bool
TestPointInRect(Rect2i rect, Vec2i testPoint)
{
    bool result =
        testPoint.x >= rect.min.x &&
        testPoint.y >= rect.min.y &&
        testPoint.x < rect.max.x &&
        testPoint.y < rect.max.y;

    return result;
}


inline bool
TestRectOverlapsRect(Rect2i rect0, Rect2i rect1)
{
    bool result = !((rect0.min.x >= rect1.max.x) ||
                    (rect0.max.x <= rect1.min.x) ||
                    (rect0.min.y >= rect1.max.y) ||
                    (rect0.max.y <= rect1.min.y));
                   
    return result;
}

// Rect2u

struct Rect2u
{
    // NOTE - min/max are a half-open interval [min, max)
    Vec2u min;
    Vec2u max;
};

inline Rect2u
Rect2uFromMinAndMax(Vec2u min, Vec2u max)
{
    Rect2u result;
    result.min = min;
    result.max = max;
    return result;
}

inline Rect2u
Rect2uFromMinDim(Vec2u min, Vec2u dim)
{
    Rect2u result;
    result.min = min;
    result.max = min + dim;
    return result;
}

inline Vec2u
GetDim(Rect2u rect)
{
    Vec2u result = rect.max - rect.min;
    return result;
}

inline bool
IsDimZeroOrNegative(Rect2u rect)
{
    bool result = rect.min.x >= rect.max.x || rect.min.y >= rect.max.y;
    return result;
}

inline bool
TestPointInRect(Rect2u rect, Vec2u testPoint)
{
    bool result =
        testPoint.x >= rect.min.x &&
        testPoint.y >= rect.min.y &&
        testPoint.x < rect.max.x &&
        testPoint.y < rect.max.y;

    return result;
}

inline bool
TestRectOverlapsRect(Rect2u rect0, Rect2u rect1)
{
    bool result = !((rect0.min.x >= rect1.max.x) ||
                    (rect0.max.x <= rect1.min.x) ||
                    (rect0.min.y >= rect1.max.y) ||
                    (rect0.max.y <= rect1.min.y));
                   
    return result;
}

// Circle2

struct Circle2
{
    Vec2 center;
    f32 radius;
};

inline Circle2
CircleFromCenterRad(Vec2 center, f32 radius)
{
    Circle2 result;
    result.center = center;
    result.radius = radius;
    return result;
}

inline bool
TestRectOverlapsCircle(Rect2 rect, Circle2 circle)
{
    Vec2 rectClosest = circle.center;

    if (circle.center.x < rect.min.x)
        rectClosest.x = rect.min.x;
    else if (circle.center.x > rect.max.x)
        rectClosest.x = rect.max.x;

    if (circle.center.y < rect.min.y)
        rectClosest.y = rect.min.y;
    else if (circle.center.y > rect.max.y)
        rectClosest.y = rect.max.y;

    if (LengthSq(circle.center - rectClosest) <= circle.radius * circle.radius)
        return true;

    return false;
}

inline bool
TestCircleOverlapsCircle(Circle2 c0, Circle2 c1)
{
    f32 distSq = LengthSq(c0.center - c1.center);
    f32 r = c0.radius + c1.radius;
    f32 rSq = r * r;

    bool result = distSq < rSq;
    return result;
}

struct CircleOverlapTestResult
{
    bool overlaps;
    Vec2 penetration;
};

inline CircleOverlapTestResult
FullTestCircleOverlapsCircle(Circle2 c0, Circle2 c1)
{
    CircleOverlapTestResult result = {};
    f32 distSq = LengthSq(c0.center - c1.center);
    f32 r = c0.radius + c1.radius;
    f32 rSq = r * r;

    if (distSq < rSq)
    {
        result.overlaps = true;

        Vec2 displacementDir = NormalizeSafeXAxis(c0.center - c1.center);
        f32 displacementDist = c0.radius + c1.radius - Sqrt(distSq);
        result.penetration = displacementDir * displacementDist;
    }
    
    return result;
}

enum class WindingOrder : u8
{
    NIL = 0,

    Cw,
    Ccw,

    ENUM_COUNT
};
