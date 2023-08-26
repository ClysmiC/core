// TODO
// - Do I want a way to differentiate between empty string and null string? (I.e., empty string bytes
//   points somewhere, but cBytes is still 0? That'd be kinda weird, but I do see value in a distinction)
// - Do I want to make this a typedef for Slice<char> ?
// - Implement StringBuilder... should it be a typedef for a DynArray<char> ?

// NOTE - ZString is a zero terminated string, a.k.a. "c-string". You won't (well, shouldn't!) find "c-string"
//  in the code-base though, since that is too similar to cStrings which would mean "count of strings"

function int
ZStringLength(char* string)
{
    int result = 0;
    char* cursor = string;
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

    char* bytes;
    int cBytes;

    // TODO - Capacity? String "building" capabilities?

    String() = default;

    // HMM - Should this allow implicit from zString? Or write conversion function? Or is that redundant with StringFromLit?
    explicit String(char* zString) { this->bytes = zString; this->cBytes = ZStringLength(bytes); }
    String(char* zString, uint cBytes) { this->bytes = zString; this->cBytes = cBytes; }

    char & operator[](int iByte) { return bytes[iByte]; }
};

#define StringFromLit(stringLit) String(stringLit, ArrayLen(stringLit) - 1) // NOTE - ArrayLen runs at compile time for string literals, which is why this macro is better than the ctor if you know the string at comp-time

enum class Null_Terminate : u8
{
    NO = 0,
    NIL = 0,

    YES
};

function void
CopyZString(char* src, char* dst, int cBytesDst, Null_Terminate nullTerminateDst=Null_Terminate::YES)
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

function void
CopyString(String src, char* dst, int cBytesDst, Null_Terminate nullTerminateDst=Null_Terminate::YES)
{
    if (cBytesDst == 0) return;

    char* srcCursor = src.bytes;
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

function void
CopyString(String src, String dst)
{
    CopyString(src, dst.bytes, dst.cBytes, Null_Terminate::NO);
    dst.cBytes = min(dst.cBytes, src.cBytes);
}

function char*
DuplicateZString(char* src, Memory_Region memory)
{
    int cBytesSrc = ZStringLength(src);
    int cBytesDst = cBytesSrc + 1;

    char* dst = (char*)allocate(memory, cBytesDst);
    CopyZString(src, dst, cBytesDst);
    return dst;
}

function String
DuplicateZStringToString(char* src, Memory_Region memory)
{
    String dst;
    dst.cBytes = ZStringLength(src);
    dst.bytes = (char*)allocate(memory, dst.cBytes);
    CopyZString(src, dst.bytes, dst.cBytes, Null_Terminate::NO);
    return dst;
}

function String
DuplicateString(String src, Memory_Region memory)
{
    String dst;
    dst.cBytes = src.cBytes;
    dst.bytes = (char*)allocate(memory, dst.cBytes);
    CopyString(src, dst);
    return dst;
}

function char*
DuplicateStringToZString(String src, Memory_Region memory)
{
    uint cBytesDst = src.cBytes + 1;
    char* dst = (char*)allocate(memory, cBytesDst);
    CopyString(src, dst, cBytesDst, Null_Terminate::YES);
    return dst;
}

function String
StringFromZString(
    char* src,
    Memory_Region memory=nullptr)
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
        dst.bytes = (char*)allocate(memory, dst.cBytes);
        CopyZString(src, dst.bytes, dst.cBytes, Null_Terminate::NO);
    }

    return dst;
}

function char*
ZStringFromString(String src, Memory_Region memory)
{
    int cBytesDst = src.cBytes + 1;
    char* dst = (char*)allocate(memory, cBytesDst);
    CopyString(src, dst, cBytesDst, Null_Terminate::YES);
    return dst;
}


// @Cleanup - Passing the dest buffers deep in the parameter list make it kinda hard to grep for places where
//  I am initializing string buffers. Any string/stringview abstraction I may write in the future
//  should try to alleviate this

