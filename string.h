// TODO
// - Do I want a way to differentiate between empty string and null string? (I.e., empty string bytes
//   points somewhere, but cBytes is still 0? That'd be kinda weird, but I do see value in a distinction)
// - Do I want to make this a typedef for Slice<char> ?
// - Implement StringBuilder... should it be a typedef for a DynArray<char> ?

// NOTE - ZString is a zero terminated string, a.k.a. "c-string". You won't (well, shouldn't!) find "c-string"
//  in the code-base though, since that is too similar to cStrings which would mean "count of strings"

internal int
ZStringLength(char * string)
{
    int result = 0;
    char * cursor = string;
    while (*cursor)
    {
        result++;
        cursor++;
    }

    return result;
}

struct String
{
    // NOTE - Unicode compatible via UTF-8

    char * bytes;
    int cBytes;

    // TODO - Capacity? String "building" capabilities?

    String() = default;

    // HMM - Should this allow implicit from zString? Or write conversion function? Or is that redundant with StringFromLit?
    explicit String(char * zString) { this->bytes = zString; this->cBytes = ZStringLength(bytes); }
    String(char * zString, uint cBytes) { this->bytes = zString; this->cBytes = cBytes; }

    char & operator[](int iByte) { return bytes[iByte]; }
};

#define StringFromLit(stringLit) String(stringLit, ArrayLen(stringLit) - 1) // NOTE - ArrayLen runs at compile time for string literals, which is why this macro is better than the ctor if you know the string at comp-time

enum class NullTerminate : u8
{
    No = 0,
    Yes
};

internal void
CopyZString(char * src, char * dst, int cBytesDst, NullTerminate nullTerminateDst=NullTerminate::Yes)
{
    if (cBytesDst == 0) return;

    int cBytesDstCopy = ((bool)nullTerminateDst) ? cBytesDst - 1 : cBytesDst;

    int iByteDst = 0;
    while (*src && iByteDst < cBytesDstCopy)
    {
        *dst = *src;

        iByteDst++;
        src++;
        dst++;
    }

    if ((bool)nullTerminateDst) *dst = '\0';
}

// HMM - Should a String carry around if it is null-terminated or not? Need a convenient way to print strings...
//  Or maybe look into using printf("%.*s", length, string); ???

internal void
CopyString(String src, char * dst, int cBytesDst, NullTerminate nullTerminateDst=NullTerminate::Yes)
{
    if (cBytesDst == 0) return;

    char * srcCursor = src.bytes;
    int cBytesDstCopy = ((bool)nullTerminateDst) ? cBytesDst - 1 : cBytesDst;

    int iByteDst = 0;
    while (iByteDst < src.cBytes && iByteDst < cBytesDstCopy)
    {
        *dst = *srcCursor;

        iByteDst++;
        srcCursor++;
        dst++;
    }

    if ((bool)nullTerminateDst) *dst = '\0';
}

internal void
CopyString(String src, String dst)
{
    CopyString(src, dst.bytes, dst.cBytes, NullTerminate::No);
    dst.cBytes = Min(dst.cBytes, src.cBytes);
}

internal char *
DuplicateZString(char * src, MemoryRegion memory)
{
    int cBytesSrc = ZStringLength(src);
    int cBytesDst = cBytesDst + 1;

    char * dst = (char *)Allocate(memory, cBytesDst);
    CopyZString(src, dst, cBytesDst);
    return dst;
}

internal String
DuplicateZStringToString(char * src, MemoryRegion memory)
{
    String dst;
    dst.cBytes = ZStringLength(src);
    dst.bytes = (char *)Allocate(memory, dst.cBytes);
    CopyZString(src, dst.bytes, dst.cBytes, NullTerminate::No);
    return dst;
}

internal String
DuplicateString(String src, MemoryRegion memory)
{
    String dst;
    dst.cBytes = src.cBytes;
    dst.bytes = (char *)Allocate(memory, dst.cBytes);
    CopyString(src, dst);
    return dst;
}

internal char *
DuplicateStringToZString(String src, MemoryRegion memory)
{
    uint cBytesDst = src.cBytes + 1;
    char * dst = (char *)Allocate(memory, cBytesDst);
    CopyString(src, dst, cBytesDst, NullTerminate::Yes);
    return dst;
}

