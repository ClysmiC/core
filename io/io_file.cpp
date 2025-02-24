// --- File reader

function Io_Slice_Reader
io_slice_reader_create_from_file(String filename, Memory_Region memory, Io_Fn_File_Read file_read_all)
{
    Slice<u8> data;
    if (!file_read_all(
            filename,
            memory,
            &data,
            Null_Terminate::NO))
    {
        data = {};
    }

    Io_Slice_Reader result = io_slice_reader_create(data, memory);
    return result;
}



// --- File writer

inline void
io_file_writer_begin(Io_Vtable* io, String name)
{
    Io_File_Writer* io_file = (Io_File_Writer*)io;
    io_file->filename = name;
}

inline void
io_file_writer_end(Io_Vtable* io)
{
    Io_File_Writer* io_file = (Io_File_Writer*)io;
    if (io_file->filename.length > 0)
    {
        VERIFY(io_file->file_write_all_pb(io_file->filename, io_file->io_pb.pb));
    }
}

inline Io_File_Writer
io_file_writer_create(Memory_Region memory, int bytes_per_page, Io_Fn_File_Write_From_Pb file_write_all_pb)
{
    Io_File_Writer result = {};
    result.io_pb = io_pb_create(memory, bytes_per_page);
    result.io_pb.vtable.begin = io_file_writer_begin;
    result.io_pb.vtable.end = io_file_writer_end;
    result.file_write_all_pb = file_write_all_pb;
    return result;
}



// --- JSON writer

inline void
io_json_writer_begin(Io_Vtable* io, String name)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;

    // @HACK - would be nice to allow more control over the extension, but I don't want the "name" to
    //  include a file extension... maybe make an extension field on thie Io_Json_Writer?
    if (!string_ends_with_ignore_case(name, STR(".json")))
    {
        name = string_concat(name, STR(".json"), io_json->memory);
    }

    io_file_writer_begin(io, name);
}

inline void
io_json_writer_end(Io_Vtable* io)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;
    ASSERT(io_json->ctx_stack.count == 0);

    io_file_writer_end(io);
}

function void
io_json_writer_indent(Io_Json_Writer* io_json, int indent_delta=0)
{
    static u8 space_buffer[] = {
        ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' ',
        ' ', ' ', ' ', ' '
    };

    static int constexpr SPACES_PER_INDENT = 2;

    int indent_count = io_json->ctx_stack.count + indent_delta;
    int space_count = indent_count * SPACES_PER_INDENT;
    while (space_count > 0)
    {
        io_pb_atom_blob(
            (Io_Vtable*)io_json,
            slice_create(space_buffer, min((int)ARRAY_LEN(space_buffer), space_count)),
            String{});

        space_count -= ARRAY_LEN(space_buffer);
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
        Io_Json_Writer_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->item_count > 0)
        {
            io_pb_atom_u8(io, &comma, {});
        }

        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json);

        if (prev_ctx->type != Io_Json_Ctx::ARRAY)
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
    Io_Json_Writer_Ctx* ctx = array_append_new(&io_json->ctx_stack);
    ctx->type = Io_Json_Ctx::OBJECT;
    ctx->item_count = 0;
}

function void
io_json_writer_object_end(Io_Vtable* io)
{
    Io_Json_Writer* io_json = (Io_Json_Writer*)io;
    if (io_json->ctx_stack.count > 0 &&
        array_peek_last(&io_json->ctx_stack)->type == Io_Json_Ctx::OBJECT)
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
        Io_Json_Writer_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->item_count > 0)
        {
            io_pb_atom_u8(io, &comma, {});
        }

        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json);

        if (prev_ctx->type != Io_Json_Ctx::ARRAY)
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
    Io_Json_Writer_Ctx* ctx = array_append_new(&io_json->ctx_stack);
    ctx->type = Io_Json_Ctx::ARRAY;
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
        array_peek_last(&io_json->ctx_stack)->type == Io_Json_Ctx::ARRAY)
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

        Io_Json_Writer_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->item_count > 0)
        {
            io_pb_atom_u8(io, &comma, {});
        }

        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json);

        if (prev_ctx->type != Io_Json_Ctx::ARRAY)
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

        Io_Json_Writer_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->item_count > 0)
        {
            io_pb_atom_u8(io, &comma, {});
        }

        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json);

        if (prev_ctx->type != Io_Json_Ctx::ARRAY)
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
io_json_writer_atom_f64(Io_Vtable* io, f64* value, String name)
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

        Io_Json_Writer_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->item_count > 0)
        {
            io_pb_atom_u8(io, &comma, {});
        }

        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json);

        if (prev_ctx->type != Io_Json_Ctx::ARRAY)
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
        value_str.length = stbsp_snprintf((char*)printf_buffer, ARRAY_LEN(printf_buffer), "%.6f", *value);
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
        u8 backslash = '\\';

        Io_Json_Writer_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->item_count > 0)
        {
            io_pb_atom_u8(io, &comma, {});
        }

        io_pb_atom_u8(io, &new_line, {});
        io_json_writer_indent(io_json);

        if (prev_ctx->type != Io_Json_Ctx::ARRAY)
        {
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_blob(io, slice_create(name), {});
            io_pb_atom_u8(io, &quote, {});
            io_pb_atom_u8(io, &colon, {});
            io_pb_atom_u8(io, &space, {});
        }

        io_pb_atom_u8(io, &quote, {});

#if 1
        Slice<u8> slice = slice_create(*string);
        for (u8 c: slice)
        {
            if (c == '\r') continue;    // lets stick with \n for new line...

            if (c == '\"' || c == '\'' || c == '\\' || c =='\n' || c == '\t')
            {
                io_pb_atom_u8(io, &backslash, {});
            }

            if (c == '\n')          c = 'n';
            else if (c == '\t')     c = 't';
            io_pb_atom_u8(io, &c, {});
        }
#else
        io_pb_atom_blob(io, slice_create(*string), {});
#endif
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

