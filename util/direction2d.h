// --- Cardinal and Ordinal directions. Useful for UI and 2D stuff

enum class CardinalDir : u8
{
    NIL = 0,

    N = 1,
    E = 2,
    S = 3,
    W = 4,

    ENUM_COUNT,
};
DefineEnumOps(CardinalDir, u8);

enum class OrdinalDir : u8
{
    NIL = 0,

    NE = 1,
    SE = 2,
    SW = 3,
    NW = 4,

    ENUM_COUNT
};
DefineEnumOps(OrdinalDir, u8);

inline Vec2
CornerPos(Rect2 rect, OrdinalDir dir)
{
    // NOTE - +Y is up (N)

    Vec2 result = {};
    switch (dir)
    {
        case OrdinalDir::NE:    result = rect.max; break;
        case OrdinalDir::SE:    result = Vec2(rect.max.x, rect.min.y); break;
        case OrdinalDir::SW:    result = rect.min; break;
        case OrdinalDir::NW:    result = Vec2(rect.min.x, rect.max.y); break;

        DefaultNilInvalidEnum(OrdinalDir);
    }

    return result;
}

enum class Dir : u8
{
    NIL = 0,

    N = 1,
    NE = 2,
    E = 3,
    SE = 4,
    S = 5,
    SW = 6,
    W = 7,
    NW = 8,

    ENUM_COUNT,
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

enum class Dir_Flags : u8
{
    NIL = 0,

    // @Cleanup - This doesn't really iterate the same way Dir does...
    // HMM - We can't shift 1 << Dir to get to Dir_Flags either :((
    N = (1 << 0),
    E = (1 << 1),
    S = (1 << 2),
    W = (1 << 3),

    NE = Dir_Flags::N | Dir_Flags::E,
    NW = Dir_Flags::N | Dir_Flags::W,
    SE = Dir_Flags::S | Dir_Flags::E,
    SW = Dir_Flags::S | Dir_Flags::W,
};
DefineFlagOps(Dir_Flags, u8);


enum class MajorOrder : u8
{
    Row = 0,
    NIL = 0,

    Column,

    ENUM_COUNT
};