function void
CopySubstring(
    char* src,
    int iSrcStart,
    int cntCharSrcToCopy, // NOTE - This is a count, not an end index!
    char* dst,
    int cBytesDst)
{
    if (cBytesDst <= 0)
        return;

    int cntCharDst = cBytesDst - 1; // Leave 1 byte for null terminator
    int iCharDst = 0;

    int cntCharToCopy = min(cntCharDst, cntCharSrcToCopy);

    char* cursor = src + iSrcStart;
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

function void
StringConcat(
    const char* srcA,
    const char* srcB,
    char* dst,
    int cBytesDst)
{
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

function String
StringInsert(
    String srcA,
    int iInsert,
    String srcB,
    Memory_Region memory)
{
    if (iInsert < 0)
    {
        AssertFalseWarn;
        iInsert = 0;
    }
    else if (iInsert > srcA.cBytes)
    {
        AssertFalseWarn;
        iInsert = srcA.cBytes;
    }

    String result;
    result.cBytes = srcA.cBytes + srcB.cBytes;
    result.bytes = (char*)allocate(memory, result.cBytes);
    mem_copy(result.bytes, srcA.bytes, iInsert);
    mem_copy(result.bytes + iInsert, srcB.bytes, srcB.cBytes);
    mem_copy(result.bytes + iInsert + srcB.cBytes, srcA.bytes + iInsert, srcA.cBytes - iInsert);
    return result;
}

function String
StringConcat(String srcA, String srcB, Memory_Region memory)
{
    String result;
    result.cBytes = srcA.cBytes + srcB.cBytes;
    result.bytes = (char*)allocate(memory, result.cBytes);
    mem_copy(result.bytes, srcA.bytes, srcA.cBytes);
    mem_copy(result.bytes + srcA.cBytes, srcB.bytes, srcB.cBytes);
    return result;
}

function bool
AreStringsEqual(char* str0, char* str1)
{
    Assert(str0);
    Assert(str1);

    char* cursor0 = str0;
    char* cursor1 = str1;

    while (*cursor0 && *cursor1)
    {
        if (*cursor0 != *cursor1)
            return false;

        *cursor0++;
        *cursor1++;
    }

    return (*cursor0 == '\0' && *cursor1 == '\0');
}

function bool
AreStringsEqual(String str0, char* str1)
{
    char* cursor0 = str0.bytes;
    char* endCursor0 = str0.bytes + str0.cBytes;

    char* cursor1 = str1;

    while ((cursor0 < endCursor0) && *cursor1)
    {
        if (*cursor0 != *cursor1)
            return false;

        *cursor0++;
        *cursor1++;
    }

    return (cursor0 == endCursor0) && (*cursor1 == '\0');
}

function bool
AreStringsEqual(char* str0, String str1)
{
    bool result = AreStringsEqual(str1, str0);
    return result;
}

function bool
AreStringsEqual(String str0, String str1)
{
    if (str0.cBytes != str1.cBytes) return false;
    if (str0.bytes == str1.bytes)   return true;

    char* cursor0 = str0.bytes;
    char* endCursor0 = str0.bytes + str0.cBytes;

    char* cursor1 = str1.bytes;
    char* endCursor1 = str1.bytes + str1.cBytes;

    while ((cursor0 < endCursor0) && (cursor1 < endCursor1))
    {
        if (*cursor0 != *cursor1)
            return false;

        *cursor0++;
        *cursor1++;
    }

    return (cursor0 == endCursor0) && (cursor1 == endCursor1);
}

function char
AsciiLowerCase(char ascii)
{
    char result = ascii;
    if (ascii >= 'A' && ascii <= 'Z')
    {
        result -= ('A' - 'a');
    }

    return result;
}

function char
AsciiUpperCase(char ascii)
{
    char result = ascii;
    if (ascii >= 'a' && ascii <= 'z')
    {
        result += ('A' - 'a');
    }

    return result;
}

function bool
AreStringsEqualIgnoreCase(char* str0, char* str1)
{
    Assert(str0);
    Assert(str1);

    char* cursor0 = str0;
    char* cursor1 = str1;

    while (*cursor0 && *cursor1)
    {
        // @Punt - Doesn't support unicode strings
        if (AsciiLowerCase(*cursor0) != AsciiLowerCase(*cursor1))
            return false;

        *cursor0++;
        *cursor1++;
    }

    return (*cursor0 == '\0' && *cursor1 == '\0');
}

function bool
AreStringsEqualIgnoreCase(String str0, char* str1)
{
    char* cursor0 = str0.bytes;
    char* endCursor0 = str0.bytes + str0.cBytes;

    char* cursor1 = str1;

    while ((cursor0 < endCursor0) && *cursor1)
    {
        // @Punt - Doesn't support unicode strings
        if (AsciiLowerCase(*cursor0) != AsciiLowerCase(*cursor1))
            return false;

        *cursor0++;
        *cursor1++;
    }

    return (cursor0 == endCursor0) && (*cursor1 == '\0');
}

function bool
AreStringsEqualIgnoreCase(char* str0, String str1)
{
    bool result = AreStringsEqual(str1, str0);
    return result;
}

function bool
AreStringsEqualIgnoreCase(String str0, String str1)
{
    if (str0.cBytes != str1.cBytes) return false;
    if (str0.bytes == str1.bytes)   return true;

    char* cursor0 = str0.bytes;
    char* endCursor0 = str0.bytes + str0.cBytes;

    char* cursor1 = str1.bytes;
    char* endCursor1 = str1.bytes + str1.cBytes;

    while ((cursor0 < endCursor0) && (cursor1 < endCursor1))
    {
        // @Punt - Doesn't support unicode strings
        if (AsciiLowerCase(*cursor0) != AsciiLowerCase(*cursor1))
            return false;

        *cursor0++;
        *cursor1++;
    }

    return (cursor0 == endCursor0) && (cursor1 == endCursor1);
}

function bool
StringHasPrefix(char* str, char* prefix)
{
    char* strCursor = str;
    char* prefixCursor = prefix;

    while (*strCursor && *prefixCursor)
    {
        if (*strCursor != *prefixCursor)
            return false;

        *strCursor++;
        *prefixCursor++;
    }

    return (*prefixCursor == '\0');
}

function bool
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

function bool
StringHasSuffix(char* str, char* suffix)
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

function bool
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

// TODO - get rid of core dependency on stb_sprintf?
#include "lib/stb/stb_sprintf.h"

function int
ZStringFromPrintf_v(char* buffer, int cBytesBuffer, char const* fmt, va_list varargs)
{
    int result = stbsp_vsnprintf(buffer, cBytesBuffer, fmt, varargs);
    return result;
}

function int
ZStringFromPrintf(char* buffer, int cBytesBuffer, char const* fmt, ...)
{
    va_list varargs;
    va_start(varargs, fmt);
    int result = ZStringFromPrintf_v(buffer, cBytesBuffer, fmt, varargs);
    va_end(varargs);
    return result;
}

function String
StringFromPrintf_v(Memory_Region memory, int cBytesBuffer, char const* fmt, va_list varargs)
{
    // TODO - I don't like having a cBytesBuffer param...
    //  but this requires some sort of re-allocing string builder (which we don't yet have)
    //  to work on arbitrarily sized strings...

    String result;
    result.bytes = (char*)allocate(memory, sizeof(char) * cBytesBuffer);
    result.cBytes = stbsp_vsnprintf(result.bytes, cBytesBuffer, fmt, varargs);
    return result;
}

function String
StringFromPrintf(Memory_Region memory, int cBytesBuffer, char const* fmt, ...)
{
    // TODO - I don't like having a cBytesBuffer param...
    //  but this requires some sort of re-allocing string builder (which we don't yet have)
    //  to work on arbitrarily sized strings...

    va_list varargs;
    va_start(varargs, fmt);
    String result = StringFromPrintf_v(memory, cBytesBuffer, fmt, varargs);
    va_end(varargs);
    return result;
}

function int
IndexOfFirst(String string, char c)
{
    for (int iByte = 0; iByte < string.cBytes; iByte++)
    {
        if (string[iByte] == c) return iByte;
    }

    return -1;
}

function int
IndexOfLast(String string, char c)
{
    for (int iByte = string.cBytes - 1; iByte >= 0; iByte--)
    {
        if (string[iByte] == c) return iByte;
    }

    return -1;
}

function bool
IsEmpty(String string)
{
    return (string.cBytes == 0);
}

// @Cleanup - move to platform ?
function String
GetDirectoryFromFullFilename(String fullFilename)
{
    String result = {};

    int iLastSlash = max(IndexOfLast(fullFilename, '\\'), IndexOfLast(fullFilename, '/'));

    if (iLastSlash < 0) return result;

    result.bytes = fullFilename.bytes;
    result.cBytes = iLastSlash + 1;
    return result;
}

function bool
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
function bool
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

        i32 expUnsigned = 0;
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

function bool
TryParseF32FromEntireString(String string, f32 * poResult, char separator=0)
{
    f64 doubleFloat;
    bool result = TryParseF64FromEntireString(string, &doubleFloat, separator);

    *poResult = (f32)doubleFloat; // TODO - overflow check (underflow check too?)
    return result;
}

function bool
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
