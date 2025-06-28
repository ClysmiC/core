inline int u64_compare(u64 const& lhs, u64 const& rhs) { return (lhs > rhs) - (lhs < rhs); }
inline int u32_compare(u32 const& lhs, u32 const& rhs) { return (lhs > rhs) - (lhs < rhs); }
inline int u16_compare(u16 const& lhs, u16 const& rhs) { return (lhs > rhs) - (lhs < rhs); }
inline int u8_compare(u8 const& lhs, u8 const& rhs) { return (lhs > rhs) - (lhs < rhs); }
inline int i64_compare(i64 const& lhs, i64 const& rhs) { return (lhs > rhs) - (lhs < rhs); }
inline int i32_compare(i32 const& lhs, i32 const& rhs) { return (lhs > rhs) - (lhs < rhs); }
inline int i16_compare(i16 const& lhs, i16 const& rhs) { return (lhs > rhs) - (lhs < rhs); }
inline int i8_compare(i8 const& lhs, i8 const& rhs) { return (lhs > rhs) - (lhs < rhs); }

// NOTE - does not correctly handle NAN
inline int f32_compare(f32 const& lhs, f32 const& rhs) { return (lhs > rhs) - (lhs < rhs); }
inline int f64_compare(f64 const& lhs, f64 const& rhs) { return (lhs > rhs) - (lhs < rhs); }

// NOTE - If not found, returns a negative value that can be bit-flipped with ~
//    to find the index that the item *should* go into to keep the list sorted
template <class T, class FN_COMPARATOR>
int BinarySearch(Slice<T> sorted, T const& item, int iLow, int iHigh, FN_COMPARATOR compare)
{
    if (sorted.count <= 0)
        return ~0;

    iLow = clamp(iLow, 0, sorted.count - 1);
    iHigh = clamp(iHigh, 0, sorted.count - 1); // NOTE - iHigh is inclusive! Hmm should it be?

    while (iHigh >= iLow)
    {
        int iCandidate = (iLow + iHigh) / 2;
        T* candidate = sorted.items + iCandidate;

        int compareResult = compare(item, *candidate);
        if (compareResult == 0)
            return iCandidate;

        if (compareResult > 0)
        {
            iLow = iCandidate + 1;
        }
        else if (compareResult < 0)
        {
            iHigh = iCandidate - 1;
        }
    }

    int result = ~iLow;
    ASSERT(result < 0);
    return result;
}

template <class T, class FN_COMPARATOR>
int BinarySearch(Slice<T> sorted, T const& item, FN_COMPARATOR compare)
{
    int result = BinarySearch(sorted, item, 0, sorted.count - 1, compare);
    return result;
}

template <class T, class FN_COMPARATOR>
void InsertSorted(DynArray<T> * array, T const& item, FN_COMPARATOR compare)
{
    int index = BinarySearch(slice_create(*array), item, compare);
    if (index < 0)
    {
        index = ~index;
    }

    Insert(array, item, index);
}

template <class T, class FN_COMPARATOR>
void BubbleSort(Slice<T> slice, FN_COMPARATOR compare)
{
    for (int i = 0; i < slice.count - 1; i++)
    {
        bool swappedAny = false;
        for (int j = 0; j < slice.count - 1 - i; j++)
        {
            T* t0 = slice + j;
            T* t1 = slice + j + 1;

            if (compare(*t0, *t1) > 0)
            {
                T temp = *t0;
                *t0 = *t1;
                *t1 = temp;
                swappedAny = true;
            }
        }

        // Early-out if we detect already sorted. Advantage of bubble sort! :)
        if (!swappedAny)
            return;
    }
}

template <class T, class FN_COMPARATOR, class CONTEXT>
void BubbleSort(Slice<T> slice, FN_COMPARATOR compare, CONTEXT& context)
{
    for (int i = 0; i < slice.count - 1; i++)
    {
        bool swappedAny = false;
        for (int j = 0; j < slice.count - 1 - i; j++)
        {
            T* t0 = slice + j;
            T* t1 = slice + j + 1;

            if (compare(*t0, *t1, context) > 0)
            {
                T temp = *t0;
                *t0 = *t1;
                *t1 = temp;
                swappedAny = true;
            }
        }

        // Early-out if we detect already sorted. Advantage of bubble sort! :)
        if (!swappedAny)
            return;
    }
}

template <class T, class FN_COMPARATOR>
function void
BubbleSort(DynArray<T>* array, FN_COMPARATOR compare)
{
    BubbleSort(slice_create(*array), compare);
}

template <class T, class FN_COMPARATOR>
function void
BubbleSort(T* items, int count, FN_COMPARATOR compare)
{
    BubbleSort(slice_create(items, count), compare);
}

template <class T, class FN_COMPARATOR>
void BubbleSortByPtr(Slice<T> slice, FN_COMPARATOR compare)
{
    for (int i = 0; i < slice.count - 1; i++)
    {
        bool swappedAny = false;
        for (int j = 0; j < slice.count - 1 - i; j++)
        {
            T* t0 = slice + j;
            T* t1 = slice + j + 1;

            if (compare(t0, t1) > 0)
            {
                T temp = *t0;
                *t0 = *t1;
                *t1 = temp;
                swappedAny = true;
            }
        }

        // Early-out if we detect already sorted. Advantage of bubble sort! :)
        if (!swappedAny)
            return;
    }
}

template <class T, class FN_COMPARATOR>
function void
BubbleSortByPtr(DynArray<T>* array, FN_COMPARATOR compare)
{
    // NOTE - The array param technically doesn't need to be a pointer, but since
    //  we are potentially modifying the items array, a pointer communicates the intent better.

    BubbleSortByPtr(slice_create(*array), compare);
}

template <class T, class FN_COMPARATOR>
function void
BubbleSortByPtr(T* items, int count, FN_COMPARATOR compare)
{
    BubbleSortByPtr(slice_create(items, count), compare);
}
