namespace fxp
{
// TODO - consteval gives best compile-time determinism guarantee for converting float values to fixed values, but requires C++20.
#if (__cplusplus >= 202002L)
 #define FXPCONSTEVAL consteval
#else
 #define FXPCONSTEVAL constexpr
#endif

template<class> struct integer_type { static bool constexpr is_supported = false; };
template<> struct integer_type<i32> { static bool constexpr is_supported = true; using unsigned_t = u32; };
template<> struct integer_type<i64> { static bool constexpr is_supported = true; using unsigned_t = u64; };

using denom_t = i32;

template<class T, denom_t DBITS>
struct Value
{
    STATIC_ASSERT(DBITS >= 0);
    STATIC_ASSERT(integer_type<T>::is_supported);

    static denom_t constexpr D_BITS = DBITS;
    static denom_t constexpr D = (1 << DBITS);
    T n;

    // --- Implicitly convert from...

    constexpr Value() = default;

    template <class T_OTHER>
    constexpr Value(Value<T_OTHER, DBITS> const& v) : n(T(v.n)) {}
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

// --- Math operations

template<class T, denom_t DBITS>
Value<T, DBITS> constexpr
operator+(
    Value<T, DBITS> v0,
    Value<T, DBITS> v1)
{
    Value<T, DBITS> result;
    result.n = v0.n + v1.n;
    return result;
}

template<class T, denom_t DBITS>
Value<T, DBITS> constexpr&
operator+=(
    Value<T, DBITS>& v0,
    Value<T, DBITS> v1)
{
    v0 = v0 + v1;
    return v0;
}

template<class T, denom_t DBITS>
Value<T, DBITS> constexpr
operator-(
    Value<T, DBITS> v0,
    Value<T, DBITS> v1)
{
    Value<T, DBITS> result;
    result.n = v0.n - v1.n;
    return result;
}

template<class T, denom_t DBITS>
Value<T, DBITS> constexpr&
operator-=(
    Value<T, DBITS>& v0,
    Value<T, DBITS> v1)
{
    v0 = v0 - v1;
    return v0;
}

template<class T, denom_t DBITS>
Value<T, DBITS> constexpr
operator*(
    Value<T, DBITS> v0,
    Value<T, DBITS> v1)
{
    Value<T, DBITS> result;
    i64 n = (i64)v0.n * (i64)v1.n;
    n /= (1 << DBITS);
    result.n = (T)n;
    return result;
}

template<class T, denom_t DBITS>
Value<T, DBITS> constexpr&
operator*=(
    Value<T, DBITS>& v0,
    Value<T, DBITS> v1)
{
    v0 = v0 * v1;
    return v0;
}

template<class T, denom_t DBITS>
Value<T, DBITS> constexpr
operator/(
    Value<T, DBITS> v0,
    Value<T, DBITS> v1)
{
    Value<T, DBITS> result;
    result.n = (T)(((1 << DBITS) * (i64)v0.n) / v1.n);
    return result;
}

template<class T, denom_t DBITS>
Value<T, DBITS> constexpr&
operator/=(
    Value<T, DBITS>& v0,
    Value<T, DBITS> v1)
{
    v0 = v0 / v1;
    return v0;
}

template<class T, denom_t DBITS>
bool constexpr
operator==(
    Value<T, DBITS> v0,
    Value<T, DBITS> v1)
{
    bool result = (v0.n == v1.n);
    return result;
}

template<class T, denom_t DBITS>
bool constexpr
operator!=(
    Value<T, DBITS> v0,
    Value<T, DBITS> v1)
{
    bool result = (v0.n != v1.n);
    return result;
}

template<class T, denom_t DBITS>
bool constexpr
operator>(
    Value<T, DBITS> v0,
    Value<T, DBITS> v1)
{
    bool result = (v0.n > v1.n);
    return result;
}

template<class T, denom_t DBITS>
bool constexpr
operator>=(
    Value<T, DBITS> v0,
    Value<T, DBITS> v1)
{
    bool result = (v0.n >= v1.n);
    return result;
}

template<class T, denom_t DBITS>
bool constexpr
operator<(
    Value<T, DBITS> v0,
    Value<T, DBITS> v1)
{
    bool result = (v0.n < v1.n);
    return result;
}

template<class T, denom_t DBITS>
bool constexpr
operator<=(
    Value<T, DBITS> v0,
    Value<T, DBITS> v1)
{
    bool result = (v0.n <= v1.n);
    return result;
}

// TODO - delete this function in favor of vec scaling functions
template<class T, denom_t DBITS>
Value<T, DBITS>
sqrt(Value<T, DBITS> v)
{
    Value<T, DBITS> result = {};
    if (v.n <= 0)
        return result;

    int msb = 0;
    bitscan_msb_index((integer_type<T>::unsigned_t)v.n, &msb);

    int denom_msb = 0;
    bitscan_msb_index((u32)(1 << DBITS), &denom_msb);

    // Initial guess
    result = T(1) << ((msb - denom_msb) / 2);
    if (result.n <= 0)
    {
        // Avoid divide by 0
        result = v;
    }

    // Empirically, all integer 22.10 values from 0 to (1<<21)-1 converge to within < 0.035% of the actual value after 3 iterations
    int constexpr ITERATION_COUNT = 3;
    Value<T, DBITS> constexpr TWO(2);

    // Newton's Method
    for (int i = 0; i < ITERATION_COUNT; i++)
    {
        Value<T, DBITS> div = v / result;
        result = (result + div) / TWO;
    }

    return result;
}

#undef FXPCONSTEVAL

} // namespace fxp

