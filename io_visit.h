// Polymorphic "visitor" pattern for (de)serialization

struct Io_Vtable
{
    void (*begin)(Io_Vtable* io, String name);
    void (*end)(Io_Vtable* io);

    void (*object_begin)(Io_Vtable* io, String name, bool is_external);
    void (*object_end)(Io_Vtable* io);
    void (*array_begin_i32)(Io_Vtable* io, i32* length, String name, bool is_external);
    void (*array_begin_u32)(Io_Vtable* io, u32* length, String name, bool is_external);
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
inline void io_object_begin_nop(Io_Vtable* io, String name, bool is_external) { return; }
inline void io_object_end_nop(Io_Vtable* io) { return; }
inline void io_array_begin_i32_nop(Io_Vtable* io, i32* length, String name, bool is_external) { return; }
inline void io_array_begin_u32_nop(Io_Vtable* io, u32* length, String name, bool is_external) { return; }
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
inline void io_atom_blob_nop(Io_Vtable* io, Slice<u8> bytes, String name) { ASSERT_FALSE_WARN; return; }

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
io_pb_array_begin_i32(Io_Vtable* io_, i32* length, String name, bool is_external)
{
    Io_Push_Buffer* io = (Io_Push_Buffer*)io_;
    push_buffer_append(&io->pb, *length);
}

inline void
io_pb_array_begin_u32(Io_Vtable* io_, u32* length, String name, bool is_external)
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
io_slice_array_begin_i32(Io_Vtable* io_, i32* length, String name, bool is_external)
{
    Io_Slice_Reader* io = (Io_Slice_Reader*)io_;
    *length = *slice_read<i32>(&io->reader);
}

inline void
io_slice_array_begin_u32(Io_Vtable* io_, u32* length, String name, bool is_external)
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



// -- I/O visitor that writes to a file

struct Io_File_Writer
{
    Io_Push_Buffer io_pb;
    String filename;
    bool(*file_write_all_pb)(String, Push_Buffer const& pb);
};

inline void
io_file_writer_begin(Io_Vtable* io_, String name)
{
    Io_File_Writer* io_file = (Io_File_Writer*)io_;
    io_file->filename = name;
}

inline void
io_file_writer_end(Io_Vtable* io_)
{
    Io_File_Writer* io_file = (Io_File_Writer*)io_;
    if (io_file->filename.length > 0)
    {
        VERIFY(io_file->file_write_all_pb(io_file->filename, io_file->io_pb.pb));
    }
}

inline Io_File_Writer
io_file_writer_create(Memory_Region memory, int bytes_per_page, bool(*file_write_all_pb)(String, Push_Buffer const& pb))
{
    Io_File_Writer result = {};
    result.io_pb = io_pb_create(memory, bytes_per_page);
    result.io_pb.vtable.begin = io_file_writer_begin;
    result.io_pb.vtable.end = io_file_writer_end;
    result.file_write_all_pb = file_write_all_pb;
    return result;
}



// --- I/O visitor that writes to a json file

struct Io_Json_Ctx
{
    enum
    {
        OBJECT = 0,
        NIL = 0,

        ARRAY
    } context;

    int item_count;
};

struct Io_Json_Writer
{
    Io_File_Writer io_file;
    DynArray<Io_Json_Ctx> ctx_stack;
    int bytes_per_page;
    Memory_Region memory;
};

inline void
io_json_writer_begin(Io_Vtable* io, String name)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;
    if (!string_ends_with_ignore_case(name, STRING(".json")))
    {
        name = StringConcat(name, STRING(".json"), io_json->memory);
    }

    io_file_writer_begin(io, name);
}

inline void
io_json_writer_end(Io_Vtable* io)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;
    ASSERT(io_json->ctx_stack.count == 0);

    io_file_writer_end((Io_Vtable*)&io_json->io_file);
}

function void
io_json_writer_indent(Io_Json_Writer* io_json, int indent_delta=0)
{
    static u8 tab_buffer[] = { '\t', '\t', '\t', '\t', '\t', '\t', '\t', '\t' };

    int tab_count = io_json->ctx_stack.count + indent_delta;
    while (tab_count > 0)
    {
        io_pb_atom_blob((Io_Vtable*)io_json, slice_create(tab_buffer, min((int)ARRAY_LEN(tab_buffer), tab_count)), {});
        tab_count -= ARRAY_LEN(tab_buffer);
    }
}

