//  --- Dynamic array

template <typename T>
struct DynArray
{
    T* items;
    i32 count;
    i32 capacity;
    Memory_Region memory;

    DynArray<T>() = default;
    explicit DynArray<T>(Memory_Region memory) { *this = {}; this->memory = memory; }

    T* begin() const { return items; }
    T* end() const { return items + count; }
    T& operator[](int iItem) const { return items[iItem]; }

    // HMM - This isn't an obvious overload, but it makes it work the same way as a raw ptr, which I like
    T* operator+ (int iItem) const { return items + iItem; }
};

template <typename T>
function IterByPtr<T>
ByPtr(DynArray<T> array)
{
    IterByPtr<T> result = IterByPtr<T>(array.items, array.count);
    return result;
}

template <typename T>
function Slice<T>
slice_create(DynArray<T> array)
{
    Slice<T> result;
    result.items = array.items;
    result.count = array.count;
    return result;
}

template <typename T>
function void
EnsureCapacity(DynArray<T>* array, int capacity)
{
    if (array->capacity < capacity)
    {
        int newCapacity = max(array->capacity, 1);
        while (newCapacity < capacity)
        {
            // Always grow by doubling in size
            // HMM - Overflow/infinite loop check?
            // HMM - Maybe we should reserve more initial capacity then 1 item?

            newCapacity *= 2;
        }

        array->items = (T*)reallocate_tracked(array->memory, array->items, sizeof(T) * newCapacity);
        array->capacity = newCapacity;
    }
}

template <typename T>
function void
Append(DynArray<T>* array, const T & item)
{
    EnsureCapacity(array, array->count + 1);
    array->items[array->count] = item;
    array->count++;
}

template <typename T>
function u32
array_append_and_id(DynArray<T>* array, const T & item)
{
    u32 result = array->count;
    EnsureCapacity(array, array->count + 1);
    array->items[array->count] = item;
    array->count++;
    return result;
}

template <typename T>
function T*
array_append_new(DynArray<T>* array)
{
    EnsureCapacity(array, array->count + 1);
    T* result = array->items + array->count;
    array->count++;
    return result;
}

template <typename T>
function void
Insert(DynArray<T>* array, const T & item, int iItem)
{
    EnsureCapacity(array, array->count + 1);

    int cntItemShift = array->count - iItem; // @ArrayBoundsCheck
    T* dst = array->items + iItem + 1;
    T* src = array->items + iItem;
    mem_move(dst, src, sizeof(T) * cntItemShift);

    array->items[iItem] = item;
    array->count++;
}

template <typename T>
function void
array_prepend(DynArray<T>* array, const T & item)
{
    Insert(array, item, 0);
}

// @Slow - Prefer RemoveUnordered when possible
template <typename T>
function T
RemoveAt(DynArray<T>* array, int iItem)
{
    if (iItem < 0 || iItem >= array->count)
    {
        ASSERT_FALSE;
        return T{};
    }

    T result = array->items[iItem];

    int cntItemShift = array->count - iItem - 1;
    T* dst = array->items + iItem;
    T* src = dst + 1;
    mem_move(dst, src, sizeof(T) * cntItemShift);

    array->count--;

    return result;
}

template <typename T>
function T
RemoveUnorderedAt(DynArray<T>* array, int iItem)
{
    if (iItem < 0 || iItem >= array->count)
    {
        ASSERT_FALSE;
        return T{};
    }

    T result = array->items[iItem];
    array->items[iItem] = array->items[array->count - 1];
    array->count--;
    return result;
}

template <typename T>
function T
array_remove_last(DynArray<T>* array)
{
    if (array->count <= 0)
    {
        ASSERT_FALSE;
        return T{};
    }

    T result = array->items[array->count - 1];
    array->count--;

    return result;
}

template <typename T>
function T*
array_peek_last(DynArray<T>* array)
{
    if (array->count <= 0)
    {
        ASSERT_FALSE;
        return nullptr;
    }

    T* result = array->items + array->count - 1;
    return result;
}

template <typename T>
function bool
Remove(DynArray<T>* array, T item)
{
    // @Slow - Prefer RemoveUnordered when possible
    for (int i = 0; i < array->count; i++)
    {
        // TODO - version that accepts an equality function param
        if (array->items[i] == item)
        {
            RemoveAt(array, i);
            return true;
        }
    }

    return false;
}

