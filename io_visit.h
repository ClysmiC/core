struct Io_Vtable
{
    void (*begin)(Io_Vtable* io, String name);
    void (*end)(Io_Vtable* io);

    void (*object_begin)(Io_Vtable* io, String name);
    void (*object_end)(Io_Vtable* io);
    void (*array_begin_i32)(Io_Vtable* io, i32* length, String name);
    void (*array_begin_u32)(Io_Vtable* io, u32* length, String name);
    void (*array_end)(Io_Vtable* io);

    void (*atom_u8)(Io_Vtable* io, u8* value, String name);
    void (*atom_u16)(Io_Vtable* io, u16* value, String name);
    void (*atom_u32)(Io_Vtable* io, u32* value, String name);
    void (*atom_u64)(Io_Vtable* io, u64* value, String name);
    void (*atom_i8)(Io_Vtable* io, i8* value, String name);
    void (*atom_i16)(Io_Vtable* io, i16* value, String name);
    void (*atom_i32)(Io_Vtable* io, i32* value, String name);
    void (*atom_i64)(Io_Vtable* io, i64* value, String name);
    void (*atom_string)(Io_Vtable* io, String* value, Memory_Region memory, String name);
    void (*atom_blob)(Io_Vtable* io, Slice<u8> bytes, String name);

    // HMM -- just use a shared variable for this instead of a virtual function call?
    bool (*is_deserializing)(Io_Vtable* io);

    // TODO
    // void error(u32 error_code);
    // u32 get_error_code();
};

inline void io_begin_nop(Io_Vtable* io, String name) { return; }
inline void io_end_nop(Io_Vtable* io) { return; }
inline void io_object_begin_nop(Io_Vtable* io, String name) { return; }
inline void io_object_end_nop(Io_Vtable* io) { return; }
inline void io_array_begin_i32_nop(Io_Vtable* io, i32* length, String name) { return; }
inline void io_array_begin_u32_nop(Io_Vtable* io, u32* length, String name) { return; }
inline void io_array_end_nop(Io_Vtable* io) { return; }
inline void io_atom_u8_nop(Io_Vtable* io, u8* value, String name) { return; }
inline void io_atom_u16_nop(Io_Vtable* io, u16* value, String name) { return; }
inline void io_atom_u32_nop(Io_Vtable* io, u32* value, String name) { return; }
inline void io_atom_u64_nop(Io_Vtable* io, u64* value, String name) { return; }
inline void io_atom_i8_nop(Io_Vtable* io, i8* value, String name) { return; }
inline void io_atom_i16_nop(Io_Vtable* io, i16* value, String name) { return; }
inline void io_atom_i32_nop(Io_Vtable* io, i32* value, String name) { return; }
inline void io_atom_i64_nop(Io_Vtable* io, i64* value, String name) { return; }
inline void io_atom_string_nop(Io_Vtable* io, String* value, Memory_Region memory, String name) { return; }
inline void io_atom_blob_nop(Io_Vtable* io, Slice<u8> bytes, String name) { return; }

inline bool io_return_false(Io_Vtable* io) { return false; }
inline bool io_return_true(Io_Vtable* io) { return true; }

inline bool io_supports_string(Io_Vtable* io)
{
    bool result = (io->atom_string != io_atom_string_nop);
    return result;
}

inline bool io_supports_blob(Io_Vtable* io)
{
    bool result = (io->atom_blob != io_atom_blob_nop);
    return result;
}

static Io_Vtable const IO_VTABLE_NOP = {
    io_begin_nop,
    io_end_nop,
    io_object_begin_nop,
    io_object_end_nop,
    io_array_begin_i32_nop,
    io_array_begin_u32_nop,
    io_array_end_nop,
    io_atom_u8_nop,
    io_atom_u16_nop,
    io_atom_u32_nop,
    io_atom_u64_nop,
    io_atom_i8_nop,
    io_atom_i16_nop,
    io_atom_i32_nop,
    io_atom_i64_nop,
    io_atom_string_nop,
    io_atom_blob_nop,
    io_return_false,    // is_deserializing
};



// --- I/O visitor that writes to a push buffer

struct Io_Push_Buffer
{
    Io_Vtable vtable;
    Push_Buffer pb;
};

inline void
io_pb_array_begin_i32(Io_Vtable* io_, i32* length, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, *length);
}

inline void
io_pb_array_begin_u32(Io_Vtable* io_, u32* length, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, *length);
}

inline void
io_pb_atom_u8(Io_Vtable* io_, u8* value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, *value);
}

inline void
io_pb_atom_u16(Io_Vtable* io_, u16* value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, *value);
}

inline void
io_pb_atom_u32(Io_Vtable* io_, u32* value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, *value);
}

inline void
io_pb_atom_u64(Io_Vtable* io_, u64* value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, *value);
}

inline void
io_pb_atom_i8(Io_Vtable* io_, i8* value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, *value);
}

inline void
io_pb_atom_i16(Io_Vtable* io_, i16* value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, *value);
}
inline void
io_pb_atom_i32(Io_Vtable* io_, i32* value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, *value);
}

inline void
io_pb_atom_i64(Io_Vtable* io_, i64* value, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, *value);
}

inline void
io_pb_atom_blob(Io_Vtable* io_, Slice<u8> bytes, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    u8* dst = (u8*)push_buffer_append_new_bytes(&io->pb, bytes.count);
    mem_copy(dst, bytes.items, bytes.count);
}

