// --- zstr (Zero terminated STRing, a.k.a. "c-string")

function int
zstr_length(char* zstr)
{
    int result = 0;
    char* cursor = zstr;
    while (*cursor)
    {
        result++;
        cursor++;
    }

    return result;
}

function bool string_eq(struct String const& str0, struct String const& str1);

struct String
{
    u8* data;
    int length;     // count of bytes

    // TODO - Capacity? String "building" capabilities?
    String() = default;

    u8* begin() { return data; }
    u8* end()   { return data + length; }
    u8 const* begin() const { return data; }
    u8 const* end() const { return data + length; }

    explicit String(char* zstr) { this->data = (u8*)zstr; this->length = zstr_length(zstr); }
    String(char* zstr, uint byte_count) { this->data = (u8*)zstr; this->length = byte_count; }

    u8 & operator[](int i) const { return data[i]; }
    bool operator == (String const& other) { return string_eq(*this, other); }
};

inline String
string_create(u8* data, int length)
{
    String result;
    result.data = data;
    result.length = max(0, length);
    return result;
}

inline String
string_create_empty()
{
    String result;
    result.data = nullptr;
    result.length = 0;
    return result;
}

// NOTE - ArrayLen runs at compile time for string literals, which is why this
//  macro is better than the ctor if you know the string at comp-time
#define STR(zstr_literal) String(zstr_literal, ARRAY_LEN(zstr_literal) - 1)

enum class Null_Terminate : u8
{
    NO = 0,
    NIL = 0,

    YES
};

function void
zstr_copy(char* src, char* dst, int dst_length, Null_Terminate null_terminate=Null_Terminate::YES)
{
    if (dst_length == 0) return;

    int dst_length_to_copy = ((bool)null_terminate) ? dst_length - 1 : dst_length;

    int i = 0;
    while (*src && i < dst_length_to_copy)
    {
        *dst = *src;

        // @Slow
        i++;
        src++;
        dst++;
    }

    if ((bool)null_terminate) *dst = '\0';
}

// HMM - Should a String carry around if it is null-terminated or not? Need a convenient way to print strings...
//  Or maybe look into using printf("%.*s", length, string); ???

function void
string_copy(String src, u8* dst, int dst_length, Null_Terminate null_terminate=Null_Terminate::YES)
{
    if (dst_length == 0) return;

    u8* srcCursor = src.data;
    int dst_length_to_copy = ((bool)null_terminate) ? dst_length - 1 : dst_length;

    int iByteDst = 0;
    while (iByteDst < src.length && iByteDst < dst_length_to_copy)
    {
        *dst = *srcCursor;

        iByteDst++;
        srcCursor++;
        dst++;
    }

    if ((bool)null_terminate) *dst = '\0';
}

function char*
zstr_create(char* src, Memory_Region memory)
{
    int lengthSrc = zstr_length(src);
    int dst_length = lengthSrc + 1;

    char* dst = (char*)allocate(memory, dst_length);
    zstr_copy(src, dst, dst_length);
    return dst;
}

// HMM - rename these to string_copy?
function String
string_create(char* src, Memory_Region memory)
{
    String dst;
    dst.length = zstr_length(src);
    dst.data = (u8*)allocate(memory, dst.length);
    zstr_copy(src, (char*)dst.data, dst.length, Null_Terminate::NO);
    return dst;
}

function String
string_create(String src, Memory_Region memory)
{
    String result;
    result.length = src.length;
    if (result.length > 0)
    {
        result.data = (u8*)allocate(memory, result.length + 1);
        string_copy(src, result.data, result.length + 1);
    }
    else
    {
        result.data = nullptr;
    }
    return result;
}

function String
string_create_from_escaped(String src, Memory_Region memory)
{
    String result = {};
    if (src.length > 0)
    {
        result.data = (u8*)allocate(memory, src.length + 1);

        bool escaped_prev = false;
        for (int i = 0; i < src.length; i++)
        {
            char c = src[i];
            if (escaped_prev)
            {
                switch (c)
                {
                    case 'n': c = '\n'; break;
                    case 't': c = '\t'; break;
                    case 'r': c = '\r'; break;
                    case 'b': c = '\b'; break;
                    case 'f': c = '\f'; break;
                }

                escaped_prev = false;
            }
            else if (src[i] == '\\')
            {
                escaped_prev = true;
                continue;
            }

            result.data[result.length] = c;
            result.length++;
        }

        ASSERT(result.length <= src.length);
    }
    else
    {
        result.data = nullptr;
    }
    return result;
}

