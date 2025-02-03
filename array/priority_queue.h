// --- Priority Queue

template <typename T>
struct Priority_Queue
{
    DynArray<T> array;
    int (*compare)(T const& a, T const& b);
};

template <typename T>
function Priority_Queue<T>
priority_queue_create(
    Memory_Region memory,
    int (*compare)(T const& a, T const& b))
{
    Priority_Queue<T> result;
    result.array = DynArray<T>(memory);
    result.compare = compare;
    return result;
}

template <typename T>
function bool
priority_queue_is_empty(Priority_Queue<T> const& queue)
{
    bool result = array_is_empty(queue.array);
    return result;
}

template <typename T>
function void
priority_queue_insert(Priority_Queue<T>* queue, T const& item)
{
    DynArray<T>* array = &queue->array;
    Append(array, item);

    // Heapify "up"
    int index = array->count - 1;
    while (index > 0)
    {
        int parent_index = (index - 1) / 2;
        T* inserted = *array + index;
        T* parent = *array + parent_index;

        if (queue->compare(*inserted, *parent) >= 0)
            break;

        mem_swap(inserted, parent);
        index = parent_index;
    }
}

template <typename T>
function T
priority_queue_pop(Priority_Queue<T>* queue)
{
    if (queue->array.count <= 0)
    {
        ASSERT_FALSE;
        return T{};
    }

    // Unordered remove
    DynArray<T>& array = queue->array;
    T result = RemoveUnorderedAt(&array, 0);

    // Heapify "down"
    if (array.count > 1)
    {
        int index = 0;
        while (true)
        {
            int left_index = 2 * index + 1;
            int right_index = 2 * index + 2;
            int smallest_index = index;

            if (left_index < array.count &&
                queue->compare(array[left_index], array[smallest_index]) < 0)
            {
                smallest_index = left_index;
            }

            if (right_index < array.count &&
                queue->compare(array[right_index], array[smallest_index]) < 0)
            {
                smallest_index = right_index;
            }

            if (smallest_index == index)
            {
                break;
            }

            mem_swap(array + index, array + smallest_index);
            index = smallest_index;
        }
    }

    return result;
}