inline void
io_json_writer_atom_f32(Io_Vtable* io, f32* value, String name)
{
    f64 v = *value;
    io_json_writer_atom_f64(io, &v, name);
}

function Io_Json_Writer
io_json_writer_create(Memory_Region memory, int bytes_per_page, Io_Fn_File_Write_From_Pb file_write_all_pb)
{
    Io_Json_Writer result = {};
    result.memory = memory;
    result.ctx_stack = DynArray<Io_Json_Writer_Ctx>(memory);
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
    result.io_file.io_pb.vtable.atom_f32 = io_json_writer_atom_f32;
    result.io_file.io_pb.vtable.atom_f64 = io_json_writer_atom_f64;
    result.io_file.io_pb.vtable.atom_string = io_json_writer_atom_string;
    result.io_file.io_pb.vtable.atom_blob = io_atom_blob_nop;

    return result;
}



// --- JSON writer. Specific objects/arrays can be written as external JSON files.

function Io_Json_Writer
io_json_writer_ext_push_new_writer(Io_Json_Writer_Ext* iox, String name, int bytes_per_page, Io_Fn_File_Write_From_Pb file_write_all_pb)
{
    Io_Json_Writer result = io_json_writer_create(iox->memory, bytes_per_page, file_write_all_pb);

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
        Io_File_Writer* io_file_prev = &array_peek_last(&iox->writer_stack)->io_file;
        io_json_writer_ext_push_new_writer(iox, name, io_file_prev->io_pb.pb.pages->capacity_b, io_file_prev->file_write_all_pb);
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
        Io_File_Writer* io_file_prev = &array_peek_last(&iox->writer_stack)->io_file;
        io_json_writer_ext_push_new_writer(iox, name, io_file_prev->io_pb.pb.pages->capacity_b, io_file_prev->file_write_all_pb);
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
IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER(f32)
IO_JSON_WRITER_EXT_DEFINE_ATOM_WRAPPER(f64)
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
io_json_writer_ext_create(Memory_Region memory, int bytes_per_page, Io_Fn_File_Write_From_Pb file_write_all_pb)
{
    Io_Json_Writer_Ext result = {};
    result.memory = memory;
    result.writer_stack = DynArray<Io_Json_Writer>(memory);

    // NOTE - Creating the first writer in the stack here, rather than begin(..), since we have the params we need
    Append(&result.writer_stack, io_json_writer_create(memory, bytes_per_page, file_write_all_pb));

    result.vtable.flags |= Io_Visitor_Flags::TEXT;
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
    result.vtable.atom_f32 = io_json_writer_ext_atom_f32;
    result.vtable.atom_f64 = io_json_writer_ext_atom_f64;
    result.vtable.atom_string = io_json_writer_ext_atom_string;
    result.vtable.atom_blob = io_atom_blob_nop;
    return result;
}



// --- JSON file reader

inline String
string_create(Io_Json_Reader* io_json, Io_Json_Value value)
{
    String result = string_create(io_json->io_slice.reader.buffer + value.start_index, value.length);
    return result;
}

inline int
io_json_reader_consume_whitespace(Io_Json_Reader* io_json)
{
    Slice_Reader* slice_reader = &io_json->io_slice.reader;
    int result = 0;

    while(!slice_reader_is_finished(*slice_reader))
    {
        u8 c = slice_reader->buffer[slice_reader->bytes_read];
        if (char_is_whitespace(c))
        {
            slice_read_bytes(slice_reader, 1);
            result++;
        }
        else
        {
            break;
        }
    }

    return result;
}

inline bool
io_json_reader_match_and_consume(Io_Json_Reader* io_json, String string)
{
    io_json_reader_consume_whitespace(io_json);

    Slice_Reader* slice_reader = &io_json->io_slice.reader;
    int slice_bytes_remaining = slice_reader->buffer.count - slice_reader->bytes_read;

    if (slice_bytes_remaining < string.length)
        return false;

    ASSERT(!slice_reader_is_finished(*slice_reader));

    String json_str;
    json_str.data = slice_reader->buffer + slice_reader->bytes_read;
    json_str.length = string.length;

    bool result = string_eq(string, json_str);
    if (result)
    {
        slice_read_bytes(slice_reader, string.length);
    }

    return result;
}

function int
io_json_reader_match_and_consume_all(Io_Json_Reader* io_json, Slice<u8 const> valid_chars)
{
    Slice_Reader* slice_reader = &io_json->io_slice.reader;
    int result = 0;

    while (!slice_reader_is_finished(*slice_reader))
    {
        u8 c = slice_reader->buffer[slice_reader->bytes_read];

        bool is_valid = false;
        for (u8 valid_char: valid_chars)
        {
            if (c == valid_char)
            {
                is_valid = true;
                slice_read_bytes(slice_reader, 1);
                result++;
                break;
            }
        }

        if (!is_valid)
        {
            break;
        }
    }

    return result;
}

function int
io_json_reader_consume_all_until_unescaped(Io_Json_Reader* io_json, u8 until, bool *found)
{
    Slice_Reader* slice_reader = &io_json->io_slice.reader;
    int result = 0;

    while (!slice_reader_is_finished(*slice_reader))
    {
        u8 c = slice_reader->buffer[slice_reader->bytes_read];
        if (c == until)
        {
            // Check if previous character was escape char
            bool is_escaped =
                slice_reader->bytes_read > 0 &&
                slice_reader->buffer[slice_reader->bytes_read - 1] == '\\';

            if (!is_escaped)
            {
                *found = true;
                return result;
            }
        }

        slice_read_bytes(slice_reader, 1);
        result++;
    }

    // We advanced the slice reader to the end and didn't find it.
    *found = false;
    return result;
}

function String
io_json_reader_parse_and_consume_string(Io_Json_Reader* io_json)
{
    io_json_reader_consume_whitespace(io_json);
    VERIFY(io_json_reader_match_and_consume(io_json, STR("\"")));

    bool found_end_quote = false;
    String result;
    result.data = io_json->io_slice.reader.buffer + io_json->io_slice.reader.bytes_read;
    result.length = io_json_reader_consume_all_until_unescaped(io_json, '\"', &found_end_quote);

    if (found_end_quote)
    {
        VERIFY(io_json_reader_match_and_consume(io_json, STR("\"")));
    }
    else
    {
        result = string_create_empty();
    }

    return result;
}

function Io_Json_Value
io_json_reader_parse_and_consume_value(Io_Json_Reader* io_json)
{
    Slice_Reader* slice_reader = &io_json->io_slice.reader;
    io_json_reader_consume_whitespace(io_json);
    if (slice_reader_is_finished(*slice_reader))
        return {};

    i32 start_index = slice_reader->bytes_read;
    i32 end_index = -1;
    i32 sub_value_count = 0;
    Io_Json_Value_Type value_type = {};

    u8 c = slice_reader->buffer[start_index];
    switch (c)
    {
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        {
            value_type = Io_Json_Value_Type::NUMBER;

            int decimal_count = 0;
            do
            {
                slice_read_bytes(slice_reader, 1);
                if (slice_reader_is_finished(*slice_reader)) break;
                c = slice_reader->buffer[slice_reader->bytes_read];

            } while (char_is_decimal(c) || (c == '.' && (decimal_count++ == 0)));

            end_index = slice_reader->bytes_read;
        } break;

        case '\"':
        {
            value_type = Io_Json_Value_Type::STRING;

            slice_read_bytes(slice_reader, 1);
            bool found_end_quote = false;
            io_json_reader_consume_all_until_unescaped(io_json, '\"', &found_end_quote);
            if (!found_end_quote) break;
            VERIFY(io_json_reader_match_and_consume(io_json, STR("\"")));
            end_index = slice_reader->bytes_read;
        } break;

        case '[':
        {
            value_type = Io_Json_Value_Type::ARRAY;

            slice_read_bytes(slice_reader, 1);
            io_json_reader_consume_whitespace(io_json);

            int item_index = 0;
            while (!slice_reader_is_finished(*slice_reader))
            {
                char peek = slice_reader->buffer[slice_reader->bytes_read];
                if (peek == ']')
                {
                    // Finished parsing array
                    slice_read_bytes(slice_reader, 1);
                    end_index = slice_reader->bytes_read;
                    break;
                }

                if (item_index > 0)
                {
                    VERIFY(io_json_reader_match_and_consume(io_json, STR(",")));
                }

                Io_Json_Value item = io_json_reader_parse_and_consume_value(io_json);
                String item_string = string_create(io_json, item);
                if (item_string.length <= 0) break;

                io_json_reader_consume_whitespace(io_json);
                item_index++;
            }

            sub_value_count = item_index;
        } break;

        case '{':
        {
            value_type = Io_Json_Value_Type::OBJECT;

            slice_read_bytes(slice_reader, 1);
            io_json_reader_consume_whitespace(io_json);

            int prop_index = 0;
            while (!slice_reader_is_finished(*slice_reader))
            {
                char peek = slice_reader->buffer[slice_reader->bytes_read];
                if (peek == '}')
                {
                    // Finished parsing object
                    slice_read_bytes(slice_reader, 1);
                    end_index = slice_reader->bytes_read;
                    break;
                }

                if (prop_index > 0)
                {
                    VERIFY(io_json_reader_match_and_consume(io_json, STR(",")));
                }

                String prop_name = io_json_reader_parse_and_consume_string(io_json);
                if (prop_name.length <= 0) break;

                io_json_reader_consume_whitespace(io_json);
                VERIFY(io_json_reader_match_and_consume(io_json, STR(":")));

                Io_Json_Value prop = io_json_reader_parse_and_consume_value(io_json);
                String prop_string = string_create(io_json, prop);
                if (prop_string.length <= 0) break;

                io_json_reader_consume_whitespace(io_json);
                prop_index++;
            }

            sub_value_count = prop_index;
        } break;

        case 't':
        {
            value_type = Io_Json_Value_Type::BOOLEAN;

            VERIFY(io_json_reader_match_and_consume(io_json, STR("rue")));
            end_index = slice_reader->bytes_read;
        } break;

        case 'f':
        {
            value_type = Io_Json_Value_Type::BOOLEAN;

            VERIFY(io_json_reader_match_and_consume(io_json, STR("alse")));
            end_index = slice_reader->bytes_read;
        } break;

        case 'n':
        {
            value_type = Io_Json_Value_Type::NIL;

            VERIFY(io_json_reader_match_and_consume(io_json, STR("ull")));
            end_index = slice_reader->bytes_read;
        } break;

        default:
        {
            value_type = Io_Json_Value_Type::NIL;

            ASSERT(end_index < 0);
            ASSERT_FALSE_WARN;
        } break;
    }

    Io_Json_Value result = {};
    if (end_index > 0)
    {
        result.type = value_type;
        result.start_index = start_index;
        result.length = max(0, end_index - start_index);
        result.sub_value_count = sub_value_count;
    }

    return result;
}

// This is more complicated than it needs to be. It supports scanning ahead to an arbitrary
//  index, which isn't that useful. But it's mostly re-using the necessary complexity to scan
//  ahead to arbitrary object properties, which we *do* want to support
//  ... so we'll leave it complicated for the symmetry!
function Io_Json_Value
io_json_reader_find_array_item(Io_Json_Reader* io_json, int index)
{
    Io_Json_Reader_Ctx* ctx = array_peek_last(&io_json->ctx_stack);
    if (!ctx || ctx->type != Io_Json_Ctx::ARRAY)
    {
        ASSERT_FALSE;
        return {};
    }

    // Copy the index raw bytes into a string for the dictionary key. Little endian.
    // String hash/eq can handle 0 bytes since String is length-based, not 0-terminated.
    u8 index_string_buffer[4] = {
        (u8)(index >> 0),
        (u8)(index >> 8),
        (u8)(index >> 16),
        (u8)(index >> 24)
    };

    String index_string;
    index_string.data = index_string_buffer;
    index_string.length = 4;

    // --- Early out if we've already parsed over this index
    if (index < ctx->values.count)
    {
        if (Io_Json_Value* found = dict_find_ptr(ctx->values, index_string))
        {
            return *found;
        }
        else
        {
            ASSERT_FALSE;
            return {};
        }
    }

    // --- Parse forward (validating the JSON as we go) until we find this index
    while (!slice_reader_is_finished(io_json->io_slice.reader))
    {
        io_json_reader_consume_whitespace(io_json);
        if (io_json_reader_match_and_consume(io_json, STR("]")))
        {
            // --- Pop context. We didn't find the index.
            mem_region_end(ctx->memory);
            array_remove_last(&io_json->ctx_stack);
            return {};
        }

        // Parse comma
        if (ctx->values.count > 0)
        {
            io_json_reader_consume_whitespace(io_json);
            VERIFY(io_json_reader_match_and_consume(io_json, STR(",")));
        }

        u8 item_index_string_buffer[4] = {
            (u8)(ctx->values.count >> 0),
            (u8)(ctx->values.count >> 8),
            (u8)(ctx->values.count >> 16),
            (u8)(ctx->values.count >> 24)
        };

        String item_index_string;
        item_index_string.data = item_index_string_buffer;
        item_index_string.length = 4;

        // Parse value
        Io_Json_Value item = io_json_reader_parse_and_consume_value(io_json);
        if (Io_Json_Value* found = dict_find_ptr(ctx->values, item_index_string))
        {
            // We're trying to parse an index we've already parsed through.
            // This shouldn't be possible.
            ASSERT_FALSE;
        }
        else
        {
            // Copy the "index as string" into longer term memory before storing it as the dict key.
            String index_key = string_create(item_index_string, ctx->values.memory);
            dict_set(&ctx->values, index_key, item);
        }

        if (string_eq(item_index_string, index_string))
        {
            // Found it!
            return item;
        }
    }

    // --- Handle unexpected end of file
    ASSERT_FALSE_WARN;
    return {};
}

function Io_Json_Property
io_json_reader_consume_next_property(Io_Json_Reader* io_json)
{
    Io_Json_Reader_Ctx* ctx = array_peek_last(&io_json->ctx_stack);
    if (!ctx || ctx->type != Io_Json_Ctx::OBJECT)
    {
        ASSERT_FALSE_WARN;
        return {};
    }

    io_json_reader_consume_whitespace(io_json);
    if (io_json_reader_match_and_consume(io_json, STR("}")))
    {
        // --- Pop context. We didn't find a next property.
        mem_region_end(ctx->memory);
        array_remove_last(&io_json->ctx_stack);
        return {};
    }

    // Parse comma
    if (ctx->values.count > 0)
    {
        io_json_reader_consume_whitespace(io_json);
        VERIFY(io_json_reader_match_and_consume(io_json, STR(",")));
    }

    // Parse property name
    io_json_reader_consume_whitespace(io_json);

    // TODO - this name doesn't get un-escaped... it probably should...
    //  but it's not the end of the world if we don't support escape characters in property names... probably.
    String prop_name = io_json_reader_parse_and_consume_string(io_json);

    io_json_reader_consume_whitespace(io_json);
    VERIFY(io_json_reader_match_and_consume(io_json, STR(":")));

    // Parse value
    Io_Json_Value value = io_json_reader_parse_and_consume_value(io_json);

    if (Io_Json_Value* found = dict_find_ptr(ctx->values, prop_name))
    {
        // There are two properties with the same name!
        // Let's always use the first value for a given prop name.
        ASSERT_FALSE_WARN;
        value = *found;
    }
    else
    {
        // Dict key string points directly into file memory.
        //  It's valid as long as the file is valid.
        dict_set(&ctx->values, prop_name, value);
    }

    Io_Json_Property result;
    result.name = prop_name;
    result.value = value;
    return result;
}

function Io_Json_Value
io_json_reader_find_property(Io_Json_Reader* io_json, String name)
{
    Io_Json_Reader_Ctx* ctx = array_peek_last(&io_json->ctx_stack);
    if (!ctx || ctx->type != Io_Json_Ctx::OBJECT)
    {
        ASSERT_FALSE_WARN;
        return {};
    }

    // --- Early out if we've already parsed over this property
    if (Io_Json_Value* found = dict_find_ptr(ctx->values, name))
        return *found;

    // --- Parse forward (validating the JSON as we go) until we find this property
    while (!slice_reader_is_finished(io_json->io_slice.reader))
    {
        Io_Json_Property prop = io_json_reader_consume_next_property(io_json);
        if (prop.name.data && string_eq(prop.name, name))
        {
            // Found it!
            return prop.value;
        }
    }

    // --- Handle unexpected end of file
    ASSERT_FALSE_WARN;
    return {};
}

function Slice<f32>
io_json_reader_parse_array_f32(Io_Json_Reader* io_json, Io_Json_Value array_value)
{
    // Result is valid as long as reader memory is valid. Caller may want to copy out into own memory.
    DynArray<f32> result(io_json->memory);

    // Transiently parsing with a dummy reader; no side-effect on the parameter.
    Io_Json_Reader dummy_reader = *io_json;
    dummy_reader.io_slice.reader.bytes_read = array_value.start_index;
    io_json = &dummy_reader;

    if (VERIFY(array_value.type == Io_Json_Value_Type::ARRAY) &&
        io_json_reader_match_and_consume(io_json, STR("[")))
    {
        io_json_reader_consume_whitespace(io_json);

        // NOTE - this function doesn't strictly enforce that the string it parses is well-formed...
        //  it just guarantees that it will parse a well-formed string correctly. Presumably if
        //  the caller has an Io_Json_Value to pass in, they got it from a function that enforces
        //  it to be well-formed.
        while (!io_json_reader_match_and_consume(io_json, STR("]")))
        {
            if (result.count > 0)
            {
                io_json_reader_match_and_consume(io_json, STR(","));
                io_json_reader_consume_whitespace(io_json);
            }

            Io_Json_Value item_value = io_json_reader_parse_and_consume_value(io_json);
            io_json_reader_consume_whitespace(io_json);

            if (item_value.type != Io_Json_Value_Type::NUMBER)
            {
                ASSERT_FALSE_WARN;
                break;
            }

            String item_value_str = string_create(io_json, item_value);
            Append(&result, f32_parse(item_value_str));
        }
    }

    Slice<f32> result_slice = slice_create(result.items, result.count);
    return result_slice;
}

inline void
io_json_reader_begin(Io_Vtable* io, String name)
{
    // HMM - should io_slice_reader_create_from_file(..) be called here, instead of in create(..)?
    //  it would be give the API more granularity... but blegh
    Io_Json_Reader* io_json = (Io_Json_Reader*)io;
    ASSERT(io_json->file_loaded);
}

inline void
io_json_reader_end(Io_Vtable* io)
{
    Io_Json_Reader* io_json = (Io_Json_Reader*)io;
    io_json_reader_consume_whitespace(io_json);

    ASSERT(io_json->file_loaded);
    ASSERT(io_json->ctx_stack.count == 0);

    io_slice_reader_end(io);
}

function void
io_json_reader_object_begin(Io_Vtable* io, String name, bool is_external)
{
    Io_Json_Reader* io_json = (Io_Json_Reader*)io;

    Io_Json_Value object = {};
    bool is_root = io_json->ctx_stack.count == 0;
    if (is_root)
    {
        object = io_json_reader_parse_and_consume_value(io_json);
    }
    else
    {
        Io_Json_Reader_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->type == Io_Json_Ctx::ARRAY)
        {
            ASSERT_WARN(string_is_empty(name));
            object = io_json_reader_find_array_item(io_json, prev_ctx->values.count);
        }
        else
        {
            object = io_json_reader_find_property(io_json, name);
        }
    }

    String object_string = string_create(io_json, object);
    if (object_string.length <= 0 || object_string[0] != '{')
    {
        // TODO - error handling. this should catch all parse errors for
        //  the json object.
        ASSERT_FALSE_WARN;
        return;
    }

    ASSERT(object_string[object_string.length - 1] == '}');

    // Push context
    Io_Json_Reader_Ctx* ctx = array_append_new(&io_json->ctx_stack);
    ctx->type = Io_Json_Ctx::OBJECT;
    ctx->start_index = object.start_index;
    ctx->length = object.length;
    ctx->memory = mem_region_begin(io_json->memory, 64 * sizeof(Dict<String,i32>::Kvp));
    ctx->values = dict_create<String, Io_Json_Value>(ctx->memory, string_hash, string_eq);

    // Reset slice reader for the new context
    Slice_Reader* slice_reader = &io_json->io_slice.reader;
    slice_reader->bytes_read = object.start_index;
    slice_reader->bytes_read++;  // read past {
}