function void
io_json_writer_object_begin(Io_Vtable* io, String name, bool is_external)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;

    bool is_root_object = io_json->ctx_stack.count == 0;

    // TODO - better string builder
    u8 quote = '\"';
    u8 colon = ':';
    u8 space = ' ';
    u8 comma = ',';
    u8 open_brace = '{';
    u8 new_line = '\n';

    if (!is_root_object)
    {
        Io_Json_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->item_count > 0)
        {
            io_pb_atom_u8(io, &comma, {});
        }

        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json);

        if (prev_ctx->context != Io_Json_Ctx::ARRAY)
        {
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_blob(io, slice_create(name), {});
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_u8(io, &colon, {});
            io_pb_atom_u8(io, &space, {});
        }

        prev_ctx->item_count++;
    }

    io_pb_atom_u8(io, &open_brace, {});

    // Push context
    Io_Json_Ctx* ctx = AppendNew(&io_json->ctx_stack);
    ctx->context = Io_Json_Ctx::OBJECT;
    ctx->item_count = 0;
}

function void
io_json_writer_object_end(Io_Vtable* io)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;
    if (io_json->ctx_stack.count > 0 &&
        array_peek_last(&io_json->ctx_stack)->context == Io_Json_Ctx::OBJECT)
    {
        u8 new_line = '\n';
        u8 close_brace = '}';
        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json, -1);
        io_pb_atom_u8(io, &close_brace, {});

        // Pop context
        array_remove_last(&io_json->ctx_stack);
    }
    else
    {
        ASSERT_FALSE;
    }
}

function void
io_json_writer_array_begin_i32(Io_Vtable* io, i32* length, String name, bool is_external)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;

    bool is_root_object = io_json->ctx_stack.count == 0;

    // TODO - better string builder
    u8 new_line = '\n';
    u8 quote = '\"';
    u8 colon = ':';
    u8 space = ' ';
    u8 comma = ',';
    u8 open_bracket = '[';

    if (!is_root_object)
    {
        Io_Json_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->item_count > 0)
        {
            io_pb_atom_u8(io, &comma, {});
        }

        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json);

        if (prev_ctx->context != Io_Json_Ctx::ARRAY)
        {
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_blob(io, slice_create(name), {});
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_u8(io, &colon, {});
            io_pb_atom_u8(io, &space, {});
        }

        prev_ctx->item_count++;
    }

    io_pb_atom_u8(io, &open_bracket, {});

    // Push context
    Io_Json_Ctx* ctx = AppendNew(&io_json->ctx_stack);
    ctx->context = Io_Json_Ctx::ARRAY;
    ctx->item_count = 0;
}

function void
io_json_writer_array_begin_u32(Io_Vtable* io, u32* length, String name, bool is_external)
{
    io_json_writer_array_begin_i32(io, nullptr, name, is_external);
}

function void
io_json_writer_array_end(Io_Vtable* io)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;
    if (io_json->ctx_stack.count > 0 &&
        array_peek_last(&io_json->ctx_stack)->context == Io_Json_Ctx::ARRAY)
    {
        u8 new_line = '\n';
        u8 close_bracket = ']';
        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json, -1);
        io_pb_atom_u8(io, &close_bracket, {});

        // Pop context
        array_remove_last(&io_json->ctx_stack);
    }
    else
    {
        ASSERT_FALSE;
    }
}

function void
io_json_writer_atom_u64(Io_Vtable* io, u64* value, String name)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;
    if (io_json->ctx_stack.count > 0)
    {
        // TODO - better string builder
        u8 new_line = '\n';
        u8 quote = '\"';
        u8 colon = ':';
        u8 space = ' ';
        u8 comma = ',';

        Io_Json_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->item_count > 0)
        {
            io_pb_atom_u8(io, &comma, {});
        }

        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json);

        if (prev_ctx->context != Io_Json_Ctx::ARRAY)
        {
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_blob(io, slice_create(name), {});
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_u8(io, &colon, {});
            io_pb_atom_u8(io, &space, {});
        }

        u8 printf_buffer[32];
        String value_str;
        value_str.data = printf_buffer;
        value_str.length = stbsp_snprintf((char*)printf_buffer, ARRAY_LEN(printf_buffer), "%llu", *value);
        io_pb_atom_blob(io, slice_create(value_str), {});

        prev_ctx->item_count++;
    }
    else
    {
        // Technically, json allows an atom value as a root object, but for our
        //  purpose we only allow object and array as the root value
        ASSERT_FALSE;
    }
}

