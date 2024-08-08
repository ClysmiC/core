// --- Slice

template <typename T>
struct Slice
{
    T* items;
    int count;

    T* begin() { return items; }
    T* end()   { return items + count; }
    T& operator[](int iItem) const { return items[iItem]; }
    bool operator == (const Slice & other) { return other.items == items && other.count == count; }

    // Implicitly cast to pointer
    operator T*() const { return items; }

    // HMM - This isn't an obvious overload, but it makes it work the same way as a raw ptr, which I like
    T* operator+(int iItem) const { return items + iItem; }
};

template <typename T>
function Slice<T>
slice_create(T* ptr, int count)
{
    Slice<T> result;
    result.items = ptr;
    result.count = count;
    return result;
}

// template <typename T>
// function Slice<T>
// slice_create(T* ptr, uintptr count)
// {
//     Slice<T> result;
//     result.items = ptr;
//     result.count = (int)count;
//     return result;
// }

function Slice<u8>
slice_create(String string)
{
    Slice<u8> result;
    result.items = string.data;
    result.count = string.length;
    return result;
}

template <typename T>
function IterByPtr<T>
ByPtr(Slice<T> slice)
{
    IterByPtr<T> result = IterByPtr<T>(slice.items, slice.count);
    return result;
}

function String
string_create(Slice<char> slice)
{
    String result;
    result.data = (u8*)slice.items;
    result.length = slice.count;
    return result;
}

function String
string_create(Slice<u8> slice)
{
    String result;
    result.data = slice.items;
    result.length = slice.count;
    return result;
}

// Utility for reading values sequentially out of a buffer

struct Slice_Reader
{
    Slice<u8> buffer;
    uintptr bytes_read;
};

function Slice_Reader
slice_reader_create(Slice<u8> const& buffer)
{
    Slice_Reader result = {};
    result.buffer = buffer;
    return result;
}

function bool
slice_reader_is_finished(Slice_Reader const& reader)
{
    bool result = (reader.bytes_read >= reader.buffer.count);
    return result;
}

function void*
slice_read_bytes(Slice_Reader* reader, int byte_count)
{
    byte_count = max(0, byte_count);

    // TODO - bounds check...
    Assert(Implies(byte_count != 0, !slice_reader_is_finished(*reader)));
    Assert(reader->buffer.count - reader->bytes_read >= byte_count);

    void* result = (void*)((u8*)reader->buffer + reader->bytes_read);
    reader->bytes_read += byte_count;
    return result;
}

template <typename T>
function T*
slice_read(Slice_Reader* reader)
{
    T* result = (T*)slice_read_bytes(reader, sizeof(T));
    return result;
}

template <typename T>
function T*
slice_read_array(Slice_Reader* reader, int count)
{
    T* result = (T*)slice_read_bytes(reader, sizeof(T) * count);
    return result;
}