internal String
StringFromZString(
    char * src,
    MemoryRegion memory) // !optional =nullptr
{
    int cBytesSrc = ZStringLength(src);

    String dst;
    dst.cBytes = cBytesSrc;

    if (!memory) // @Gross - This API is weird. Most of these functions don't allow null memory regions. Should probably split this into a "Duplicate" which takes a region, and this which doesn't
    {
        dst.bytes = src;
    }
    else if (dst.cBytes > 0)
    {
        dst.bytes = (char *)Allocate(memory, dst.cBytes);
        CopyZString(src, dst.bytes, dst.cBytes, NullTerminate::No);
    }

    return dst;
}

internal char *
ZStringFromString(String src, MemoryRegion memory)
{
    int cBytesDst = src.cBytes + 1;
    char * dst = (char *)Allocate(memory, cBytesDst);
    CopyString(src, dst, cBytesDst, NullTerminate::Yes);
    return dst;
}


// @Cleanup - Passing the dest buffers deep in the parameter list make it kinda hard to grep for places where
//  I am initializing string buffers. Any string/stringview abstraction I may write in the future
//  should try to alleviate this

internal void
CopySubstring(
    char * src,
    int iSrcStart,
    int cntCharSrcToCopy, // NOTE - This is a count, not an end index!
    char * dst,
    int cBytesDst)
{
    // @Security - Audit this function for security flaws

    if (cBytesDst <= 0)
        return;

    int cntCharDst = cBytesDst - 1; // Leave 1 byte for null terminator
    int iCharDst = 0;

    int cntCharToCopy = Min(cntCharDst, cntCharSrcToCopy);

    char * cursor = src + iSrcStart;
    while (*cursor && iCharDst < cntCharToCopy)
    {
        *dst = *cursor;

        iCharDst++;
        cursor++;
        dst++;
    }

    Assert(iCharDst < cBytesDst);
    *dst = '\0';
}

internal void
StringConcat(
    const char * srcA,
    const char * srcB,
    char * dst,
    int cBytesDst)
{
    // @Security - Audit this function for security flaws

    if (cBytesDst <= 0)
        return;

    int cntCharDst = cBytesDst - 1; // Leave 1 byte for null terminator
    int iCharDst = 0;

    while (*srcA && iCharDst < cntCharDst)
    {
        *dst = *srcA;

        iCharDst++;
        srcA++;
        dst++;
    }

    while (*srcB && iCharDst < cntCharDst)
    {
        *dst = *srcB;

        iCharDst++;
        srcB++;
        dst++;
    }

    Assert(iCharDst < cBytesDst);
    AssertWarn(*srcA == '\0');
    AssertWarn(*srcB == '\0');

    *dst = '\0';
}

internal String
StringConcat(String srcA, String srcB, MemoryRegion memory)
{
    String result;
    result.cBytes = srcA.cBytes + srcB.cBytes;
    result.bytes = (char *)Allocate(memory, result.cBytes);
    CopyMemory(srcA.bytes, result.bytes, srcA.cBytes);
    CopyMemory(srcB.bytes, result.bytes + srcA.cBytes, srcB.cBytes);
    return result;
}

inline bool
AreStringsEqual(char * str0, char * str1)
{
    Assert(str0);
    Assert(str1);

    char * cursor0 = str0;
    char * cursor1 = str1;

    while (*cursor0 && *cursor1)
    {
        if (*cursor0 != *cursor1)
            return false;

        *cursor0++;
        *cursor1++;
    }

    return (*cursor0 == '\0' && *cursor1 == '\0');
}

inline bool
AreStringsEqual(String str0, char * str1)
{
    char * cursor0 = str0.bytes;
    char * endCursor0 = str0.bytes + str0.cBytes;

    char * cursor1 = str1;

    while ((cursor0 < endCursor0) && *cursor1)
    {
        if (*cursor0 != *cursor1)
            return false;

        *cursor0++;
        *cursor1++;
    }

    return (cursor0 == endCursor0) && (*cursor1 == '\0');
}

inline bool
AreStringsEqual(char * str0, String str1)
{
    bool result = AreStringsEqual(str1, str0);
    return result;
}

