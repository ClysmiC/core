namespace fxp
{

struct fix32
{
    static i32 constexpr DBITS = 10;
    static i32 constexpr D = (1 << DBITS);
    i32 n;

    // --- Implicitly convert from...

    constexpr fix32() = default;
    constexpr fix32(i8 v)   : n((i32)(v * D)) {}
    constexpr fix32(i16 v)  : n((i32)(v * D)) {}
    constexpr fix32(i32 v)  : n((i32)(v * D)) {}
    constexpr fix32(i64 v)  : n((i32)(v * D)) {}
    constexpr fix32(u8 v)   : n((i32)(v * D)) {}
    constexpr fix32(u16 v)  : n((i32)(v * D)) {}
    constexpr fix32(u32 v)  : n((i32)(v * D)) {}
    constexpr fix32(u64 v)  : n((i32)(v * D)) {}

#if (__cplusplus >= 202002L)
    consteval fix32(f32 v)   : n((i32)(v * D)) {}
    consteval fix32(f64 v)   : n((i32)(v * D)) {}
#else
    constexpr fix32(f32 v)   : n((i32)(v * D)) {}
    constexpr fix32(f64 v)   : n((i32)(v * D)) {}
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

struct fix64
{
    static i32 constexpr DBITS = 2 * fix32::DBITS;
    static i32 constexpr D = (1 << DBITS);
    i64 n;

    // --- Implicitly convert from...

    constexpr fix64() = default;
    constexpr fix64(fix32 const& v) : n((i64)(v.n * (1 << (DBITS - fix32::DBITS)))) {}
    constexpr fix64(i8 v)   : n((i64)(v * D)) {}
    constexpr fix64(i16 v)  : n((i64)(v * D)) {}
    constexpr fix64(i32 v)  : n((i64)(v * D)) {}
    constexpr fix64(i64 v)  : n((i64)(v * D)) {}
    constexpr fix64(u8 v)   : n((i64)(v * D)) {}
    constexpr fix64(u16 v)  : n((i64)(v * D)) {}
    constexpr fix64(u32 v)  : n((i64)(v * D)) {}
    constexpr fix64(u64 v)  : n((i64)(v * D)) {}

#if (__cplusplus >= 202002L)
    consteval fix64(f32 v)   : n((i64)(v * D)) {}
    consteval fix64(f64 v)   : n((i64)(v * D)) {}
#else
    constexpr fix64(f32 v)   : n((i64)(v * D)) {}
    constexpr fix64(f64 v)   : n((i64)(v * D)) {}
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
    explicit constexpr operator fix32() const
    {
        fix32 result;
        result.n = (i32)(n >> (DBITS - fix32::DBITS));
        return result;
    }
};

// --- fix32 math operations

function fix32 constexpr
operator+(fix32 v0, fix32 v1)
{
    fix32 result;
    result.n = v0.n + v1.n;
    return result;
}

function fix32 constexpr&
operator+=(fix32& v0, fix32 v1)
{
    v0 = v0 + v1;
    return v0;
}

function fix32 constexpr
operator-(fix32 v0, fix32 v1)
{
    fix32 result;
    result.n = v0.n - v1.n;
    return result;
}

function fix32 constexpr&
operator-=(fix32& v0, fix32 v1)
{
    v0 = v0 - v1;
    return v0;
}

#if 1
function fix64 constexpr
operator*(fix32 v0, fix32 v1)
{
    fix64 result;
    result.n = (i64)v0.n * (i64)v1.n;
    return result;
}

function fix32 constexpr&
operator*=(fix32& v0, fix32 v1)
{
    fix64 product = v0 * v1;
    v0.n = (i32)(product.n / (1 << (fix64::DBITS - fix32::DBITS)));
    return v0;
}
#else
function fix32 constexpr
operator*(fix32 v0, fix32 v1)
{
    fix32 result;
    i64 n = (i64)v0.n * (i64)v1.n;
    n /= (1 << fix32::DBITS);
    result.n = (i32)n;
    return result;
}

function fix32 constexpr&
operator*=(fix32& v0, fix32 v1)
{
    v0 = v0 * v1;
    return v0;
}
#endif

function fix32 constexpr
operator/(fix32 v0, fix32 v1)
{
    fix32 result;
    result.n = (i32)(((1 << fix32::DBITS) * (i64)v0.n) / v1.n);
    return result;
}

function fix32 constexpr&
operator/=(fix32& v0, fix32 v1)
{
    v0 = v0 / v1;
    return v0;
}

bool constexpr
operator==(fix32 v0, fix32 v1)
{
    bool result = (v0.n == v1.n);
    return result;
}

bool constexpr
operator!=(fix32 v0, fix32 v1)
{
    bool result = (v0.n != v1.n);
    return result;
}

bool constexpr
operator>(fix32 v0, fix32 v1)
{
    bool result = (v0.n > v1.n);
    return result;
}

bool constexpr
operator>=(fix32 v0, fix32 v1)
{
    bool result = (v0.n >= v1.n);
    return result;
}

bool constexpr
operator<(fix32 v0, fix32 v1)
{
    bool result = (v0.n < v1.n);
    return result;
}

bool constexpr
operator<=(fix32 v0, fix32 v1)
{
    bool result = (v0.n <= v1.n);
    return result;
}


// --- fix64 math operations

function fix64 constexpr
operator+(fix64 v0, fix64 v1)
{
    fix64 result;
    result.n = v0.n + v1.n;
    return result;
}

function fix64 constexpr&
operator+=(fix64& v0, fix64 v1)
{
    v0 = v0 + v1;
    return v0;
}

function fix64 constexpr
operator-(fix64 v0, fix64 v1)
{
    fix64 result;
    result.n = v0.n - v1.n;
    return result;
}

function fix64 constexpr&
operator-=(fix64& v0, fix64 v1)
{
    v0 = v0 - v1;
    return v0;
}

function fix64 constexpr
operator*(fix64 v0, fix64 v1)
{
    fix64 result;
    i64 n = v0.n * v1.n;
    n /= (1 << fix64::DBITS);
    result.n = n;
    return result;
}

function fix64 constexpr&
operator*=(fix64& v0, fix64 v1)
{
    v0 = v0 * v1;
    return v0;
}

function fix64 constexpr
operator/(fix64 v0, fix64 v1)
{
    fix64 result;
    result.n = (((1 << fix64::DBITS) * v0.n) / v1.n);
    return result;
}

function fix64 constexpr&
operator/=(fix64& v0, fix64 v1)
{
    v0 = v0 / v1;
    return v0;
}

bool constexpr
operator==(fix64 v0, fix64 v1)
{
    bool result = (v0.n == v1.n);
    return result;
}

bool constexpr
operator!=(fix64 v0, fix64 v1)
{
    bool result = (v0.n != v1.n);
    return result;
}

bool constexpr
operator>(fix64 v0, fix64 v1)
{
    bool result = (v0.n > v1.n);
    return result;
}

bool constexpr
operator>=(fix64 v0, fix64 v1)
{
    bool result = (v0.n >= v1.n);
    return result;
}

bool constexpr
operator<(fix64 v0, fix64 v1)
{
    bool result = (v0.n < v1.n);
    return result;
}

bool constexpr
operator<=(fix64 v0, fix64 v1)
{
    bool result = (v0.n <= v1.n);
    return result;
}

// TODO - delete sqrt functions in favor of vec scaling functions
function fix32
sqrt(fix32 v)
{
    fix32 result = {};
    if (v.n <= 0)
        return result;

    int msb = 0;
    bitscan_msb_index((u32)v.n, &msb);

    int denom_msb = 0;
    bitscan_msb_index((u32)(1 << fix32::DBITS), &denom_msb);

    // Initial guess
    result = 1 << ((msb - denom_msb) / 2);
    if (result.n <= 0)
    {
        // Avoid divide by 0
        result = v;
    }

    // Empirically, all integer 22.10 values from 0 to (1<<21)-1 converge to within < 0.035% of the actual value after 3 iterations
    int constexpr ITERATION_COUNT = 3;
    fix32 constexpr TWO(2);

    // Newton's Method
    for (int i = 0; i < ITERATION_COUNT; i++)
    {
        fix32 div = v / result;
        result = (result + div) / TWO;
    }

    return result;
}

// TODO - delete sqrt functions in favor of vec scaling functions
function fix64
sqrt(fix64 v)
{
    fix64 result = {};
    if (v.n <= 0)
        return result;

    int msb = 0;
    bitscan_msb_index((u64)v.n, &msb);

    int denom_msb = 0;
    bitscan_msb_index((u64)(1 << fix32::DBITS), &denom_msb);

    // Initial guess
    result = 1 << ((msb - denom_msb) / 2);
    if (result.n <= 0)
    {
        // Avoid divide by 0
        result = v;
    }

    // Empirically, all integer 22.10 values from 0 to (1<<21)-1 converge to within < 0.035% of the actual value after 3 iterations
    int constexpr ITERATION_COUNT = 3;
    fix64 constexpr TWO(2);

    // Newton's Method
    for (int i = 0; i < ITERATION_COUNT; i++)
    {
        fix64 div = v / result;
        result = (result + div) / TWO;
    }

    return result;
}

} // namespace fxp

