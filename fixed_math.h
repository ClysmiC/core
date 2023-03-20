namespace fxp
{

using intermediate_t = i64;     // Intermediate calculations are in 64 bit
using denom_t = i64;            // Denominators are 64 bit at compile time (no denom_t's stored at run-time)

// TODO - consteval gives best compile-time guarantee, but requires C++20
#if (__cplusplus >= 202002L)
 #define FXPCONSTEVAL consteval
#else
 #define FXPCONSTEVAL constexpr
#endif



// --- A fixed point value

template<class> struct integer_type { static bool constexpr is_supported = false; };
template<> struct integer_type<i32> { static bool constexpr is_supported = true; };
template<> struct integer_type<i64> { static bool constexpr is_supported = true; };
// TODO - support u32 and u64? Any extra work required?

template<class T, denom_t D>
struct Value
{
    StaticAssert(D >= 1);
    StaticAssert(integer_type<T>::is_supported);

    static denom_t constexpr d = D;

    T n;

    // --- Implicitly convert from...

    constexpr Value() = default;
    constexpr Value(i8 v)  { this->n = T(v * D); }
    constexpr Value(i16 v) { this->n = T(v * D); }
    constexpr Value(i32 v) { this->n = T(v * D); }
    constexpr Value(i64 v) { this->n = T(v * D); }
    constexpr Value(u8 v)  { this->n = T(v * D); }
    constexpr Value(u16 v) { this->n = T(v * D); }
    constexpr Value(u32 v) { this->n = T(v * D); }
    constexpr Value(u64 v) { this->n = T(v * D); }

    FXPCONSTEVAL Value(f32 v) { this->n = T(v * D); }
    FXPCONSTEVAL Value(f64 v) { this->n = T(v * D); }

    constexpr Value(Value<intermediate_t, D> const & other) { this->n = T(other.n); }

#if 0
    // --- Explicitly convert from...

    template <class T_OTHER, denom_t D_OTHER>
    explicit constexpr Value(Value<T_OTHER, D_OTHER> const & other)
    {
        if constexpr (D == D_OTHER)
        {
            this->n = n;
        }
        else
        {
            // HMM - Do I really want to round here?
            this->n = int_divide_and_round<T, OTHER_D>(other.n * D);
        }
    }
#endif

    // --- Explicitly convert to...

    explicit constexpr operator i8()  const { return i8 (n / D); }
    explicit constexpr operator i16() const { return i16(n / D); }
    explicit constexpr operator i32() const { return i32(n / D); }
    explicit constexpr operator i64() const { return i64(n / D); }
    explicit constexpr operator u8()  const { return u8 (n / D); }
    explicit constexpr operator u16() const { return u16(n / D); }
    explicit constexpr operator u32() const { return u32(n / D); }
    explicit constexpr operator u64() const { return u64(n / D); }
    explicit constexpr operator f32() const { return f32(n / (f32)D); }
    explicit constexpr operator f64() const { return f64(n / (f64)D); }
};

template <denom_t D> using Value32 = Value<i32, D>;
template <denom_t D> using Value64 = Value<i64, D>;



// --- Math operations
//  Binary operators work on 2 fixed point values and return a value with denominator specified by D_RESULT.
//  There are potentially 3 different denominators at play here but common terms are cancelled out at compile time
//  when applicable.

template<denom_t D_RESULT,
    class T0, denom_t D0,
    class T1, denom_t D1>
constexpr Value<intermediate_t, D_RESULT>
add(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    Value<intermediate_t, D_RESULT> result;

    // Cancel equal terms at compile-time
    if      constexpr   (D0 == D_RESULT && D1 == D_RESULT)  result.n = v0.n + v1.n;
    else if constexpr   (D0 == D_RESULT)                    result.n = v0.n + (D_RESULT * v1.n / D1);
    else if constexpr   (D1 == D_RESULT)                    result.n = (D_RESULT * v0.n / D0) + v1.n;
    else if constexpr   (D0 == D1)                          result.n = (D_RESULT * v0.n + D_RESULT * v1.n) / D0;
    else                                                    result.n = (D_RESULT * v0.n) / D0 + (D_RESULT * v1.n) / D1;

    return result;
}

template<denom_t D_RESULT,
    class T0, denom_t D0,
    class T1, denom_t D1>
constexpr Value<intermediate_t, D_RESULT>
subtract(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    Value<intermediate_t, D_RESULT> result;

    // Cancel equal terms at compile-time
    if      constexpr   (D0 == D_RESULT && D1 == D_RESULT)  result.n = v0.n - v1.n;
    else if constexpr   (D0 == D_RESULT)                    result.n = v0.n - (D_RESULT * v1.n / D1);
    else if constexpr   (D1 == D_RESULT)                    result.n = (D_RESULT * v0.n / D0) - v1.n;
    else if constexpr   (D0 == D1)                          result.n = (D_RESULT * v0.n - D_RESULT * v1.n) / D0;
    else                                                    result.n = (D_RESULT * v0.n) / D0 - (D_RESULT * v1.n) / D1;

    return result;
}

template<denom_t D_RESULT,
    class T0, denom_t D0,
    class T1, denom_t D1>
constexpr Value<intermediate_t, D_RESULT>
multiply(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    Value<intermediate_t, D_RESULT> result;

    // Cancel equal terms at compile-time
    if      constexpr   (D0 == D_RESULT)    result.n = (v0.n * v1.n) / D1;
    else if constexpr   (D1 == D_RESULT)    result.n = (v0.n * v1.n) / D0;
    // NOTE - Overflowing is a possible concern in this case, if the denominators are huge.
    else                                    result.n = (D_RESULT * v0.n * v1.n) / (D0 * D1);

    return result;
}

template<denom_t D_RESULT,
    class T0, denom_t D0,
    class T1, denom_t D1>
constexpr Value<intermediate_t, D_RESULT>
divide(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    Value<intermediate_t, D_RESULT> result;

    // Cancel equal terms at compile-time
    if      constexpr   (D0 == D_RESULT)    result.n = (D1 * v0.n) / v1.n;
    else if constexpr   (D0 == D1)          result.n = (D_RESULT * v0.n) / v1.n;
    // NOTE - Overflowing is a possible concern in this case, if the denominators are huge.
    else                                    result.n = (D_RESULT * D1 * v0.n) / (D0 * v1.n);

    return result;
}



// --- Fixed point quantities of a specific "unit" (e.g., meters, seconds, mph).
//  User code interacts with these, and should never really need to directly interact with a fxp::Value.
//  A few default units are defined, as well as conversions between them. Users can extend these
//  by adding their own Product<..> and Quotient<..> specializations. The underlying value types can
//  be specified/overridden by adding a Unit<..> specialization.

enum class Unit_Type : u64
{
    SCALAR = 0,
    NIL = 0,

    DISTANCE,
    ROTATIONS,
    TIME,

    SPEED,
    ACCELERATION,
};

// Default type of any unit that isn't specified otherwise via template specialization.
template<Unit_Type> struct Unit
{
    using Value_Type = Value32<1024>;
};

// Predefined conversions
template<Unit_Type, Unit_Type> struct Quotient;
template<> struct Quotient<Unit_Type::DISTANCE, Unit_Type::TIME> { static auto constexpr RESULT = Unit_Type::SPEED; };
template<> struct Quotient<Unit_Type::SPEED, Unit_Type::TIME> { static auto constexpr RESULT = Unit_Type::ACCELERATION; };

template<Unit_Type, Unit_Type> struct Product;
template<> struct Product<Unit_Type::SPEED, Unit_Type::TIME> { static auto constexpr RESULT = Unit_Type::DISTANCE; };
template<> struct Product<Unit_Type::TIME, Unit_Type::SPEED> { static auto constexpr RESULT = Unit_Type::DISTANCE; };
template<> struct Product<Unit_Type::ACCELERATION, Unit_Type::TIME> { static auto constexpr RESULT = Unit_Type::SPEED; };
template<> struct Product<Unit_Type::TIME, Unit_Type::ACCELERATION> { static auto constexpr RESULT = Unit_Type::SPEED; };

template<Unit_Type UNIT_TYPE>
struct Quantity
{
    using Value = typename Unit<UNIT_TYPE>::Value_Type;
    static Unit_Type constexpr unit_type = UNIT_TYPE;

    Value value;

    // --- Implicitly convert from...

    constexpr Quantity() = default;
    constexpr Quantity(i8 v)  : value(v) {}
    constexpr Quantity(i16 v) : value(v) {}
    constexpr Quantity(i32 v) : value(v) {}
    constexpr Quantity(i64 v) : value(v) {}
    constexpr Quantity(u8 v)  : value(v) {}
    constexpr Quantity(u16 v) : value(v) {}
    constexpr Quantity(u32 v) : value(v) {}
    constexpr Quantity(u64 v) : value(v) {}

    FXPCONSTEVAL Quantity(f32 v) : value(v) {}
    FXPCONSTEVAL Quantity(f64 v) : value(v) {}

    // --- Explicitly convert to...

    explicit constexpr operator i8()  const { return i8 (value); }
    explicit constexpr operator i16() const { return i16(value); }
    explicit constexpr operator i32() const { return i32(value); }
    explicit constexpr operator i64() const { return i64(value); }
    explicit constexpr operator u8()  const { return u8 (value); }
    explicit constexpr operator u16() const { return u16(value); }
    explicit constexpr operator u32() const { return u32(value); }
    explicit constexpr operator u64() const { return u64(value); }
    explicit constexpr operator f32() const { return f32(value); }
    explicit constexpr operator f64() const { return f64(value); }
};

using Scalar        = Quantity<Unit_Type::SCALAR>;
using Distance      = Quantity<Unit_Type::DISTANCE>;
using Rotations     = Quantity<Unit_Type::ROTATIONS>;
using Time          = Quantity<Unit_Type::TIME>;
using Speed         = Quantity<Unit_Type::SPEED>;
using Acceleration  = Quantity<Unit_Type::ACCELERATION>;



// --- Same units can always add/subtract

template<Unit_Type UNIT>
constexpr Quantity<UNIT>
operator+(Quantity<UNIT> q0, Quantity<UNIT> q1)
{
    Quantity<UNIT> result;
    result.value = add<Quantity<UNIT>::Value::d>(q0.value, q1.value);
    return result;
}
template<class FXP, Unit_Type UNIT>
constexpr Quantity<UNIT>
operator-(Quantity<UNIT> q0, Quantity<UNIT> q1)
{
    Quantity<UNIT> result;
    result.value = subtract<Quantity<UNIT>::Value::d>(q0.value, q1.value);
    return result;
}

// --- Same units can always divide and get a scalar

template<Unit_Type UNIT>
constexpr Scalar
operator/(Quantity<UNIT> q0, Quantity<UNIT> q1)
{
    Scalar result;
    result.value = divide<Scalar::Value::d>(q0.value, q1.value);
    return result;
}

// --- Units can always multiply and divide by a scalar

template<Unit_Type UNIT>
constexpr Quantity<UNIT>
operator*(Quantity<UNIT> q0, Scalar q1)
{
    Quantity<UNIT> result;
    result.value = multiply<Quantity<UNIT>::Value::d>(q0.value, q1.value);
    return result;
}
template<Unit_Type UNIT>
constexpr Quantity<UNIT>
operator*(Scalar q0, Quantity<UNIT> q1)
{
    Quantity<UNIT> result;
    result.value = multiply<Quantity<UNIT>::Value::d>(q0.value, q1.value);
    return result;
}

template<Unit_Type UNIT>
constexpr Quantity<UNIT>
operator/(Quantity<UNIT> q0, Scalar q1)
{
    Quantity<UNIT> result;
    result.value = divide<Quantity<UNIT>::Value::d>(q0.value, q1.value);
    return result;
}

// --- Any 2 quantities with a conversion defined can multiply/divide

template<Unit_Type UNIT0, Unit_Type UNIT1>
constexpr auto
operator*(Quantity<UNIT0> q0, Quantity<UNIT1> q1)
{
    static Unit_Type constexpr RESULT = Product<UNIT0, UNIT1>::RESULT;
    Quantity<RESULT> result;
    result.value = multiply<Quantity<RESULT>::Value::d>(q0.value, q1.value);
    return result;
}

template<Unit_Type UNIT0, Unit_Type UNIT1>
constexpr auto
operator/(Quantity<UNIT0> q0, Quantity<UNIT1> q1)
{
    static Unit_Type constexpr RESULT = Quotient<UNIT0, UNIT1>::RESULT;
    Quantity<RESULT> result;
    result.value = divide<Quantity<RESULT>::Value::d>(q0.value, q1.value);
    return result;
}

// TODO - move this to user code
// --- Unit definitions

template<> struct Unit<Unit_Type::DISTANCE>
{
    using Value_Type = Value32<1024 * 16>;
};

template<> struct Unit<Unit_Type::TIME>
{
    using Value_Type = Value32<64>;
};

template<> struct Unit<Unit_Type::SPEED>
{
    using Value_Type = Value32<1024 * 16 / 64>;
};

#undef FXPCONSTEVAL

} // namespace fxp


























