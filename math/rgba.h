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

function bool
operator==(Rgb const& lhs, Rgb const& rhs)
{
    bool result =
        lhs.r == rhs.r &&
        lhs.g == rhs.g &&
        lhs.b == rhs.b;
    return result;
}

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
    union
    {
        struct { f32 r, g, b, a; };
        Rgb rgb;
    };

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

    explicit constexpr  Rgba(f32 rgb) :                     r(rgb),     g(rgb),     b(rgb),     a(1.0f)     {}
};

inline bool
rgba_is_zero(Rgba const& rgba)
{
    bool result = (rgba.r == 0 && rgba.g == 0 && rgba.b == 0 && rgba.a == 0);
    return result;
}

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
    static Rgba constexpr GRAY          (0.5f, 0.5f, 0.5f, 1.0f);

    static Rgba constexpr TRANSPARENT2  (1, 1, 1, 0);   // TODO - rename to TRANSPARENT... but conflicts with windows define...
    static Rgba constexpr ZERO   (0, 0, 0, 0);
}
