// TODO - Way to reduce copypasta here?
template <typename T>
using FnComparator = int (*)(T, T);                            // @C++11 "type alias" syntax
template <typename T>
using FnComparatorByPtr = int (*)(T*, T*);                //    ...
template <typename T, typename Ctx>
using FnComparatorWithCtx = int (*)(T, T, Ctx *);            //    ...
template <typename T, typename Ctx>
using FnComparatorByPtrWithCtx = int (*)(T*, T*, Ctx *);    //    ...

// NOTE - If not found, returns a negative value that can be bit-flipped with ~
//    to find the index that the item *should* go into to keep the list sorted
template <typename T>
int BinarySearch(Slice<T> sorted, T & item, FnComparator<T> Compare)
{
    // --- Binary search to find desired index
    int iLow = 0;
    int iHigh = sorted.count - 1;

    while (iHigh >= iLow)
    {
        int iCandidate = (iLow + iHigh) / 2;
        T* candidate = sorted.items + iCandidate;

        int compareResult = Compare(item, *candidate);
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
    Assert(result < 0);
    return result;
}

template <typename T>
void InsertSorted(DynArray<T> * array, T & item, FnComparator<T> Compare)
{
    int index = BinarySearch(MakeSlice(*array), item, Compare);
    if (index < 0)
    {
        index = ~index;
    }

    Insert(array, item, index);
}

template <typename T>
void BubbleSort(Slice<T> slice, FnComparator<T> Compare)
{
    for (int i = 0; i < slice.count - 1; i++)
    {
        bool swappedAny = false;
        for (int j = 0; j < slice.count - 1 - i; j++)
        {
            T* t0 = slice + j;
            T* t1 = slice + j + 1;

            if (Compare(*t0, *t1) > 0)
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

template <typename T, typename Ctx>
void BubbleSort(Slice<T> slice, FnComparatorWithCtx<T, Ctx> Compare, Ctx * ctx)
{
    // @Sync with implementation above!

    for (int i = 0; i < slice.count - 1; i++)
    {
        bool swappedAny = false;
        for (int j = 0; j < slice.count - 1 - i; j++)
        {
            T* t0 = slice + j;
            T* t1 = slice + j + 1;

            if (Compare(*t0, *t1, ctx) > 0)
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

template <typename T>
inline void
BubbleSort(DynArray<T> * array, FnComparator<T> comparator)
{
    // NOTE - The array param technically doesn't need to be a pointer, but since
    //  we are potentially modifying the items array, a pointer communicates the intent better.

    BubbleSort(MakeSlice(*array), comparator);
}

template <typename T, typename Ctx>
inline void
BubbleSort(DynArray<T> * array, FnComparatorWithCtx<T, Ctx> comparator, Ctx * ctx)
{
    BubbleSort(MakeSlice(*array), comparator, ctx);
}