function void
io_json_reader_object_end(Io_Vtable* io)
{
    Io_Json_Reader* io_json = (Io_Json_Reader*)io;
    if (io_json->ctx_stack.count > 0)
    {
        Io_Json_Reader_Ctx* ctx = array_peek_last(&io_json->ctx_stack);
        if (ctx->type == Io_Json_Ctx::OBJECT)
        {
            // Reset slice reader for the new context
            Slice_Reader* slice_reader = &io_json->io_slice.reader;
            slice_reader->bytes_read = ctx->start_index + ctx->length;
            ASSERT(ctx->length > 0);

            // Pop context
            mem_region_end(ctx->memory);
            array_remove_last(&io_json->ctx_stack);
        }
        else
        {
            ASSERT_FALSE;
        }
    }
    else
    {
        ASSERT_FALSE;
    }
}

function void
io_json_reader_array_begin_i32(Io_Vtable* io, i32* length, String name, bool is_external)
{
    Io_Json_Reader* io_json = (Io_Json_Reader*)io;
    *length = 0;

    Io_Json_Value arr = {};
    int item_count = 0;

    bool is_root = io_json->ctx_stack.count == 0;
    if (is_root)
    {
        arr = io_json_reader_parse_and_consume_value(io_json);
    }
    else
    {
        Io_Json_Reader_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        if (prev_ctx->type == Io_Json_Ctx::ARRAY)
        {
            ASSERT_WARN(string_is_empty(name));
            arr = io_json_reader_find_array_item(io_json, prev_ctx->values.count);
        }
        else
        {
            arr = io_json_reader_find_property(io_json, name);
        }
    }

    String arr_string = string_create(io_json, arr);
    if (arr_string.length <= 0 || arr_string[0] != '[')
    {
        // TODO - error handling. this should catch all parse errors for
        //  the json array.
        ASSERT_FALSE_WARN;
        return;
    }

    ASSERT(arr_string[arr_string.length - 1] == ']');

    *length = arr.sub_value_count;

    // Push context
    Io_Json_Reader_Ctx* ctx = array_append_new(&io_json->ctx_stack);
    ctx->type = Io_Json_Ctx::ARRAY;
    ctx->start_index = arr.start_index;
    ctx->length = arr.length;
    ctx->memory = mem_region_begin(io_json->memory, 64 * sizeof(Dict<String,i32>::Kvp));
    ctx->values = dict_create<String, Io_Json_Value>(
                    ctx->memory,
                    string_hash,
                    string_eq);

    // Reset slice reader for the new context
    Slice_Reader* slice_reader = &io_json->io_slice.reader;
    slice_reader->bytes_read = arr.start_index;
    slice_reader->bytes_read++;  // read past [
}

