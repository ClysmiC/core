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
