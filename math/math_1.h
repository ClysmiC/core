#pragma once

// TODO
// options for trig, pow, etc.
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


#if 0

#if !CRT_DISABLED
function bool
IsNormalized(Vec2 v, f32 epsilon=0.001f)
{
    // TODO - can't I use LengthSq here? 1^2 is just 1...
    bool result = f32_approx_eq(Length(v), 1.0f, epsilon);
    return result;
}
#endif // !CRT_DISABLED

function bool
IsZero(Vec2 v)
{
    // TODO - Epsilon version?
    bool result = (v.x == 0.0f && v.y == 0.0f);
    return result;
}

function bool
AreOrthogonal(Vec2 v0, Vec2 v1)
{
    f32 dot = Dot(v0, v1);
    bool result = f32_approx_eq(dot, 0);
    return result;
}

function Vec2
Project(Vec2 projectee, Vec2 onto)
{
    Vec2 result = onto * Dot(projectee, onto) / LengthSq(onto);
    return result;
}

function Vec2
ProjectOntoNormalizedAxis(Vec2 projectee, Vec2 onto)
{
    Vec2 result = onto * Dot(projectee, onto);
    return result;
}

function Vec2
Reflect(Vec2 reflectee, Vec2 reflectionAxis)
{
    Vec2 result = reflectee - 2.0f * Project(reflectee, reflectionAxis);
    return result;
}

function Vec2
ReflectAcrossNormalizedAxis(Vec2 reflectee, Vec2 normalizedReflectionAxis)
{
    Vec2 result = reflectee - 2.0f * Dot(reflectee, normalizedReflectionAxis) * normalizedReflectionAxis;
    return result;
}

#if !CRT_DISABLED
function bool
IsNormalized(Vec3 v, f32 epsilon=0.001f)
{
    // TODO - can't I use LengthSq here? 1^2 is just 1...
    bool result = f32_approx_eq(Length(v), 1.0f, epsilon);
    return result;
}
#endif // !CRT_DISABLED

function bool
IsZero(Vec3 v)
{
    // TODO - Epsilon version?
    bool result = (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f);
    return result;
}

function bool
AreOrthogonal(Vec3 v0, Vec3 v1)
{
    f32 dot = Dot(v0, v1);
    bool result = f32_approx_eq(dot, 0);
    return result;
}

function Vec3
Project(Vec3 projectee, Vec3 onto)
{
    Vec3 result = onto * Dot(projectee, onto) / LengthSq(onto);
    return result;
}

function Vec3
ProjectOntoNormalizedAxis(Vec3 projectee, Vec3 onto)
{
    Vec3 result = onto * Dot(projectee, onto);
    return result;
}

function Vec3
Reflect(Vec3 reflectee, Vec3 reflectionAxis)
{
    Vec3 result = reflectee - 2.0f * Project(reflectee, reflectionAxis);
    return result;
}

function Vec3
ReflectAcrossNormalizedAxis(Vec3 reflectee, Vec3 normalizedReflectionAxis)
{
    Vec3 result = reflectee - 2.0f * Dot(reflectee, normalizedReflectionAxis) * normalizedReflectionAxis;
    return result;
}

#if !CRT_DISABLED
function bool
IsNormalized(Vec4 v, f32 epsilon=0.001f)
{
    // TODO - can't I use LengthSq here? 1^2 is just 1...
    bool result = f32_approx_eq(Length(v), 1.0f, epsilon);
    return result;
}
#endif // !CRT_DISABLED

function bool
IsZero(Vec4 v)
{
    // TODO - Epsilon version?
    bool result = (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f && v.w == 0.0f);
    return result;
}

function bool
AreOrthogonal(Vec4 v0, Vec4 v1)
{
    f32 dot = Dot(v0, v1);
    bool result = f32_approx_eq(dot, 0);
    return result;
}

#endif


#if 0
// --- Ray

struct Ray2
{
    Vec2 p0;
    Vec2 dir;

    Ray2() = default;
    Ray2(Vec2 p0, Vec2 dir)
    {
        this->p0 = p0;
        this->dir = dir;
    }
};

Vec2 PosAtT(Ray2 ray, f32 t)
{
    Vec2 result = ray.p0 + ray.dir * t;
    return result;
}