function void
io_json_reader_array_begin_u32(Io_Vtable* io, u32* length, String name, bool is_external)
{
    i32 length_i32;
    io_json_reader_array_begin_i32(io, &length_i32, name, is_external);
    *length = (u32)length_i32;
}

function void
io_json_reader_array_end(Io_Vtable* io)
{
    Io_Json_Reader* io_json = (Io_Json_Reader*)io;
    if (io_json->ctx_stack.count > 0)
    {
        Io_Json_Reader_Ctx* ctx = array_peek_last(&io_json->ctx_stack);
        if (ctx->type == Io_Json_Ctx::ARRAY)
        {
            // Reset slice reader for the new context
            Slice_Reader* slice_reader = &io_json->io_slice.reader;
            slice_reader->bytes_read = ctx->start_index + ctx->length;
            ASSERT(ctx->length > 0);

            // Pop context
            mem_region_end(ctx->memory);
            array_remove_last(&io_json->ctx_stack);
        }
        else
        {
            ASSERT_FALSE;
        }
    }
    else
    {
        ASSERT_FALSE;
    }
}

function void
io_json_reader_atom_u64(Io_Vtable* io, u64* value, String name)
{
    Io_Json_Reader* io_json = (Io_Json_Reader*)io;
    *value = 0;

    if (io_json->ctx_stack.count > 0)
    {
        Io_Json_Reader_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        Io_Json_Value atom;

        if (prev_ctx->type == Io_Json_Ctx::ARRAY)
        {
            ASSERT_WARN(string_is_empty(name));
            atom = io_json_reader_find_array_item(io_json, prev_ctx->values.count);
        }
        else
        {
            atom = io_json_reader_find_property(io_json, name);
        }

        String atom_string = string_create(io_json, atom);
        if (atom_string.length <= 0 || !char_is_decimal(atom_string[0]))
        {
            ASSERT_FALSE;
        }
        else
        {
            *value = u64_parse(atom_string);
        }
    }
    else
    {
        // Technically, json allows an atom value as a root object, but for our
        //  purpose we only allow object and array as the root value
        ASSERT_FALSE;
    }
}

