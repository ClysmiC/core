template<class T, uint N>
struct Vec
{
    StaticAssert(N > 0);
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
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = lhs[i] * rhs;

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
function T
vec_dot(Vec<T, N> lhs, Vec<T, N> rhs)
{
    T result = 0;
    for (int i = 0; i < N; i++) result += lhs[i] * rhs[i];

    return result;
}

template<class T, uint N>
function T
vec_length_sq(Vec<T, N> v)
{
    T result = vec_dot(v, v);
    return result;
}

template<class T, uint N>
function T
vec_dist_sq(Vec<T, N> v0, Vec<T, N> v1)
{
    T result = vec_length_sq(v1 - v0);
    return result;
}

#if !CRT_DISABLED
template<class T, uint N>
function T
vec_length(Vec<T, N> v)
{
    T result = vec_length_sq(v);
    result = Sqrt(result);
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
    if (length == 0)
        return fallback;

    Vec<T, N> result = v / length;
    return result;
}

template<class T, uint N>
function Vec<T, N>
vec_normalize_safe_0(Vec<T, N> v)
{
    Vec2 result = vec_normalize_safe_or(v, {});
    return result;
}
#endif

// --- Specialize for 2, 3, 4, to provide aliases like x, y, xy, z, etc.

template<class T>
struct Vec<T, 2>
{
    union
    {
        struct { T x, y; };
        struct { T u, v; };
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

    enum from_vec_t{};

    inline T& operator[](int i) { return elements[i]; }
    inline T const& operator[](int i) const { return elements[i]; }

    Vec() = default;
    explicit constexpr  Vec(T xyz) :                x(xyz),     y(xyz),     z(xyz)      {}
    explicit constexpr  Vec(Vec<T, 2> xy, T z) :    x(xy.x),    y(xy.y),    z(z)        {}
    explicit constexpr  Vec(T x, T y, T z) :        x(x),       y(y),       z(z)        {}
};

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

    enum from_vec{};

    inline T& operator[](int i) { return elements[i]; }
    inline T const& operator[](int i) const { return elements[i]; }

    Vec() = default;
    explicit constexpr  Vec(T xyzw) :                   x(xyzw),    y(xyzw),    z(xyzw),    w(xyzw)     {}
    explicit constexpr  Vec(Vec<T, 2> xy, T z, T w) :   x(xy.x),    y(xy.y),    z(z),       w(w)        {}
    explicit constexpr  Vec(Vec<T, 3> xyz, T w)     :   x(xyz.x),   y(xyz.y),   z(xyz.z),   w(w)        {}
    explicit constexpr  Vec(T x, T y, T z, T w)     :   x(x),       y(y),       z(z),       w(w)        {}
};

using Vec2 = Vec<f32, 2>;
using Vec3 = Vec<f32, 3>;
using Vec4 = Vec<f32, 4>;

using Vec2i = Vec<i32, 2>;
using Vec3i = Vec<i32, 3>;
using Vec4i = Vec<i32, 4>;

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
function Vec<T, 3>
vec_cross_xy(Vec<T, 2> lhs, Vec<T, 2> rhs)
{
    Vec<T, 3> result;
    result.x = 0;
    result.y = 0;
    result.z = lhs.x * rhs.y - lhs.y * rhs.x;
    return result;
}

struct Rgb
{
    f32 r, g, b;

    Rgb() = default;
    explicit constexpr  Rgb(f32 r, f32 g, f32 b) :  r(r),       g(g),       b(b)        {}
    explicit constexpr  Rgb(Vec3 rgb) :             r(rgb.x),   g(rgb.y),   b(rgb.z)    {}

    explicit constexpr  Rgb(u32 hex)
    : r(((hex & 0xFF0000) >> 16) / 255.0f)
    , g(((hex & 0x00FF00) >>  8) / 255.0f)
    , b(((hex & 0x0000FF) >>  0) / 255.0f)
        {}
};

namespace RGB
{
    static Rgb constexpr RED        (1, 0, 0);
    static Rgb constexpr GREEN      (0, 1, 0);
    static Rgb constexpr BLUE       (0, 0, 1);
    static Rgb constexpr YELLOW     (1, 1, 0);
    static Rgb constexpr CYAN       (0, 1, 1);
    static Rgb constexpr MAGENTA    (1, 0, 1);

    static Rgb constexpr WHITE      (1, 1, 1);
    static Rgb constexpr BLACK      (0, 0, 0);
}

struct Rgba
{
    f32 r, g, b, a;
    Rgba() = default;
    explicit constexpr  Rgba(f32 r, f32 g, f32 b, f32 a) :  r(r),       g(g),       b(b),       a(a)        {}
    explicit constexpr  Rgba(Rgb rgb) :                     r(rgb.r),   g(rgb.g),   b(rgb.b),   a(1.0f)     {}
    explicit constexpr  Rgba(Rgb rgb, f32 a) :              r(rgb.r),   g(rgb.g),   b(rgb.b),   a(a)        {}
    explicit constexpr  Rgba(Vec3 rgb) :                    r(rgb.x),   g(rgb.y),   b(rgb.z),   a(1.0f)     {}
    explicit constexpr  Rgba(Vec3 rgb, f32 a) :             r(rgb.x),   g(rgb.y),   b(rgb.z),   a(a)        {}
    explicit constexpr  Rgba(Vec4 rgba) :                   r(rgba.x),  g(rgba.y),  b(rgba.z),  a(rgba.w)   {}

    explicit constexpr  Rgba(u32 hex)
    : r(((hex & 0xFF000000) >> 24) / 255.0f)
    , g(((hex & 0x00FF0000) >> 16) / 255.0f)
    , b(((hex & 0x0000FF00) >>  8) / 255.0f)
    , a(((hex & 0x000000FF) >>  0) / 255.0f)
        {}
};

namespace RGBA
{
    static Rgba constexpr RED       (1, 0, 0, 1);
    static Rgba constexpr GREEN     (0, 1, 0, 1);
    static Rgba constexpr BLUE      (0, 0, 1, 1);
    static Rgba constexpr YELLOW    (1, 1, 0, 1);
    static Rgba constexpr CYAN      (0, 1, 1, 1);
    static Rgba constexpr MAGENTA   (1, 0, 1, 1);

    static Rgba constexpr WHITE         (1, 1, 1, 1);
    static Rgba constexpr BLACK         (0, 0, 0, 1);

    // TODO - replace this with TRANSPARENT, but apparently it
    //  collides with a windows symbol.
    // TODO - don't include windows.h
    static Rgba constexpr TRANSPARENT2   (0, 0, 0, 0);
}
