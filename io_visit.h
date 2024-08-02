// template<typename T>
// struct io_is_atom
// {
//     static bool constexpr value = false;
// };

// template<> struct io_is_atom<u8> { static bool constexpr value = true; };
// template<> struct io_is_atom<u16> { static bool constexpr value = true; };
// template<> struct io_is_atom<u32> { static bool constexpr value = true; };
// template<> struct io_is_atom<u64> { static bool constexpr value = true; };
// template<> struct io_is_atom<i8> { static bool constexpr value = true; };
// template<> struct io_is_atom<i16> { static bool constexpr value = true; };
// template<> struct io_is_atom<i32> { static bool constexpr value = true; };
// template<> struct io_is_atom<i64> { static bool constexpr value = true; };

// template<typename Io, typename T>
// function void
// io_visit(Io* io, T* t)
// {
//     // If you are visiting a non-atom, you need to define a template specialization of io_visit
//     StaticAssert(io_is_atom<T>::value);
// }

struct Io_Vtable
{
    void (*begin)(void* io, String name);
    void (*end)(void* io);

    void (*begin_struct)(void* io, String name);
    void (*end_struct)(void* io);
    void (*begin_array)(void* io, int length, String name);
    void (*end_array)(void* io);

    void (*next_atom_u8)(void* io, u8 value, String name);
    void (*next_atom_u16)(void* io, u16 value, String name);
    void (*next_atom_u32)(void* io, u32 value, String name);
    void (*next_atom_u64)(void* io, u64 value, String name);
    void (*next_atom_i8)(void* io, i8 value, String name);
    void (*next_atom_i16)(void* io, i16 value, String name);
    void (*next_atom_i32)(void* io, i32 value, String name);
    void (*next_atom_i64)(void* io, i64 value, String name);

    // Optionally supported. Returns false if not supported.
    bool (*next_atom_string)(void* io, String value, String name);
    bool (*next_atom_blob)(void* io, Slice<u8> bytes, String name);
};

inline void io_begin(void* io, String name) { return; }
inline void io_end(void* io) { return; }
inline void io_begin_struct_nop(void* io, String name) { return; }
inline void io_end_struct_nop(void* io) { return; }
inline void io_begin_array_nop(void* io, int length, String name) { return; }
inline void io_end_array_nop(void* io) { return; }
inline void io_next_atom_u8_nop(void* io, u8 value, String name) { return; }
inline void io_next_atom_u16_nop(void* io, u16 value, String name) { return; }
inline void io_next_atom_u32_nop(void* io, u32 value, String name) { return; }
inline void io_next_atom_u64_nop(void* io, u64 value, String name) { return; }
inline void io_next_atom_i8_nop(void* io, i8 value, String name) { return; }
inline void io_next_atom_i16_nop(void* io, i16 value, String name) { return; }
inline void io_next_atom_i32_nop(void* io, i32 value, String name) { return; }
inline void io_next_atom_i64_nop(void* io, i64 value, String name) { return; }
inline bool io_next_atom_string_nop(void* io, String value, String name) { return false; }
inline bool io_next_atom_blob_nop(void* io, Slice<u8> bytes, String name) { return false; }

static Io_Vtable const IO_VTABLE_NOP = {
    io_begin,
    io_end,
    io_begin_struct_nop,
    io_end_struct_nop,
    io_begin_array_nop,
    io_end_array_nop,
    io_next_atom_u8_nop,
    io_next_atom_u16_nop,
    io_next_atom_u32_nop,
    io_next_atom_u64_nop,
    io_next_atom_i8_nop,
    io_next_atom_i16_nop,
    io_next_atom_i32_nop,
    io_next_atom_i64_nop,
    io_next_atom_string_nop,
    io_next_atom_blob_nop
};



// --- I/O visitor that writes to a push buffer

struct Io_Push_Buffer
{
    Io_Vtable vtable;
    Push_Buffer pb;
};

inline void
io_pb_next_atom_u8(void* io_, u8 value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, value);
}

inline void
io_pb_next_atom_u16(void* io_, u16 value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, value);
}

inline void
io_pb_next_atom_u32(void* io_, u32 value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, value);
}

inline void
io_pb_next_atom_u64(void* io_, u64 value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, value);
}

inline void
io_pb_next_atom_i8(void* io_, i8 value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, value);
}

inline void
io_pb_next_atom_i16(void* io_, i16 value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, value);
}
inline void
io_pb_next_atom_i32(void* io_, i32 value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, value);
}

inline void
io_pb_next_atom_i64(void* io_, i64 value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, value);
}

inline bool
io_pb_next_atom_blob(void* io_, Slice<u8> bytes, String name)
{
    // NOTE - doesn't write the number of bytes... it's assumed the reader knows.
    //  See usage in io_pb_next_atom_string(..)
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    u8* dst = (u8*)push_buffer_append_new_bytes(&io->pb, bytes.count);
    mem_copy(dst, bytes.items, bytes.count);
    return true;
}

inline bool
io_pb_next_atom_string(void* io_, String value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    Slice<u8> value_bytes = slice_create(value);

    io_pb_next_atom_i32(io, value.length, {});
    io_pb_next_atom_blob(io, value_bytes, {});
    return true;
}

function Io_Push_Buffer
io_pb_create(Memory_Region memory, int bytes_per_page)
{
    // HMM
    //  - return a pointer? the vtable is kinda large...
    //  - could also let the caller supply the push-buffer instead of creating one...

    Io_Push_Buffer result;
    result.vtable = IO_VTABLE_NOP;
    result.vtable.next_atom_u8 = io_pb_next_atom_u8;
    result.vtable.next_atom_u16 = io_pb_next_atom_u16;
    result.vtable.next_atom_u32 = io_pb_next_atom_u32;
    result.vtable.next_atom_u64 = io_pb_next_atom_u64;
    result.vtable.next_atom_i8 = io_pb_next_atom_i8;
    result.vtable.next_atom_i16 = io_pb_next_atom_i16;
    result.vtable.next_atom_i32 = io_pb_next_atom_i32;
    result.vtable.next_atom_i64 = io_pb_next_atom_i64;
    result.vtable.next_atom_string = io_pb_next_atom_string;
    result.vtable.next_atom_blob = io_pb_next_atom_blob;
    result.pb = Push_Buffer(memory, bytes_per_page);
    return result;
}
