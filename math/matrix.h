// Mat3

struct Mat3
{
    f32 e[3][3];

    f32*
    operator[](int i)
    {
        // NOTE - Only overloads the first index to point to the correct array, then
        // the second index is handled by the built-in c++ [] operator
        f32* result = e[i];
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
