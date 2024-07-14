struct Gap_Buffer
{
    u8* buffer;
    i32 capacity;

    i32 gap_start;      // aka, "cursor"
    i32 gap_count;

    Memory_Region memory;
};

function void
gap_buffer_init(Gap_Buffer* buffer, Memory_Region memory, i32 OptionalArg(capacity, 128))
{
    capacity = max(capacity, 16);

    *buffer = {};

    buffer->capacity = capacity;
    buffer->buffer = allocate_array<u8>(memory, buffer->capacity);
    buffer->memory = memory;
}

function Gap_Buffer*
gap_buffer_create(Memory_Region memory, i32 OptionalArg(capacity, 128))
{
    Gap_Buffer* result = allocate<Gap_Buffer>(memory);
    gap_buffer_init(result, memory, capacity);
    return result;
}

function void
gap_buffer_insert(Gap_Buffer* gb, char c)
{
    if (gb->gap_count > 0)
    {
        gb->buffer[gb->gap_start] = c;
        gb->gap_start++;
        gb->gap_count--;
    }
    else
    {
        AssertTodo;     // allocate more memory
    }
}

function void
gap_buffer_insert(Gap_Buffer* gb, String string)
{
    if (gb->gap_count >= string.length)
    {
        mem_copy(gb->buffer + gb->gap_start, string.data, string.length);
        gb->gap_start += string.length;
        gb->gap_count -= string.length;
    }
    else
    {
        AssertTodo;     // allocate more memory
    }
}

function void
gap_buffer_set_cursor(Gap_Buffer* gb, i32 index)
{
    i32 data_length = gb->capacity - gb->gap_count;
    Assert(data_length > 0);

    index = clamp(index, 0, data_length);

    i32 diff = index - gb->gap_start;
    i32 old_gap_end = gb->gap_start + gb->gap_count;

    if (diff < 0)
    {
        // Cursor moved to the left
        i32 new_gap_end = old_gap_end + diff;
        mem_move(gb->buffer + new_gap_end, gb->buffer + index, -diff);
    }
    else if (diff > 0)
    {
        // Cursor moved to the right
        mem_move(gb->buffer + gb->gap_start, gb->buffer + old_gap_end, diff);
    }

    gb->gap_start = index;
}

function i32
gap_buffer_move_cursor_left(Gap_Buffer* gb, i32 OptionalArg(count, 1))
{
    gap_buffer_set_cursor(gb, gb->gap_start - count);
    return gb->gap_start;
}

function i32
gap_buffer_move_cursor_right(Gap_Buffer* gb, i32 OptionalArg(count, 1))
{
    gap_buffer_set_cursor(gb, gb->gap_start + count);
    return gb->gap_start;
}
