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
operator-(fix32 v)
{
    fix32 result;
    result.n = -v.n;
    return result;
}

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

function fix64 constexpr
operator*(fix32 v0, fix32 v1)
{
    fix64 result;
    result.n = (i64)v0.n * (i64)v1.n;
    return result;
}

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
operator-(fix64 v)
{
    fix64 result;
    result.n = -v.n;
    return result;
}

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
    bitscan_msb_index((u64)(1 << fix64::DBITS), &denom_msb);

    // Initial guess
    result = 1 << ((msb - denom_msb) / 2);
    if (result.n <= 0)
    {
        // Avoid divide by 0
        result = v;
    }

    int constexpr ITERATION_COUNT = 8;
    fix64 constexpr TWO(2);

    // Newton's Method
    for (int i = 0; i < ITERATION_COUNT; i++)
    {
        fix64 div = v / result;
        result = (result + div) / TWO;
    }

    return result;
}

using Vec2x = Vec<fix32, 2>;

function Vec2x
vec_scale_to_sq(Vec2x const& v, fix64 r_sq)
{
    fix64 length_sq = vec_length_sq(v);
    if (length_sq == 0)
        return {};

    Vec2x lo = {};
    Vec2x hi = v;

    while (length_sq < r_sq)
    {
        lo = hi;
        hi = hi + hi;
        length_sq *= 4;
    }

    while (true)
    {
        Vec2x mid = lo + (hi - lo) / 2;
        if (mid == lo) break;

        if (vec_length_sq(mid) < r_sq)
        {
            lo = mid;
        }
        else
        {
            hi = mid;
        }
    }

    return lo;
}

function Vec2x
vec_scale_to(Vec2x const& v, fix32 r)
{
    return vec_scale_to_sq(v, r * r);
}

function Vec2x
vec_scale_past_sq(Vec2x const& v, fix64 r_sq)
{
    fix64 length_sq = vec_length_sq(v);
    if (length_sq == 0)
        return {};

    Vec2x lo = {};
    Vec2x hi = v;

    while (length_sq < r_sq)
    {
        lo = hi;
        hi = hi + hi;
        length_sq *= 4;
    }

    while (true)
    {
        Vec2x mid = lo + (hi - lo) / 2;
        if (mid == lo) break;

        if (vec_length_sq(mid) < r_sq)
        {
            lo = mid;
        }
        else
        {
            hi = mid;
        }
    }

    return hi;
}

function Vec2x
vec_scale_past(Vec2x const& v, fix32 r)
{
    return vec_scale_past_sq(v, r * r);
}


// Fixed point 1.0.31 angle. Domain is [-1, 1)
enum class anglex : i32
{
    _0 = 0,
    _45 = 0x10000000,
    _90 = 0x20000000,
    _135 = 0x30000000,
    _180 = 0x40000000,
    _225 = 0x50000000,
    _270 = 0x60000000,
    _315 = 0x70000000,

    // _360 is the value that wraps around to negative
    _360 = int32_t(0x80000000)
};

// CORDIC rotation
// https://github.com/francisrstokes/githublog/blob/main/2024/5/10/cordic.md
function Vec2x
vec_rotate(Vec2x v, anglex a)
{
    // CORDIC is an iterative algorithm for rotating around an angle in the domain [0, pi/2).
    //  It converges on the correct answer by rotating the vector by successively smaller angles,
    //  and choosing to rotate CW or CCW on each iteration.

    // Each iteration:
    // [x'] = [ cos(theta)  -sin(theta) ]   [x]
    // [y'] = [ sin(theta)  cos(theta)  ]   [y]

    // x' = x * cos(theta) - y * sin(theta)
    // y' = x * sin(theta) + y * cos(theta)

    // Replace sin(theta) with cos(theta) * tan(theta), then factor out the cos(theta)
    // x' = cos(theta) * (x - y * tan(theta))
    // y' = cos(theta) * (x * tan(theta) + y)

    // Precalculate the product of the cosine of all iterations' thetas in 1.1.62 space and apply that term up front
    i64 constexpr cosine_product = (int64_t)(0.6072529350088812561694 * (1ll << 32));
    i64 x = v.x.n * cosine_product;
    i64 y = v.y.n * cosine_product;

    // Now that the inputs are pre-scaled by cosine_product, the iteration becomes
    // x' = x - y * tan(theta)
    // y' = y + x * tan(theta)

    // We pick our theta each iteration such that tan(theta_i) = 2**-i, so multiplying by tan(theta_i) is just a bitshift.
    // x' = x - y * (2**-i)
    // y' = y + x * (2**-i)

    // Thus, theta_table[i] is defined to be atan(2**-i) in 1.1.62 space
    // I.e., Each entry is the chosen theta we can rotate by so that multiplying by tan(theta) is just a bit shift.
    i64 constexpr theta_table[32] = {
        1152921504606846976ll,
         680609306067436544ll,
         359615265290440512ll,
         182546323762760992ll,
          91627395746647424ll,
          45858365146018112ll,
          22934778241356568ll,
          11468088963375448ll,
           5734131974037916ll,
           2867076923938204ll,
           1433539829095742ll,
            716770085439068ll,
            358385064080945ll,
            179192534710649ll,
             89596267689097ll,
             44798133886270ll,
             22399066948350ll,
             11199533474827ll,
              5599766737495ll,
              2799883368758ll,
              1399941684380ll,
               699970842190ll,
               349985421095ll,
               174992710548ll,
                87496355274ll,
                43748177637ll,
                21874088818ll,
                10937044409ll,
                 5468522205ll,
                 2734261102ll,
                 1367130551ll,
                  683565276ll,
    };
    // int constexpr iter_count = 16;   // lower precision, but probably good enough?
    int constexpr iter_count = 32;
    STATIC_ASSERT(ARRAY_LEN(theta_table) == iter_count);

    // normalize the angle to [anglex::_0, anglex::_90) shifted into 1.1.62 space
    i32 quadrant = (i32)a & 0x60000000;
    i64 angle = (i64)((i32)a & 0x1fffffff) << 32;
    STATIC_ASSERT(sizeof(anglex) == sizeof(i32));

    for (int i = 0; i < iter_count; i++)
    {
        // 0 (for positive) or -1 (for negative)
        i64 zero_or_minus_one = angle >> 63;

        // Branchless negate all of our results when sign == -1.
        //  Two's complement says (N xor -1) + 1 = -N
        i64 dx = (y ^ zero_or_minus_one) - zero_or_minus_one;
        i64 dy = (x ^ zero_or_minus_one) - zero_or_minus_one;

        // Multiply by tan(theta_i)
        x -= (dx >> i);
        y += (dy >> i);

        i64 delta_angle = (theta_table[i] ^ zero_or_minus_one) - zero_or_minus_one;
        angle -= delta_angle;
    }

    // Round and shift result back into 1.0.31 space
    Vec2x result = {};
    result.x = i32((x + 0x0000000080000000ll) >> 32);
    result.y = i32((y + 0x0000000080000000ll) >> 32);

    // Rotate the result back into the correct quadrant
    switch (quadrant)
    {
        case 0:             result = Vec2x(result.x, result.y);     break;
        case 0x20000000:    result = Vec2x(-result.y, result.x);    break;
        case 0x40000000:    result = Vec2x(-result.x, -result.y);   break;
        case 0x60000000:    result = Vec2x(result.y, -result.x);    break;
        DefaultInvalid;
    }

    return result;
}

} // namespace fxp