function void
io_json_writer_atom_i64(Io_Vtable* io, i64* value, String name)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;
    if (io_json->ctx_stack.count > 0)
    {
        // TODO - better string builder
        u8 new_line = '\n';
        u8 quote = '\"';
        u8 colon = ':';
        u8 space = ' ';
        u8 comma = ',';

        Io_Json_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->item_count > 0)
        {
            io_pb_atom_u8(io, &comma, {});
        }

        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json);

        if (prev_ctx->context != Io_Json_Ctx::ARRAY)
        {
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_blob(io, slice_create(name), {});
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_u8(io, &colon, {});
            io_pb_atom_u8(io, &space, {});
        }

        u8 printf_buffer[32];
        String value_str;
        value_str.data = printf_buffer;
        value_str.length = stbsp_snprintf((char*)printf_buffer, ARRAY_LEN(printf_buffer), "%lld", *value);
        io_pb_atom_blob(io, slice_create(value_str), {});

        prev_ctx->item_count++;
    }
    else
    {
        // Technically, json allows an atom value as a root object, but for our
        //  purpose we only allow object and array as the root value
        ASSERT_FALSE;
    }
}

function void
io_json_writer_atom_string(Io_Vtable* io, String* string, Memory_Region memory, String name)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;
    if (io_json->ctx_stack.count > 0)
    {
        // TODO - better string builder
        u8 new_line = '\n';
        u8 quote = '\"';
        u8 colon = ':';
        u8 space = ' ';
        u8 comma = ',';

        Io_Json_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->item_count > 0)
        {
            io_pb_atom_u8(io, &comma, {});
        }

        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json);

        if (prev_ctx->context != Io_Json_Ctx::ARRAY)
        {
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_blob(io, slice_create(name), {});
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_u8(io, &colon, {});
            io_pb_atom_u8(io, &space, {});
        }

        io_pb_atom_u8(io, &quote, {});
        io_pb_atom_blob(io, slice_create(*string), {});
        io_pb_atom_u8(io, &quote, {});

        prev_ctx->item_count++;
    }
    else
    {
        // Technically, json allows an atom value as a root object, but for our
        //  purpose we only allow object and array as the root value
        ASSERT_FALSE;
    }
}

inline void
io_json_writer_atom_u8(Io_Vtable* io, u8* value, String name)
{
    u64 v = *value;
    io_json_writer_atom_u64(io, &v, name);
}

inline void
io_json_writer_atom_u16(Io_Vtable* io, u16* value, String name)
{
    u64 v = *value;
    io_json_writer_atom_u64(io, &v, name);
}

inline void
io_json_writer_atom_u32(Io_Vtable* io, u32* value, String name)
{
    u64 v = *value;
    io_json_writer_atom_u64(io, &v, name);
}

inline void
io_json_writer_atom_i8(Io_Vtable* io, i8* value, String name)
{
    i64 v = *value;
    io_json_writer_atom_i64(io, &v, name);
}

inline void
io_json_writer_atom_i16(Io_Vtable* io, i16* value, String name)
{
    i64 v = *value;
    io_json_writer_atom_i64(io, &v, name);
}

inline void
io_json_writer_atom_i32(Io_Vtable* io, i32* value, String name)
{
    i64 v = *value;
    io_json_writer_atom_i64(io, &v, name);
}

