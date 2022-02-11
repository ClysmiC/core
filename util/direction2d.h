// --- Cardinal and Ordinal directions. Useful for UI and 2D stuff

enum class CardinalDir : u8
{
    Nil = 0,

    N = 1,
    E = 2,
    S = 3,
    W = 4,

    EnumCount,
};
DefineEnumOps(CardinalDir, u8);

enum class OrdinalDir : u8
{
    Nil = 0,

    NE = 1,
    SE = 2,
    SW = 3,
    NW = 4,

    EnumCount
};
DefineEnumOps(OrdinalDir, u8);

enum class Dir : u8
{
    Nil = 0,

    N = 1,
    NE = 2,
    E = 3,
    SE = 4,
    S = 5,
    SW = 6,
    W = 7,
    NW = 8,

    EnumCount,
};
DefineEnumOps(Dir, u8);

inline bool
IsCardinal(Dir dir)
{
    bool result = (dir == Dir::N) || (dir == Dir::E) || (dir == Dir::S) || (dir == Dir::W);
    return result;
}

inline bool
IsOrdinal(Dir dir)
{
    bool result = (dir == Dir::NE) || (dir == Dir::SE) || (dir == Dir::SW) || (dir == Dir::NW);
    return result;
}

enum class DirFlags : u8
{
    Nil = 0,

    // @Cleanup - This doesn't really iterate the same way Dir does...
    // HMM - We can't shift 1 << Dir to get to DirFlags either :((
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


enum class MajorOrder : u8
{
    Nil = 0,

    Row = Nil,
    Column,

    EnumCount
};