function void
io_json_reader_atom_i64(Io_Vtable* io, i64* value, String name)
{
    Io_Json_Reader* io_json = (Io_Json_Reader*)io;
    *value = 0;

    if (io_json->ctx_stack.count > 0)
    {
        Io_Json_Reader_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        Io_Json_Value atom;

        if (prev_ctx->type == Io_Json_Ctx::ARRAY)
        {
            ASSERT_WARN(string_is_empty(name));
            atom = io_json_reader_find_array_item(io_json, prev_ctx->values.count);
        }
        else
        {
            atom = io_json_reader_find_property(io_json, name);
        }

        String atom_string = string_create(io_json, atom);
        if (atom_string.length <= 0 ||
            !(char_is_decimal(atom_string[0]) || atom_string[0] == '-'))
        {
            ASSERT_FALSE;
        }
        else
        {
            *value = i64_parse(atom_string);
        }
    }
    else
    {
        // Technically, json allows an atom value as a root object, but for our
        //  purpose we only allow object and array as the root value
        ASSERT_FALSE;
    }
}

function void
io_json_reader_atom_f64(Io_Vtable* io, f64* value, String name)
{
    Io_Json_Reader* io_json = (Io_Json_Reader*)io;
    *value = 0;

    if (io_json->ctx_stack.count > 0)
    {
        Io_Json_Reader_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        Io_Json_Value atom;

        if (prev_ctx->type == Io_Json_Ctx::ARRAY)
        {
            ASSERT_WARN(string_is_empty(name));
            atom = io_json_reader_find_array_item(io_json, prev_ctx->values.count);
        }
        else
        {
            atom = io_json_reader_find_property(io_json, name);
        }

        String atom_string = string_create(io_json, atom);
        if (atom_string.length <= 0 ||
            !(char_is_decimal(atom_string[0]) || atom_string[0] == '-'))
        {
            ASSERT_FALSE;
        }
        else
        {
            *value = f64_parse(atom_string);
        }
    }
    else
    {
        // Technically, json allows an atom value as a root object, but for our
        //  purpose we only allow object and array as the root value
        ASSERT_FALSE;
    }
}