#if 0

// HMM - enable inexact floating point exception for the simulation to detect accidental usage of floating point?
// see https://www.gamasutra.com/blogs/DanielCollier/20151124/251987/Minimizing_the_Pain_of_Lockstep_Multiplayer.php

enum class RawBits32 : u32 {};
enum class RawBits64 : u64 {};

// 52.12 fixed point number

namespace FIX64
{
static constexpr int fracBits = 12;
static constexpr u64 intMask  = 0b1111111111111111111111111111111111111111111111111111000000000000;
static constexpr u64 fracMask = 0b0000000000000000000000000000000000000000000000000000111111111111;
}

struct fix64
{
    s64 rawValue_;

    // NOTE - No effort taken to detect or be robust against overflow. Should we?
    
    // NOTE - Rule of thumb is that it things will implicitly cast/construct from
    //  float -> fixed, but require explicit going from fixed -> float. This makes
    //  it convenient to get things into fixed, but helps prevent accidentally
    //  going back into float
    
    // NOTE - Constructors are all constexpr to make sure we are getting known constant
    //  Values at COMPILE TIME and aren't relying on float computations at RUTIME. This
    //  might not be necessary for non-float constructor, but I haven't needed those to
    //  be non-constexpr yet.
    
    fix64() = default;
    constexpr fix64(s32 i)
        : rawValue_(i << FIX64::fracBits)
    {
    }

    constexpr fix64(s64 i)
        : rawValue_(i << FIX64::fracBits)
    {
        
    }

    constexpr fix64(u32 i)
        : rawValue_(i << FIX64::fracBits)
    {
    }

    constexpr fix64(u64 i)
        : rawValue_(i << FIX64::fracBits)
    {
        
    }

    constexpr fix64(f32 f)
        : rawValue_(RoundF32ToS32Constexpr(f * (1 << FIX64::fracBits)))
    {
    }

    constexpr fix64(RawBits64 v)
        : rawValue_((s64)v)
    {
    }

    explicit constexpr operator s32() const
    {
        // TODO - Round?
        // NOTE - MSVC does arithmetic right shift, which is what we want.
        // TODO - Enforce arithmetic shift on other compilers?
        return (s32)(rawValue_ >> FIX64::fracBits);
    }

    explicit constexpr operator s64() const
    {
        // TODO - Round?
        // NOTE - MSVC does arithmetic right shift, which is what we want.
        // TODO - Enforce arithmetic shift on other compilers?
        return (s64)(rawValue_ >> FIX64::fracBits);
    }

    explicit constexpr operator u32() const
    {
        // TODO - Round?
        // NOTE - MSVC does arithmetic right shift, which is what we want.
        // TODO - Enforce arithmetic shift on other compilers?
        return (u32)(rawValue_ >> FIX64::fracBits);
    }