template <typename T>
function bool
RemoveUnordered(DynArray<T>* array, T item)
{
    for (int i = 0; i < array->count; i++)
    {
        if (array->items[i] == item)
        {
            RemoveUnorderedAt(array, i);
            return true;
        }
    }

    return false;
}

template <typename T>
function bool
array_is_empty(DynArray<T> array)
{
    bool result = (array.count == 0);
    return result;
}

template <typename T>
function void
Clear(DynArray<T>* array, bool shouldFreeMemory=false)
{
    array->count = 0;
    if (shouldFreeMemory)
    {
        free_tracked_allocation(array->memory, array->items);

        Memory_Region memoryCopy = array->memory;
        *array = {};
        array->memory = memoryCopy;
    }
}

template <typename T>
function T*
RawPtr(DynArray<T>* array)
{
    T* result = array->items;
    return result;
}



// --- BufferBuilder
//  Convenience wrapper for dynamic array of bytes.

struct BufferBuilder
{
    DynArray<u8> bytes;

    BufferBuilder() = default;
    explicit BufferBuilder(Memory_Region memory) { *this = {}; bytes = DynArray<u8>(memory); }
};

function Slice<u8>
slice_create(BufferBuilder builder)
{

    Slice<u8> result = slice_create(builder.bytes);
    return result;
}

function void
Clear(BufferBuilder* builder, bool shouldFreeMemory=false)
{
    Clear(&builder->bytes, shouldFreeMemory);
}

function bool
IsEmpty(BufferBuilder* builder)
{
    bool result = (builder->bytes.count == 0);
    return result;
}

function u8*
buffer_append_new_bytes(BufferBuilder* builder, int length)
{
    int lengthOld = builder->bytes.count;
    int lengthNew = lengthOld + length;

    EnsureCapacity(&builder->bytes, lengthNew);
    builder->bytes.count = lengthNew;

    return builder->bytes + lengthOld;
}

function void
Append(BufferBuilder* builder, u8 value)
{
    u8 * ptr = buffer_append_new_bytes(builder, 1);
    *ptr = value;
}

function void
Append(BufferBuilder* builder, u16 value)
{
    u16 * ptr = (u16 *)buffer_append_new_bytes(builder, 2);
    *ptr = value;
}

function void
Append(BufferBuilder* builder, u32 value)
{
    u32 * ptr = (u32 *)buffer_append_new_bytes(builder, 4);
    *ptr = value;
}

function void
AppendStringCopy(BufferBuilder* builder, String string)
{
    if (string_is_empty(string))
        return;

    u8* ptr = (u8*)buffer_append_new_bytes(builder, string.length);
    string_copy(string, ptr, string.length, Null_Terminate::NO);
}

function u8 *
RawPtr(BufferBuilder* builder)
{
    u8 * result = builder->bytes.items;
    return result;
}



// --- Push_Buffer
//  Simple paged linear allocator.
//  Can push heterogeneous (mixed-size) items indefinitely, then read them back in sequence.
//  Memory address of pushed items is stable, as the buffer grows via a linked-list of chunks ("pages").
//  Doesn't remember types. User is responsible for reading the same types out in the order they were pushed in.

struct Push_Buffer
{
    struct Page_Header
    {
        int allocated_b; // Includes header
        int capacity_b;  // ...
        Page_Header* pNext;
    };

    Memory_Region memory;
    Page_Header* pages;
    Page_Header* pageTail;
    u64 lengthPushed;

    Push_Buffer() = default;
    Push_Buffer(Memory_Region memory, int lengthPerPage)
    {
        *this = {};
        this->memory = memory;

        lengthPerPage += sizeof(Page_Header);

        Page_Header* page = (Page_Header *)allocate(this->memory, lengthPerPage);
        page->allocated_b = sizeof(Page_Header);
        page->capacity_b = lengthPerPage;
        page->pNext = nullptr;

        this->pages = page;
        this->pageTail = page;
    }
};