function void
io_json_reader_atom_string(Io_Vtable* io, String* string, Memory_Region memory, String name)
{
    Io_Json_Reader* io_json = (Io_Json_Reader*)io;
    *string = {};

    if (io_json->ctx_stack.count > 0)
    {
        Io_Json_Reader_Ctx* prev_ctx = array_peek_last(&io_json->ctx_stack);
        Io_Json_Value atom;

        if (prev_ctx->type == Io_Json_Ctx::ARRAY)
        {
            ASSERT_WARN(string_is_empty(name));
            atom = io_json_reader_find_array_item(io_json, prev_ctx->values.count);
        }
        else
        {
            atom = io_json_reader_find_property(io_json, name);
        }

        String atom_string = string_create(io_json, atom);
        if (atom_string.length < 2 || atom_string[0] != '\"')
        {
            ASSERT_FALSE;
        }
        else
        {
            // Trim off quotes
            ASSERT(atom_string[atom_string.length - 1] == '\"');
            atom_string.data++;
            atom_string.length -= 2;

            *string = string_create_from_escaped(atom_string, memory);
        }
    }
    else
    {
        // Technically, json allows an atom value as a root object, but for our
        //  purpose we only allow object and array as the root value
        ASSERT_FALSE;
    }

}

inline void
io_json_reader_atom_u8(Io_Vtable* io, u8* value, String name)
{
    u64 v = *value;
    io_json_reader_atom_u64(io, &v, name);

    if (v > U8::MAX)
    {
        ASSERT_FALSE_WARN;
        *value = U8::MAX;
    }

    *value = (u8)v;
}

inline void
io_json_reader_atom_u16(Io_Vtable* io, u16* value, String name)
{
    u64 v = *value;
    io_json_reader_atom_u64(io, &v, name);

    if (v > U16::MAX)
    {
        ASSERT_FALSE_WARN;
        *value = U16::MAX;
    }
    else
    {
        *value = (u16)v;
    }
}

