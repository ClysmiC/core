// Resources
// https://utf8everywhere.org/
// https://en.wikipedia.org/wiki/UTF-8
// https://en.wikipedia.org/wiki/UTF-16

namespace UTF
{
    // HMM - Should this be an error enum? Idk.
    static constexpr u32 invalidCodePoint = 0xFFFFFFFF;
}

// --- UTF-8

namespace UTF8
{

struct CodeUnitMetadata
{
    u8 markerMask;
    u8 markerValue;
    u8 cBitsMarker;
    u8 codePointMask;
    u8 cBitsCodePoint;
    u32 maxCodePoint; // Used to test for "overlong" encoding. Only valid for leading byte metadata.
};

enum class CodeUnitType : u8
{
    Continuation = 0,
    NIL = 0,

    LeadLen1,
    LeadLen2,
    LeadLen3,
    LeadLen4,

    ENUM_COUNT
};

// Leading byte
static CodeUnitMetadata codeUnitMetadata[] =
{
    { 0b11000000, 0b10000000, 2, 0b00111111, 6, 0 /*unused*/ },                // Continuation
    { 0b10000000, 0b00000000, 1, 0b01111111, 7, 0b1111111 },                // LeadLen1
    { 0b11100000, 0b11000000, 3, 0b00011111, 5, 0b11111111111 },            // LeadLen2
    { 0b11110000, 0b11100000, 4, 0b00001111, 4, 0b1111111111111111 },        // LeadLen3
    { 0b11111000, 0b11110000, 5, 0b00000111, 3, 0b111111111111111111111 },    // LeadLen4
};

} // namespace UTF8

inline bool
IsCodeUnitType(u8 codeUnit, UTF8::CodeUnitType type)
{
    UTF8::CodeUnitMetadata * meta = UTF8::codeUnitMetadata + (int)type;
    bool result = (codeUnit & meta->markerMask) == meta->markerValue;
    return result;
}

 u32
