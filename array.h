//  Helper for iterating a packed array by pointer using for-each syntax.
//  usage:
//      for (Child* child : IterByPtr(children, child_count))

template <typename T>
struct IterByPtr
{
    T* items;
    int count;

    IterByPtr() = default;
    IterByPtr(T* items, int count)
    {
        this->items = items;
        this->count = count;
    }
    
    struct Ptr
    {
        // A pointer that returns itself when dereferenced (tricks C++ into iterating by pointer)
        
        T* ptr;

        Ptr() = default;
        explicit Ptr(T * ptr) { this->ptr = ptr; }
        T* operator*() { return ptr; }
        void operator++() { ptr++; }
        bool operator!=(Ptr const& other) { return ptr != other.ptr; }
    };
    
    Ptr begin() { return Ptr(items); }
    Ptr end()   { return Ptr(items + count); }
};

//  Array that retains size information. Same as a "slice" in Go-lang.

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
MakeSlice(T* ptr, int count)
{
    Slice<T> result;
    result.items = ptr;
    result.count = count;
    return result;
}

template <typename T>
function Slice<T>
MakeSlice(T* ptr, uintptr count)
{
    Slice<T> result;
    result.items = ptr;
    result.count = (int)count;
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
MakeString(Slice<char> slice)
{
    String result;
    result.data = (u8*)slice.items;
    result.length = slice.count;
    return result;
}

function String
MakeString(Slice<u8> slice)
{
    String result;
    result.data = slice.items;
    result.length = slice.count;
    return result;
}

// Utility for reading values sequentially out of a buffer

struct Buffer_Reader
{
    Slice<u8> buffer;
    uintptr bytes_read;
};

function Buffer_Reader
buffer_reader_create(Slice<u8> const& buffer)
{
    Buffer_Reader result = {};
    result.buffer = buffer;
    return result;
}

function bool
IsFinishedReading(Buffer_Reader* reader)
{
    bool result = (reader->bytes_read >= reader->buffer.count);
    return result;
}

function void*
Read(Buffer_Reader* reader, uintptr byte_count)
{
    // TODO - return null here?
    //  As long as I'm not hitting these asserts, just leave these as asserts.... for speed reading :)
    Assert(!IsFinishedReading(reader));
    Assert(reader->buffer.count - reader->bytes_read >= byte_count);

    void* result = (void*)((u8*)reader->buffer + reader->bytes_read);
    reader->bytes_read += byte_count;
    return result;
}

template <typename T>
function T*
Read(Buffer_Reader* reader)
{
    T* result = (T*)Read(reader, sizeof(T));
    return result;
}

template <typename T>
function T*
ReadArray(Buffer_Reader* reader, uintptr count)
{
    T* result = (T*)Read(reader, sizeof(T) * count);
    return result;
}

//  Simple dynamic array. Wraps a tracked allocation in a Memory_Region.

template <typename T>
struct DynArray
{
    T* items;
    int count;
    int capacity;
    Memory_Region memory;

    DynArray<T>() = default;
    explicit DynArray<T>(Memory_Region memory) { *this = {}; this->memory = memory; }
    
    T* begin() { return items; }
    T* end()   { return items + count; }
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
MakeSlice(DynArray<T> array)
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
function T*
AppendNew(DynArray<T>* array)
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
RemoveAt(DynArray<T>* array, int iItem)
{
    // HMM - Should this return the removed item?
    
    // @Slow - Prefer RemoveUnordered when possible

    if (iItem < array->count)
    {
        int cntItemShift = array->count - iItem - 1;
        
        T* dst = array->items + iItem;
        T* src = dst + 1;
        mem_move(dst, src, sizeof(T) * cntItemShift);
        
        array->count--;
    }
}

template <typename T>
function void
RemoveUnorderedAt(DynArray<T>* array, int iItem)
{
    // HMM - Should this return the removed item?
    
    if (iItem < array->count)
    {
        array->items[iItem] = array->items[array->count - 1];
        array->count--;
    }
}

template <typename T>
function void
RemoveLast(DynArray<T>* array)
{
    if (array->count > 0)
    {
        array->count--;
    }
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
IsEmpty(DynArray<T> array)
{
    bool result = (array.count == 0);
    return result;
}

template <typename T>
function void
Clear(DynArray<T>* array, bool shouldFreeMemory=false)
{
    array->count = 0;
    if (array->count > 0 && shouldFreeMemory)
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
MakeSlice(BufferBuilder builder)
{

    Slice<u8> result = MakeSlice(builder.bytes);
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
AppendNewBytes(BufferBuilder* builder, int length)
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
    u8 * ptr = AppendNewBytes(builder, 1);
    *ptr = value;
}

function void
Append(BufferBuilder* builder, u16 value)
{
    u16 * ptr = (u16 *)AppendNewBytes(builder, 2);
    *ptr = value;
}

function void
Append(BufferBuilder* builder, u32 value)
{
    u32 * ptr = (u32 *)AppendNewBytes(builder, 4);
    *ptr = value;
}

function void
AppendStringCopy(BufferBuilder* builder, String string)
{
    if (IsEmpty(string))
        return;

    u8* ptr = (u8*)AppendNewBytes(builder, string.length);
    CopyString(string, ptr, string.length, Null_Terminate::NO);
}

function u8 *
RawPtr(BufferBuilder* builder)
{
    u8 * result = builder->bytes.items;
    return result;
}

// --- PushBuffer
//  Simple paged linear allocator.
//  Can push heterogeneous (mixed-size) items indefinitely, then read them back in sequence.
//  Doesn't remember types. User is responsible for reading the same types out in the order they were pushed in.

struct PushBuffer
{
    struct PageHeader
    {
        uintptr allocated_b; // Includes header
        uintptr capacity_b;  // ...
        PageHeader* pNext;
    };
    
    Memory_Region memory;
    PageHeader* pages;
    PageHeader* pageTail;
    uintptr lengthPushed;

    PushBuffer() = default;
    PushBuffer(Memory_Region memory, int lengthPerPage)
    {
        *this = {};
        this->memory = memory;

        lengthPerPage += sizeof(PageHeader);
        
        PageHeader* page = (PageHeader *)allocate(this->memory, lengthPerPage);
        page->allocated_b = sizeof(PageHeader);
        page->capacity_b = lengthPerPage;
        page->pNext = nullptr;

        this->pages = page;
        this->pageTail = page;
    }
};

function void*
AppendNewBytes(PushBuffer* buffer, uintptr length)
{
    auto* page = buffer->pageTail;

    uintptr lengthFree = page->capacity_b - page->allocated_b;
    if (lengthFree < length)
    {
        uintptr lengthNewPage = max(page->capacity_b, length + sizeof(PushBuffer::PageHeader));

        page = (PushBuffer::PageHeader*)allocate(buffer->memory, lengthNewPage);
        page->allocated_b = sizeof(PushBuffer::PageHeader);
        page->capacity_b = lengthNewPage;
        page->pNext = nullptr;

        buffer->pageTail->pNext = page;
        buffer->pageTail = page;
    }

    void* result = (u8*)page + page->allocated_b;

    page->allocated_b += length;
    buffer->lengthPushed += length;
    Assert(page->allocated_b <= page->capacity_b);

    return result;
}

template <typename T>
function T*
AppendNew(PushBuffer* buffer)
{
    T* result = (T*)AppendNewBytes(buffer, sizeof(T));
    return result;
}

template <typename T>
function T*
AppendNewArray(PushBuffer* buffer, uintptr count)
{
    T* result = (T*)AppendNewBytes(buffer, sizeof(T) * count);
    return result;
}

template <typename T>
function void
Append(PushBuffer* buffer, T const& t)
{
    T* newT = AppendNew<T>(buffer);
    *newT = t;
}

function void
AppendStringCopy(PushBuffer* buffer, String string)
{
    int lengthNeeded = sizeof(i32) + string.length;
    u8* bytes = (u8 *)AppendNewBytes(buffer, lengthNeeded);

    // Write the string length
    *((i32 *)bytes) = string.length;

    // Then, write the payload
    mem_copy(bytes + sizeof(i32), string.data, string.length);
}

function void AdvanceByteCursor(struct PushBufferReader* reader, int advance); // @Hgen - Need to support core module...

struct PushBufferReader
{
    PushBuffer::PageHeader* page;
    int iByteInPage;

    PushBufferReader() = default;
    PushBufferReader(PushBuffer* push_buffer)
    {
        this->page = push_buffer->pages;
        this->iByteInPage = 0;
        AdvanceByteCursor(this, sizeof(PushBuffer::PageHeader));
    }
};

function bool
IsFinishedReading(PushBufferReader* reader)
{
    bool result = (reader->page == nullptr);
    return result;
}

template <typename T>
function T*
Read(PushBufferReader* reader)
{
    // NOTE - Reads by pointer... consider if I want to expose separate by-ptr and by-value versions

    // NOTE - We rely on the user reading the exact same stream of values that they wrote. Asserts below
    //  can usually detect if that assumption breaks.
    
    Assert(!IsFinishedReading(reader));
    Assert(reader->page->allocated_b - reader->iByteInPage >= sizeof(T));
    
    // TODO - endianness?
    T* result = (T*)((u8*)reader->page + reader->iByteInPage);
    AdvanceByteCursor(reader, sizeof(T));

    return result;
}

function String
ReadStringCopy(PushBufferReader* reader, Memory_Region memory)
{
    // NOTE - This reads by "copy", which is different than reading by pointer above...

    Assert(!IsFinishedReading(reader));

    i32 length = *Read<i32>(reader);

    Assert(!IsFinishedReading(reader));
    Assert(reader->page->allocated_b - reader->iByteInPage >= length);

    String result;
    result.length = length;
    result.data = (u8*)allocate(memory, length);
    mem_copy(result.data, reader->page + reader->iByteInPage, length);
    AdvanceByteCursor(reader, length);

    return result;
}

function void
AdvanceByteCursor(PushBufferReader* reader, int advance)
{
    reader->iByteInPage += advance;

    if (reader->iByteInPage >= reader->page->allocated_b)
    {
        Assert(reader->iByteInPage == reader->page->allocated_b); // Should end exactly on the boundary, not past it...
        reader->page = reader->page->pNext;
        reader->iByteInPage = sizeof(PushBuffer::PageHeader);
    }
}

// --- EnumTable
//  Array indexed by an enum. By default, Nil is an invalid index.
//  You need to use the DefineEnumOps macro to use an EnumTable!

template <typename ENUM_KEY, typename T_VALUE, ENUM_KEY START=ENUM_KEY::NIL + 1>
struct EnumTable
{
    T_VALUE items[ENUM_KEY::ENUM_COUNT - START];
    
    T_VALUE& operator[](ENUM_KEY index)
    {
#if ENUM_TABLE_BOUNDS_CHECK
        Assert(index >= START && index < ENUM_KEY::ENUM_COUNT);
#endif
        u64 offset_index = u64(index - START);
        return items[offset_index];
    }

    T_VALUE const& operator[](ENUM_KEY index) const
    {
#if ENUM_TABLE_BOUNDS_CHECK
        Assert(index >= START && index < ENUM_KEY::ENUM_COUNT);
#endif
        u64 offset_index = u64(index - START);
        return items[offset_index];
    }

    // HMM - This isn't an obvious overload, but it makes it work the same way as a raw ptr, which I like
    T_VALUE* operator+(ENUM_KEY index)
    {
#if ENUM_TABLE_BOUNDS_CHECK
        Assert(index >= START && index < ENUM_KEY::ENUM_COUNT);
#endif

        return &items[(int)(index - START)];
    }
};

template <typename ENUM_KEY, typename T_VALUE>
using EnumTableAllowNil = EnumTable<ENUM_KEY, T_VALUE, ENUM_KEY::NIL>;

// TODO - DynEnumTable -- growable enum table for indexing with unbounded ID enums
