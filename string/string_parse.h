// TODO - replace
#include <cstdlib>

function int
int_parse(String string)
{
    // @HACK - fixed buffer to guarantee null-termination, since atoi expects it
    char zstr[1024];
    int length = min(1023, string.length);
    zstr[length] = '\0';
    mem_copy(zstr, string.data, length);

    // TODO - better API that includes errors... atoi silently returns 0
    int result = std::atoi(zstr);
    return result;
}

function u8
u8_parse(String string)
{
    char zstr[1024];
    int length = min(1023, string.length);
    mem_copy(zstr, string.data, length);
    zstr[length] = '\0';

    char* end;
    errno = 0;
    unsigned long result = std::strtoul(zstr, &end, 10);

    if (errno || *end != '\0' || result > U8::MAX)
    {
        ASSERT_FALSE;
        result = 0;
    }

    return (u8)result;
}

function u16
u16_parse(String string)
{
    char zstr[1024];
    int length = min(1023, string.length);
    mem_copy(zstr, string.data, length);
    zstr[length] = '\0';

    char* end;
    errno = 0;
    unsigned long result = std::strtoul(zstr, &end, 10);

    if (errno || *end != '\0' || result > U16::MAX)
    {
        ASSERT_FALSE;
        result = 0;
    }

    return (u16)result;
}

function u32
u32_parse(String string)
{
    char zstr[1024];
    int length = min(1023, string.length);
    mem_copy(zstr, string.data, length);
    zstr[length] = '\0';

    char* end;
    errno = 0;
    unsigned long result = std::strtoul(zstr, &end, 10);

    if (errno || *end != '\0' || result > U32::MAX)
    {
        ASSERT_FALSE;
        result = 0;
    }

    return (u32)result;
}

function u64
u64_parse(String string)
{
    char zstr[1024];
    int length = min(1023, string.length);
    mem_copy(zstr, string.data, length);
    zstr[length] = '\0';

    char* end;
    errno = 0;
    unsigned long long result = std::strtoull(zstr, &end, 10);

    if (errno || *end != '\0')
    {
        ASSERT_FALSE;
        result = 0;
    }

    return (u64)result;
}

function i8
i8_parse(String string)
{
    char zstr[1024];
    int length = min(1023, string.length);
    mem_copy(zstr, string.data, length);
    zstr[length] = '\0';

    char* end;
    errno = 0;
    long result = std::strtol(zstr, &end, 10);

    if (errno || *end != '\0' || result < I8::MIN || result > I8::MAX)
    {
        ASSERT_FALSE;
        result = 0;
    }

    return (i8)result;
}

function i16
i16_parse(String string)
{
    char zstr[1024];
    int length = min(1023, string.length);
    mem_copy(zstr, string.data, length);
    zstr[length] = '\0';

    char* end;
    errno = 0;
    long result = std::strtol(zstr, &end, 10);

    if (errno || *end != '\0' || result < I16::MIN || result > I16::MAX)
    {
        ASSERT_FALSE;
        result = 0;
    }

    return (i16)result;
}

function i32
i32_parse(String string)
{
    char zstr[1024];
    int length = min(1023, string.length);
    mem_copy(zstr, string.data, length);
    zstr[length] = '\0';

    char* end;
    errno = 0;
    long result = std::strtol(zstr, &end, 10);

    if (errno || *end != '\0' || result < I32::MIN || result > I32::MAX)
    {
        ASSERT_FALSE;
        result = 0;
    }

    return (i32)result;
}

function i64
i64_parse(String string)
{
    char zstr[1024];
    int length = min(1023, string.length);
    mem_copy(zstr, string.data, length);
    zstr[length] = '\0';

    char* end;
    errno = 0;
    long long result = std::strtoll(zstr, &end, 10);

    if (errno || *end != '\0')
    {
        ASSERT_FALSE;
        result = 0;
    }

    return (i64)result;
}
function f32
float_parse(String string)
{
    // @HACK - fixed buffer to guarantee null-termination, since atof expects it
    char zstr[1024];
    int length = min(1023, string.length);
    zstr[length] = '\0';
    mem_copy(zstr, string.data, length);

    // TODO - better API that includes errors... atoi silently returns 0
    double result = std::atof(zstr);
    return (f32)result;
}