function Io_Json_Writer
io_json_writer_create(Memory_Region memory, int bytes_per_page, bool(*file_write_all_pb)(String, Push_Buffer const& pb))
{
    Io_Json_Writer result = {};
    result.memory = memory;
    result.ctx_stack = DynArray<Io_Json_Ctx>(memory);
    EnsureCapacity(&result.ctx_stack, 16);

    result.io_file = io_file_writer_create(memory, bytes_per_page, file_write_all_pb);
    result.io_file.io_pb.vtable.begin = io_json_writer_begin;
    result.io_file.io_pb.vtable.end = io_json_writer_end;
    result.io_file.io_pb.vtable.object_begin = io_json_writer_object_begin;
    result.io_file.io_pb.vtable.object_end = io_json_writer_object_end;
    result.io_file.io_pb.vtable.array_begin_i32 = io_json_writer_array_begin_i32;
    result.io_file.io_pb.vtable.array_begin_u32 = io_json_writer_array_begin_u32;
    result.io_file.io_pb.vtable.array_end = io_json_writer_array_end;
    result.io_file.io_pb.vtable.atom_u8 = io_json_writer_atom_u8;
    result.io_file.io_pb.vtable.atom_u16 = io_json_writer_atom_u16;
    result.io_file.io_pb.vtable.atom_u32 = io_json_writer_atom_u32;
    result.io_file.io_pb.vtable.atom_u64 = io_json_writer_atom_u64;
    result.io_file.io_pb.vtable.atom_i8 = io_json_writer_atom_i8;
    result.io_file.io_pb.vtable.atom_i16 = io_json_writer_atom_i16;
    result.io_file.io_pb.vtable.atom_i32 = io_json_writer_atom_i32;
    result.io_file.io_pb.vtable.atom_i64 = io_json_writer_atom_i64;
    result.io_file.io_pb.vtable.atom_string = io_json_writer_atom_string;
    result.io_file.io_pb.vtable.atom_blob = io_atom_blob_nop;

    return result;
}



// --- I/O visitor that writes to one or more JSON files.
//      Specific objects/arrays can be written as external JSON files.

struct Io_Json_Writer_Ext
{
    Io_Vtable vtable;
    DynArray<Io_Json_Writer> writer_stack;
};

function Io_Json_Writer
io_json_writer_ext_push_new_writer(Io_Json_Writer_Ext* iox, Io_Json_Writer const& writer_template, String name)
{
    Io_Json_Writer result = io_json_writer_create(
                                writer_template.io_file.io_pb.pb.memory,
                                writer_template.io_file.io_pb.pb.pages->capacity_b,
                                writer_template.io_file.file_write_all_pb);

    Append(&iox->writer_stack, result);
    io_json_writer_begin((Io_Vtable*)array_peek_last(&iox->writer_stack), name);
    return result;
}

function void
io_json_writer_ext_pop_writer(Io_Json_Writer_Ext* iox)
{
    Io_Json_Writer* io_json = array_peek_last(&iox->writer_stack);
    io_json_writer_end((Io_Vtable*)io_json);
    array_remove_last(&iox->writer_stack);
}

function void
io_json_writer_ext_object_begin(Io_Vtable* io, String name, bool is_external)
{
    Io_Json_Writer_Ext* iox = (Io_Json_Writer_Ext*)io;
    if (is_external)
    {
        io_json_writer_ext_push_new_writer(iox, *array_peek_last(&iox->writer_stack), name);
    }

    Io_Json_Writer* io_json = array_peek_last(&iox->writer_stack);
    io_json_writer_object_begin((Io_Vtable*)io_json, name, is_external);
}

function void
io_json_writer_ext_object_end(Io_Vtable* io)
{
    Io_Json_Writer_Ext* iox = (Io_Json_Writer_Ext*)io;
    Io_Json_Writer* io_json = array_peek_last(&iox->writer_stack);
    io_json_writer_object_end((Io_Vtable*)io_json);

    // NOTE - the bottom writer on the stack gets popped by end(..),
    //  to be more symmetrical with it's initialized in create(..)
    // HMM - maybe we just force the root object/array's is_external param to true,
    //  then we can just rely on that and not this weird create/end logic.
    if (iox->writer_stack.count > 1 &&
        io_json->ctx_stack.count == 0)
    {
        io_json_writer_ext_pop_writer(iox);
    }
}

function void
io_json_writer_ext_array_begin_i32(Io_Vtable* io, i32* length, String name, bool is_external)
{
    Io_Json_Writer_Ext* iox = (Io_Json_Writer_Ext*)io;
    if (is_external)
    {
        io_json_writer_ext_push_new_writer(iox, *array_peek_last(&iox->writer_stack), name);
    }

    Io_Json_Writer* io_json = array_peek_last(&iox->writer_stack);
    io_json_writer_array_begin_i32((Io_Vtable*)io_json, length, name, is_external);
}

function void
io_json_writer_ext_array_begin_u32(Io_Vtable* io, u32* length, String name, bool is_external)
{
    io_json_writer_ext_array_begin_i32(io, nullptr, name, is_external);
}

