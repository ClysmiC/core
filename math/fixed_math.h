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

using denom_t = i32;

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

    // template <class T_OTHER, denom_t D_OTHER>
    // explicit constexpr Value(Value<T_OTHER, D_OTHER> const& v) : n(T(v.n * D / D_OTHER)) {}

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
// template <denom_t D> using intermediate_t = Value<i64, D>;

// --- Math operations

template<class T, denom_t D>
Value<T, D> constexpr
operator+(
    Value<T, D> v0,
    Value<T, D> v1)
{
    Value<T, D> result;
    result.n = v0.n + v1.n;
    return result;
}

template<class T, denom_t D>
Value<T, D> constexpr&
operator+=(
    Value<T, D>& v0,
    Value<T, D> v1)
{
    v0 = v0 + v1;
    return v0;
}

template<class T, denom_t D>
Value<T, D> constexpr
operator-(
    Value<T, D> v0,
    Value<T, D> v1)
{
    Value<T, D> result;
    result.n = v0.n - v1.n;
    return result;
}

template<class T, denom_t D>
Value<T, D> constexpr&
operator-=(
    Value<T, D>& v0,
    Value<T, D> v1)
{
    v0 = v0 - v1;
    return v0;
}

template<class T, denom_t D>
Value<T, D> constexpr
operator*(
    Value<T, D> v0,
    Value<T, D> v1)
{
    Value<T, D> result;
    // Intermediate calculation is 64 bit to mitigate overflows
    i64 n = (i64)v0.n * (i64)v1.n;
    n /= D;
    result.n = (T)n;
    return result;
}

template<class T, denom_t D>
Value<T, D> constexpr&
operator*=(
    Value<T, D>& v0,
    Value<T, D> v1)
{
    v0 = v0 * v1;
    return v0;
}

template<class T, denom_t D>
Value<T, D> constexpr
operator/(
    Value<T, D> v0,
    Value<T, D> v1)
{
    Value<T, D> result;
    result.n = (D * v0.n) / v1.n;
    return result;
}

template<class T, denom_t D>
Value<T, D> constexpr&
operator/=(
    Value<T, D>& v0,
    Value<T, D> v1)
{
    v0 = v0 / v1;
    return v0;
}

template<class T, denom_t D>
bool constexpr
operator==(
    Value<T, D> v0,
    Value<T, D> v1)
{
    bool result = (v0.n == v1.n);
    return result;
}

template<class T, denom_t D>
bool constexpr
operator!=(
    Value<T, D> v0,
    Value<T, D> v1)
{
    bool result = (v0.n != v1.n);
    return result;
}

template<class T, denom_t D>
bool constexpr
operator>(
    Value<T, D> v0,
    Value<T, D> v1)
{
    bool result = (v0.n > v1.n);
    return result;
}

template<class T, denom_t D>
bool constexpr
operator>=(
    Value<T, D> v0,
    Value<T, D> v1)
{
    bool result = (v0.n >= v1.n);
    return result;
}

template<class T, denom_t D>
bool constexpr
operator<(
    Value<T, D> v0,
    Value<T, D> v1)
{
    bool result = (v0.n < v1.n);
    return result;
}

template<class T, denom_t D>
bool constexpr
operator<=(
    Value<T, D> v0,
    Value<T, D> v1)
{
    bool result = (v0.n <= v1.n);
    return result;
}


#undef FXPCONSTEVAL

} // namespace fxp