function void*
push_buffer_append_new_bytes(Push_Buffer* buffer, int length)
{
    length = max(0, length);

    auto* page = buffer->pageTail;

    int lengthFree = page->capacity_b - page->allocated_b;
    if (lengthFree < length)
    {
        int lengthNewPage = max(page->capacity_b, length + (int)sizeof(Push_Buffer::Page_Header));

        page = (Push_Buffer::Page_Header*)allocate(buffer->memory, lengthNewPage);
        page->allocated_b = sizeof(Push_Buffer::Page_Header);
        page->capacity_b = lengthNewPage;
        page->pNext = nullptr;

        buffer->pageTail->pNext = page;
        buffer->pageTail = page;
    }

    void* result = (u8*)page + page->allocated_b;

    page->allocated_b += length;
    buffer->lengthPushed += length;
    ASSERT(page->allocated_b <= page->capacity_b);

    return result;
}

template <typename T>
function T*
push_buffer_append_new(Push_Buffer* buffer)
{
    T* result = (T*)push_buffer_append_new_bytes(buffer, sizeof(T));
    return result;
}

template <typename T>
function T*
push_buffer_append_new_array(Push_Buffer* buffer, int count)
{
    T* result = (T*)push_buffer_append_new_bytes(buffer, sizeof(T) * count);
    return result;
}

template <typename T>
function void
push_buffer_append(Push_Buffer* buffer, T const& t)
{
    T* new_t = push_buffer_append_new<T>(buffer);
    *new_t = t;
}

template <>
inline void
push_buffer_append(Push_Buffer* buffer, String const& string)
{
    int lengthNeeded = sizeof(i32) + string.length;
    u8* bytes = (u8 *)push_buffer_append_new_bytes(buffer, lengthNeeded);

    // Write the string length
    *((i32 *)bytes) = string.length;

    // Then, write the payload
    mem_copy(bytes + sizeof(i32), string.data, string.length);
}

function void push_slice_reader_advance(struct Push_Slice_Reader* reader, int advance); // @Hgen - Need to support core module...

struct Push_Slice_Reader
{
    Push_Buffer::Page_Header* page;
    int iByteInPage;

    Push_Slice_Reader() = default;
    Push_Slice_Reader(Push_Buffer* push_buffer)
    {
        this->page = push_buffer->pages;
        this->iByteInPage = 0;
        push_slice_reader_advance(this, sizeof(Push_Buffer::Page_Header));
    }
};

function bool
push_slice_reader_is_finished(Push_Slice_Reader const& reader)
{
    bool result = (reader.page == nullptr);
    return result;
}

template <typename T>
function T*
push_buffer_read(Push_Slice_Reader* reader)
{
    // NOTE - Reads by pointer... consider if I want to expose separate by-ptr and by-value versions

    // NOTE - We rely on the user reading the exact same stream of values that they wrote. Asserts below
    //  can usually detect if that assumption breaks.

    ASSERT(!push_slice_reader_is_finished(*reader));
    ASSERT(reader->page->allocated_b - reader->iByteInPage >= sizeof(T));

    // TODO - endianness?
    T* result = (T*)((u8*)reader->page + reader->iByteInPage);
    push_slice_reader_advance(reader, sizeof(T));

    return result;
}

function String
push_buffer_read_string_and_create(Push_Slice_Reader* reader, Memory_Region memory)
{
    // NOTE - This reads by "copy", which is different than reading by pointer above...

    ASSERT(!push_slice_reader_is_finished(*reader));

    i32 length = *push_buffer_read<i32>(reader);

    ASSERT(!push_slice_reader_is_finished(*reader));
    ASSERT(reader->page->allocated_b - reader->iByteInPage >= length);

    String result;
    result.length = length;
    result.data = (u8*)allocate(memory, length);
    mem_copy(result.data, reader->page + reader->iByteInPage, length);
    push_slice_reader_advance(reader, length);

    return result;
}

function void
push_slice_reader_advance(Push_Slice_Reader* reader, int advance)
{
    reader->iByteInPage += advance;

    if (reader->iByteInPage >= reader->page->allocated_b)
    {
        ASSERT(reader->iByteInPage == reader->page->allocated_b); // Should end exactly on the boundary, not past it...
        reader->page = reader->page->pNext;
        reader->iByteInPage = sizeof(Push_Buffer::Page_Header);
    }
}