function void
string_replace_char(String* string, char from, char to)
{
    for (u8& c: *string)
    {
        if (c == from) c = to;
    }
}

function char*
zstr_create(String src, Memory_Region memory)
{
    uint dst_length = src.length + 1;
    char* dst = (char*)allocate(memory, dst_length);
    string_copy(src, (u8*)dst, dst_length, Null_Terminate::YES);
    return dst;
}

#if 0   // disabled 2024-07-13
function String
string_create(
    char* src,
    Memory_Region memory=nullptr)
{
    int lengthSrc = zstr_length(src);

    String dst;
    dst.length = lengthSrc;

    if (!memory) // @Gross - This API is weird. Most of these functions don't allow null memory regions. Should probably split this into a "Duplicate" which takes a region, and this which doesn't
    {
        dst.data = (u8*)src;
    }
    else if (dst.length > 0)
    {
        dst.data = (u8*)allocate(memory, dst.length);
        zstr_copy(src, (char*)dst.data, dst.length, Null_Terminate::NO);
    }

    return dst;
}
#endif

// @Cleanup - Passing the dest buffers deep in the parameter list make it kinda hard to grep for places where
//  I am initializing string buffers. Any string/stringview abstraction I may write in the future
//  should try to alleviate this

function void
CopySubstring(
    char* src,
    int iSrcStart,
    int cntCharSrcToCopy, // NOTE - This is a count, not an end index!
    char* dst,
    int dst_length)
{
    if (dst_length <= 0)
        return;

    int cntCharDst = dst_length - 1; // Leave 1 byte for null terminator
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

    ASSERT(iCharDst < dst_length);
    *dst = '\0';
}

function void
string_concat(
    const char* srcA,
    const char* srcB,
    char* dst,
    int dst_length)
{
    if (dst_length <= 0)
        return;

    int cntCharDst = dst_length - 1; // Leave 1 byte for null terminator
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

    ASSERT(iCharDst < dst_length);
    ASSERT_WARN(*srcA == '\0');
    ASSERT_WARN(*srcB == '\0');

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
        ASSERT_FALSE_WARN;
        iInsert = 0;
    }
    else if (iInsert > srcA.length)
    {
        ASSERT_FALSE_WARN;
        iInsert = srcA.length;
    }

    String result;
    result.length = srcA.length + srcB.length;
    result.data = (u8*)allocate(memory, result.length);
    mem_copy(result.data, srcA.data, iInsert);
    mem_copy(result.data + iInsert, srcB.data, srcB.length);
    mem_copy(result.data + iInsert + srcB.length, srcA.data + iInsert, srcA.length - iInsert);
    return result;
}

function String
string_concat(String srcA, String srcB, Memory_Region memory)
{
    String result;
    result.length = srcA.length + srcB.length;
    result.data = (u8*)allocate(memory, result.length);
    mem_copy(result.data, srcA.data, srcA.length);
    mem_copy(result.data + srcA.length, srcB.data, srcB.length);
    return result;
}

