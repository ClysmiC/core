template<class T, uint N>
struct Vec
{
    STATIC_ASSERT(N > 0);
    static uint constexpr COUNT = N;

    T elements[N];

    inline T& operator[](int i) { return elements[i]; }
    inline T const& operator[](int i) const { return elements[i]; }

    Vec() = default;
};

template<class T, class U, uint N>
function Vec<T, N>
vec_convert(Vec<U, N> from)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = T(from[i]);
    return result;
}

template<class T, uint N>
function Vec<T, N>
operator+(Vec<T, N> rhs)
{
    return rhs;
}

template<class T, uint N>
function Vec<T, N>
operator-(Vec<T, N> rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = -rhs[i];

    return result;
}

template<class T, uint N>
function Vec<T, N>
operator+(Vec<T, N> lhs, Vec<T, N> rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = lhs[i] + rhs[i];

    return result;
}

template<class T, uint N>
function Vec<T, N>&
operator+=(Vec<T, N>& lhs, Vec<T, N> rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

template<class T, uint N>
function Vec<T, N>
operator-(Vec<T, N> lhs, Vec<T, N> rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = lhs[i] - rhs[i];

    return result;
}

template<class T, uint N>
function Vec<T, N>&
operator-=(Vec<T, N>& lhs, Vec<T, N> rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

template<class T, uint N, class U>
function Vec<T, N>
operator*(Vec<T, N> lhs, U rhs)
{
    // TODO - better support for Vec2x ... this automatically converts result back to fix32
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = (T)(lhs[i] * rhs);

    return result;
}

template<class T, uint N, class U>
function Vec<T, N>
operator*(U lhs, Vec<T, N> rhs)
{
    // NOTE - assumes commutative property
    Vec<T, N> result = rhs * lhs;
    return result;
}

template<class T, uint N, class U>
function Vec<T, N>&
operator*=(Vec<T, N>& lhs, U rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

template<class T, uint N, class U>
function Vec<T, N>
operator/(Vec<T, N> lhs, U rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = lhs[i] / rhs;

    return result;
}

template<class T, uint N, class U>
function Vec<T, N>
operator/(U lhs, Vec<T, N> rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = lhs / rhs[i];

    return result;
}

template<class T, uint N, class U>
function Vec<T, N>&
operator/=(Vec<T, N>& lhs, U rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

template<class T, uint N>
function bool
operator==(Vec<T, N> const& lhs, Vec<T, N> const& rhs)
{
    bool result = true;
    for (int i = 0; i < N; i++) result &= (lhs[i] == rhs[i]);

    return result;
}

template<class T, uint N>
function bool
operator!=(Vec<T, N> const& lhs, Vec<T, N> const& rhs)
{
    bool result = !(lhs == rhs);
    return result;
}

template<class T, uint N>
function Vec<T, N>
vec_hadamard(Vec<T, N> lhs, Vec<T, N> rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = lhs[i] * rhs[i];

    return result;
}

template<class T, uint N>
function Vec<T, N>
vec_hadamard_divide_safe_0(Vec<T, N> lhs, Vec<T, N> rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = divide_safe_0(lhs[i], rhs[i]);

    return result;
}

template<class T, uint N>
function Vec<T, N>
vec_min(Vec<T, N> v0, Vec<T, N> v1)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = min(v0[i], v1[i]);

    return result;
}

template<class T, uint N>
function Vec<T, N>
vec_max(Vec<T, N> v0, Vec<T, N> v1)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = max(v0[i], v1[i]);

    return result;
}

template<class T, uint N>
function Vec<T, N>
vec_abs(Vec<T, N> v)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = Abs(v[i]);

    return result;
}

template<uint N>
function Vec<i32, N>
vec_floor(Vec<f32, N> v)
{
    Vec<i32, N> result;
    for (int i = 0; i < N; i++) result[i] = i32_from_f32_floor(v[i]);

    return result;
}

template<uint N>
function Vec<i32, N>
vec_ceil(Vec<f32, N> v)
{
    Vec<i32, N> result;
    for (int i = 0; i < N; i++) result[i] = i32_from_f32_ceil(v[i]);

    return result;
}

template<class T, uint N>
function auto
vec_dot(Vec<T, N> lhs, Vec<T, N> rhs)
{
    decltype(lhs[0] * rhs[0]) result = 0;
    for (int i = 0; i < N; i++) result += lhs[i] * rhs[i];

    return result;
}

template<class T, uint N>
function auto
vec_length_sq(Vec<T, N> v)
{
    auto result = vec_dot(v, v);
    return result;
}

template<class T, uint N>
function auto
vec_dist_sq(Vec<T, N> v0, Vec<T, N> v1)
{
    auto result = vec_length_sq(v1 - v0);
    return result;
}

#if !CRT_DISABLED
template<class T, uint N>
function T
vec_length(Vec<T, N> v)
{
    auto result_sq = vec_length_sq(v);
    T result = (T)sqrt(result_sq);
    return result;
}

template<class T, uint N>
function Vec<T, N>
vec_normalize_unsafe(Vec<T, N> v)
{
    Vec<T, N> result = v / vec_length(v);
    return result;
}

template<class T, uint N>
function Vec<T, N>
vec_normalize_safe_or(Vec<T, N> v, Vec<T, N> fallback)
{
    T length = vec_length(v);
    if (length == T(0))
        return fallback;

    Vec<T, N> result = v / length;
    return result;
}

template<class T, uint N>
function Vec<T, N>
vec_normalize_safe_0(Vec<T, N> v)
{
    Vec<T, N> result = vec_normalize_safe_or(v, {});
    return result;
}
#endif

template<class T, uint N>
function Vec<T, N>
vec_project(Vec<T, N> vec, Vec<T, N> onto)
{
    Vec<T, N> result = onto * vec_dot(vec, onto) / vec_length_sq(onto);
    return result;
}

template<class T, uint N>
function Vec<T, N>
vec_reject(Vec<T, N> vec, Vec<T, N> onto)
{
    Vec<T, N> result = vec - vec_project(vec, onto);
    return result;
}

template<class T, uint N>
function bool
vec_is_zero(Vec<T, N> const& vec)
{
    Vec<T, N> constexpr ZERO = Vec<T, N>{};
    bool result = (vec == ZERO);
    return result;
}

// --- Specialize for 2, 3, 4, to provide aliases like x, y, xy, z, etc.

template<class T>
struct Vec<T, 2>
{
    union
    {
        struct { T x, y; };
        T elements[2];
    };

    inline T& operator[](int i) { return elements[i]; }
    inline T const& operator[](int i) const { return elements[i]; }

    Vec() = default;

    explicit constexpr  Vec(T xy) :         x(xy),      y(xy)       {}
    explicit constexpr  Vec(T x, T y) :     x(x),       y(y)        {}
};

template<class T>
struct Vec<T, 3>
{
    union
    {
        struct { T x, y, z; };
        T elements[3];
        Vec<T, 2> xy;
    };

    inline T& operator[](int i) { return elements[i]; }
    inline T const& operator[](int i) const { return elements[i]; }

    Vec() = default;
    explicit constexpr  Vec(T xyz) :                x(xyz),     y(xyz),     z(xyz)      {}
    explicit constexpr  Vec(Vec<T, 2> xy, T z) :    x(xy.x),    y(xy.y),    z(z)        {}
    explicit constexpr  Vec(T x, T y, T z) :        x(x),       y(y),       z(z)        {}
};

template<class T>
function Vec<T, 3>
vec_cross(Vec<T, 3> lhs, Vec<T, 3> rhs)
{
    Vec<T, 3> result;
    result.x = lhs.y * rhs.z - lhs.z * rhs.y;
    result.y = lhs.z * rhs.x - lhs.x * rhs.z;
    result.z = lhs.x * rhs.y - lhs.y * rhs.x;
    return result;
}

template<class T>
function auto
vec_cross_xy(Vec<T, 2> lhs, Vec<T, 2> rhs)
{
    auto result = lhs.x * rhs.y - lhs.y * rhs.x;
    return result;
}

template<class T>
struct Vec<T, 4>
{
    union
    {
        struct { T x, y, z, w; };
        T elements[4];
        Vec<T, 2> xy;
        Vec<T, 3> xyz;
    };

    inline T& operator[](int i) { return elements[i]; }
    inline T const& operator[](int i) const { return elements[i]; }

    Vec() = default;
    explicit constexpr  Vec(T xyzw) :                   x(xyzw),    y(xyzw),    z(xyzw),    w(xyzw)     {}
    explicit constexpr  Vec(Vec<T, 2> xy, T z, T w) :   x(xy.x),    y(xy.y),    z(z),       w(w)        {}
    explicit constexpr  Vec(Vec<T, 3> xyz, T w)     :   x(xyz.x),   y(xyz.y),   z(xyz.z),   w(w)        {}
    explicit constexpr  Vec(T x, T y, T z, T w)     :   x(x),       y(y),       z(z),       w(w)        {}
};

// --- Convenience aliases

using Vec2 = Vec<f32, 2>;
using Vec3 = Vec<f32, 3>;
using Vec4 = Vec<f32, 4>;

using Vec2i = Vec<i32, 2>;
using Vec3i = Vec<i32, 3>;
using Vec4i = Vec<i32, 4>;

using Vec2i64 = Vec<i64, 2>;
using Vec3i64 = Vec<i64, 3>;
using Vec4i64 = Vec<i64, 4>;
