#define DefaultInvalid default: { ASSERT_FALSE; } break
#define DefaultNilInvalid case 0: default: { ASSERT_FALSE; } break
#define DefaultInvalidEnum(ENUM) case ENUM::ENUM_COUNT: default: { ASSERT_FALSE; } break
#define DefaultNilInvalidEnum(ENUM) case ENUM::NIL: case ENUM::ENUM_COUNT: default: { ASSERT_FALSE; } break

#define DefaultInvalidFallthrough default: { ASSERT_FALSE; }
#define DefaultInvalidEnumFallthrough(ENUM) case ENUM::ENUM_COUNT: default: { ASSERT_FALSE; }
#define DefaultNilInvalidEnumFallthrough(ENUM) case ENUM::NIL: case ENUM::ENUM_COUNT: default: { ASSERT_FALSE; }

#define DefineFlagOpsSkipIoTemplate(ENUM, INT_TYPE) \
    constexpr ENUM operator|(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs | (INT_TYPE)rhs); } \
    constexpr ENUM & operator|=(ENUM & lhs, ENUM rhs) { lhs = lhs | rhs; return lhs; } \
    constexpr ENUM operator&(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs & (INT_TYPE)rhs); } \
    constexpr ENUM & operator&=(ENUM & lhs, ENUM rhs) { lhs = lhs & rhs; return lhs; } \
    constexpr ENUM operator^(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs ^ (INT_TYPE)rhs); } \
    constexpr ENUM & operator^=(ENUM & lhs, ENUM rhs) { lhs = lhs ^ rhs; return lhs; } \
    bool constexpr IsFlagSet(ENUM flags, ENUM query) { return (flags & query) == query; } \
    bool constexpr IsAnyFlagSet(ENUM flags, ENUM query) { return (INT_TYPE)(flags & query) != 0; } \
    constexpr ENUM operator~(ENUM e) { return (ENUM)~(INT_TYPE)e; } \
    STATIC_ASSERT(sizeof(ENUM) == sizeof(INT_TYPE))


#define DefineFlagOps(ENUM, INT_TYPE) \
    DefineFlagOpsSkipIoTemplate(ENUM, INT_TYPE); \
    STATIC_ASSERT(sizeof(ENUM) == sizeof(INT_TYPE))
// template<> struct io_is_atom<ENUM> { static bool constexpr value = true; };

    // ID's often increment/decrement by an integer
#define DefineIdOps(ENUM, INT_TYPE) \
    bool constexpr operator==(ENUM lhs, INT_TYPE rhs) { return (INT_TYPE)lhs == rhs; } \
    bool constexpr operator==(INT_TYPE lhs, ENUM rhs) { return rhs == lhs; } \
    constexpr ENUM& operator++(ENUM & value) { value = (ENUM)((INT_TYPE)value + 1); return value; } \
    constexpr ENUM& operator++(ENUM & value, int) { value = (ENUM)((INT_TYPE)value + 1); return value; } \
    constexpr ENUM& operator--(ENUM & value) { value = (ENUM)((INT_TYPE)value - 1); return value; } \
    constexpr ENUM& operator--(ENUM & value, int) { value = (ENUM)((INT_TYPE)value - 1); return value; } \
    constexpr ENUM operator+(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs + (INT_TYPE)rhs); } \
    constexpr ENUM operator+(ENUM lhs, INT_TYPE rhs) { return (ENUM)((INT_TYPE)lhs + rhs); } \
    constexpr ENUM operator+(INT_TYPE lhs, ENUM rhs) { return (ENUM)(lhs + (INT_TYPE)rhs); } \
    constexpr ENUM& operator+=(ENUM & lhs, ENUM rhs) { lhs = lhs + rhs; return lhs; } \
    constexpr ENUM& operator+=(ENUM & lhs, INT_TYPE rhs) { lhs = lhs + rhs; return lhs; } \
    constexpr ENUM operator-(ENUM lhs, ENUM rhs) { return (ENUM)((INT_TYPE)lhs - (INT_TYPE)rhs); } \
    constexpr ENUM operator-(ENUM lhs, INT_TYPE rhs) { return (ENUM)((INT_TYPE)lhs - rhs); } \
    constexpr ENUM& operator-=(ENUM & lhs, ENUM rhs) { lhs = lhs - rhs; return lhs; } \
    constexpr ENUM& operator-=(ENUM & lhs, INT_TYPE rhs) { lhs = lhs - rhs; return lhs; } \
    STATIC_ASSERT(sizeof(ENUM) == sizeof(INT_TYPE))

#define DefineEnumOps(ENUM, INT_TYPE) \
    DefineIdOps(ENUM, INT_TYPE); \
    bool constexpr IsEnumValid(ENUM value) { return value > ENUM::NIL && value < ENUM::ENUM_COUNT; } \
    STATIC_ASSERT(sizeof(ENUM) == sizeof(INT_TYPE))

#define DefineEnumOpsAllowNil(ENUM, INT_TYPE) \
    DefineIdOps(ENUM, INT_TYPE); \
    bool constexpr IsEnumValid(ENUM value) { return value >= ENUM::NIL && value < ENUM::ENUM_COUNT; } \
    STATIC_ASSERT(sizeof(ENUM) == sizeof(INT_TYPE))

#define ForEnum(ENUM, it) for (ENUM it = (ENUM)1; it < ENUM::ENUM_COUNT; it++)
#define ForEnumAllowNil(ENUM, it) for (ENUM it = ENUM::NIL; it < ENUM::ENUM_COUNT; it++)
