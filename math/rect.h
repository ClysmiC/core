struct Range2
{
    f32 min;
    f32 max;

    Range2() = default;
    Range2(f32 min, f32 max) : min(min), max(max) {}
    Range2(Range2 const& other) : min(other.min), max(other.max) {}
};

function f32
range_dim(Range2 range)
{
    f32 result = range.max - range.min;
    return result;
}

template<class T, uint N>
struct Rect
{
    Vec<T, N> min;
    Vec<T, N> max;

    Rect() = default;
};

template<class T, class U, uint N>
function Rect<T, N>
rect_convert(Rect<U, N> other)
{
    Rect<T, N> result;
    result.min = vec_convert<T>(other.min);
    result.max = vec_convert<T>(other.max);
    return result;
}

using Rect2 = Rect<f32, 2>;
using Rect3 = Rect<f32, 3>;
using Rect2i = Rect<i32, 2>;
using Rect3i = Rect<i32, 3>;

template<class T, uint N>
function Vec<T, N>
rect_center(Rect<T, N> const& rect)
{
    Vec<T, N> result = 0.5f * (rect.min + rect.max);
    return result;
}

template<class T, uint N>
function Vec<T, N>
rect_dim(Rect<T, N> const& rect)
{
    Vec<T, N> result = rect.max - rect.min;
    return result;
}

template<class T, uint N>
function Rect<T, N>
rect_from_min_max(Vec<T, N> min, Vec<T, N> max)
{
    Rect<T, N> result;
    result.min = min;
    result.max = max;
    return result;
}

template<class T, uint N>
function Rect<T, N>
rect_from_points(Vec<T, N> const& p0, Vec<T, N> p1)
{
    Rect<T, N> result;
    result.min = vec_min(p0, p1);
    result.max = vec_max(p0, p1);
    return result;
}

template<class T, uint N>
function Rect<T, N>
rect_from_min_dim(Vec<T, N> const& min, Vec<T, N> dim)
{
    Rect<T, N> result;
    result.min = min;
    result.max = min + dim;
    return result;
}

template<class T, uint N>
function Rect<T, N>
rect_from_center_dim(Vec<T, N> const& center, Vec<T, N> dim)
{
    Rect<T, N> result;
    result.min = center - 0.5f * dim;
    result.max = center + 0.5f * dim;
    return result;
}

template<class T, uint N>
function Rect<T, N>
rect_from_center_half_dim(Vec<T, N> const& center, Vec<T, N> half_dim)
{
    Rect<T, N> result;
    result.min = center - half_dim;
    result.max = center + half_dim;
    return result;
}

template<class T, uint N>
function Rect<T, N>
rect_from_rect_offset(Rect<T, N> const& original, T offset)
{
    Rect2 result;
    result.min = original.min + Vec<T, N>(offset);
    result.max = original.max + Vec<T, N>(offset);
    return result;
}

template<class T, uint N>
function Rect<T, N>
rect_from_rect_offset(Rect<T, N> const& original, Vec<T, N> offset)
{
    Rect2 result;
    result.min = original.min + offset;
    result.max = original.max + offset;
    return result;
}

template<class T, uint N>
function bool
rect_contains_point(Rect<T, N> const& rect, Vec<T, N> point)
{
    bool result = true;
    for (int i = 0; i < N; i++)
    {
        result &= (point.elements[i] >= rect.min.elements[i] && point.elements[i] < rect.max.elements[i]);
    }

    return result;
}

template<class T, uint N>
function bool
rect_is_zero(Rect<T, N> const& rect)
{
    bool result = true;
    for (int i = 0; i < N; i++)
    {
        result &= (rect.min.elements[i] == T{} && rect.max.elements[i] == T{});
    }
    return result;
}