    explicit constexpr operator u64() const
    {
        // TODO - Round?
        // NOTE - MSVC does arithmetic right shift, which is what we want.
        // TODO - Enforce arithmetic shift on other compilers?
        return (u64)(rawValue_ >> FIX64::fracBits);
    }

    explicit constexpr operator f32() const
    {
        s64 intPart = (s64)(*this);
        f32 fracPart = (rawValue_ & FIX64::fracMask) / (f32)(1 << FIX64::fracBits);

        f32 result = intPart + fracPart;
        return result;
    }

    explicit constexpr operator f64() const
    {
        f64 result = (f32)(*this);
        return result;
    }
};

inline fix64
Fix64FromF64UnsafeForSim(f64 Value)
{
    // Unsafe for deterministic because we are converting from f64 to fix64 at runtime, which
    //  we don't want to do for Values that are part of the simulation logic.
    
    fix64 result;
    result.rawValue_ = RoundF64ToS64(Value * (1 << FIX64::fracBits));
    return result;
}

namespace FIX64
{

static constexpr fix64 one((RawBits64)(FIX64::fracMask + 1));
static constexpr fix64 min((RawBits64)(0x8000000000000000));
static constexpr fix64 max((RawBits64)(0x7FFFFFFFFFFFFFFF));

static constexpr fix64 smallestPositive((RawBits64)0x0000000000000001);
static constexpr fix64 smallestNegative((RawBits64)0xFFFFFFFFFFFFFFFF);
    
static constexpr fix64 halfPi((RawBits64)6433);
static constexpr fix64 pi((RawBits64)12868);
static constexpr fix64 threeHalvesPi((RawBits64)19301);
static constexpr fix64 twoPi((RawBits64)25735);

static constexpr fix64 toDeg((RawBits64)234684);
static constexpr fix64 toRad((RawBits64)71);

static constexpr fix64 goldenRatio((RawBits64)6627);

}

inline fix64
Fix64Raw(s64 rawValue)
{
    fix64 result;
    result.rawValue_ = rawValue;
    return result;
}

inline constexpr fix64
operator-(fix64 fx)
{
    fix64 result;
    result.rawValue_ = -fx.rawValue_;
    return result;
}
    
inline constexpr fix64
operator+(fix64 lhs, fix64 rhs)
{
    fix64 result;
    result.rawValue_ = lhs.rawValue_ + rhs.rawValue_;
    return result;
}

inline constexpr fix64 &
operator+=(fix64 & lhs, fix64 rhs)
{
    lhs = lhs + rhs;
    return lhs;
}
    
inline constexpr fix64
operator-(fix64 lhs, fix64 rhs)
{
    fix64 result;
    result.rawValue_ = lhs.rawValue_ - rhs.rawValue_;
    return result;
}

