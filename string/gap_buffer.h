struct Gap_Buffer
{
    u8* buffer;
    i32 capacity;

    i32 gap_start;      // aka, "cursor"
    i32 gap_size;

    Memory_Region memory;
};

function void
gap_buffer_init(Gap_Buffer* gb, Memory_Region memory, i32 capacity=128)
{
    capacity = max(capacity, 16);

    *gb = {};

    gb->capacity = capacity;
    gb->gap_size = capacity;
    gb->buffer = allocate_array_tracked<u8>(memory, capacity);
    gb->memory = memory;
}

function Gap_Buffer*
gap_buffer_create(Memory_Region memory, i32 capacity=128)
{
    Gap_Buffer* result = allocate<Gap_Buffer>(memory);
    gap_buffer_init(result, memory, capacity);
    return result;
}

function void
gap_buffer_ensure_gap_size(Gap_Buffer* gb, i32 minimum_gap_size)
{
    minimum_gap_size = max(minimum_gap_size, 0);
    if (gb->gap_size < minimum_gap_size)
    {
        i32 capacity_old = gb->capacity;
        i32 right_of_gap_size = gb->capacity - gb->gap_start - gb->gap_size;
        Assert(right_of_gap_size >= 0);

        i32 gap_size_delta = minimum_gap_size - gb->gap_size;
        i32 minimum_capacity_new = gb->capacity + gap_size_delta;

        i32 capacity_new = gb->capacity;
        Assert(capacity_new < minimum_capacity_new);
        while (capacity_new < minimum_capacity_new)
        {
            capacity_new *= 2;
        }

        gb->buffer = reallocate_array_tracked(gb->memory, gb->buffer, capacity_new);
        gb->gap_size += (capacity_new - gb->capacity);
        gb->capacity = capacity_new;

        mem_move(gb->buffer + gb->capacity - right_of_gap_size,
                 gb->buffer + gb->gap_start + gb->gap_size,
                 right_of_gap_size);
    }
}

function void
gap_buffer_insert(Gap_Buffer* gb, char c)
{
    gap_buffer_ensure_gap_size(gb, 1);

    gb->buffer[gb->gap_start] = c;
    gb->gap_start++;
    gb->gap_size--;

    Assert(gb->gap_start + gb->gap_size <= gb->capacity);
    Assert(gb->gap_size >= 0);
}

function void
gap_buffer_insert(Gap_Buffer* gb, String string)
{
    gap_buffer_ensure_gap_size(gb, string.length);

    mem_copy(gb->buffer + gb->gap_start, string.data, string.length);
    gb->gap_start += string.length;
    gb->gap_size -= string.length;

    Assert(gb->gap_start + gb->gap_size <= gb->capacity);
    Assert(gb->gap_size >= 0);
}

function void
gap_buffer_set_cursor(Gap_Buffer* gb, i32 index)
{
    i32 data_length = gb->capacity - gb->gap_size;
    Assert(data_length > 0);

    index = clamp(index, 0, data_length);

    i32 diff = index - gb->gap_start;
    i32 gap_end_old = gb->gap_start + gb->gap_size;

    if (diff < 0)
    {
        // Cursor moved to the left
        i32 gap_end_new = gap_end_old + diff;
        mem_move(gb->buffer + gap_end_new, gb->buffer + index, -diff);
    }
    else if (diff > 0)
    {
        // Cursor moved to the right
        mem_move(gb->buffer + gb->gap_start, gb->buffer + gap_end_old, diff);
    }

    gb->gap_start = index;
}

function i32
gap_buffer_move_cursor_left(Gap_Buffer* gb, i32 count=1)
{
    gap_buffer_set_cursor(gb, gb->gap_start - count);
    return gb->gap_start;
}

function i32
gap_buffer_move_cursor_right(Gap_Buffer* gb, i32 count=1)
{
    gap_buffer_set_cursor(gb, gb->gap_start + count);
    return gb->gap_start;
}

// function void
// gap_buffer_remove(Gap_Buffer* gb, i32 start, i32 count)
// {
//     AssertTodo;
// }

function void
gap_buffer_backspace(Gap_Buffer* gb)
{
    if (gb->gap_start > 0)
    {
        gb->gap_start--;
        gb->gap_size++;
    }
}

function String
string_create(Gap_Buffer const& gb, Memory_Region memory)
{
    String result;
    result.length = gb.capacity - gb.gap_size;
    result.data = (u8*)allocate(memory, result.length + 1);

    i32 gap_end = gb.gap_start + gb.gap_size;

    mem_copy(result.data, gb.buffer, gb.gap_start);
    mem_copy(result.data + gb.gap_start, gb.buffer + gap_end, gb.capacity - gap_end);
    result.data[result.length] = '\0';
    return result;
}