inline bool
AreStringsEqual(String str0, String str1)
{
    if (str0.cBytes != str1.cBytes) return false;
    if (str0.bytes == str1.bytes)     return true;

    char * cursor0 = str0.bytes;
    char * endCursor0 = str0.bytes + str0.cBytes;

    char * cursor1 = str1.bytes;
    char * endCursor1 = str1.bytes + str1.cBytes;

    while ((cursor0 < endCursor0) && (cursor1 < endCursor1))
    {
        if (*cursor0 != *cursor1)
            return false;

        *cursor0++;
        *cursor1++;
    }

    return (cursor0 == endCursor0) && (cursor1 == endCursor1);
}

inline bool
StringHasPrefix(char * str, char * prefix)
{
    char * strCursor = str;
    char * prefixCursor = prefix;

    while (*strCursor && *prefixCursor)
    {
        if (*strCursor != *prefixCursor)
            return false;

        *strCursor++;
        *prefixCursor++;
    }

    return (*prefixCursor == '\0');
}

inline bool
StringHasPrefix(String str, String prefix)
{
    if (prefix.cBytes > str.cBytes) return false;

    for (int iChar = 0; iChar < prefix.cBytes; iChar++)
    {
        char s = str[iChar];
        char x = prefix[iChar];

        if (s != x) return false;
    }

    return true;
}

inline bool
StringHasSuffix(char * str, char * suffix)
{
    int strLength = ZStringLength(str);
    int suffixLength = ZStringLength(suffix);

    if (suffixLength > strLength) return false;

    int iCharSuffix = 0;
    for (int iCharStr = strLength - suffixLength; iCharStr < strLength; iCharStr++)
    {
        char s = str[iCharStr];
        char x = suffix[iCharSuffix];

        if (s != x) return false;

        iCharSuffix++;
    }

    return true;
}

inline bool
StringHasSuffix(String str, String suffix)
{
    if (suffix.cBytes > str.cBytes) return false;

    int iCharSuffix = 0;
    for (int iCharStr = str.cBytes - suffix.cBytes; iCharStr < str.cBytes; iCharStr++)
    {
        char s = str[iCharStr];
        char x = suffix[iCharSuffix];

        if (s != x) return false;

        iCharSuffix++;
    }

    return true;
}

// @DependencyLeak
#define STB_SPRINTF_IMPLEMENTATION
#include "lib/stb/stb_sprintf.h"

internal int
ZStringFromPrintf(char * buffer, int cBytesBuffer, char * fmt, ...)
{
    va_list varargs;
    va_start(varargs, fmt);

    int result = stbsp_vsnprintf(buffer, cBytesBuffer, fmt, varargs);

    va_end(varargs);

    return result;
}

internal String
StringFromPrintf(MemoryRegion memory, int cBytesBuffer, char * fmt, ...)
{
    // TODO - I don't like having a cBytesBuffer param...
    //  but this requires some sort of re-allocing string builder (which we don't yet have)
    //  to work on arbitrarily sized strings...

    va_list varargs;
    va_start(varargs, fmt);

    String result;
    result.bytes = (char *)Allocate(memory, sizeof(char) * cBytesBuffer);
    result.cBytes = stbsp_vsnprintf(result.bytes, cBytesBuffer, fmt, varargs);

    va_end(varargs);

    return result;
}

internal int
IndexOfFirst(String string, char c)
{
    for (int iByte = 0; iByte < string.cBytes; iByte++)
    {
        if (string[iByte] == c) return iByte;
    }

    return -1;
}

internal int
IndexOfLast(String string, char c)
{
    for (int iByte = string.cBytes - 1; iByte >= 0; iByte--)
    {
        if (string[iByte] == c) return iByte;
    }

    return -1;
}

internal bool
IsEmpty(String string)
{
    return (string.cBytes == 0);
}

// @Cleanup - move to platform ?
internal String
GetDirectoryFromFullFilename(String fullFilename)
{
    String result = {};

    int iLastSlash = Max(IndexOfLast(fullFilename, '\\'), IndexOfLast(fullFilename, '/'));

    if (iLastSlash < 0) return result;

    result.bytes = fullFilename.bytes;
    result.cBytes = iLastSlash + 1;
    return result;
}