struct Ray3
{
    Vec3 p0;
    Vec3 dir;

    Ray3() = default;
    Ray3(Vec3 p0, Vec3 dir)
    {
        this->p0 = p0;
        this->dir = dir;
    }
    explicit Ray3(Ray2 ray)
    {
        this->p0 = Vec3(ray.p0, 0);
        this->dir = Vec3(ray.dir, 0);
    }
};

Vec3 PosAtT(Ray3 ray, f32 t)
{
    Vec3 result = ray.p0 + ray.dir * t;
    return result;
}
#endif


// Mat3

struct Mat3
{
    f32 e[3][3];
    
    f32 *
    operator[](int i)
    {
        // NOTE - Only overloads the first index to point to the correct array, then
        // the second index is handled by built-in c++
    
        f32 * result = e[i];
        return result;
    }
};

function Mat3
Mat3Identity()
{
    Mat3 result = {{
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 }
        }};
    
    return result;
}

 Mat3
Mat3Translate(Vec2 t)
{
    Mat3 result = {{
            { 1, 0, t.x },
            { 0, 1, t.y },
            { 0, 0, 1 }
        }};
    return result;
}

 Mat3
Mat3Scale(f32 s)
{
    Mat3 result = {{
            { s, 0, 0 },
            { 0, s, 0 },
            { 0, 0, 1 }
        }};

    return result;
}

 Mat3
Mat3Scale(Vec2 s)
{
    Mat3 result = {{
            { s.x, 0, 0 },
            { 0, s.y, 0 },
            { 0, 0, 1 }
        }};

    return result;
}

 Mat3
operator*(Mat3& lhs, Mat3& rhs)
{
    // TODO - simd-ize?
    // https://tavianator.com/2016/matrix_multiply.html
    
    Mat3 result = {};
    for(int down = 0; down < 3; down++)
    {
        for(int across = 0; across < 3; across++)
        {
            for(int i = 0; i < 3; i++)
            {
                result[down][across] += (lhs[down][i] * rhs[i][across]);
            }
        }
    }
    
    return result;
}

function f32 *
RowMajorPtr(Mat3 * mat)
{
    return mat->e[0];
}

// Mat4

struct Mat4
{
    f32 e[4][4];

    const f32 *
    operator[](int i)
    {
        // NOTE - Only overloads the first index to point to the correct array, then
        // the second index is handled by built-in c++
    
        f32 * result = e[i];
        return result;
    }
};

function Mat4
operator*(const Mat4& lhs, const Mat4& rhs)
{
    // TODO - simd-ize?
    // https://tavianator.com/2016/matrix_multiply.html
    
    Mat4 result = {};
    for(int down = 0; down < 4; down++)
    {
        for(int across = 0; across < 4; across++)
        {
            for(int i = 0; i < 4; i++)
            {
                result.e[down][across] += (lhs.e[down][i] * rhs.e[i][across]);
            }
        }
    }
    
    return result;
}

function Vec4
operator*(const Mat4& m, const Vec4& v)
{
    Vec4 result;
    result.x = m.e[0][0] * v.x   +   m.e[0][1] * v.y   +   m.e[0][2] * v.z   +   m.e[0][3] * v.w;
    result.y = m.e[1][0] * v.x   +   m.e[1][1] * v.y   +   m.e[1][2] * v.z   +   m.e[1][3] * v.w;
    result.z = m.e[2][0] * v.x   +   m.e[2][1] * v.y   +   m.e[2][2] * v.z   +   m.e[2][3] * v.w;
    result.w = m.e[3][0] * v.x   +   m.e[3][1] * v.y   +   m.e[3][2] * v.z   +   m.e[3][3] * v.w;
    return result;
}

