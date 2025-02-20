// -- I/O visitor that writes to a file

using Io_Fn_File_Read = bool (*)(String filename, Memory_Region memory, Slice<u8> *out, Null_Terminate);
using Io_Fn_File_Write_From_Pb = bool (*)(String filename, Push_Buffer const& pb);

struct Io_File_Writer
{
    Io_Push_Buffer io_pb;
    String filename;
    Io_Fn_File_Write_From_Pb file_write_all_pb;
};

struct Io_Json_Ctx
{
    enum Type
    {
        OBJECT = 0,
        NIL = 0,

        ARRAY,

        ENUM_COUNT
    } type;
};

// --- Context for writing a single JSON array, or object

struct Io_Json_Writer_Ctx : Io_Json_Ctx
{
    int item_count;
};

// --- I/O visitor that writes to a json file

struct Io_Json_Writer
{
    Io_File_Writer io_file;
    Memory_Region memory;
    DynArray<Io_Json_Writer_Ctx> ctx_stack;
};



// --- I/O visitor that writes to one or more JSON files.
//      Specific objects/arrays can be written as external JSON files.

struct Io_Json_Writer_Ext
{
    Io_Vtable vtable;
    Memory_Region memory;
    DynArray<Io_Json_Writer> writer_stack;
};



// --- Context for reading a single JSON array, or object.

struct Io_Json_Value
{
    int start_index;        // view into the json string
    int length;             //  ...

    int sub_value_count;    // Object and Array values track how many sub-values they have
                            //  as they're parsed, so the visitor caller can know how many
                            //  items to visit in the array.
};

struct Io_Json_Reader_Ctx : Io_Json_Ctx
{
    Memory_Region memory;
    int start_index;
    int length;

    // Keys are property names for objects, and 4-byte index strings for arrays.
    Dict<String, Io_Json_Value> values;
};

// --- I/O visitor that reads from a json file.

struct Io_Json_Reader
{
    static u8 constexpr valid_uint_chars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

    Io_Slice_Reader io_slice;
    Memory_Region memory;
    DynArray<Io_Json_Reader_Ctx> ctx_stack;
    bool file_loaded;
};

// --- I/O visitor that reads from one or more JSON files.
//      Specific objects/arrays can be read as external JSON files.

struct Io_Json_Reader_Ext
{
    Io_Vtable vtable;
    Memory_Region memory;
    DynArray<Io_Json_Reader> reader_stack;
    Io_Fn_File_Read file_read_all;
};
