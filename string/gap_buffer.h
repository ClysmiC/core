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
gap_size_ensure(Gap_Buffer* gb, int minimum_gap_size)
{
    minimum_gap_size = max(0, minimum_gap_size);
    if (gb->gap_size < minimum_gap_size)
    {
        int capacity_old = gb->capacity;
        int right_of_gap_size = gb->capacity - gb->gap_start - gb->gap_size;
        ASSERT(right_of_gap_size >= 0);

        int gap_size_delta = minimum_gap_size - gb->gap_size;
        int minimum_capacity_new = gb->capacity + gap_size_delta;

        int capacity_new = gb->capacity;
        ASSERT(capacity_new < minimum_capacity_new);
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
gap_buffer_capacity_ensure(Gap_Buffer* gb, int minimum_capacity)
{
    minimum_capacity = max(0, minimum_capacity);
    if (gb->capacity < minimum_capacity)
    {
        int minimum_gap_size = gb->gap_size + (minimum_capacity - gb->capacity);
        gap_size_ensure(gb, minimum_gap_size);

        ASSERT(gb->capacity >= minimum_capacity);
    }
}

function void
gap_buffer_copy_contents(Gap_Buffer* dst, Gap_Buffer const& src)
{
    gap_buffer_capacity_ensure(dst, src.capacity);
    dst->gap_start = src.gap_start;

    // dst may have greater capacity than src
    dst->gap_size = src.gap_size + (dst->capacity - src.capacity);
    ASSERT(dst->gap_size >= src.gap_size);
    ASSERT(dst->capacity >= src.capacity);

    int src_gap_end = src.gap_start + src.gap_size;
    int dst_gap_end = dst->gap_start + dst->gap_size;

    mem_copy(dst->buffer, src.buffer, src.gap_start);
    mem_copy(dst->buffer + dst_gap_end, src.buffer + src_gap_end, src.capacity - src_gap_end);
}

function void
gap_buffer_insert(Gap_Buffer* gb, char c)
{
    gap_size_ensure(gb, 1);

    gb->buffer[gb->gap_start] = c;
    gb->gap_start++;
    gb->gap_size--;

    ASSERT(gb->gap_start + gb->gap_size <= gb->capacity);
    ASSERT(gb->gap_size >= 0);
}

function void
gap_buffer_insert(Gap_Buffer* gb, String string)
{
    gap_size_ensure(gb, string.length);

    mem_copy(gb->buffer + gb->gap_start, string.data, string.length);
    gb->gap_start += string.length;
    gb->gap_size -= string.length;

    ASSERT(gb->gap_start + gb->gap_size <= gb->capacity);
    ASSERT(gb->gap_size >= 0);
}

function void
gap_buffer_cursor_set(Gap_Buffer* gb, int index)
{
    int data_length = gb->capacity - gb->gap_size;
    ASSERT(data_length >= 0);

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
gap_buffer_cursor_to_start(Gap_Buffer* gb)
{
    gap_buffer_cursor_set(gb, 0);
    return gb->gap_start;
}

function int
gap_buffer_cursor_to_end(Gap_Buffer* gb)
{
    gap_buffer_cursor_set(gb, gb->capacity - gb->gap_size);
    return gb->gap_start;
}

function int
gap_buffer_cursor_move_left(Gap_Buffer* gb, int count=1)
{
    gap_buffer_cursor_set(gb, gb->gap_start - count);
    return gb->gap_start;
}

function int
gap_buffer_cursor_move_right(Gap_Buffer* gb, int count=1)
{
    gap_buffer_cursor_set(gb, gb->gap_start + count);
    return gb->gap_start;
}

function Range
selection_compute(Gap_Buffer const& gb, int selection_mark)
{
    Range result;

    result.start = min(gb.gap_start, selection_mark);
    int end = max(gb.gap_start, selection_mark);

    int data_length = gb.capacity - gb.gap_size;
    result.start = clamp(result.start, 0, data_length);
    end = clamp(end, result.start, data_length);

    result.length = end - result.start;
    ASSERT(result.length >= 0);

    return result;
}

// Remove all characters between gap_start and selection_mark
function void
gap_buffer_clear_selection(Gap_Buffer* gb, int selection_mark)
{
    Range selection = selection_compute(*gb, selection_mark);
    if (selection.length <= 0)
        return;

    gb->gap_start = selection.start;
    gb->gap_size += selection.length;;

    ASSERT(gb->gap_start + gb->gap_size <= gb->capacity);
}

function void
gap_buffer_clear_left(Gap_Buffer* gb, int count=1)
{
    count = clamp(count, 0, gb->gap_start);
    gb->gap_start -= count;
    gb->gap_size += count;
}

function void
gap_buffer_clear_left_all(Gap_Buffer* gb)
{
    gb->gap_size += gb->gap_start;
    gb->gap_start = 0;
}

function void
gap_buffer_clear_right(Gap_Buffer* gb, int count=1)
{
    int right_size = gb->capacity - (gb->gap_start + gb->gap_size);
    count = clamp(count, 0, right_size);
    gb->gap_size += count;
}

function void
gap_buffer_clear_right_all(Gap_Buffer* gb)
{
    int right_size = gb->capacity - (gb->gap_start + gb->gap_size);
    gb->gap_size += right_size;
}

function void
gap_buffer_clear(Gap_Buffer* gb)
{
    gb->gap_start = 0;
    gb->gap_size = gb->capacity;
}

function int
word_boundary_locate_left(Gap_Buffer const& gb)
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
word_boundary_locate_right(Gap_Buffer const& gb)
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
    String result = {};
    result.length = gb.capacity - gb.gap_size;
    if (result.length > 0)
    {
        result.data = (u8*)allocate(memory, result.length + 1);

        int gap_end = gb.gap_start + gb.gap_size;

        mem_copy(result.data, gb.buffer, gb.gap_start);
        mem_copy(result.data + gb.gap_start, gb.buffer + gap_end, gb.capacity - gap_end);
        result.data[result.length] = '\0';
    }

    return result;
}

function String
string_create_from_selection(Gap_Buffer const& gb, int selection_mark, Memory_Region memory)
{
    String result = {};
    Range selection = selection_compute(gb, selection_mark);
    if (selection.length <= 0)
        return result;

    result.length = selection.length;
    result.data = (u8*)allocate(memory, result.length + 1);

    if (selection.start < gb.gap_start)
    {
        // Due to the nature of a gap_buffer, it shouldn't be possible for
        //  the selection to straddle the gap... since the gap inherently
        //  represents one side of the selection
        ASSERT(gb.gap_start - selection.start == selection.length);
        mem_copy(result.data, gb.buffer + selection.start, selection.length);
        result.data[result.length] = '\0';
    }
    else
    {
        ASSERT(selection.start == gb.gap_start);
        ASSERT(selection.start + gb.gap_size + selection.length <= gb.capacity);
        mem_copy(result.data, gb.buffer + selection.start + gb.gap_size, selection.length);
        result.data[result.length] = '\0';
    }

    return result;
}