NextCodePointUtf8_(u8 * bytesUtf8, int cBytesUtf8, int* pI)
{
    int i = *pI;

    if (i < 0 || i >= cBytesUtf8)
        goto LError;

    u32 result = UTF::invalidCodePoint;

    // NOTE - Immediately cast up to u32 to avoid any surprises when bit-twiddling
    u32 b0 = (u32)bytesUtf8[i++];
    if (IsCodeUnitType((u8)b0, UTF8::CodeUnitType::LeadLen1))
    {
        result = (u32)b0;
    }
    else if (IsCodeUnitType((u8)b0, UTF8::CodeUnitType::LeadLen2))
    {
        if (i >= cBytesUtf8)
            goto LError;

        u32 b1 = (u32)bytesUtf8[i++];
        if (!IsCodeUnitType((u8)b1, UTF8::CodeUnitType::Continuation))
            goto LError;

        UTF8::CodeUnitMetadata * leadByteMeta = UTF8::codeUnitMetadata + (int)UTF8::CodeUnitType::LeadLen2;
        UTF8::CodeUnitMetadata * contByteMeta = UTF8::codeUnitMetadata + (int)UTF8::CodeUnitType::Continuation;
        u32 codePoint =
            ((u32)(b0 & leadByteMeta->codePointMask) << (1 * contByteMeta->cBitsCodePoint)) |
            ((u32)(b1 & contByteMeta->codePointMask));

        // "overlong" encoding
        if (codePoint <= UTF8::codeUnitMetadata[(int)UTF8::CodeUnitType::LeadLen1].maxCodePoint)
            goto LError;

        result = codePoint;
    }
    else if (IsCodeUnitType((u8)b0, UTF8::CodeUnitType::LeadLen3))
    {
        if (i >= cBytesUtf8 - 1)
            goto LError;

        u32 b1 = (u32)bytesUtf8[i++];
        u32 b2 = (u32)bytesUtf8[i++];
        if (!IsCodeUnitType((u8)b1, UTF8::CodeUnitType::Continuation) ||
            !IsCodeUnitType((u8)b2, UTF8::CodeUnitType::Continuation))
            goto LError;

        UTF8::CodeUnitMetadata * leadByteMeta = UTF8::codeUnitMetadata + (int)UTF8::CodeUnitType::LeadLen3;
        UTF8::CodeUnitMetadata * contByteMeta = UTF8::codeUnitMetadata + (int)UTF8::CodeUnitType::Continuation;
        u32 codePoint =
            ((u32)(b0 & leadByteMeta->codePointMask) << (2 * contByteMeta->cBitsCodePoint)) |
            ((u32)(b1 & contByteMeta->codePointMask) << (1 * contByteMeta->cBitsCodePoint)) |
            ((u32)(b2 & contByteMeta->codePointMask));

        // "overlong" encoding
        if (codePoint <= UTF8::codeUnitMetadata[(int)UTF8::CodeUnitType::LeadLen2].maxCodePoint)
            goto LError;

        result = codePoint;
    }
    else if (IsCodeUnitType((u8)b0, UTF8::CodeUnitType::LeadLen4))
    {
        if (i >= cBytesUtf8 - 2)
            goto LError;

        u32 b1 = (u32)bytesUtf8[i++];
        u32 b2 = (u32)bytesUtf8[i++];
        u32 b3 = (u32)bytesUtf8[i++];
        if (!IsCodeUnitType((u8)b1, UTF8::CodeUnitType::Continuation) ||
            !IsCodeUnitType((u8)b2, UTF8::CodeUnitType::Continuation) ||
            !IsCodeUnitType((u8)b3, UTF8::CodeUnitType::Continuation))
            goto LError;

        UTF8::CodeUnitMetadata * leadByteMeta = UTF8::codeUnitMetadata + (int)UTF8::CodeUnitType::LeadLen4;
        UTF8::CodeUnitMetadata * contByteMeta = UTF8::codeUnitMetadata + (int)UTF8::CodeUnitType::Continuation;
        u32 codePoint =
            ((u32)(b0 & leadByteMeta->codePointMask) << (3 * contByteMeta->cBitsCodePoint)) |
            ((u32)(b1 & contByteMeta->codePointMask) << (2 * contByteMeta->cBitsCodePoint)) |
            ((u32)(b2 & contByteMeta->codePointMask) << (1 * contByteMeta->cBitsCodePoint)) |
            ((u32)(b3 & contByteMeta->codePointMask));

        // "overlong" encoding
        if (codePoint <= UTF8::codeUnitMetadata[(int)UTF8::CodeUnitType::LeadLen3].maxCodePoint)
            goto LError;

        result = codePoint;
    }
    else
    {
        goto LError;
    }

    *pI = i;
    return result;

LError:    // @goto
    return UTF::invalidCodePoint;
}

// --- UTF-16

 u32
NextCodePointUtf16_(u8 * bytesUtf16, int cBytesUtf16, int* pI, Endianness endianness=Endianness::LITTLE)
{
    int i = *pI;

    if (i < 0 || i >= cBytesUtf16 - 1)
        goto LError;

    // NOTE - Immediately cast up to u32 to avoid any surprises when bit-twiddling
    u32 b0 = (u32)bytesUtf16[i++];
    u32 b1 = (u32)bytesUtf16[i++];

    u32 * lowByte =  (endianness == Endianness::LITTLE) ? &b0 : &b1;
    u32 * highByte = (endianness == Endianness::LITTLE) ? &b1 : &b0;

    u32 result = UTF::invalidCodePoint;
    u32 codeUnit0 = (*highByte << 8) | *lowByte;
    if (codeUnit0 <= 0xD7FF || (codeUnit0 >= 0xE000 && codeUnit0 <= 0xFFFF))
    {
        result = codeUnit0;
    }
    else if (codeUnit0 >= 0xD800 && codeUnit0 <= 0xDBFF)
    {
        if (i >= cBytesUtf16 - 1)
            goto LError;

        u32 b2 = (u32)bytesUtf16[i++];
        u32 b3 = (u32)bytesUtf16[i++];

        u32 * lowByte =  (endianness == Endianness::LITTLE) ? &b2 : &b3;
        u32 * highByte = (endianness == Endianness::LITTLE) ? &b3 : &b2;

        u32 codeUnit1 = (*highByte << 8) | *lowByte;

        u32 highSurrogateIntermediate = (codeUnit0 - 0xD800) << 10;
        u32 lowSurrogateIntermediate = (codeUnit1 - 0xDC00);
        result = highSurrogateIntermediate + lowSurrogateIntermediate + 0x10000;
    }

    // TODO - some kind of final validation to make sure the code point we generated is legal?

    *pI = i;
    return result;

LError:    // @goto
    return UTF::invalidCodePoint;
}