inline void
io_pb_atom_string(Io_Vtable* io_, String* value, Memory_Region memory, String name)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    Slice<u8> value_bytes = slice_create(*value);

    io_pb_atom_i32(io_, (i32*)&value->length, {});

    Slice<u8> blob;
    if (io_->is_deserializing(io_))
    {
        blob.items = (u8*)allocate(memory, value->length);
        blob.count = value->length;
        value->data = blob.items;
    }
    else
    {
        blob = slice_create(*value);
    }

    io_pb_atom_blob(io_, blob, {});
}

function Io_Push_Buffer
io_pb_create(Memory_Region memory, int bytes_per_page)
{
    // HMM
    //  - return a pointer? the vtable is kinda large...
    //  - could also let the caller supply the push-buffer instead of creating one...

    Io_Push_Buffer result;
    result.vtable = IO_VTABLE_NOP;
    result.vtable.array_begin_i32 = io_pb_array_begin_i32;
    result.vtable.array_begin_u32 = io_pb_array_begin_u32;
    result.vtable.atom_u8 = io_pb_atom_u8;
    result.vtable.atom_u16 = io_pb_atom_u16;
    result.vtable.atom_u32 = io_pb_atom_u32;
    result.vtable.atom_u64 = io_pb_atom_u64;
    result.vtable.atom_i8 = io_pb_atom_i8;
    result.vtable.atom_i16 = io_pb_atom_i16;
    result.vtable.atom_i32 = io_pb_atom_i32;
    result.vtable.atom_i64 = io_pb_atom_i64;
    result.vtable.atom_string = io_pb_atom_string;
    result.vtable.atom_blob = io_pb_atom_blob;
    result.pb = Push_Buffer(memory, bytes_per_page);
    return result;
}



// --- I/O visitor that reads from a slice

struct Io_Slice_Reader
{
    Io_Vtable vtable;
    Slice_Reader reader;
};

inline void
io_slice_array_begin_i32(Io_Vtable* io_, i32* length, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    *length = *slice_read<i32>(&io->reader);
}

inline void
io_slice_array_begin_u32(Io_Vtable* io_, u32* length, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    *length = *slice_read<u32>(&io->reader);
}

inline void
io_slice_atom_u8(Io_Vtable* io_, u8* value, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    *value = *slice_read<u8>(&io->reader);
}

inline void
io_slice_atom_u16(Io_Vtable* io_, u16* value, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    *value = *slice_read<u16>(&io->reader);
}

inline void
io_slice_atom_u32(Io_Vtable* io_, u32* value, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    *value = *slice_read<u32>(&io->reader);
}

inline void
io_slice_atom_u64(Io_Vtable* io_, u64* value, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    *value = *slice_read<u64>(&io->reader);
}

inline void
io_slice_atom_i8(Io_Vtable* io_, i8* value, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    *value = *slice_read<i8>(&io->reader);
}

inline void
io_slice_atom_i16(Io_Vtable* io_, i16* value, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    *value = *slice_read<i16>(&io->reader);
}
inline void
io_slice_atom_i32(Io_Vtable* io_, i32* value, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    *value = *slice_read<i32>(&io->reader);
}

inline void
io_slice_atom_i64(Io_Vtable* io_, i64* value, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    *value = *slice_read<i64>(&io->reader);
}

inline void
io_slice_atom_blob(Io_Vtable* io_, Slice<u8> io_bytes, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;

    // TODO - bounds check
    u8* src = (u8*)slice_read_bytes(&io->reader, io_bytes.count);
    mem_copy(io_bytes.items, src, io_bytes.count);
}

inline void
io_slice_atom_string(Io_Vtable* io_, String* value, Memory_Region memory, String name)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    io_slice_atom_i32(io_, (i32*)&value->length, {});

    Slice<u8> blob;
    if (io_->is_deserializing(io_))
    {
        blob.items = (u8*)allocate(memory, value->length);
        blob.count = value->length;
        value->data = blob.items;
    }
    else
    {
        blob = slice_create(*value);
    }

    io_slice_atom_blob(io_, blob, {});
}

inline void
io_slice_end(Io_Vtable* io_)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    ASSERT(slice_reader_is_finished(io->reader));
}

function Io_Slice_Reader
io_slice_reader_create(Slice<u8> const& slice)
{
    // HMM
    //  - return a pointer? the vtable is kinda large...
    //  - could also let the caller supply the push-buffer instead of creating one...

    Io_Slice_Reader result;
    result.vtable = IO_VTABLE_NOP;
    result.vtable.end = io_slice_end;
    result.vtable.array_begin_i32 = io_slice_array_begin_i32;
    result.vtable.array_begin_u32 = io_slice_array_begin_u32;
    result.vtable.atom_u8 = io_slice_atom_u8;
    result.vtable.atom_u16 = io_slice_atom_u16;
    result.vtable.atom_u32 = io_slice_atom_u32;
    result.vtable.atom_u64 = io_slice_atom_u64;
    result.vtable.atom_i8 = io_slice_atom_i8;
    result.vtable.atom_i16 = io_slice_atom_i16;
    result.vtable.atom_i32 = io_slice_atom_i32;
    result.vtable.atom_i64 = io_slice_atom_i64;
    result.vtable.atom_string = io_slice_atom_string;
    result.vtable.atom_blob = io_slice_atom_blob;
    result.vtable.is_deserializing = io_return_true;
    result.reader = slice_reader_create(slice);
    return result;
}

bool constexpr IO_PATCH = false;
