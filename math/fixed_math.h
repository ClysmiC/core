namespace fxp
{
// TODO - consteval gives best compile-time determinism guarantee for converting float values to fixed values, but requires C++20.
#if (__cplusplus >= 202002L)
 #define FXPCONSTEVAL consteval
#else
 #define FXPCONSTEVAL constexpr
#endif

// --- A unitless fixed point value

template<class> struct integer_type { static bool constexpr is_supported = false; };
template<> struct integer_type<i32> { static bool constexpr is_supported = true; };
template<> struct integer_type<i64> { static bool constexpr is_supported = true; };
// TODO - support u32 and u64? Any extra work required?

using denom_t = i64;            // Compile-time calculations with denominators are in 64 bit

template<class T, denom_t D>
struct Value
{
    STATIC_ASSERT(D >= 1);
    STATIC_ASSERT(integer_type<T>::is_supported);

    static denom_t constexpr d = D;
    T n;

    // --- Implicitly convert from...

    constexpr Value() = default;

    template <class T_OTHER>
    constexpr Value(Value<T_OTHER, D> const& v) : n(T(v.n)) {}
    constexpr Value(i8 v)   : n(T(v * D)) {}
    constexpr Value(i16 v)  : n(T(v * D)) {}
    constexpr Value(i32 v)  : n(T(v * D)) {}
    constexpr Value(i64 v)  : n(T(v * D)) {}
    constexpr Value(u8 v)   : n(T(v * D)) {}
    constexpr Value(u16 v)  : n(T(v * D)) {}
    constexpr Value(u32 v)  : n(T(v * D)) {}
    constexpr Value(u64 v)  : n(T(v * D)) {}

    FXPCONSTEVAL Value(f32 v)   : n(T(v * D)) {}
    FXPCONSTEVAL Value(f64 v)   : n(T(v * D)) {}

    // --- Explicitly convert from...

    template <class T_OTHER, denom_t D_OTHER>
    explicit constexpr Value(Value<T_OTHER, D_OTHER> const& v) : n(T(v.n * D / D_OTHER)) {}

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

// All intermediate calculations are 64 bit.
template <denom_t D> using Intermediate = Value<i64, D>;

// --- Math operations
//  Binary operators work on 2 fixed point values and return a value with denominator specified by D_RESULT.
//  There are potentially 3 different denominators at play here but common terms are cancelled out at compile time
//  when applicable.

template<denom_t D_RESULT, class T0, denom_t D0, class T1, denom_t D1>
Intermediate<D_RESULT> constexpr
add(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    Intermediate<D0> i0(v0);
    Intermediate<D1> i1(v1);
    Intermediate<D_RESULT> result;

    // Cancel equal terms at compile-time
    if      constexpr (D0 == D_RESULT && D1 == D_RESULT)    result.n = i0.n + i1.n;
    else if constexpr (D0 == D_RESULT)                      result.n = i0.n + (D_RESULT * i1.n / D1);
    else if constexpr (D1 == D_RESULT)                      result.n = (D_RESULT * i0.n / D0) + i1.n;
    else if constexpr (D0 == D1)                            result.n = (D_RESULT * (i0.n + i1.n)) / D0;
    else                                                    result.n = (D_RESULT * i0.n) / D0 + (D_RESULT * i1.n) / D1;

    return result;
}

template<denom_t D_RESULT, class T0, denom_t D0, class T1, denom_t D1>
Intermediate<D_RESULT> constexpr
subtract(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    Intermediate<D0> i0(v0);
    Intermediate<D1> i1(v1);
    Intermediate<D_RESULT> result;

    // Cancel equal terms at compile-time
    if      constexpr (D0 == D_RESULT && D1 == D_RESULT)    result.n = i0.n - i1.n;
    else if constexpr (D0 == D_RESULT)                      result.n = i0.n - (D_RESULT * i1.n / D1);
    else if constexpr (D1 == D_RESULT)                      result.n = (D_RESULT * i0.n / D0) - i1.n;
    else if constexpr (D0 == D1)                            result.n = (D_RESULT * (i0.n - i1.n)) / D0;
    else                                                    result.n = (D_RESULT * i0.n) / D0 - (D_RESULT * i1.n) / D1;

    return result;
}

template<denom_t D_RESULT, class T0, denom_t D0, class T1, denom_t D1>
Intermediate<D_RESULT> constexpr
multiply(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    Intermediate<D0> i0(v0);
    Intermediate<D1> i1(v1);
    Intermediate<D_RESULT> result;

    // Cancel equal terms at compile-time
    if      constexpr (D0 == D_RESULT)  result.n = (i0.n * i1.n) / D1;
    else if constexpr (D1 == D_RESULT)  result.n = (i0.n * i1.n) / D0;
    // NOTE - Overflowing is a possible concern in this case, if the denominators are huge.
    else                                result.n = (D_RESULT * i0.n * i1.n) / (D0 * D1);

    return result;
}

template<denom_t D_RESULT, class T0, denom_t D0, class T1, denom_t D1>
Intermediate<D_RESULT> constexpr
divide(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    Intermediate<D0> i0(v0);
    Intermediate<D1> i1(v1);
    Intermediate<D_RESULT> result;

    // Cancel equal terms at compile-time
    if      constexpr (D0 == D_RESULT)  result.n = (D1 * i0.n) / i1.n;
    else if constexpr (D0 == D1)        result.n = (D_RESULT * i0.n) / i1.n;
    // NOTE - Overflowing is a possible concern in this case, if the denominators are huge.
    else                                result.n = (D_RESULT * D1 * i0.n) / (D0 * i1.n);

    return result;
}

template<class T0, denom_t D0, class T1, denom_t D1>
bool constexpr
operator==(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    bool result;
    if constexpr (D0 >= D1) { using I = Intermediate<D0>; result = (I(v0).n == I(v1).n); }
    else                    { using I = Intermediate<D1>; result = (I(v0).n == I(v1).n); }
    return result;
}

template<class T0, denom_t D0, class T1, denom_t D1>
bool constexpr
operator!=(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    bool result;
    if constexpr (D0 >= D1) { using I = Intermediate<D0>; result = (I(v0).n != I(v1).n); }
    else                    { using I = Intermediate<D1>; result = (I(v0).n != I(v1).n); }
    return result;
}

template<class T0, denom_t D0, class T1, denom_t D1>
bool constexpr
operator>(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    bool result;
    if constexpr (D0 >= D1) { using I = Intermediate<D0>; result = (I(v0).n > I(v1).n); }
    else                    { using I = Intermediate<D1>; result = (I(v0).n > I(v1).n); }
    return result;
}

template<class T0, denom_t D0, class T1, denom_t D1>
bool constexpr
operator>=(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    bool result;
    if constexpr (D0 >= D1) { using I = Intermediate<D0>; result = (I(v0).n >= I(v1).n); }
    else                    { using I = Intermediate<D1>; result = (I(v0).n >= I(v1).n); }
    return result;
}

template<class T0, denom_t D0, class T1, denom_t D1>
bool constexpr
operator<(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    bool result;
    if constexpr (D0 >= D1) { using I = Intermediate<D0>; result = (I(v0).n < I(v1).n); }
    else                    { using I = Intermediate<D1>; result = (I(v0).n < I(v1).n); }
    return result;
}

template<class T0, denom_t D0, class T1, denom_t D1>
bool constexpr
operator<=(
    Value<T0, D0> v0,
    Value<T1, D1> v1)
{
    bool result;
    if constexpr (D0 >= D1) { using I = Intermediate<D0>; result = (I(v0).n <= I(v1).n); }
    else                    { using I = Intermediate<D1>; result = (I(v0).n <= I(v1).n); }
    return result;
}

// --- Fixed point "quantities" of a specific unit (e.g., meters, seconds).
//  User code interacts with these, and should never really need to directly interact with a fxp::Value.
//  A few default units are defined, as well as conversions between them. Users can extend these
//  by adding their own Product<..> and Quotient<..> specializations. The underlying value types and
//  denominators can be specified/overridden by adding a Unit<..> specialization.

enum class Si_Unit_Type : u64
{
    SCALAR = 0,     // unitless fixed point
    // SCALAR_INT,     // unitless integer (fixed point with divisor 1)

    DISTANCE,
    ROTATIONS,
    TIME,

    SPEED,
    ACCELERATION,

    ROTATION_SPEED,
};

// Note there is no Value64 defined. Users should prefer to use Value32 for their unit types.
//  This makes you less likely to overflow in intermediate calculations. Although you still can
//  if you have large denominators and do big multiplications.
template <denom_t D> using Value32 = Value<i32, D>;

// Default underlying type of a unit.
// Can be overridden with a Unit<..> specialization
template<Si_Unit_Type> struct Si_Unit { using Value = Value32<1024>; };

// --- Conversions between unit types
template<Si_Unit_Type, Si_Unit_Type> struct Quotient;
template<Si_Unit_Type, Si_Unit_Type> struct Product;

// Conversions
template<> struct Quotient<Si_Unit_Type::DISTANCE, Si_Unit_Type::TIME>        { static auto constexpr RESULT = Si_Unit_Type::SPEED; };
template<> struct Quotient<Si_Unit_Type::ROTATIONS, Si_Unit_Type::TIME>       { static auto constexpr RESULT = Si_Unit_Type::ROTATION_SPEED; };
template<> struct Quotient<Si_Unit_Type::SPEED, Si_Unit_Type::TIME>           { static auto constexpr RESULT = Si_Unit_Type::ACCELERATION; };

template<> struct Product<Si_Unit_Type::SPEED, Si_Unit_Type::TIME>            { static auto constexpr RESULT = Si_Unit_Type::DISTANCE; };
template<> struct Product<Si_Unit_Type::TIME, Si_Unit_Type::SPEED>            { static auto constexpr RESULT = Si_Unit_Type::DISTANCE; };
template<> struct Product<Si_Unit_Type::ACCELERATION, Si_Unit_Type::TIME>     { static auto constexpr RESULT = Si_Unit_Type::SPEED; };
template<> struct Product<Si_Unit_Type::TIME, Si_Unit_Type::ACCELERATION>     { static auto constexpr RESULT = Si_Unit_Type::SPEED; };
template<> struct Product<Si_Unit_Type::ROTATION_SPEED, Si_Unit_Type::TIME>   { static auto constexpr RESULT = Si_Unit_Type::ROTATIONS; };
template<> struct Product<Si_Unit_Type::TIME, Si_Unit_Type::ROTATION_SPEED>   { static auto constexpr RESULT = Si_Unit_Type::ROTATIONS; };

// Scalar operations
template<> struct Quotient<Si_Unit_Type::SCALAR, Si_Unit_Type::SCALAR> { static auto constexpr RESULT = Si_Unit_Type::SCALAR; };
template<> struct Product<Si_Unit_Type::SCALAR, Si_Unit_Type::SCALAR> { static auto constexpr RESULT = Si_Unit_Type::SCALAR; };
template<Si_Unit_Type UNIT_TYPE> struct Quotient<UNIT_TYPE, Si_Unit_Type::SCALAR> { static auto constexpr RESULT = UNIT_TYPE; };
template<Si_Unit_Type UNIT_TYPE> struct Quotient<Si_Unit_Type::SCALAR, UNIT_TYPE> { static auto constexpr RESULT = UNIT_TYPE; };
template<Si_Unit_Type UNIT_TYPE> struct Product<UNIT_TYPE, Si_Unit_Type::SCALAR> { static auto constexpr RESULT = UNIT_TYPE; };
template<Si_Unit_Type UNIT_TYPE> struct Product<Si_Unit_Type::SCALAR, UNIT_TYPE> { static auto constexpr RESULT = UNIT_TYPE; };

template<Si_Unit_Type UNIT_TYPE>
struct Quantity
{
    using Value = typename Si_Unit<UNIT_TYPE>::Value;
    static Si_Unit_Type constexpr si_unit_type = UNIT_TYPE;

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

    template<Si_Unit_Type OTHER_UNIT_TYPE>
    constexpr Quantity(Quantity<OTHER_UNIT_TYPE> const& v) : value(v.value)
    {
        STATIC_ASSERT(UNIT_TYPE == OTHER_UNIT_TYPE ||
                     UNIT_TYPE == Si_Unit_Type::SCALAR ||
                     OTHER_UNIT_TYPE == Si_Unit_Type::SCALAR);
    }

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



// --- Quantity aliases. Users should use these.

using Scalar            = Quantity<Si_Unit_Type::SCALAR>;
using Distance          = Quantity<Si_Unit_Type::DISTANCE>;
using Rotations         = Quantity<Si_Unit_Type::ROTATIONS>;
using Time              = Quantity<Si_Unit_Type::TIME>;
using Speed             = Quantity<Si_Unit_Type::SPEED>;
using Acceleration      = Quantity<Si_Unit_Type::ACCELERATION>;
using Rotation_Speed    = Quantity<Si_Unit_Type::ROTATION_SPEED>;



// --- Quantities with same units can always add/subtract together

template<Si_Unit_Type UNIT>
Quantity<UNIT> constexpr
operator+(Quantity<UNIT> lhs, Quantity<UNIT> rhs)
{
    Quantity<UNIT> result;
    result.value = add<Quantity<UNIT>::Value::d>(lhs.value, rhs.value);
    return result;
}
template<Si_Unit_Type UNIT>
Quantity<UNIT> constexpr&
operator+=(Quantity<UNIT>& lhs, Quantity<UNIT> rhs)
{
    lhs = lhs + rhs;
    return lhs;
}
template<Si_Unit_Type UNIT>
Quantity<UNIT> constexpr
operator-(Quantity<UNIT> lhs, Quantity<UNIT> rhs)
{
    Quantity<UNIT> result;
    result.value = subtract<Quantity<UNIT>::Value::d>(lhs.value, rhs.value);
    return result;
}
template<Si_Unit_Type UNIT>
Quantity<UNIT> constexpr&
operator-=(Quantity<UNIT>& lhs, Quantity<UNIT> rhs)
{
    lhs = lhs - rhs;
    return lhs;
}



// --- Quantities with same units can compare for equality, less than, greater than.

template<Si_Unit_Type UNIT>
bool constexpr
operator==(Quantity<UNIT> lhs, Quantity<UNIT> rhs)
{
    bool result = (lhs.value == rhs.value);
    return result;
}

template<Si_Unit_Type UNIT>
bool constexpr
operator!=(Quantity<UNIT> lhs, Quantity<UNIT> rhs)
{
    bool result = (lhs.value != rhs.value);
    return result;
}

template<Si_Unit_Type UNIT>
bool constexpr
operator>(Quantity<UNIT> lhs, Quantity<UNIT> rhs)
{
    bool result = (lhs.value > rhs.value);
    return result;
}

template<Si_Unit_Type UNIT>
bool constexpr
operator>=(Quantity<UNIT> lhs, Quantity<UNIT> rhs)
{
    bool result = (lhs.value >= rhs.value);
    return result;
}

template<Si_Unit_Type UNIT>
bool constexpr
operator<(Quantity<UNIT> lhs, Quantity<UNIT> rhs)
{
    bool result = (lhs.value < rhs.value);
    return result;
}

template<Si_Unit_Type UNIT>
bool constexpr
operator<=(Quantity<UNIT> lhs, Quantity<UNIT> rhs)
{
    bool result = (lhs.value <= rhs.value);
    return result;
}



// --- Quantities with same units can always divide and get a scalar result

template<Si_Unit_Type UNIT>
Scalar constexpr
operator/(Quantity<UNIT> lhs, Quantity<UNIT> rhs)
{
    Scalar result;
    result.value = divide<Scalar::Value::d>(lhs.value, rhs.value);
    return result;
}
template<Si_Unit_Type UNIT>
Quantity<UNIT> constexpr&
operator/=(Quantity<UNIT>& lhs, Quantity<UNIT> rhs)
{
    lhs = lhs / rhs;
    return lhs;
}



// --- Quantities can always multiply and divide by a scalar

// template<Si_Unit_Type UNIT>
// Quantity<UNIT> constexpr
// operator*(Quantity<UNIT> const& lhs, Scalar const& rhs)
// {
//     Quantity<UNIT> result;
//     result.value = multiply<Quantity<UNIT>::Value::d>(lhs.value, rhs.value);
//     return result;
// }
// template<Si_Unit_Type UNIT>
// Quantity<UNIT> constexpr
// operator*(Scalar const& lhs, Quantity<UNIT> const& rhs)
// {
//     Quantity<UNIT> result;
//     result.value = multiply<Quantity<UNIT>::Value::d>(lhs.value, rhs.value);
//     return result;
// }
// template<Si_Unit_Type UNIT>
// Quantity<UNIT> constexpr&
// operator*=(Quantity<UNIT>& lhs, Scalar const& rhs)
// {
//     lhs = lhs * rhs;
//     return lhs;
// }

// template<Si_Unit_Type UNIT>
// Quantity<UNIT> constexpr
// operator/(Quantity<UNIT> const& lhs, Scalar const& rhs)
// {
//     Quantity<UNIT> result;
//     result.value = divide<Quantity<UNIT>::Value::d>(lhs.value, rhs.value);
//     return result;
// }
// template<Si_Unit_Type UNIT>
// Quantity<UNIT> constexpr&
// operator/=(Quantity<UNIT>& lhs, Scalar const& rhs)
// {
//     lhs = lhs / rhs;
//     return lhs;
// }



// --- Any 2 quantities whose units have Product/Quotient defined can multiply/divide

template<Si_Unit_Type UNIT0, Si_Unit_Type UNIT1>
auto constexpr
operator*(Quantity<UNIT0> lhs, Quantity<UNIT1> rhs)
{
    static Si_Unit_Type constexpr RESULT_TYPE = Product<UNIT0, UNIT1>::RESULT;
    static Si_Unit_Type constexpr RESULT_TYPE_OPPOSITE = Product<UNIT1, UNIT0>::RESULT;
    STATIC_ASSERT(RESULT_TYPE == RESULT_TYPE_OPPOSITE);      // User should define both A*B and B*A to produce the same result unit

    // TODO - This should widen out to a 64-bit intermediate quantity like Value does...
    //  narrowing to 32 bits in this operator defeats the purpose of the 64 bit intermediates.
    Quantity<RESULT_TYPE> result;
    result.value = multiply<Quantity<RESULT_TYPE>::Value::d>(lhs.value, rhs.value);
    return result;
}

template<Si_Unit_Type UNIT0, Si_Unit_Type UNIT1>
auto constexpr
operator*=(Quantity<UNIT0>& lhs, Quantity<UNIT1> rhs)
{
    static Si_Unit_Type constexpr RESULT_TYPE = Product<UNIT0, UNIT1>::RESULT;
    static Si_Unit_Type constexpr RESULT_TYPE_OPPOSITE = Product<UNIT1, UNIT0>::RESULT;
    STATIC_ASSERT(RESULT_TYPE == RESULT_TYPE_OPPOSITE);      // User should define both A*B and B*A to produce the same result unit
    STATIC_ASSERT(RESULT_TYPE == UNIT0);

    // TODO - This should widen out to a 64-bit intermediate quantity like Value does...
    //  narrowing to 32 bits in this operator defeats the purpose of the 64 bit intermediates.
    lhs = lhs * rhs;
    return lhs;
}

template<Si_Unit_Type UNIT0, Si_Unit_Type UNIT1>
auto constexpr
operator/(Quantity<UNIT0> lhs, Quantity<UNIT1> rhs)
{
    static Si_Unit_Type constexpr RESULT_TYPE = Quotient<UNIT0, UNIT1>::RESULT;
    Quantity<RESULT_TYPE> result;
    result.value = divide<Quantity<RESULT_TYPE>::Value::d>(lhs.value, rhs.value);
    return result;
}

template<Si_Unit_Type UNIT0, Si_Unit_Type UNIT1>
auto constexpr
operator/=(Quantity<UNIT0>& lhs, Quantity<UNIT1> rhs)
{
    static Si_Unit_Type constexpr RESULT_TYPE = Quotient<UNIT0, UNIT1>::RESULT;
    static Si_Unit_Type constexpr RESULT_TYPE_OPPOSITE = Quotient<UNIT1, UNIT0>::RESULT;
    STATIC_ASSERT(RESULT_TYPE == RESULT_TYPE_OPPOSITE);      // User should define both A*B and B*A to produce the same result unit
    STATIC_ASSERT(RESULT_TYPE == UNIT0);

    lhs = lhs / rhs;
    return lhs;
}

#undef FXPCONSTEVAL

} // namespace fxp
