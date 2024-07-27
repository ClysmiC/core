struct Gap_Buffer
{
    u8* buffer;
    int capacity;

    int gap_start;      // aka, "cursor", "caret"
    int gap_size;

    Memory_Region memory;
};

function void
gap_buffer_init(Gap_Buffer* gb, Memory_Region memory, int capacity=128)
{
    capacity = max(capacity, 16);

    *gb = {};

    gb->capacity = capacity;
    gb->gap_size = capacity;
    gb->buffer = allocate_array_tracked<u8>(memory, capacity);
    gb->memory = memory;
}

function Gap_Buffer*
gap_buffer_create(Memory_Region memory, int capacity=128)
{
    Gap_Buffer* result = allocate<Gap_Buffer>(memory);
    gap_buffer_init(result, memory, capacity);
    return result;
}

function void
gap_buffer_ensure_gap_size(Gap_Buffer* gb, int minimum_gap_size)
{
    minimum_gap_size = max(minimum_gap_size, 0);
    if (gb->gap_size < minimum_gap_size)
    {
        int capacity_old = gb->capacity;
        int right_of_gap_size = gb->capacity - gb->gap_start - gb->gap_size;
        Assert(right_of_gap_size >= 0);

        int gap_size_delta = minimum_gap_size - gb->gap_size;
        int minimum_capacity_new = gb->capacity + gap_size_delta;

        int capacity_new = gb->capacity;
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
gap_buffer_set_cursor(Gap_Buffer* gb, int index)
{
    int data_length = gb->capacity - gb->gap_size;
    Assert(data_length >= 0);

    index = clamp(index, 0, data_length);

    int diff = index - gb->gap_start;
    int gap_end_old = gb->gap_start + gb->gap_size;

    if (diff < 0)
    {
        // Cursor moved to the left
        int gap_end_new = gap_end_old + diff;
        mem_move(gb->buffer + gap_end_new, gb->buffer + index, -diff);
    }
    else if (diff > 0)
    {
        // Cursor moved to the right
        mem_move(gb->buffer + gb->gap_start, gb->buffer + gap_end_old, diff);
    }

    gb->gap_start = index;
}

function int
gap_buffer_set_cursor_to_start(Gap_Buffer* gb)
{
    gap_buffer_set_cursor(gb, 0);
    return gb->gap_start;
}

function int
gap_buffer_set_cursor_to_end(Gap_Buffer* gb)
{
    gap_buffer_set_cursor(gb, gb->capacity - gb->gap_size);
    return gb->gap_start;
}

function int
gap_buffer_move_cursor_left(Gap_Buffer* gb, int count=1)
{
    gap_buffer_set_cursor(gb, gb->gap_start - count);
    return gb->gap_start;
}

function int
gap_buffer_move_cursor_right(Gap_Buffer* gb, int count=1)
{
    gap_buffer_set_cursor(gb, gb->gap_start + count);
    return gb->gap_start;
}

// Remove all characters between gap_start and selection_mark
function void
gap_buffer_remove_selection(Gap_Buffer* gb, int selection_mark)
{
    int selection_start = min(gb->gap_start, selection_mark);
    int selection_end = max(gb->gap_start, selection_mark);

    int data_length = gb->capacity - gb->gap_size;
    selection_start = clamp(selection_start, 0, data_length);
    selection_end = clamp(selection_end, selection_start, data_length);

    if (selection_start == selection_end)
        return;

    Assert(selection_end > selection_start);

    gb->gap_start = selection_start;
    gb->gap_size += (selection_end - selection_start);

    Assert(gb->gap_start + gb->gap_size <= gb->capacity);
}

function void
gap_buffer_delete_left(Gap_Buffer* gb, int count=1)
{
    count = clamp(count, 0, gb->gap_start);
    gb->gap_start -= count;
    gb->gap_size += count;
}

function void
gap_buffer_delete_left_all(Gap_Buffer* gb)
{
    gb->gap_size += gb->gap_start;
    gb->gap_start = 0;
}

function void
gap_buffer_delete_right(Gap_Buffer* gb, int count=1)
{
    int right_size = gb->capacity - (gb->gap_start + gb->gap_size);
    count = clamp(count, 0, right_size);
    gb->gap_size += count;
}

function void
gap_buffer_delete_right_all(Gap_Buffer* gb)
{
    int right_size = gb->capacity - (gb->gap_start + gb->gap_size);
    gb->gap_size += right_size;
}

function int
gap_buffer_locate_word_boundary_left(Gap_Buffer const& gb)
{
    int result = 0;
    int cursor = gb.gap_start;

    if (cursor > 0)
    {
        cursor--;

        bool starts_with_multi_whitespace =
            cursor > 0 &&
            char_is_whitespace(gb.buffer[cursor]) &&
            char_is_whitespace(gb.buffer[cursor - 1]);

        cursor -= starts_with_multi_whitespace ? 2 : 1;
        result += starts_with_multi_whitespace ? 2 : 1;

        while (cursor >= 0 &&
               char_is_whitespace(gb.buffer[cursor]) == starts_with_multi_whitespace)
        {
            cursor--;
            result++;
        }
    }

    return result;
}

function int
gap_buffer_locate_word_boundary_right(Gap_Buffer const& gb)
{
    int result = 0;
    int cursor = gb.gap_start + gb.gap_size;

    if (cursor < gb.capacity)
    {
        bool starts_with_multi_whitespace =
            cursor + 1 < gb.capacity &&
            char_is_whitespace(gb.buffer[cursor]) &&
            char_is_whitespace(gb.buffer[cursor + 1]);

        cursor += starts_with_multi_whitespace ? 2 : 1;
        result += starts_with_multi_whitespace ? 2 : 1;

        while (cursor < gb.capacity &&
               char_is_whitespace(gb.buffer[cursor]) == starts_with_multi_whitespace)
        {
            cursor++;
            result++;
        }
    }

    return result;
}

function String
string_create(Gap_Buffer const& gb, Memory_Region memory)
{
    String result;
    result.length = gb.capacity - gb.gap_size;
    result.data = (u8*)allocate(memory, result.length + 1);

    int gap_end = gb.gap_start + gb.gap_size;

    mem_copy(result.data, gb.buffer, gb.gap_start);
    mem_copy(result.data + gb.gap_start, gb.buffer + gap_end, gb.capacity - gap_end);
    result.data[result.length] = '\0';
    return result;
}

