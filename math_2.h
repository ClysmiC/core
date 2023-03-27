// Vec2::Vec2(Vec2x v)
// {
//     this->x = (f32)v.x;
//     this->y = (f32)v.y;
// }

Vec2::Vec2(Vec2i v)
{
    this->x = (f32)v.x;
    this->y = (f32)v.y;
}

// Vec3::Vec3(Vec3x v)
// {
//     this->x = (f32)v.x;
//     this->y = (f32)v.y;
//     this->z = (f32)v.z;
// }

// Vec3::Vec3(Vec2x xy, fix64 z)
// {
//     this->x = (f32)xy.x;
//     this->y = (f32)xy.y;
//     this->z = (f32)z;
// }

Vec3::Vec3(Vec3i v)
{
    this->x = (f32)v.x;
    this->y = (f32)v.y;
    this->z = (f32)v.z;
}

// Vec4::Vec4(Vec4x v)
// {
//     this->x = (f32)v.x;
//     this->y = (f32)v.y;
//     this->z = (f32)v.z;
//     this->w = (f32)v.w;
// }

Vec4::Vec4(Vec4i v)
{
    this->x = (f32)v.x;
    this->y = (f32)v.y;
    this->z = (f32)v.z;
    this->w = (f32)v.w;
}

// Rect2::Rect2(Rect2x r)
// {
//     this->min = Vec2(r.min);
//     this->max = Vec2(r.max);
// }

Rect2::Rect2(Rect2i r)
{
    this->min = Vec2(r.min);
    this->max = Vec2(r.max);
}

// Rect3::Rect3(Rect3x r)
// {
//     this->min = Vec3(r.min);
//     this->max = Vec3(r.max);
// }












template<class T, int N>
struct Vec
{
    StaticAssert(N > 0);
    static int constexpr COUNT = N;

    T elements[N];

    inline T& operator[](int i) { return elements[i]; }
    inline T const& operator[](int i) const { return elements[i]; }
};


template<class T, int N>
function Vec<T, N>
operator-(Vec<T, N> const& rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = -rhs[i];

    return result;
}

template<class T, int N>
function Vec<T, N>
operator+(Vec<T, N> const& lhs, Vec<T, N> const& rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = lhs[i] + rhs[i];

    return result;
}

template<class T, int N>
function Vec<T, N>&
operator+=(Vec<T, N>& lhs, Vec<T, N> const& rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

template<class T, int N>
function Vec<T, N>
operator-(Vec<T, N> const& lhs, Vec<T, N> const& rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = lhs[i] - rhs[i];

    return result;
}

template<class T, int N>
function Vec<T, N>&
operator-=(Vec<T, N>& lhs, Vec<T, N> const& rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

template<class T, int N, class O>
function Vec<T, N>
operator*(Vec<T, N> const& lhs, O const& rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = lhs[i] * rhs;

    return result;
}

template<class T, int N, class O>
function Vec<T, N>
operator*(O const& lhs, Vec<T, N> const& rhs)
{
    // NOTE - assumes commutative property
    Vec<T, N> result = rhs * lhs;
    return result;
}

template<class T, int N, class O>
function Vec<T, N>&
operator*=(Vec<T, N>& lhs, O const& rhs)
{
    lhs = lhs * rhs;
    return lhs;
}

template<class T, int N, class O>
function Vec<T, N>
operator/(Vec<T, N> const& lhs, O const& rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = lhs[i] / rhs;

    return result;
}

template<class T, int N, class O>
function Vec<T, N>
operator/(O const& lhs, Vec<T, N> const& rhs)
{
    Vec<T, N> result;
    for (int i = 0; i < N; i++) result[i] = lhs / rhs[i];

    return result;
}

template<class T, int N, class O>
function Vec<T, N>&
operator/=(Vec<T, N>& lhs, O const& rhs)
{
    lhs = lhs / rhs;
    return lhs;
}



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
};

template<class T>
struct Vec<T, 4>
{
    union
    {
        struct { T x, y, z, w; };
        struct { T r, g, b, a; };   // TODO - use a different type for RGBA, not Vec4
        T elements[4];
        Vec<T, 2> xy;
        Vec<T, 3> xyz;
        Vec<T, 4> xyzw;
        Vec<T, 3> rgb;
    };

    inline T& operator[](int i) { return elements[i]; }
    inline T const& operator[](int i) const { return elements[i]; }
};

using Vector2 = Vec<f32, 2>;
using Vector3 = Vec<f32, 3>;
using Vector4 = Vec<f32, 4>;

using Vector2x = Vec<fxp::Distance, 2>;