inline constexpr fix64 &
operator-=(fix64 & lhs, fix64 rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline constexpr fix64
operator*(fix64 lhs, fix64 rhs)
{
    // TODO - Overflow check? Use double-wide number for intermediate Value?
    
    fix64 result;
    result.rawValue_ = (lhs.rawValue_ * rhs.rawValue_) >> FIX64::fracBits;
    return result;
}

inline constexpr fix64 &
operator*=(fix64 & lhs, fix64 rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline constexpr fix64
operator/(fix64 lhs, fix64 rhs)
{
    // TODO - Overflow check? Use double-wide number for intermediate Value?
    
    fix64 result;
    result.rawValue_ = (lhs.rawValue_ << FIX64::fracBits) / rhs.rawValue_;
    return result;
}

inline constexpr fix64 &
operator/=(fix64 & lhs, fix64 rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

inline constexpr fix64
operator%(fix64 lhs, fix64 rhs)
{
    fix64 result;
    result.rawValue_ = lhs.rawValue_ % rhs.rawValue_;
    return result;
}

inline constexpr fix64 &
operator%=(fix64 & lhs, fix64 rhs)
{
    lhs = lhs % rhs;
    return lhs;
}

inline constexpr bool
operator==(fix64 lhs, fix64 rhs)
{
    bool result = (lhs.rawValue_ == rhs.rawValue_);
    return result;
}

inline constexpr bool
operator!=(fix64 lhs, fix64 rhs)
{
    bool result = (lhs.rawValue_ != rhs.rawValue_);
    return result;
}

inline constexpr bool
operator>(fix64 lhs, fix64 rhs)
{
    bool result = (lhs.rawValue_ > rhs.rawValue_);
    return result;
}

inline constexpr bool
operator>=(fix64 lhs, fix64 rhs)
{
    bool result = (lhs.rawValue_ >= rhs.rawValue_);
    return result;
}

inline constexpr bool
operator<(fix64 lhs, fix64 rhs)
{
    bool result = (lhs.rawValue_ < rhs.rawValue_);
    return result;
}

inline constexpr bool
operator<=(fix64 lhs, fix64 rhs)
{
    bool result = (lhs.rawValue_ <= rhs.rawValue_);
    return result;
}

// Misc

inline fix64
Truncate(fix64 f)
{
    fix64 result = f;
    result.rawValue_ &= FIX64::intMask;

    return result;
}

inline fix64
Floor(fix64 f)
{
    fix64 result;
    if (f >= 0)     // HMM - branchless way to do this?
    {
        result = Truncate(f);
    }
    else
    {
        result = Truncate(f) - 1;
    }

    return result;
}

inline fix64
Ceil(fix64 f)
{
    fix64 result = Floor(f);
    result += 1;
    return result;
}

inline fix64
Mod(fix64 lhs, fix64 rhs)
{
    Assert(rhs > 0);

    if (lhs.rawValue_ == -51473)
    {
        bool brk = true;
    }

    s32 flooredDivide = FloorF32ToS32((f32)lhs / (f32)rhs);
    fix64 result = lhs - rhs * flooredDivide;

    return result;
}

inline fix64
Square(fix64 Value)
{
    fix64 result = Value * Value;
    return result;
}

inline fix64
Lerp(fix64 a, fix64 b, fix64 t)
{
    fix64 result = ((1 - t) * a) + (t * b);
    return result;
}

inline fix64
Abs(fix64 Value)
{
    if (Value < 0)
        return -Value;

    return Value;
}

inline fix64
SignOf(fix64 Value)
{
    fix64 result = (Value >= 0) ? 1 : -1;
    return result;
}

// NOTE - Bigger default epsilon for fix64 Values than f32 because calculations
//  tend to be a bit imprecise
inline bool
ApproxEq(
    fix64 lhs,
    fix64 rhs,
    fix64 epsilon=0.01f)
{
    fix64 delta = Abs(lhs - rhs);
    bool result = (delta <= epsilon);
    return result;
}

inline fix64
Clamp(fix64 Value, fix64 min, fix64 max)
{
    fix64 result = Value;
    result = Max(result, min);
    result = Min(result, max);
    return result;
}

inline fix64
Clamp01(fix64 Value)
{
    fix64 result = Clamp(Value, 0, 1);
    return result;
}

inline fix64
ClampPlusMinus1(fix64 Value)
{
    fix64 result = Clamp(Value, -1, 1);
    return result;
}

inline fix64
DivideSafeN(fix64 numerator, fix64 denominator, fix64 n)
{
    fix64 result = n;
    if (denominator != 0)
    {
        result = numerator / denominator;
    }
    
    return result;
}

inline fix64
DivideSafe1(fix64 numerator, fix64 denominator)
{
    fix64 result = DivideSafeN(numerator, denominator, 1);
    return result;
}

inline fix64
DivideSafe0(fix64 numerator, fix64 denominator)
{
    fix64 result = DivideSafeN(numerator, denominator, 0);
    return result;
}

// iangle

enum class Degrees : s32 {};

struct iangle
{
    // 0 - 2^fracBits maps to 0 - 360 degrees
    
    u32 rawValue_;

    iangle() = default;
    iangle(RawBits32 v)
    {
        rawValue_ = (u32)v;
    }
    iangle(Degrees deg)
    {
        rawValue_ = ((s32)deg << FIX64::fracBits) / 360;
    }
    explicit iangle(fix64 radians)
    {
        rawValue_ = (u32)(radians / FIX64::twoPi).rawValue_;
    }

    explicit operator fix64()
    {
        fix64 result = fix64((RawBits64)(rawValue_ & FIX64::fracMask));
        result *= FIX64::twoPi;
        return result;
    }
    
    explicit operator f32()
    {
        f32 result = (rawValue_ & FIX64::fracMask) / (f32)FIX64::one.rawValue_;
        result *= F32::twoPi;
        
        return result;
    }
};

namespace IANGLE
{

static const iangle zero((RawBits32)0);
static const iangle twoPi((RawBits32)(1 << FIX64::fracBits));
static const iangle pi((RawBits32)(IANGLE::twoPi.rawValue_ >> 1));
static const iangle halfPi((RawBits32)(IANGLE::pi.rawValue_ >> 1));
static const iangle threeHalvesPi((RawBits32)(IANGLE::halfPi.rawValue_ * 3));

}


inline iangle
operator-(iangle fx)
{
    iangle result;
    result.rawValue_ = -(s32)fx.rawValue_;
    return result;
}
    
inline iangle
operator+(iangle lhs, iangle rhs)
{
    iangle result;
    result.rawValue_ = lhs.rawValue_ + rhs.rawValue_;
    return result;
}

inline iangle &
operator+=(iangle & lhs, iangle rhs)
{
    lhs = lhs + rhs;
    return lhs;
}
    
inline iangle
operator-(iangle lhs, iangle rhs)
{
    iangle result;
    result.rawValue_ = lhs.rawValue_ - rhs.rawValue_;
    return result;
}

inline iangle &
operator-=(iangle & lhs, iangle rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline iangle
operator*(iangle lhs, int rhs)
{
    iangle result;
    result.rawValue_ = (u32)((s32)lhs.rawValue_ * rhs);
    return result;
}

inline iangle
operator*(int lhs, iangle rhs)
{
    iangle result;
    result.rawValue_ = (u32)(lhs * (s32)rhs.rawValue_);
    return result;
}

inline iangle &
operator*=(iangle & lhs, int rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline iangle
operator*(iangle lhs, fix64 rhs)
{
    iangle result;
    result.rawValue_ = (u32)((fix64((RawBits64)lhs.rawValue_) * rhs).rawValue_);
    return result;
}

inline iangle
operator*(fix64 lhs, iangle rhs)
{
    iangle result = rhs * lhs;
    return result;
}

inline iangle &
operator*=(iangle & lhs, fix64 rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline iangle
operator/(iangle lhs, int rhs)
{
    iangle result;
    result.rawValue_ = (u32)((s32)lhs.rawValue_ / rhs);
    return result;
}

inline iangle &
operator/=(iangle & lhs, int rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

#define CONSIDER_EQUIVALENT_IANGLES_EQUAL 1

inline bool
operator==(iangle lhs, iangle rhs)
{
#if CONSIDER_EQUIVALENT_IANGLES_EQUAL
    bool result = (lhs.rawValue_ & FIX64::fracMask) == (rhs.rawValue_ & FIX64::fracMask);
#else
    bool result = (lhs.rawValue_  == rhs.rawValue_);
#endif
    return result;
}

inline bool
operator!=(iangle lhs, iangle rhs)
{
    bool result = !(lhs == rhs);
    return result;
}

inline bool
operator>(iangle lhs, iangle rhs)
{
#if CONSIDER_EQUIVALENT_IANGLES_EQUAL
    bool result = (lhs.rawValue_ & FIX64::fracMask) > (rhs.rawValue_ & FIX64::fracMask);
#else
    bool result = (lhs.rawValue_ > rhs.rawValue_);
#endif
    return result;
}

inline bool
operator>=(iangle lhs, iangle rhs)
{
#if CONSIDER_EQUIVALENT_IANGLES_EQUAL
    bool result = (lhs.rawValue_ & FIX64::fracMask) >= (rhs.rawValue_ & FIX64::fracMask);
#else
    bool result = (lhs.rawValue_ >= rhs.rawValue_);
#endif
    return result;
}

inline bool
operator<(iangle lhs, iangle rhs)
{
#if CONSIDER_EQUIVALENT_IANGLES_EQUAL
    bool result = (lhs.rawValue_ & FIX64::fracMask) < (rhs.rawValue_ & FIX64::fracMask);
#else
    bool result = (lhs.rawValue_ < rhs.rawValue_);
#endif
    return result;
}

inline bool
operator<=(iangle lhs, iangle rhs)
{
#if CONSIDER_EQUIVALENT_IANGLES_EQUAL
    bool result = (lhs.rawValue_ & FIX64::fracMask) <= (rhs.rawValue_ & FIX64::fracMask);
#else
    bool result = (lhs.rawValue_ <= rhs.rawValue_);
#endif
    return result;
}


// Look-up tables / numerical methods

 fix64
Sqrt(fix64 f)
{
    // Newton's Method

    // NOTE - There is a faster implementation floating around there from Graphics Gems V, but I don't really
    //  understand how it works
    
    if (f == 0)
        return 0;

    Assert(f > 0);

    fix64 candidate = 1;
    fix64 candidatePrev;

    // Newton's method until stable
    // TODO - Upper bound on iterations? Acceptable epsilon instead of waiting to stabilize?
    do
    {
        candidatePrev = candidate;
        candidate = 0.5f * (candidate + f / candidate);

        // @HACK - Can get infinite loop if candidate and candidate prev oscillate back and forth.
        //  Abs check should prevent that, but it's gross just like the rest of this function...
    } while (Abs(candidate - candidatePrev).rawValue_ > 1);

    return candidate;
}

#include "fixed_math_lut.h"
inline fix64
Cos(iangle angle)
{
    fix64 result;
    result.rawValue_ = s_iangleCosLut_[angle.rawValue_ & FIX64::fracMask];
    return result;
}

inline fix64
Sin(iangle angle)
{
    iangle angleCosEquivalent = IANGLE::halfPi - angle;
    fix64 result = Cos(angleCosEquivalent);
    return result;
}

inline fix64
Tan(iangle angle)
{
    fix64 result;
    result.rawValue_ = s_iangleTanLut_[angle.rawValue_ & FIX64::fracMask];
    return result;
}

inline iangle
Acos(fix64 Value)
{
    Assert(Value >= -1 && Value <= 1);
    Value  = ClampPlusMinus1(Value);
    Value += 1;
    
    Assert(Value.rawValue_ < ArrayLen(s_iangleAcosLut));
    iangle result;
    result.rawValue_ = s_iangleAcosLut[Value.rawValue_];
    return result;
}

inline iangle
Asin(fix64 Value)
{
    iangle result = IANGLE::halfPi - Acos(Value);
    return result;
}

inline iangle
Atan(fix64 Value)
{
    // Binary search the tan lut between pi / 2 and 3 * pi / 2
    // TODO - have a version of the LUT that is a "level order" version of the balanced
    //  binary tree containing all LUT Values. This would give us great cache locality
    //  as we search http://bannalia.blogspot.com/2015/06/cache-friendly-binary-search.html

    iangle lower = IANGLE::halfPi;
    iangle upper = IANGLE::threeHalvesPi;
    upper.rawValue_ -= 1;

    iangle result;
    while (true)
    {
        iangle candidate = (upper + lower) / 2;
        fix64 candidateValue;
        candidateValue.rawValue_= s_iangleTanLut_[candidate.rawValue_];
        
        if (candidateValue > Value)
        {
            upper.rawValue_ = candidate.rawValue_ - 1;
        }
        else if (candidateValue < Value)
        {
            lower.rawValue_ = candidate.rawValue_ + 1;
        }
        else
        {
            result = candidate;
            break;
        }

        if (lower.rawValue_ > upper.rawValue_)
        {
            result = candidate;
            break;
        }
    }

    // Shift result to between -pi / 2 and pi / 2

    result -= IANGLE::pi;
    return result;
}

inline iangle
Atan2(fix64 y, fix64 x)
{
    iangle result;   
    if (x == 0)
    {
        if (y > 0)
        {
            result = IANGLE::halfPi;
        }
        else if (y < 0)
        {
            result = IANGLE::threeHalvesPi;
        }
        else
        {
            AssertFalse;
            result = IANGLE::zero;
        }
    }
    else
    {
        iangle atanRaw = Atan(y / x);
        if (x > 0)
        {
            result = atanRaw;
        }
        else
        {
            if (y >= 0)
            {
                result = atanRaw + IANGLE::pi;
            }
            else
            {
                result = atanRaw - IANGLE::pi;
            }
        }
    }

    return result;
}

// Vectors

union Vec2x
{
    struct
    {
        fix64 x, y;
    };

    fix64 e[2];

    Vec2x() = default;
    Vec2x(fix64 x, fix64 y)
    {
        this->x = x;
        this->y = y;
    }
    Vec2x(Vec2 v)
    {
        this->x = v.x;
        this->y = v.y;
    }
};

union Vec3x
{
    struct
    {
        fix64 x, y, z;
    };

    struct
    {
        fix64 r, g, b;
    };

    struct
    {
        Vec2x xy;
        fix64 pad0_;
    };

    struct
    {
        fix64 pad1_;
        Vec2x yz;
    };
    
    fix64 e[3];

    Vec3x() = default;
    Vec3x(fix64 x, fix64 y, fix64 z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    Vec3x(Vec2x xy, fix64 z)
    {
        this->x = xy.x;
        this->y = xy.y;
        this->z = z;
    }
    Vec3x(Vec3 v)
    {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
    }
};

union Vec4x
{
    struct
    {
        fix64 x, y, z, w;
    };

    struct
    {
        fix64 r, g, b, a;
    };

    struct
    {
        Vec3x rgb;
        fix64 pad_;
    };

    fix64 e[4];

    Vec4x() = default;
    Vec4x(fix64 x, fix64 y, fix64 z, fix64 w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
    Vec4x(Vec2x xy, fix64 z, fix64 w)
    {
        this->x = xy.x;
        this->y = xy.y;
        this->z = z;
        this->w = w;
    }
    Vec4x(Vec3x xyz, fix64 w)
    {
        this->x = xyz.x;
        this->y = xyz.y;
        this->z = xyz.z;
        this->w = w;
    }
    Vec4x(Vec4 v)
    {
        this->x = v.x;
        this->y = v.x;
        this->z = v.z;
        this->w = v.w;
    }
};

// Vec2x

inline Vec2x
Vec2xFill(fix64 scalar)
{
    Vec2x result(scalar, scalar);
    return result;
}

inline Vec2x
Vec2xMin(Vec2x v0, Vec2x v1)
{
    Vec2x result(Min(v0.x, v1.x), Min(v0.y, v1.y));
    return result;
}

inline Vec2x
Vec2xMin(Vec2x * candidates, int count)
{
    Vec2x result = {};

    if (count > 0)
    {
        result = candidates[0];
    }

    for (int iCandidate = 1; iCandidate < count; iCandidate++)
    {
        result = Vec2xMin(result, candidates[iCandidate]);
    }

    return result;
}

inline Vec2x
Vec2xMax(Vec2x v0, Vec2x v1)
{
    Vec2x result(Max(v0.x, v1.x), Max(v0.y, v1.y));
    return result;
}

inline Vec2x
Vec2xMax(Vec2x * candidates, int count)
{
    Vec2x result =  {};

    if (count > 0)
    {
        result = candidates[0];
    }

    for (int iCandidate = 1; iCandidate < count; iCandidate++)
    {
        result = Vec2xMax(result, candidates[iCandidate]);
    }

    return result;
}

// NOTE - Narrows on 32 bit int systems!
inline Vec2i
Floor(Vec2x v)
{
    Vec2i result;
    result.x = (int)Floor(v.x);
    result.y = (int)Floor(v.y);
    return result;
}

inline Vec2i
Ceil(Vec2x v)
{
    Vec2i result = Floor(v);
    result += Vec2iFill(1);
    return result;
}

inline Vec2x
operator-(Vec2x rhs)
{
    Vec2x result;
    result.x = -rhs.x;
    result.y = -rhs.y;
    return result;
}

inline Vec2x
operator+(Vec2x lhs, Vec2x rhs)
{
    Vec2x result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    return result;
}

inline Vec2x &
operator+=(Vec2x & lhs, Vec2x rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec2x
operator-(Vec2x lhs, Vec2x rhs)
{
    Vec2x result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    return result;
}

inline Vec2x &
operator-=(Vec2x & lhs, Vec2x rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec2x
operator*(Vec2x lhs, fix64 rhs)
{
    Vec2x result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    return result;
}

inline Vec2x
operator*(fix64 lhs, Vec2x rhs)
{
    Vec2x result = rhs * lhs;
    return result;
}

inline Vec2x &
operator*=(Vec2x & lhs, fix64 rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec2x
operator/(Vec2x lhs, fix64 rhs)
{
    Vec2x result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    return result;
}

inline Vec2x &
operator/=(Vec2x & lhs, fix64 rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

inline fix64
Dot(Vec2x v0, Vec2x v1)
{
    fix64 result = v0.x * v1.x + v0.y * v1.y;
    return result;
}

inline Vec2x
Perp(Vec2x v)
{
    Vec2x result;
    result.x = -v.y;
    result.y = v.x;
    return result;
}

inline fix64
LengthSq(Vec2x v)
{
    fix64 result = Dot(v, v);
    return result;
}

inline fix64
Length(Vec2x v)
{
    fix64 result = LengthSq(v);
    result = Sqrt(result);
    return result;
}

inline Vec2x
Lerp(Vec2x a, Vec2x b, fix64 t)
{
    Vec2x result = ((1 - t) * a) + (t * b);
    return result;
}

inline Vec2x
Clamp(Vec2x v, fix64 min, fix64 max)
{
    Vec2x result;
    result.x = Clamp(v.x, min, max);
    result.y = Clamp(v.y, min, max);
    return result;
}

inline Vec2x
Clamp01(Vec2x v)
{
    Vec2x result = Clamp(v, 0, 1);
    return result;
}

inline Vec2x
PrepareForNormalize(Vec2x v)
{
    // @Slow ?
    if (Abs(v.x) + Abs(v.y) < 1.0f)
    {
        // NOTE - Small Values don't have enough precision for the squaring that
        //  happens in the length computation. Scale the vector up first!
    
        v *= 4096;
    }
    
    return v;
}

inline Vec2x
NormalizeUnsafe(Vec2x v)
{
    v = PrepareForNormalize(v);
    
    Vec2x result = v / Length(v);
    return result;
}

inline Vec2x
MakeReasonablyShortUnsafe(Vec2x v)
{
    // Useful for things like raycasts, where we don't necessarily need a normalized vector, but
    //  would like something reasonably short, so we don't have a massive number in a numerator, which
    //  might cause our 52.12 fixed point numbers to lose too much precision

    Vec2x result = v;
    fix64 reasonableLengthSq = Square(5);
    while (LengthSq(result) > reasonableLengthSq)
    {
        result /= 2;
    }

    return result;
}

inline Vec2x
NormalizeSafeOr(Vec2x v, Vec2x fallback)
{
    v = PrepareForNormalize(v);
    
    fix64 length = Length(v);
    Vec2x result = fallback;
    if (length != 0)
    {
        result = v / length;
    }
    return result;
}

inline Vec2x
NormalizeSafe0(Vec2x v)
{
    Vec2x result = NormalizeSafeOr(v, Vec2xFill(0));
    return result;
}

inline Vec2x
NormalizeSafeXAxis(Vec2x v)
{
    Vec2x result = NormalizeSafeOr(v, Vec2x(1, 0));
    return result;
}

inline Vec2x
NormalizeSafeYAxis(Vec2x v)
{
    Vec2x result = NormalizeSafeOr(v, Vec2x(0, 1));
    return result;
}

inline Vec2x
LimitLength(Vec2x v, fix64 maxLength)
{
    Vec2x result;
    
    fix64 lengthSq = LengthSq(v);
    if (lengthSq > Square(maxLength))
    {
        fix64 length = Sqrt(lengthSq);
        result = v * (maxLength / length);
    }
    else
    {
        result = v;
    }

    return result;
}

inline bool
IsNormalized(Vec2x v, fix64 epsilon=0.01f)
{
    bool result = ApproxEq(Length(v), 1.0f, epsilon);
    return result;
}

inline bool
IsZero(Vec2x v)
{
    // TODO - Epsilon version?
    bool result = (v.x == 0 && v.y == 0);
    return result;
}

inline bool
AreOrthogonal(Vec2x v0, Vec2x v1)
{
    fix64 dot = Dot(v0, v1);
    bool result = ApproxEq(dot, 0);
    return result;
}

inline Vec2x
Project(Vec2x projectee, Vec2x onto)
{
    Vec2x result = onto * Dot(projectee, onto) / LengthSq(onto);
    return result;
}

inline Vec2x
ProjectOntoNormalizedAxis(Vec2x projectee, Vec2x onto)
{
    Vec2x result = onto * Dot(projectee, onto);
    return result;
}

inline Vec2x
Reflect(Vec2x reflectee, Vec2x reflectionAxis)
{
    Vec2x result = reflectee - 2 * Project(reflectee, reflectionAxis);
    return result;
}

inline Vec2x
ReflectAcrossNormalizedAxis(Vec2x reflectee, Vec2x normalizedReflectionAxis)
{
    Vec2x result = reflectee - 2 * Dot(reflectee, normalizedReflectionAxis) * normalizedReflectionAxis;
    return result;
}

inline Vec2x
Hadamard(Vec2x v0, Vec2x v1)
{
    Vec2x result;
    result.x = v0.x * v1.x;
    result.y = v0.y * v1.y;
    return result;
}

inline Vec2x
HadamardDivideSafe0(Vec2x v0, Vec2x v1)
{
    Vec2x result;
    result.x = DivideSafe0(v0.x, v1.x);
    result.y = DivideSafe0(v0.y, v1.y);
    return result;
}

// Vec3x

inline Vec3x
Vec3xFill(fix64 scalar)
{
    Vec3x result(scalar, scalar, scalar);
    return result;
}

inline Vec3x
Vec3xMin(Vec3x v0, Vec3x v1)
{
    Vec3x result(Min(v0.x, v1.x), Min(v0.y, v1.y), Min(v0.z, v1.z));
    return result;
}

inline Vec3x
Vec3xMin(Vec3x * candidates, int count)
{
    Vec3x result =  {};

    if (count > 0)
    {
        result = candidates[0];
    }

    for (int iCandidate = 1; iCandidate < count; iCandidate++)
    {
        result = Vec3xMin(result, candidates[iCandidate]);
    }

    return result;
}

inline Vec3x
Vec3xMax(Vec3x v0, Vec3x v1)
{
    Vec3x result(Max(v0.x, v1.x), Max(v0.y, v1.y), Max(v0.z, v1.z));
    return result;
}

inline Vec3x
Vec3xMax(Vec3x * candidates, int count)
{
    Vec3x result =  {};

    if (count > 0)
    {
        result = candidates[0];
    }

    for (int iCandidate = 1; iCandidate < count; iCandidate++)
    {
        result = Vec3xMax(result, candidates[iCandidate]);
    }

    return result;
}

inline Vec3x
operator-(Vec3x rhs)
{
    Vec3x result;
    result.x = -rhs.x;
    result.y = -rhs.y;
    result.z = -rhs.z;
    return result;
}

inline Vec3x
operator+(Vec3x lhs, Vec3x rhs)
{
    Vec3x result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    return result;
}

inline Vec3x &
operator+=(Vec3x & lhs, Vec3x rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec3x
operator-(Vec3x lhs, Vec3x rhs)
{
    Vec3x result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    return result;
}

inline Vec3x &
operator-=(Vec3x & lhs, Vec3x rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec3x
operator*(Vec3x lhs, fix64 rhs)
{
    Vec3x result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    result.z = lhs.z * rhs;
    return result;
}

inline Vec3x
operator*(fix64 lhs, Vec3x rhs)
{
    Vec3x result = rhs * lhs;
    return result;
}

inline Vec3x &
operator*=(Vec3x & lhs, fix64 rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec3x
operator/(Vec3x lhs, fix64 rhs)
{
    Vec3x result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    result.z = lhs.z / rhs;
    return result;
}

inline Vec3x &
operator/=(Vec3x & lhs, fix64 rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

inline fix64
Dot(Vec3x v0, Vec3x v1)
{
    fix64 result = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
    return result;
}

inline fix64
LengthSq(Vec3x v)
{
    fix64 result = Dot(v, v);
    return result;
}

inline fix64
Length(Vec3x v)
{
    fix64 result = LengthSq(v);
    result = Sqrt(result);
    return result;
}

inline Vec3x
Lerp(Vec3x a, Vec3x b, fix64 t)
{
    Vec3x result = ((1 - t) * a) + (t * b);
    return result;
}

inline Vec3x
Clamp(Vec3x v, fix64 min, fix64 max)
{
    Vec3x result;
    result.x = Clamp(v.x, min, max);
    result.y = Clamp(v.y, min, max);
    result.z = Clamp(v.z, min, max);
    return result;
}

inline Vec3x
Clamp01(Vec3x v)
{
    Vec3x result = Clamp(v, 0, 1);
    return result;
}

inline Vec3x
PrepareForNormalize(Vec3x v)
{
    // @Slow ?
    if (Abs(v.x) + Abs(v.y) + Abs(v.z) < 1.0f)
    {
        // NOTE - Small Values don't have enough precision for the squaring that
        //  happens in the length computation. Scale the vector up first!
    
        v *= 4096;
    }
    
    v *= 4096;
    return v;
}

inline Vec3x
NormalizeUnsafe(Vec3x v)
{
    v = PrepareForNormalize(v);
    
    Vec3x result = v / Length(v);
    return result;
}

inline Vec3x
NormalizeSafeOr(Vec3x v, Vec3x fallback)
{
    v = PrepareForNormalize(v);
    
    fix64 length = Length(v);
    Vec3x result = fallback;
    if (length != 0)
    {
        result = v / length;
    }
    return result;
}

inline Vec3x
NormalizeSafe0(Vec3x v)
{
    Vec3x result = NormalizeSafeOr(v, Vec3xFill(0));
    return result;
}

inline Vec3x
NormalizeSafeXAxis(Vec3x v)
{
    Vec3x result = NormalizeSafeOr(v, Vec3x(1, 0, 0));
    return result;
}

inline Vec3x
NormalizeSafeYAxis(Vec3x v)
{
    Vec3x result = NormalizeSafeOr(v, Vec3x(0, 1, 0));
    return result;
}

inline Vec3x
NormalizeSafeZAxis(Vec3x v)
{
    Vec3x result = NormalizeSafeOr(v, Vec3x(0, 0, 1));
    return result;
}

inline bool
IsNormalized(Vec3x v, fix64 epsilon=0.001f)
{
    bool result = ApproxEq(Length(v), 1.0f, epsilon);
    return result;
}

inline bool
IsZero(Vec3x v)
{
    // TODO - Epsilon version?
    bool result = (v.x == 0 && v.y == 0 && v.z == 0);
    return result;
}

inline bool
AreOrthogonal(Vec3x v0, Vec3x v1)
{
    fix64 dot = Dot(v0, v1);
    bool result = ApproxEq(dot, 0);
    return result;
}

inline Vec3x
Project(Vec3x projectee, Vec3x onto)
{
    Vec3x result = onto * Dot(projectee, onto) / LengthSq(onto);
    return result;
}

inline Vec3x
ProjectOntoNormalizedAxis(Vec3x projectee, Vec3x onto)
{
    Vec3x result = onto * Dot(projectee, onto);
    return result;
}

inline Vec3x
Reflect(Vec3x reflectee, Vec3x reflectionAxis)
{
    Vec3x result = reflectee - 2 * Project(reflectee, reflectionAxis);
    return result;
}

inline Vec3x
ReflectAcrossNormalizedAxis(Vec3x reflectee, Vec3x normalizedReflectionAxis)
{
    Vec3x result = reflectee - 2 * Dot(reflectee, normalizedReflectionAxis) * normalizedReflectionAxis;
    return result;
}

inline Vec3x
Hadamard(Vec3x v0, Vec3x v1)
{
    Vec3x result;
    result.x = v0.x * v1.x;
    result.y = v0.y * v1.y;
    result.z = v0.z * v1.z;
    return result;
}

inline Vec3x
HadamardDivideSafe0(Vec3x v0, Vec3x v1)
{
    Vec3x result;
    result.x = DivideSafe0(v0.x, v1.x);
    result.y = DivideSafe0(v0.y, v1.y);
    result.z = DivideSafe0(v0.z, v1.z);
    return result;
}

// Vec4x

inline Vec4x
Vec4xFill(fix64 scalar)
{
    Vec4x result(scalar, scalar, scalar, scalar);
    return result;
}

inline Vec4x
operator-(Vec4x rhs)
{
    Vec4x result;
    result.x = -rhs.x;
    result.y = -rhs.y;
    result.z = -rhs.z;
    result.w = -rhs.w;
    return result;
}

inline Vec4x
operator+(Vec4x lhs, Vec4x rhs)
{
    Vec4x result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    result.w = lhs.w + rhs.w;
    return result;
}

inline Vec4x &
operator+=(Vec4x & lhs, Vec4x rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

inline Vec4x
operator-(Vec4x lhs, Vec4x rhs)
{
    Vec4x result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    result.w = lhs.w - rhs.w;
    return result;
}

inline Vec4x &
operator-=(Vec4x & lhs, Vec4x rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

inline Vec4x
operator*(Vec4x lhs, fix64 rhs)
{
    Vec4x result;
    result.x = lhs.x * rhs;
    result.y = lhs.y * rhs;
    result.z = lhs.z * rhs;
    result.w = lhs.w * rhs;
    return result;
}

inline Vec4x
operator*(fix64 lhs, Vec4x rhs)
{
    Vec4x result = rhs * lhs;
    return result;
}

inline Vec4x &
operator*=(Vec4x & lhs, fix64 rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

inline Vec4x
operator/(Vec4x lhs, fix64 rhs)
{
    Vec4x result;
    result.x = lhs.x / rhs;
    result.y = lhs.y / rhs;
    result.z = lhs.z / rhs;
    result.w = lhs.w / rhs;
    return result;
}

inline Vec4x &
operator/=(Vec4x & lhs, fix64 rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

inline fix64
Dot(Vec4x v0, Vec4x v1)
{
    fix64 result = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z + v0.w * v1.w;
    return result;
}

inline fix64
LengthSq(Vec4x v)
{
    fix64 result = Dot(v, v);
    return result;
}

inline fix64
Length(Vec4x v)
{
    fix64 result = LengthSq(v);
    result = Sqrt(result);
    return result;
}

inline Vec4x
Lerp(Vec4x a, Vec4x b, fix64 t)
{
    Vec4x result = ((1 - t) * a) + (t * b);
    return result;
}

inline Vec4x
Clamp(Vec4x v, fix64 min, fix64 max)
{
    Vec4x result;
    result.x = Clamp(v.x, min, max);
    result.y = Clamp(v.y, min, max);
    result.z = Clamp(v.z, min, max);
    result.w = Clamp(v.w, min, max);
    return result;
}

inline Vec4x
Clamp01(Vec4x v)
{
    Vec4x result = Clamp(v, 0, 1);
    return result;
}

inline Vec4x
PrepareForNormalize(Vec4x v)
{
    // @Slow ?
    if (Abs(v.x) + Abs(v.y) + Abs(v.z) + Abs(v.w) < 1.0f)
    {
        // NOTE - Small Values don't have enough precision for the squaring that
        //  happens in the length computation. Scale the vector up first!
    
        v *= 4096;
    }
    
    v *= 4096;
    return v;
}

inline Vec4x
NormalizeUnsafe(Vec4x v)
{
    v = PrepareForNormalize(v);
    
    Vec4x result = v / Length(v);
    return result;
}

inline Vec4x
NormalizeSafeOr(Vec4x v, Vec4x fallback)
{
    v = PrepareForNormalize(v);
    
    fix64 length = Length(v);
    Vec4x result = fallback;
    if (length != 0)
    {
        result = v / length;
    }
    return result;
}

inline Vec4x
NormalizeSafe0(Vec4x v)
{
    Vec4x result = NormalizeSafeOr(v, Vec4xFill(0));
    return result;
}

inline Vec4x
NormalizeSafeXAxis(Vec4x v)
{
    Vec4x result = NormalizeSafeOr(v, Vec4x(1, 0, 0, 0));
    return result;
}

inline Vec4x
NormalizeSafeYAxis(Vec4x v)
{
    Vec4x result = NormalizeSafeOr(v, Vec4x(0, 1, 0, 0));
    return result;
}

inline Vec4x
NormalizeSafeZAxis(Vec4x v)
{
    Vec4x result = NormalizeSafeOr(v, Vec4x(0, 0, 1, 0));
    return result;
}

inline Vec4x
NormalizeSafeWAxis(Vec4x v)
{
    Vec4x result = NormalizeSafeOr(v, Vec4x(0, 0, 0, 1));
    return result;
}

inline bool
IsNormalized(Vec4x v, fix64 epsilon=0.001f)
{
    bool result = ApproxEq(Length(v), 1.0f, epsilon);
    return result;
}

inline bool
IsZero(Vec4x v)
{
    // TODO - Epsilon version?
    bool result = (v.x == 0 && v.y == 0 && v.z == 0 && v.w == 0);
    return result;
}

inline bool
AreOrthogonal(Vec4x v0, Vec4x v1)
{
    fix64 dot = Dot(v0, v1);
    bool result = ApproxEq(dot, 0);
    return result;
}

// Ray

struct Ray2x
{
    Vec2x p0;
    Vec2x dir;

    Ray2x() = default;
    Ray2x(Vec2x p0, Vec2x dir)
    {
        this->p0 = p0;
        this->dir = dir;
    }
};

Vec2x PosAtT(Ray2x ray, fix64 t)
{
    Vec2x result = ray.p0 + ray.dir * t;
    return result;
}

struct Ray3x
{
    Vec3x p0;
    Vec3x dir;

    Ray3x() = default;
    Ray3x(Vec3x p0, Vec3x dir)
    {
        this->p0 = p0;
        this->dir = dir;
    }
    explicit Ray3x(Ray2x ray)
    {
        this->p0 = Vec3x(ray.p0, 0);
        this->dir = Vec3x(ray.dir, 0);
    }
};

Vec3x PosAtT(Ray3x ray, fix64 t)
{
    Vec3x result = ray.p0 + ray.dir * t;
    return result;
}

// Rects

struct Rect2x
{
    Vec2x min;
    Vec2x max;
};

struct Rect3x
{
    Vec3x min;
    Vec3x max;
};

// Rect2x

inline Rect2x
RectFromMinAndMax(Vec2x min, Vec2x max)
{
    Rect2x result;
    result.min = min;
    result.max = max;
    return result;
}

inline Rect2x
RectFromPoints(Vec2x point0, Vec2x point1)
{
    Rect2x result;
    result.min = Vec2xMin(point0, point1);
    result.max = Vec2xMax(point0, point1);
    return result;
}

inline Rect2x
RectFromCenterHalfDim(Vec2x center, Vec2x halfDim)
{
    Rect2x result;
    result.min = center - halfDim;
    result.max = center + halfDim;
    return result;
}

inline Rect2x
RectFromCenterHalfDim(Vec2x center, fix64 halfDim)
{
    return RectFromCenterHalfDim(center, Vec2xFill(halfDim));
}

inline Rect2x
RectFromCenterDim(Vec2x center, Vec2x dim)
{
    Rect2x result = RectFromCenterHalfDim(center, dim * 0.5f);
    return result;
}

inline Rect2x
RectFromCenterDim(Vec2x center, fix64 dim)
{
    return RectFromCenterDim(center, Vec2xFill(dim));
}

inline Rect2x
RectFromMinDim(Vec2x min, Vec2x dim)
{
    Rect2x result;
    result.min = min;
    result.max = min + dim;
    return result;
}

inline Rect2x
RectFromMinDim(Vec2x min, fix64 dim)
{
    Rect2x result;
    result.min = min;
    result.max = min + Vec2xFill(dim);
    return result;
}

inline Rect2x
RectFromMaxDim(Vec2x max, Vec2x dim)
{
    Rect2x result;
    result.min = max - dim;
    result.max = max;
    return result;
}

inline Rect2x
RectFromMaxDim(Vec2x max, fix64 dim)
{
    Rect2x result;
    result.min = max - Vec2xFill(dim);
    result.max = max;
    return result;
}

inline Rect2x
RectFromRectAndMargin(Rect2x original, fix64 margin)
{
    Rect2x result = original;
    result.min -= Vec2xFill(margin);
    result.max += Vec2xFill(margin);
    return result;
}

inline Rect2x
RectFromRectAndMargin(Rect2x original, Vec2x margin)
{
    Rect2x result = original;
    result.min -= margin;
    result.max += margin;
    return result;
}

inline Rect2x
RectFromRectAndOffset(Rect2x original, fix64 offset)
{
    Rect2x result;
    result.min = original.min + Vec2xFill(offset);
    result.max = original.max + Vec2xFill(offset);
    return result;
}

inline Rect2x
RectFromRectAndOffset(Rect2x original, Vec2x offset)
{
    Rect2x result;
    result.min = original.min + offset;
    result.max = original.max + offset;
    return result;
}

inline Vec2x
GetCenter(Rect2x rect)
{
    Vec2x result = 0.5f * (rect.min + rect.max);
    return result;
}

inline Vec2x
GetDim(Rect2x rect)
{
    Vec2x result = rect.max - rect.min;
    return result;
}

inline bool
IsDimZeroOrNegative(Rect2x rect)
{
    bool result = (rect.max.x <= rect.min.x) || (rect.max.y <= rect.min.y);
    return result;
}

inline bool
TestPointInRect(Rect2x rect, Vec2x testPoint)
{
    bool result =
        testPoint.x >= rect.min.x &&
        testPoint.y >= rect.min.y &&
        testPoint.x < rect.max.x &&
        testPoint.y < rect.max.y;

    return result;
}

inline bool
TestRectOverlapsRect(Rect2x rect0, Rect2x rect1)
{
    bool result = !((rect0.min.x >= rect1.max.x) ||
                    (rect0.max.x <= rect1.min.x) ||
                    (rect0.min.y >= rect1.max.y) ||
                    (rect0.max.y <= rect1.min.y));

    return result;
}
    
// Rect3

inline Rect3x
RectFromMinAndMax(Vec3x min, Vec3x max)
{
    Rect3x result;
    result.min = min;
    result.max = max;
    return result;
}

inline Rect3x
RectFromPoints(Vec3x point0, Vec3x point1)
{
    Rect3x result;
    result.min = Vec3xMin(point0, point1);
    result.max = Vec3xMax(point0, point1);
    return result;
}

inline Rect3x
RectFromCenterHalfDim(Vec3x center, Vec3x halfDim)
{
    Rect3x result;
    result.min = center - halfDim;
    result.max = center + halfDim;
    return result;
}

inline Rect3x
RectFromCenterDim(Vec3x center, Vec3x dim)
{
    Rect3x result = RectFromCenterHalfDim(center, dim * 0.5f);
    return result;
}

inline Rect3x
RectFromMinDim(Vec3x min, Vec3x dim)
{
    Rect3x result;
    result.min = min;
    result.max = min + dim;
    return result;
}

inline Rect3x
RectFromRectAndMargin(Rect3x original, fix64 margin)
{
    Rect3x result = original;
    result.min -= Vec3xFill(margin);
    result.max += Vec3xFill(margin);
    return result;
}

inline Rect3x
RectFromMaxDim(Vec3x max, Vec3x dim)
{
    Rect3x result;
    result.min = max - dim;
    result.max = max;
    return result;
}

inline Rect3x
RectFromMaxDim(Vec3x max, fix64 dim)
{
    Rect3x result;
    result.min = max - Vec3xFill(dim);
    result.max = max;
    return result;
}

inline Rect3x
RectFromRectAndMargin(Rect3x original, Vec3x margin)
{
    Rect3x result = original;
    result.min -= margin;
    result.max += margin;
    return result;
}

inline Rect3x
RectFromRectAndOffset(Rect3x original, fix64 offset)
{
    Rect3x result;
    result.min = original.min + Vec3xFill(offset);
    result.max = original.max + Vec3xFill(offset);
    return result;
}

inline Rect3x
RectFromRectAndOffset(Rect3x original, Vec3x offset)
{
    Rect3x result;
    result.min = original.min + offset;
    result.max = original.max + offset;
    return result;
}

inline Vec3x
GetCenter(Rect3x rect)
{
    Vec3x result = 0.5f * (rect.min + rect.max);
    return result;
}

inline Vec3x
GetDim(Rect3x rect)
{
    Vec3x result = rect.max - rect.min;
    return result;
}

inline bool
IsDimZeroOrNegative(Rect3x rect)
{
    bool result = (rect.max.x <= rect.min.x) || (rect.max.y <= rect.min.y) || (rect.max.z <= rect.min.z);
    return result;
}

inline bool
TestPointInRect(Rect3x rect, Vec3x testPoint)
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
TestRectOverlapsRect(Rect3x rect0, Rect3x rect1)
{
    bool result = !((rect0.min.x >= rect1.max.x) ||
                    (rect0.max.x <= rect1.min.x) ||
                    (rect0.min.y >= rect1.max.y) ||
                    (rect0.max.y <= rect1.min.y) ||
                    (rect0.min.z >= rect1.max.z) ||
                    (rect0.max.z <= rect1.min.z));
                   
    return result;
}

// Circle

struct Circle2x
{
    Vec2x center;
    fix64 radius;
};

inline Circle2x
CircleFromCenterRad(Vec2x center, fix64 radius)
{
    Circle2x result;
    result.center = center;
    result.radius = radius;
    return result;
}

inline bool
TestRectOverlapsCircle(Rect2x rect, Circle2x circle)
{
    Vec2x rectClosest = circle.center;

    if (circle.center.x < rect.min.x)
        rectClosest.x = rect.min.x;
    else if (circle.center.x > rect.max.x)
        rectClosest.x = rect.max.x;

    if (circle.center.y < rect.min.y)
        rectClosest.y = rect.min.y;
    else if (circle.center.y > rect.max.y)
        rectClosest.y = rect.max.y;

    if (LengthSq(circle.center - rectClosest) <= Square(circle.radius))
        return true;

    return false;
}

inline bool
TestCircleOverlapsCircle(Circle2x c0, Circle2x c1)
{
    fix64 distSq = LengthSq(c0.center - c1.center);
    bool result = distSq < Square(c0.radius + c1.radius);
    return result;
}

struct CircleOverlapTestResultX
{
    bool overlaps;
    Vec2x penetration;
};

inline CircleOverlapTestResultX
FullTestCircleOverlapsCircle(Circle2x c0, Circle2x c1)
{
    CircleOverlapTestResultX result = {};
    fix64 distSq = LengthSq(c0.center - c1.center);
    if (distSq < Square(c0.radius + c1.radius))
    {
        result.overlaps = true;

        Vec2x displacementDir = NormalizeSafeXAxis(c0.center - c1.center);
        fix64 displacementDist = c0.radius + c1.radius - Sqrt(distSq);
        result.penetration = displacementDir * displacementDist;
    }
    
    return result;
}

#endif
