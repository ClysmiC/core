// --- Cardinal and Ordinal directions. Useful for UI and 2D stuff

enum class CardinalDir : u8
{
    Nil = 0,

    N,
    E,
    S,
    W,

    EnumCount
};

enum class OrdinalDir : u8
{
    Nil = 0,

    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW,

    EnumCount
};

enum class DirFlags : u8
{
    Nil = 0,

    N = (1 << 0),
    E = (1 << 1),
    S = (1 << 2),
    W = (1 << 3),

    NE = DirFlags::N | DirFlags::E,
    NW = DirFlags::N | DirFlags::W,
    SE = DirFlags::S | DirFlags::E,
    SW = DirFlags::S | DirFlags::W,
};
DefineFlagOps(DirFlags, u8);