inline void
io_json_reader_atom_u32(Io_Vtable* io, u32* value, String name)
{
    u64 v = *value;
    io_json_reader_atom_u64(io, &v, name);

    if (v > U32::MAX)
    {
        ASSERT_FALSE_WARN;
        *value = U32::MAX;
    }
    else
    {
        *value = (u32)v;
    }
}

inline void
io_json_reader_atom_i8(Io_Vtable* io, i8* value, String name)
{
    i64 v = *value;
    io_json_reader_atom_i64(io, &v, name);

    if (v < I8::MIN)
    {
        ASSERT_FALSE_WARN;
        *value = I8::MIN;
    }
    else if (v > I8::MAX)
    {
        ASSERT_FALSE_WARN;
        *value = I8::MAX;
    }
    else
    {
        *value = (i8)v;
    }
}

inline void
io_json_reader_atom_i16(Io_Vtable* io, i16* value, String name)
{
    i64 v = *value;
    io_json_reader_atom_i64(io, &v, name);

    if (v < I16::MIN)
    {
        ASSERT_FALSE_WARN;
        *value = I16::MIN;
    }
    else if (v > I16::MAX)
    {
        ASSERT_FALSE_WARN;
        *value = I16::MAX;
    }
    else
    {
        *value = (i16)v;
    }
}

inline void
io_json_reader_atom_i32(Io_Vtable* io, i32* value, String name)
{
    i64 v = *value;
    io_json_reader_atom_i64(io, &v, name);

    if (v < I32::MIN)
    {
        ASSERT_FALSE_WARN;
        *value = I32::MIN;
    }
    else if (v > I32::MAX)
    {
        ASSERT_FALSE_WARN;
        *value = I32::MAX;
    }
    else
    {
        *value = (i32)v;
    }
}

inline void
io_json_reader_atom_f32(Io_Vtable* io, f32* value, String name)
{
    f64 v = *value;
    io_json_reader_atom_f64(io, &v, name);
    *value = (f32)v;
}

function Io_Json_Reader
io_json_reader_create(String filename, Memory_Region memory, Io_Fn_File_Read file_read_all)
{
    // @HACK - would be nice to allow more control over the extension, but I don't want the "name" to
    //  include a file extension... maybe make an extension field on thie Io_Json_Reader?
    if (!string_ends_with_ignore_case(filename, STR(".json")))
    {
        filename = string_concat(filename, STR(".json"), memory);
    }

    Io_Json_Reader result = {};
    result.memory = memory;
    result.ctx_stack = DynArray<Io_Json_Reader_Ctx>(memory);
    EnsureCapacity(&result.ctx_stack, 16);

    result.io_slice = io_slice_reader_create_from_file(filename, memory, file_read_all);
    result.file_loaded = (result.io_slice.reader.buffer.count > 0);

    result.io_slice.vtable.flags |= (Io_Visitor_Flags::DESERIALIZING | Io_Visitor_Flags::TEXT);
    result.io_slice.vtable.begin = io_json_reader_begin;
    result.io_slice.vtable.end = io_json_reader_end;
    result.io_slice.vtable.object_begin = io_json_reader_object_begin;
    result.io_slice.vtable.object_end = io_json_reader_object_end;
    result.io_slice.vtable.array_begin_i32 = io_json_reader_array_begin_i32;
    result.io_slice.vtable.array_begin_u32 = io_json_reader_array_begin_u32;
    result.io_slice.vtable.array_end = io_json_reader_array_end;
    result.io_slice.vtable.atom_u8 = io_json_reader_atom_u8;
    result.io_slice.vtable.atom_u16 = io_json_reader_atom_u16;
    result.io_slice.vtable.atom_u32 = io_json_reader_atom_u32;
    result.io_slice.vtable.atom_u64 = io_json_reader_atom_u64;
    result.io_slice.vtable.atom_i8 = io_json_reader_atom_i8;
    result.io_slice.vtable.atom_i16 = io_json_reader_atom_i16;
    result.io_slice.vtable.atom_i32 = io_json_reader_atom_i32;
    result.io_slice.vtable.atom_i64 = io_json_reader_atom_i64;
    result.io_slice.vtable.atom_f32 = io_json_reader_atom_f32;
    result.io_slice.vtable.atom_f64 = io_json_reader_atom_f64;
    result.io_slice.vtable.atom_string = io_json_reader_atom_string;
    result.io_slice.vtable.atom_blob = io_atom_blob_nop;

    return result;
}



// --- JSON reader. Specific objects/arrays can be read as external JSON files.

function Io_Json_Reader
io_json_reader_ext_push_new_reader(Io_Json_Reader_Ext* iox, String name, Io_Fn_File_Read file_read_all)
{
    Io_Json_Reader result = io_json_reader_create(name, iox->memory, file_read_all);

    Append(&iox->reader_stack, result);
    io_json_reader_begin((Io_Vtable*)array_peek_last(&iox->reader_stack), name);
    return result;
}

function void
io_json_reader_ext_pop_reader(Io_Json_Reader_Ext* iox)
{
    Io_Json_Reader* io_json = array_peek_last(&iox->reader_stack);
    io_json_reader_end((Io_Vtable*)io_json);
    array_remove_last(&iox->reader_stack);
}

function void
io_json_reader_ext_object_begin(Io_Vtable* io, String name, bool is_external)
{
    Io_Json_Reader_Ext* iox = (Io_Json_Reader_Ext*)io;
    if (is_external)
    {
        io_json_reader_ext_push_new_reader(iox, name, iox->file_read_all);
    }

    Io_Json_Reader* io_json = array_peek_last(&iox->reader_stack);
    io_json_reader_object_begin((Io_Vtable*)io_json, name, is_external);
}

function void
io_json_reader_ext_object_end(Io_Vtable* io)
{
    Io_Json_Reader_Ext* iox = (Io_Json_Reader_Ext*)io;
    Io_Json_Reader* io_json = array_peek_last(&iox->reader_stack);
    io_json_reader_object_end((Io_Vtable*)io_json);

    // NOTE - the bottom reader on the stack gets popped by end(..),
    //  to be more symmetrical with it's initialized in create(..)
    // HMM - maybe we just force the root object/array's is_external param to true,
    //  then we can just rely on that and not this weird create/end logic.
    if (iox->reader_stack.count > 1 &&
        io_json->ctx_stack.count == 0)
    {
        io_json_reader_ext_pop_reader(iox);
    }
}