function Mat4
Mat4Identity()
{
    Mat4 result = {{
            { 1, 0, 0, 0 },
            { 0, 1, 0, 0 },
            { 0, 0, 1, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

function Mat4
Mat4Translate(Vec3 t)
{
    Mat4 result = {{
            { 1, 0, 0, t.x },
            { 0, 1, 0, t.y },
            { 0, 0, 1, t.z },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

function Mat4
Mat4Scale(f32 s)
{
    Mat4 result = {{
            { s, 0, 0, 0 },
            { 0, s, 0, 0 },
            { 0, 0, s, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

function Mat4
Mat4Scale(Vec3 s)
{
    Mat4 result = {{
            { s.x, 0, 0, 0 },
            { 0, s.y, 0, 0 },
            { 0, 0, s.z, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

#if !CRT_DISABLED

function Mat4
Mat4RotateX(f32 radians)
{
    f32 c = Cos(radians);
    f32 s = Sin(radians);

    Mat4 result = {{
            { 1, 0, 0, 0 },
            { 0, c,-s, 0 },
            { 0, s, c, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

function Mat4
Mat4RotateY(f32 radians)
{
    f32 c = Cos(radians);
    f32 s = Sin(radians);

    Mat4 result = {{
            { c, 0, s, 0 },
            { 0, 1, 0, 0 },
            {-s, 0, c, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

function Mat4
Mat4RotateZ(f32 radians)
{
    f32 c = Cos(radians);
    f32 s = Sin(radians);

    Mat4 result = {{
            { c,-s, 0, 0 },
            { s, c, 0, 0 },
            { 0, 0, 1, 0 },
            { 0, 0, 0, 1 }
        }};
    
    return result;
}

#endif // !CRT_DISABLED

// NOTE - For this game, the coordinate systems are as follows

// World Space
//  X is to the "east"
//  Y is to the "north"
//  Z is "up"

// Camera Space
//  X is to the "right"
//  Y is "up"
//  Z is "backwards"

function Mat4
Mat4Ortho(f32 height, f32 aspectRatio, f32 farDist, f32 zoom=1.0f)
{
    if (f32_approx_eq(zoom, 0)) zoom = 1.0f;

    // See FGED Vol 2 - Chapter 6.3.4
    
    f32 width = height * aspectRatio;

    // Apply zoom
    width /= zoom;
    height /= zoom;

    // The far distance is positive for user convenience, but since Z- is forward
    //  in camera space, the actual Z value is negated.
    f32 farZCamera = -farDist;

#if 1
    // Used for Opengl where NDC has Z [-1 to 1]
    f32 k0 = 2.0f / farZCamera;
    f32 k1 = -1;
#else
    // Used (untested) for DirectX where NDC has Z [0 to 1]
    f32 k0 = 1.0f / farZCamera;
    f32 k1 = 0;
#endif
    
    // NOTE - This matrix is simpler than the general form, because it places
    //  the near plane at the camera and centers the camera w/r/t width and height.
    //  This allows many terms to cancel.
    
    Mat4 result = {{
            { 2.0f / width, 0, 0, 0 },
            { 0, 2.0f / height, 0, 0 },
            { 0, 0, k0, k1 },
            { 0, 0, 0, 1 }
        }};

    return result;
}

#if !CRT_DISABLED

function Mat4
Mat4Perspective(f32 fovVertical, f32 aspectRatio, f32 nearDist, f32 farDist)
{
    // NOTE - FOV is in radians
    
    // See FGED Vol 2 - Chapter 6.3.1
    
    // NOTE - The image is not formed on the near plane, rather it is formed on the projection plane at
    //  distance g from the camera. g's value is chosen so that the projection plane's interesection with
    //  the frustum has a vertical distance of 2 (range [-1, 1]). Because the camera points in the negative
    //  z direction, g has a negative value.

    // Projection plane (in camera space)
    // ____________________________
    // |           +1              |
    // |                           |
    // |-s                         |+s
    // |___________-1______________|

    f32 g = 1.0f / Tan(0.5f * fovVertical);
    f32 s = aspectRatio;
    
    // NOTE - The distances are positive for user convenience, but since Z- is forward
    //  in camera space, the actual Z values should be negated. However, since Z+ is
    //  forward in clip space, we would negate them again. So ultimately, the convenient
    //  positive values that get passed in are the correct ones!
    
    f32 n = nearDist;
    f32 f = farDist;

#if 1
    // Used for Opengl where NDC has Z [-1 to 1]
    // NOTE - Derived by solving the following system of equations:
    // -a + b / n = -1
    // -a + b / f = 1
    
    f32 k = 2 * f / (n - f);
    f32 a = k + 1;
    f32 b = n * k;
#else
    // Used (untested) for DirectX where NDC has Z [0 to 1]
    // NOTE - Derived by solving the following system of equations:
    // -a + b / n = 0
    // -a + b / f = 1
    
    f32 k = f / (n - f);
    f32 a = k;
    f32 b = n * k;
#endif

    // NOTE - The -1 in the final row ensures that the projected w values is positive for objects in
    //  front of the camera (they have negative z values in camera space, so -1 * zCam is positive)
    
    Mat4 result = {{
            { g / s, 0, 0, 0 },
            { 0, g, 0, 0 },
            { 0, 0, a, b },
            { 0, 0, -1, 0 }
        }};
    
    return result;
}

function Mat4
Mat4LookAtDir(Vec3 pos, Vec3 dir, Vec3 up)
{
    dir = vec_normalize_safe_or(dir, Vec3(0, 1, 0));
    up = vec_normalize_safe_or(up, dir + Vec3(0, 0, 1));

    Vec3 right = vec_cross(dir, up);
    up = vec_cross(right, dir);

    // NOTE - rotation is transposed from the matrix that would move item into
    //  the space defined by the camera, and translation is negated. This is due
    //  to the fact that the view matrix doesn't move the camera, but moves everything
    //  else in the opposite fashion.

    // NOTE - dir is negated because in camera space, dir corresponds to our negative z axis
    
    Mat4 rotation = {{
            { right.x, right.y, right.z, 0 },
            { up.x, up.y, up.z, 0 },
            { -dir.x, -dir.y, -dir.z, 0 },
            { 0, 0, 0, 1 }
        }};

    Mat4 translation = Mat4Translate(-pos);

    Mat4 result = rotation * translation;
    return result;
}

#if 0
function Mat4
Mat4LookAtTarget(Vec3 pos, Vec3 target, Vec3 up)
{
    Vec3 dir = target - pos;
    Mat4 result = Mat4LookAtDir(pos, dir, up);
    return result;
}
#endif

#endif // !CRT_DISABLED

function const f32 *
RowMajorPtr(const Mat4 * mat)
{
    return mat->e[0];
}

#if 0
// Circle2

struct Circle2
{
    Vec2 center;
    f32 radius;
};

function Circle2
CircleFromCenterRad(Vec2 center, f32 radius)
{
    Circle2 result;
    result.center = center;
    result.radius = radius;
    return result;
}

function bool
TestRectOverlapsCircle(Rect2 rect, Circle2 circle)
{
    Vec2 rectClosest = circle.center;

    if (circle.center.x < rect.min.x)
        rectClosest.x = rect.min.x;
    else if (circle.center.x > rect.max.x)
        rectClosest.x = rect.max.x;

    if (circle.center.y < rect.min.y)
        rectClosest.y = rect.min.y;
    else if (circle.center.y > rect.max.y)
        rectClosest.y = rect.max.y;

    if (LengthSq(circle.center - rectClosest) <= circle.radius * circle.radius)
        return true;

    return false;
}

function bool
TestCircleOverlapsCircle(Circle2 c0, Circle2 c1)
{
    f32 distSq = LengthSq(c0.center - c1.center);
    f32 r = c0.radius + c1.radius;
    f32 rSq = r * r;

    bool result = distSq < rSq;
    return result;
}

struct CircleOverlapTestResult
{
    bool overlaps;
    Vec2 penetration;
};

#if !CRT_DISABLED
function CircleOverlapTestResult
FullTestCircleOverlapsCircle(Circle2 c0, Circle2 c1)
{
    CircleOverlapTestResult result = {};
    f32 distSq = LengthSq(c0.center - c1.center);
    f32 r = c0.radius + c1.radius;
    f32 rSq = r * r;

    if (distSq < rSq)
    {
        result.overlaps = true;

        Vec2 displacementDir = NormalizeSafeXAxis(c0.center - c1.center);
        f32 displacementDist = c0.radius + c1.radius - sqrt(distSq);
        result.penetration = displacementDir * displacementDist;
    }
    
    return result;
}
#endif // !CRT_DISABLED

enum class WindingOrder : u8
{
    NIL = 0,

    Cw,
    Ccw,

    ENUM_COUNT
};

#endif