function bool
zstr_eq(char* str0, char* str1)
{
    ASSERT(str0);
    ASSERT(str1);

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

function u32
string_hash(String const& str)
{
    u32 result = StartHash(str.data, str.length);
    return result;
}

function u32
string_hash_lowercase(String const& str)
{
    u32 result = StartHash();

    // @Slow
    for (int i = 0; i < str.length; i++)
    {
        // TODO - this isn't even forcing to lowercase? Uh... don't trust this code.
        result = BuildHash((void*)(str.data + i), 1, result);
    }

    return result;
}

function bool
string_eq(String str0, char* str1)
{
    u8* cursor0 = str0.data;
    u8* endCursor0 = str0.data + str0.length;

    u8* cursor1 = (u8*)str1;

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
string_eq(char* str0, String str1)
{
    bool result = string_eq(str1, str0);
    return result;
}

function bool
string_eq(String const& str0, String const& str1)
{
    if (str0.length != str1.length) return false;
    if (str0.data == str1.data)   return true;

    u8* cursor0 = str0.data;
    u8* endCursor0 = str0.data + str0.length;

    u8* cursor1 = str1.data;
    u8* endCursor1 = str1.data + str1.length;

    while ((cursor0 < endCursor0) && (cursor1 < endCursor1))
    {
        if (*cursor0 != *cursor1)
            return false;

        cursor0++;
        cursor1++;
    }

    return true;
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
zstr_eq_ignore_case(char* str0, char* str1)
{
    ASSERT(str0);
    ASSERT(str1);

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
string_eq_ignore_case(String const& str0, char* str1)
{
    u8* cursor0 = str0.data;
    u8* endCursor0 = str0.data + str0.length;

    u8* cursor1 = (u8*)str1;

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
string_eq_ignore_case(char* str0, String const& str1)
{
    bool result = string_eq_ignore_case(str1, str0);
    return result;
}

function bool
string_eq_ignore_case(String const& str0, String const& str1)
{
    if (str0.length != str1.length) return false;
    if (str0.data == str1.data)   return true;

    u8* cursor0 = str0.data;
    u8* endCursor0 = str0.data + str0.length;

    u8* cursor1 = str1.data;
    u8* endCursor1 = str1.data + str1.length;

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
string_ends_with(String const& str, String const& suffix)
{
    if (str.length < suffix.length)
        return false;

    int prefix_length = str.length - suffix.length;
    String end = string_create(str.data + prefix_length, suffix.length);

    bool result = string_eq(end, suffix);
    return result;
}

function bool
string_ends_with_ignore_case(String const& str, String const& suffix)
{
    if (str.length < suffix.length)
        return false;

    int prefix_length = str.length - suffix.length;
    String end = string_create(str.data + prefix_length, suffix.length);

    bool result = string_eq_ignore_case(end, suffix);
    return result;
}

// TODO - get rid of core dependency on stb_sprintf?
#include "lib/stb/stb_sprintf.h"

function int
zstrFromPrintf_v(char* buffer, int lengthBuffer, char const* fmt, va_list varargs)
{
    int result = stbsp_vsnprintf(buffer, lengthBuffer, fmt, varargs);
    return result;
}

function int
zstrFromPrintf(char* buffer, int lengthBuffer, char const* fmt, ...)
{
    va_list varargs;
    va_start(varargs, fmt);
    int result = zstrFromPrintf_v(buffer, lengthBuffer, fmt, varargs);
    va_end(varargs);
    return result;
}

function String
StringFromPrintf_v(Memory_Region memory, int lengthBuffer, char const* fmt, va_list varargs)
{
    // TODO - I don't like having a lengthBuffer param...
    //  but this requires some sort of re-allocing string builder (which we don't yet have)
    //  to work on arbitrarily sized strings...

    String result;
    result.data = (u8*)allocate(memory, lengthBuffer);  // @LEAK
    result.length = stbsp_vsnprintf((char*)result.data, lengthBuffer, fmt, varargs);
    return result;
}

function String
StringFromPrintf(Memory_Region memory, int lengthBuffer, char const* fmt, ...)
{
    // TODO - I don't like having a lengthBuffer param...
    //  but this requires some sort of re-allocing string builder (which we don't yet have)
    //  to work on arbitrarily sized strings...

    va_list varargs;
    va_start(varargs, fmt);
    String result = StringFromPrintf_v(memory, lengthBuffer, fmt, varargs);
    va_end(varargs);
    return result;
}

function int
string_index_of_first(String string, char c)
{
    for (int iByte = 0; iByte < string.length; iByte++)
    {
        if (string[iByte] == c)
            return iByte;
    }

    return -1;
}

function int
string_index_of_last(String string, char c)
{
    for (int iByte = string.length - 1; iByte >= 0; iByte--)
    {
        if (string[iByte] == c)
            return iByte;
    }

    return -1;
}

function bool
string_is_empty(String string)
{
    return (string.length == 0);
}

// @Cleanup - move to platform ?
function String
GetDirectoryFromFullFilename(String fullFilename)
{
    String result = {};

    int iLastSlash = max(string_index_of_last(fullFilename, '\\'), string_index_of_last(fullFilename, '/'));

    if (iLastSlash < 0) return result;

    result.data = fullFilename.data;
    result.length = iLastSlash + 1;
    return result;
}

function bool
char_is_decimal(char c)
{
    bool result = (c >= '0' && c <= '9');
    return result;
}


function bool
char_is_whitespace(char c)
{
    // TODO - support unicode
    // https://learn.microsoft.com/en-us/dotnet/api/system.char.iswhitespace?view=net-8.0

    bool result =
        (c == ' ') ||
        (c == '\n') ||
        (c == '\r') ||
        (c == '\t') ||
        (c == '\v') ||
        (c == '\f');

    return result;
}