function void
io_json_reader_ext_array_begin_i32(Io_Vtable* io, i32* length, String name, bool is_external)
{
    Io_Json_Reader_Ext* iox = (Io_Json_Reader_Ext*)io;
    if (is_external)
    {
        // @Hack
        if (!string_ends_with_ignore_case(name, STR(".json")))
        {
            name = string_concat(name, STR(".json"), iox->memory);
        }
        io_json_reader_ext_push_new_reader(iox, name, iox->file_read_all);
    }

    Io_Json_Reader* io_json = array_peek_last(&iox->reader_stack);
    io_json_reader_array_begin_i32((Io_Vtable*)io_json, length, name, is_external);
}

function void
io_json_reader_ext_array_begin_u32(Io_Vtable* io, u32* length, String name, bool is_external)
{
    i32 length_i32;
    io_json_reader_ext_array_begin_i32(io, &length_i32, name, is_external);
    *length = (u32)length_i32;
}

function void
io_json_reader_ext_array_end(Io_Vtable* io)
{
    Io_Json_Reader_Ext* iox = (Io_Json_Reader_Ext*)io;
    Io_Json_Reader* io_json = array_peek_last(&iox->reader_stack);
    io_json_reader_array_end((Io_Vtable*)io_json);

    // NOTE - the bottom reader on the stack gets popped by end(..),
    //  to be more symmetrical with it's initialized in create(..)
    // HMM - maybe we just force the root object/array's is_external param to true,
    //  then we can just rely on that and not this weird create/end logic.
    if (iox->reader_stack.count > 1 &&
        io_json->ctx_stack.count == 0)
    {
        io_json_reader_ext_pop_reader(iox);
    }
}

#define IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER(type)                            \
inline void                                                                     \
io_json_reader_ext_atom_##type(Io_Vtable* io, type* value, String name)         \
{                                                                               \
    Io_Json_Reader_Ext* iox = (Io_Json_Reader_Ext*)io;                          \
    Io_Json_Reader* io_json = array_peek_last(&iox->reader_stack);              \
    io_json_reader_atom_##type((Io_Vtable*)io_json, value, name);               \
}

IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER(u8)
IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER(u16)
IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER(u32)
IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER(u64)
IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER(i8)
IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER(i16)
IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER(i32)
IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER(i64)
IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER(f32)
IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER(f64)
#undef IO_JSON_READER_EXT_DEFINE_ATOM_WRAPPER

inline void
io_json_reader_ext_atom_string(Io_Vtable* io, String* value, Memory_Region memory, String name)
{
    Io_Json_Reader_Ext* iox = (Io_Json_Reader_Ext*)io;
    Io_Json_Reader* io_json = array_peek_last(&iox->reader_stack);
    io_json_reader_atom_string((Io_Vtable*)io_json, value, memory, name);
}

function void
io_json_reader_ext_begin(Io_Vtable* io, String name)
{
    Io_Json_Reader_Ext* iox = (Io_Json_Reader_Ext*)io;

    // A reader is already pushed to the stack from create(..)
    ASSERT(iox->reader_stack.count == 1);
    io_json_reader_begin((Io_Vtable*)array_peek_last(&iox->reader_stack), name);
}

function void
io_json_reader_ext_end(Io_Vtable* io)
{
    Io_Json_Reader_Ext* iox = (Io_Json_Reader_Ext*)io;
    io_json_reader_end((Io_Vtable*)array_peek_last(&iox->reader_stack));

    ASSERT(iox->reader_stack.count == 1);
    array_remove_last(&iox->reader_stack);
    ASSERT(iox->reader_stack.count == 0);
}

function Io_Json_Reader_Ext
io_json_reader_ext_create(String name, Memory_Region memory, Io_Fn_File_Read file_read_all)
{
    Io_Json_Reader_Ext result = {};
    result.memory = memory;
    result.reader_stack = DynArray<Io_Json_Reader>(memory);

    // NOTE - Creating the first reader in the stack here, rather than begin(..), since we have the params we need
    Append(&result.reader_stack, io_json_reader_create(name, memory, file_read_all));

    result.vtable.flags |= (Io_Visitor_Flags::TEXT | Io_Visitor_Flags::DESERIALIZING);
    result.vtable.begin = io_json_reader_ext_begin;
    result.vtable.end = io_json_reader_ext_end;
    result.vtable.object_begin = io_json_reader_ext_object_begin;
    result.vtable.object_end = io_json_reader_ext_object_end;
    result.vtable.array_begin_i32 = io_json_reader_ext_array_begin_i32;
    result.vtable.array_begin_u32 = io_json_reader_ext_array_begin_u32;
    result.vtable.array_end = io_json_reader_ext_array_end;
    result.vtable.atom_u8 = io_json_reader_ext_atom_u8;
    result.vtable.atom_u16 = io_json_reader_ext_atom_u16;
    result.vtable.atom_u32 = io_json_reader_ext_atom_u32;
    result.vtable.atom_u64 = io_json_reader_ext_atom_u64;
    result.vtable.atom_i8 = io_json_reader_ext_atom_i8;
    result.vtable.atom_i16 = io_json_reader_ext_atom_i16;
    result.vtable.atom_i32 = io_json_reader_ext_atom_i32;
    result.vtable.atom_i64 = io_json_reader_ext_atom_i64;
    result.vtable.atom_f32 = io_json_reader_ext_atom_f32;
    result.vtable.atom_f64 = io_json_reader_ext_atom_f64;
    result.vtable.atom_string = io_json_reader_ext_atom_string;
    result.vtable.atom_blob = io_atom_blob_nop;
    result.file_read_all = file_read_all;
    return result;
}