// --- Conversion

inline u8
MakeUtf8CodeUnit_Fast(u32 codePoint, UTF8::CodeUnitType codeUnitType, int cFollowingContinuationBytes)
{
    using namespace UTF8;

    CodeUnitMetadata * meta = codeUnitMetadata + (int)codeUnitType;
    CodeUnitMetadata * contMeta = codeUnitMetadata + (int)CodeUnitType::Continuation;
    u8 result = (u8)(((codePoint >> (cFollowingContinuationBytes * contMeta->cBitsCodePoint)) & meta->codePointMask) | meta->markerValue);
    return result;
}

 String
Utf8FromUtf16(u8 * bytesUtf16, int cBytesUtf16, Memory_Region memory, Endianness endianness=Endianness::LITTLE)
{
    String result = {};

    if (cBytesUtf16 & 1)
        goto LError;

    int cBytesRequired = 0;
    {
        int iByteScan = 0;
        while (iByteScan < cBytesUtf16)
        {
            u32 codePoint = NextCodePointUtf16_(bytesUtf16, cBytesUtf16, &iByteScan, endianness);
            if (codePoint == UTF::invalidCodePoint)
                goto LError;

            if (codePoint <= 0x7F)
            {
                cBytesRequired += 1;
            }
            else if (codePoint <= 0x7FF)
            {
                cBytesRequired += 2;
            }
            else if (codePoint <= 0xFFFF)
            {
                cBytesRequired += 3;
            }
            else
            {
                cBytesRequired += 4;
            }
        }
    }

    result.data = (char*)allocate(memory, cBytesRequired);
    result.length = cBytesRequired;

    int iByteWrite = 0;
    {
        int iByteScan = 0;
        while (iByteScan < cBytesUtf16)
        {
            using namespace UTF8;

            u32 codePoint = NextCodePointUtf16_(bytesUtf16, cBytesUtf16, &iByteScan, endianness);
            Assert(codePoint != UTF::invalidCodePoint);    // Was already validated when we were counting bytes

            if (codePoint <= 0x7F)
            {
                result.data[iByteWrite++] = MakeUtf8CodeUnit_Fast(codePoint, CodeUnitType::LeadLen1, 0);
            }
            else if (codePoint <= 0x7FF)
            {
                result.data[iByteWrite++] = MakeUtf8CodeUnit_Fast(codePoint, CodeUnitType::LeadLen2, 1);
                result.data[iByteWrite++] = MakeUtf8CodeUnit_Fast(codePoint, CodeUnitType::Continuation, 0);
            }
            else if (codePoint <= 0xFFFF)
            {
                result.data[iByteWrite++] = MakeUtf8CodeUnit_Fast(codePoint, CodeUnitType::LeadLen3, 2);
                result.data[iByteWrite++] = MakeUtf8CodeUnit_Fast(codePoint, CodeUnitType::Continuation, 1);
                result.data[iByteWrite++] = MakeUtf8CodeUnit_Fast(codePoint, CodeUnitType::Continuation, 0);
            }
            else
            {
                result.data[iByteWrite++] = MakeUtf8CodeUnit_Fast(codePoint, CodeUnitType::LeadLen4, 3);
                result.data[iByteWrite++] = MakeUtf8CodeUnit_Fast(codePoint, CodeUnitType::Continuation, 2);
                result.data[iByteWrite++] = MakeUtf8CodeUnit_Fast(codePoint, CodeUnitType::Continuation, 1);
                result.data[iByteWrite++] = MakeUtf8CodeUnit_Fast(codePoint, CodeUnitType::Continuation, 0);
            }
        }
    }

    Assert(iByteWrite == cBytesRequired);
    return result;

LError:    // @goto
    result = {};    // @Hack - Need a better way to handle this
    return result;
}