function void
io_json_writer_ext_array_end(Io_Vtable* io)
{
    Io_Json_Writer_Ext* iox = (Io_Json_Writer_Ext*)io;
    Io_Json_Writer* io_json = array_peek_last(&iox->writer_stack);
    io_json_writer_array_end((Io_Vtable*)io_json);

    // NOTE - the bottom writer on the stack gets popped by end(..),
    //  to be more symmetrical with it's initialized in create(..)
    // HMM - maybe we just force the root object/array's is_external param to true,
    //  then we can just rely on that and not this weird create/end logic.
    if (iox->writer_stack.count > 1 &&
        io_json->ctx_stack.count == 0)
    {
        io_json_writer_ext_pop_writer(iox);
    }
}

#define IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER(type)                            \
inline void                                                                     \
io_json_writer_ext_atom_##type(Io_Vtable* io, type* value, String name)         \
{                                                                               \
    Io_Json_Writer_Ext* iox = (Io_Json_Writer_Ext*)io;                          \
    Io_Json_Writer* io_json = array_peek_last(&iox->writer_stack);              \
    io_json_writer_atom_##type((Io_Vtable*)io_json, value, name);               \
}

IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER(u8)
IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER(u16)
IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER(u32)
IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER(u64)
IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER(i8)
IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER(i16)
IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER(i32)
IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER(i64)
#undef IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER

inline void
io_json_writer_ext_atom_string(Io_Vtable* io, String* value, Memory_Region memory, String name)
{
    Io_Json_Writer_Ext* iox = (Io_Json_Writer_Ext*)io;
    Io_Json_Writer* io_json = array_peek_last(&iox->writer_stack);
    io_json_writer_atom_string((Io_Vtable*)io_json, value, memory, name);
}

function void
io_json_writer_ext_begin(Io_Vtable* io, String name)
{
    Io_Json_Writer_Ext* iox = (Io_Json_Writer_Ext*)io;

    // A writer is already pushed to the stack from create(..)
    ASSERT(iox->writer_stack.count == 1);
    io_json_writer_begin((Io_Vtable*)array_peek_last(&iox->writer_stack), name);
}

function void
io_json_writer_ext_end(Io_Vtable* io)
{
    Io_Json_Writer_Ext* iox = (Io_Json_Writer_Ext*)io;
    io_json_writer_end((Io_Vtable*)array_peek_last(&iox->writer_stack));

    ASSERT(iox->writer_stack.count == 1);
    array_remove_last(&iox->writer_stack);
    ASSERT(iox->writer_stack.count == 0);
}

function Io_Json_Writer_Ext
io_json_writer_ext_create(Memory_Region memory, int bytes_per_page, bool(*file_write_all_pb)(String, Push_Buffer const& pb))
{
    Io_Json_Writer_Ext result = {};
    result.writer_stack = DynArray<Io_Json_Writer>(memory);

    // NOTE - Creating the first writer in the stack here, rather than begin(..), since we have the params we need
    Append(&result.writer_stack, io_json_writer_create(memory, bytes_per_page, file_write_all_pb));

    result.vtable.begin = io_json_writer_ext_begin;
    result.vtable.end = io_json_writer_ext_end;
    result.vtable.object_begin = io_json_writer_ext_object_begin;
    result.vtable.object_end = io_json_writer_ext_object_end;
    result.vtable.array_begin_i32 = io_json_writer_ext_array_begin_i32;
    result.vtable.array_begin_u32 = io_json_writer_ext_array_begin_u32;
    result.vtable.array_end = io_json_writer_ext_array_end;
    result.vtable.atom_u8 = io_json_writer_ext_atom_u8;
    result.vtable.atom_u16 = io_json_writer_ext_atom_u16;
    result.vtable.atom_u32 = io_json_writer_ext_atom_u32;
    result.vtable.atom_u64 = io_json_writer_ext_atom_u64;
    result.vtable.atom_i8 = io_json_writer_ext_atom_i8;
    result.vtable.atom_i16 = io_json_writer_ext_atom_i16;
    result.vtable.atom_i32 = io_json_writer_ext_atom_i32;
    result.vtable.atom_i64 = io_json_writer_ext_atom_i64;
    result.vtable.atom_string = io_json_writer_ext_atom_string;
    result.vtable.atom_blob = io_atom_blob_nop;
    result.vtable.is_deserializing = io_return_false;
    return result;
}


// bool constexpr IO_PATCH = false;