inline u32
SafeTruncateU64(u64 value)
{
    Assert(value <= 0xFFFFFFFF);
    u32 result = (u32)value;
    return result;
}


inline bool
IsDecimalDigit(char c)
{
    bool result = (c >= '0' && c <= '9');
    return result;
}

// @Slow (and naive/wrong)
// @Punt - Naive calculations probably not very precise for extreme inputs, but I don't really want to pull in stdlib just for strtof
// @Punt - Not handling overflow or underflow (for the final result, nor intermediate results)
// @Punt - Doesn't totally handle certain classes of malformed strings. It kind of assumes that you've done some pre-scanning work and are
//  passing a valid-ish string.
internal bool
TryParseF64FromEntireString(String string, f64 * poResult, char separator=0)
{
    int iCursor = 0;

    // Parse + or -

    f64 resultMultiply = 1.0;
    if (string.cBytes > 0 && string[0] == '-')
    {
        resultMultiply = -1;
        iCursor++;
    }
    else if (string.cBytes > 0 && string[0] == '+')
    {
        iCursor++;
    }

    // Parse whole part

    f64 resultWhole = 0;
    while (iCursor < string.cBytes)
    {
        char c = string[iCursor];
        if (separator && c == separator)
            continue;

        if (!IsDecimalDigit(c))
            break;

        iCursor++;

        resultWhole *= 10;
        resultWhole += (c - '0');
    }

    // Parse frac part

    f64 resultFrac = 0;
    if (iCursor < string.cBytes && string[iCursor] == '.')
    {
        iCursor++;

        f64 fracDigitMultiplier = 0.1;
        while (iCursor < string.cBytes)
        {
            char c = string[iCursor];
            if (separator && (c == separator))
                continue;

            if (!IsDecimalDigit(c))
                break;

            iCursor++;

            resultFrac += (c - '0') * fracDigitMultiplier;
            fracDigitMultiplier *= 0.1f;
        }
    }

    Assert(resultFrac < 1.0);

    // Parse exponent

    if (iCursor < string.cBytes &&
        (string[iCursor] == 'e' || string[iCursor] == 'E'))
    {
        iCursor++;

        bool isExpPos = true;
        if (string[iCursor] == '+')
        {
            iCursor++;
        }
        else if (string[iCursor] == '-')
        {
            iCursor++;
            isExpPos = false;
        }

        s32 expUnsigned = 0;
        while (iCursor < string.cBytes)
        {
            // NOTE - Scanner should ensure well-formed DecimalFloatLiteral has at least 1 decimal digit here.
            //  Parser doesn't check.

            char c = string[iCursor];
            if (separator && (c == separator))
                continue;

            if (!IsDecimalDigit(c))
                break;

            iCursor++;

            expUnsigned *= 10;
            expUnsigned += (c - '0');
        }

        if (isExpPos)
        {
            for (int i = 0; i < expUnsigned; i++)
            {
                resultMultiply *= 10;
            }
        }
        else
        {
            for (int i = 0; i < expUnsigned; i++)
            {
                resultMultiply *= 0.1;
            }
        }
    }

    *poResult = (resultWhole + resultFrac) * resultMultiply;

    bool result = (iCursor == string.cBytes);
    return result;
}

internal bool
TryParseF32FromEntireString(String string, f32 * poResult, char separator=0)
{
    f64 doubleFloat;
    bool result = TryParseF64FromEntireString(string, &doubleFloat, separator);

    *poResult = (f32)doubleFloat; // TODO - overflow check (underflow check too?)
    return result;
}

internal bool
TryParseU64FromEntireString(String string, u64 * poResult, char separator=0)
{
    *poResult = 0;

    bool hasSignificantDigit = false;

    int iCursor = 0;
    for ( ; iCursor < string.cBytes; iCursor++)
    {
        char c = string[iCursor];
        if (!IsDecimalDigit(c))
            break;

        if (separator && (c == separator))
            continue;

        if (!hasSignificantDigit && c == '0')
            continue;

        hasSignificantDigit = true;

        u64 before = *poResult;
        *poResult *= 10;
        *poResult += (c - '0');

        if (*poResult <= before)
            return false;
    }

    bool result = (iCursor == string.cBytes);
    return result;
}
