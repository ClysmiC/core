#pragma once

// TODO
// options for trig, pow, etc. to replace CRT
// - cephes
// - sollya

// @Cleanup
#define IncrementIfZero(value) do {(value) = (decltype(value))((value) + !bool(value));} while(0)
#define DecrementIfNonZero(value) do {(value) = (decltype(value))((value) - bool(value));} while(0)

struct Range
{
    int start;
    int length;
};

function bool
f32_approx_eq(
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
lerp(T const& a, T const& b, f32 u)
{
    auto result = ((1 - u) * a) + (u * b);
    return result;
}

template <class T>
function T
min(T const& a, T const& b)
{
    T result = ((a) <  (b)) ? (a) : (b);
    return result;
}

template <class T>
function T
max(T const& a, T const& b)
{
    T result = ((a) >  (b)) ? (a) : (b);
    return result;
}

template <class T>
constexpr T
clamp(T const& value, T const& minimum, T const& maximum)
{
    T result;
    if (value < minimum)
    {
        result = minimum;
    }
    else if (value > maximum)
    {
        result = maximum;
    }
    else
    {
        result = value;
    }
    return result;
}

template <class T>
function T
clamp_01(T const& value)
{
    T result = clamp(value, (T)0, (T)1);
    return result;
}

function f32
divide_safe_n(f32 numerator, f32 denominator, f32 n)
{
    f32 result = n;
    if (denominator != 0.0f)
    {
        result = numerator / denominator;
    }
    return result;
}

function f32
divide_safe_1(f32 numerator, f32 denominator)
{
    f32 result = divide_safe_n(numerator, denominator, 1);
    return result;
}

function f32
divide_safe_0(f32 numerator, f32 denominator)
{
    f32 result = divide_safe_n(numerator, denominator, 0);
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

enum class Signed_Axis : u8
{
    NIL = 0,
    
    X_PLUS,
    X_MINUS,
    Y_PLUS,
    Y_MINUS,
    Z_PLUS,
    Z_MINUS,

    ENUM_COUNT
};
