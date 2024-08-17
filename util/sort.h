// TODO - Way to reduce copypasta here?
// template <class T>
// using FnComparator = int (*)(T const&, T const&);
// template <class T>
// using FnComparatorByPtr = int (*)(T const*, T const*);
// template <class T, class CTX>
// using FnComparatorWithCtx = int (*)(T const&, T const&, CTX *);
// template <class T, class CTX>
// using FnComparatorByPtrWithCtx = int (*)(T const*, T const*, CTX *);

// NOTE - If not found, returns a negative value that can be bit-flipped with ~
//    to find the index that the item *should* go into to keep the list sorted
template <class T, class FN_COMPARATOR>
int BinarySearch(Slice<T> sorted, T const& item, FN_COMPARATOR compare)
{
    // --- Binary search to find desired index
    int iLow = 0;
    int iHigh = sorted.count - 1;

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
    // NOTE - The array param technically doesn't need to be a pointer, but since
    //  we are potentially modifying the items array, a pointer communicates the intent better.

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
