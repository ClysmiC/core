// IDEAS:
// - Make right side the bump side and left side the tracked side. This would make tracked blocks able to potentially re-alloc in place
// - Make headers smaller by having a u32 offset for left/right instead of pointers. This limits allocations to 8gb per (mul u32 by 4 and enforce 4-byte alignment)
//
// TODO:
// - Alignment
// - Audit for degenerate cases where things like overflow allocations are somehow too small to store an overflow + free header
// - Clear to Zero on allocations should be a separate call or compile-time template param?

enum class TrackedState : u8
{
    FreeUnshared,
    FreeShared,
    Allocated,
};

struct TrackedBlockHeader
{
    uintptr cBytes;               // Includes header
    TrackedBlockHeader * left;  // Physically adjacent in memory
    TrackedBlockHeader * right; // ...
    TrackedState state;
};

struct FreeBlockHeader : TrackedBlockHeader
{
    FreeBlockHeader * next;
    FreeBlockHeader * prev;
};

struct OverflowHeader
{
    uintptr cBytes; // Includes header
    OverflowHeader * next;
};

#if DEBUG_BUILD
struct DebugHistory
{
    struct Entry
    {
        enum Type
        {
            Free,
            Allocate
        };

        Type type;
        void* address;
    };

    int iEntryNext;
    Entry entries[64];
};

void
AddDebugEntry(DebugHistory * history, DebugHistory::Entry entry)
{
    history->entries[history->iEntryNext] = entry;
    history->iEntryNext++;
    history->iEntryNext %= ArrayLen(history->entries);
}
#endif

struct MemoryRegionHeader
{
#if DEBUG_BUILD
    DebugHistory debugHistory;
#endif

    uintptr cBytesBudget;
    MemoryRegionHeader * patron; // Provides us with memory. Must be an ancestor
    MemoryRegionHeader * parent; // When parent gets freed, we get freed too
    MemoryRegionHeader * first_child;
    MemoryRegionHeader * next_sibling;
    MemoryRegionHeader * prev_sibling;
    OverflowHeader * overflow;
    FreeBlockHeader * trackedList;
    FreeBlockHeader * sharedList;
};

namespace MEM_ALLOC // @Cleanup - need to put this after struct definitions because of sizeof dependency...
{
static constexpr uintptr cBytesTooSmallToBotherTracking = sizeof(FreeBlockHeader) + 63;
static constexpr uintptr cBytesMinimumRegion = sizeof(MemoryRegionHeader) + 256;
}

// Opaque type provided by API
typedef MemoryRegionHeader * MemoryRegion;

enum AllocType : u8
{
    Untracked,
    Tracked
};

function MemoryRegion
BeginRootMemoryRegion(u8 * bytes, uintptr cBytes)
{
    if (cBytes < MEM_ALLOC::cBytesMinimumRegion)
    {
        // Not sure how I want to handle this...

        AssertTodo;
        return nullptr;
    }

    MemoryRegionHeader * header = (MemoryRegionHeader *)bytes;
    *header = {};
    header->cBytesBudget = cBytes;
    header->sharedList = (FreeBlockHeader *)(header + 1);
    *header->sharedList = {};
    header->sharedList->cBytes = header->cBytesBudget - sizeof(MemoryRegionHeader);
    header->sharedList->state = TrackedState::FreeShared;
    return header;
}

inline void
ChangeHeadSizeAndMaybeSort(FreeBlockHeader ** ppHead, uintptr cBytesNew)
{
    FreeBlockHeader * pHeadOrig = *ppHead;

    if (cBytesNew == 0)
    {
        // NOTE - 0 has special meaning. It means to remove the head node entirely.
        //  We don't bother updating the size

        *ppHead = pHeadOrig->next;
        if (*ppHead)
        {
            (*ppHead)->prev = nullptr;
        }
    }
    else
    {
        Assert(cBytesNew > MEM_ALLOC::cBytesTooSmallToBotherTracking);
        uintptr cBytesOrig_debug = pHeadOrig->cBytes; // NOTE - only used for an assert

        pHeadOrig->cBytes = cBytesNew;
        bool isHeadTooSmall = pHeadOrig->next && pHeadOrig->next->cBytes > pHeadOrig->cBytes;

        if (isHeadTooSmall)
        {
            Assert(cBytesNew < cBytesOrig_debug);

            // Set the head to the bigger node
            *ppHead = pHeadOrig->next;
            (*ppHead)->prev = nullptr;

            if (cBytesNew > 0)
            {
                Assert(cBytesNew > MEM_ALLOC::cBytesTooSmallToBotherTracking);

                // Find the node that should point to the old head
                FreeBlockHeader * biggerThanOrig = *ppHead;
                while (biggerThanOrig->next &&
                       biggerThanOrig->next->cBytes > pHeadOrig->cBytes)
                {
                    biggerThanOrig = biggerThanOrig->next;
                }

                // Insert the old head after the found node
                pHeadOrig->prev = biggerThanOrig;
                pHeadOrig->next = biggerThanOrig->next;
                biggerThanOrig->next = pHeadOrig;

                if (pHeadOrig->next)
                {
                    pHeadOrig->next->prev = pHeadOrig;
                }
            }
        }
    }
}

// "Clear To Zero"?

enum class CTZ : u8
{
    NO = 0,
    NIL = 0,

    YES,
};

// @Cleanup - need these because core doesn't run hgen
void* Allocate(MemoryRegion region, uintptr cBytes, CTZ clearToZero=CTZ::NIL);
void* AllocateTracked(MemoryRegion region, uintptr cBytes, CTZ clearToZero=CTZ::NIL);

inline void*
AllocateFromSystem(uintptr cBytes)
{
    // TOOD - Better way to expose/configure allocations from the system/OS?
    return new u8[cBytes];
}

inline void
FreeFromSystem(void* allocation)
{
    // TOOD - Better way to expose/configure allocations from the system/OS?
    delete[] allocation;
}

inline FreeBlockHeader *
EnsureBlockWithSize(MemoryRegionHeader * regionHeader, uintptr cBytes, AllocType allocType)
{
    // NOTE - Caller is responsible for adding bytes for TrackedHeader if making a tracked allocation

    // Return tracked block if we've got one (and it's what was asked for)

    if (allocType == AllocType::Tracked &&
        regionHeader->trackedList &&
        regionHeader->trackedList->cBytes >= cBytes)
    {
        return regionHeader->trackedList;
    }

    // Allocate an overflow region from our patron if shared block is too small

    if (!regionHeader->sharedList ||
        regionHeader->sharedList->cBytes < cBytes)
    {
        // HMM - Maybe make regions have more control over how much they grow when they overflow?

        cBytes += sizeof(OverflowHeader);
        cBytes = max(cBytes, regionHeader->cBytesBudget);

        OverflowHeader * overflowHeader;
        if (regionHeader->patron)
        {
            overflowHeader = (OverflowHeader *)AllocateTracked(regionHeader->patron, cBytes);
        }
        else
        {
            overflowHeader = (OverflowHeader *)AllocateFromSystem(cBytes);
        }

        // Maintain overflow list

        overflowHeader->cBytes = cBytes;
        overflowHeader->next = regionHeader->overflow;
        regionHeader->overflow = overflowHeader;

        // Initialize free-block and make it the head of the free list

        FreeBlockHeader * overflowFree = (FreeBlockHeader *)(overflowHeader + 1);
        *overflowFree = {};
        overflowFree->cBytes = cBytes - sizeof(OverflowHeader);

        overflowFree->next = regionHeader->sharedList;
        if (overflowFree->next)
        {
            overflowFree->next->prev = overflowFree;
        }

        regionHeader->sharedList = overflowFree;
    }

    return regionHeader->sharedList;
}

void*
Allocate(MemoryRegion region, uintptr cBytes, CTZ clearToZero)
{
    // HMM - Gracefully handle 0 byte allocation?

    Assert(region);

    void* result;

    MemoryRegionHeader * regionHeader = region;

    // Make sure our shared block is big enough!

    FreeBlockHeader * shared = EnsureBlockWithSize(regionHeader, cBytes, AllocType::Untracked);
    Assert(shared == regionHeader->sharedList);

    // Split shared block into the untracked allocation (left) and ...

    result = shared;

    TrackedBlockHeader * rightOfShared = shared->right;

    uintptr splitcBytes = shared->cBytes - cBytes;
    if (splitcBytes > MEM_ALLOC::cBytesTooSmallToBotherTracking)
    {
        // ... a smaller shared block (right)

        FreeBlockHeader * split = (FreeBlockHeader *)((u8 *)shared + cBytes);

        // @Slow - Checking these conditions for what should be a simple bump allocate.
        //  This is more reason to grow the shared portion from high to low.
        //  It'd move this cost into AllocateTracked, where we are more willing to
        //  pay allocation overhead costs

        if (shared->prev)
        {
            shared->prev->next = split;
        }

        if (shared->next)
        {
            shared->next->prev = split;
        }

        MoveMemory(shared, split, sizeof(FreeBlockHeader));
        split->left = nullptr;
        if (rightOfShared)
        {
            rightOfShared->left = split;
        }

        regionHeader->sharedList = split;
        ChangeHeadSizeAndMaybeSort(&regionHeader->sharedList, splitcBytes);
    }
    else
    {
        // ... nothing! The remaining block is too small to fit anything of use

        if (rightOfShared)
        {
            rightOfShared->left = nullptr;
        }

        ChangeHeadSizeAndMaybeSort(&regionHeader->sharedList, 0); // 0 removes node altogether
    }

    if ((bool)clearToZero)
    {
        ZeroMemory(result, cBytes);
    }

    return result;
}

template <typename T>
T*
Allocate(
    MemoryRegion region,
    CTZ clearToZero=CTZ::NIL)
{
    T* result = (T*)Allocate(region, sizeof(T), clearToZero);
    return result;
}

template <typename T>
T*
AllocateArray(
    MemoryRegion region,
    uintptr count,
    CTZ clearToZero=CTZ::NIL)
{
    T* result = (T*)Allocate(region, sizeof(T) * count, clearToZero);
    return result;
}

void*
AllocateTracked(MemoryRegion region, uintptr cBytes, CTZ clearToZero)
{
    // HMM - Gracefully handle 0 byte allocation?

    Assert(region);

    void* result;

    MemoryRegionHeader * regionHeader = region;

    // Make sure our tracked or free block is big enough!

    uintptr cBytesOrig = cBytes;
    cBytes += sizeof(TrackedBlockHeader);
    cBytes = max(cBytes, sizeof(FreeBlockHeader)); // Make sure the allocation is big enough that if it gets freed, we can store a free block header in place

    FreeBlockHeader * free = EnsureBlockWithSize(regionHeader, cBytes, AllocType::Tracked);
    TrackedBlockHeader * rightOfFree = free->right;
    TrackedBlockHeader * leftOfFree = free->left;

    bool isFreeBlockFromTrackedList = (free == regionHeader->trackedList);
    Assert(Implies(!isFreeBlockFromTrackedList, free == regionHeader->sharedList));

    // Split free block into the tracked allocation (right) and ...

    // If we would've split off something
    //  that is smaller than the metadata we need to do tracking, just lump it in with the
    //  memory we give back to the user. Otherwise it'd fragment our left/right merging.

    uintptr splitcBytes = free->cBytes - cBytes;
    if (splitcBytes <= MEM_ALLOC::cBytesTooSmallToBotherTracking)
    {
        splitcBytes = 0;
        cBytes = free->cBytes; // HMM - Maybe we should have a way to tell the user that they got a little extra memory?
    }

    TrackedBlockHeader * resultHeader = (TrackedBlockHeader *)((u8 *)free + splitcBytes);
    resultHeader->cBytes = cBytes;
    resultHeader->right = free->right;
    resultHeader->state = TrackedState::Allocated;

    if (resultHeader->right)
    {
        resultHeader->right->left = resultHeader;
    }

    result = (u8 *)(resultHeader + 1);

    if (splitcBytes > MEM_ALLOC::cBytesTooSmallToBotherTracking)
    {
        // ... a smaller shared block (left)

        FreeBlockHeader * split = free;
        split->right = resultHeader;
        resultHeader->left = split;

        if (isFreeBlockFromTrackedList)
        {
            ChangeHeadSizeAndMaybeSort(&regionHeader->trackedList, splitcBytes);
        }
        else
        {
            ChangeHeadSizeAndMaybeSort(&regionHeader->sharedList, splitcBytes);
        }
    }
    else
    {
        Assert(splitcBytes == 0);

        // ... nothing! The remaining block is too small to fit anything of use.

        if (isFreeBlockFromTrackedList)
        {
            ChangeHeadSizeAndMaybeSort(&regionHeader->trackedList, 0); // 0 removes node altogether
        }
        else
        {
            ChangeHeadSizeAndMaybeSort(&regionHeader->sharedList, 0); // 0 removes node altogether
        }
    }

    // @Slow - Should be a separate call or compile-time template param?
    if (clearToZero != CTZ::NIL)
    {
        ZeroMemory(result, cBytesOrig);
    }

#if DEBUG_BUILD
    DebugHistory::Entry entry;
    entry.type = DebugHistory::Entry::Type::Allocate;
    entry.address = result;
    AddDebugEntry(&regionHeader->debugHistory, entry);
#endif

    return result;
}

template <typename T>
T*
AllocateTracked(
    MemoryRegion region,
    CTZ clearToZero=CTZ::NIL)
{
    T* result = (T*)AllocateTracked(region, sizeof(T), clearToZero);
    return result;
}

void
RemoveFromList(FreeBlockHeader ** ppHead, FreeBlockHeader * pItem)
{
    if (*ppHead == pItem)
    {
        *ppHead = pItem->next;
        if (*ppHead)
        {
            (*ppHead)->prev = nullptr;
        }
    }
    else
    {
        if (pItem->prev)
        {
            pItem->prev->next = pItem->next;
        }

        if (pItem->next)
        {
            pItem->next->prev = pItem->prev;
        }
    }
}

void
AddToListSorted(FreeBlockHeader ** ppHead, FreeBlockHeader * pItem)
{
    FreeBlockHeader ** ppNextToFix = ppHead;
    while ((*ppNextToFix) &&
           (*ppNextToFix)->cBytes > pItem->cBytes)
    {
        ppNextToFix = &(*ppNextToFix)->next;
    }

    if (ppNextToFix == ppHead)
    {
        pItem->prev = nullptr;
    }
    else
    {
        pItem->prev = (FreeBlockHeader *)((u8 *)ppNextToFix - offsetof(FreeBlockHeader, next));
    }

    pItem->next = *ppNextToFix;
    if (pItem->next)
    {
        pItem->next->prev = pItem;
    }

    *ppNextToFix = pItem;
}

void
FreeTrackedAllocation(MemoryRegion region, void* allocation)
{
    Assert(region);

    MemoryRegionHeader * regionHeader = region;
    TrackedBlockHeader * trackedHeader = (TrackedBlockHeader *)((u8 *)allocation - sizeof(TrackedBlockHeader));
    Assert(trackedHeader->state == TrackedState::Allocated);

    TrackedBlockHeader * left = trackedHeader->left;
    TrackedBlockHeader * right = trackedHeader->right;
    if (right && right->state == TrackedState::FreeUnshared)
    {
        // Merge w/ free right

        trackedHeader->cBytes += right->cBytes;

        trackedHeader->right = right->right;
        if (trackedHeader->right)
        {
            trackedHeader->right->left = trackedHeader;
        }

        RemoveFromList(&regionHeader->trackedList, (FreeBlockHeader *)right);
    }

    bool mergedIntoShared = false;

    if (left && left->state == TrackedState::FreeUnshared)
    {
        // Merge w/ free left

        left->cBytes += trackedHeader->cBytes;

        left->right = trackedHeader->right;
        if (left->right)
        {
            left->right->left = left;
        }

        RemoveFromList(&regionHeader->trackedList, (FreeBlockHeader *)left);

        trackedHeader = left;
    }
    else if (left && left->state == TrackedState::FreeShared)
    {
        // Merge w/ shared left

        left->cBytes += trackedHeader->cBytes;

        left->right = trackedHeader->right;
        if (left->right)
        {
            left->right->left = left;
        }

        RemoveFromList(&regionHeader->sharedList, (FreeBlockHeader *)left);

        trackedHeader = left;
        mergedIntoShared = true;
    }

    if (mergedIntoShared)
    {
        trackedHeader->state = TrackedState::FreeShared;
        AddToListSorted(&regionHeader->sharedList, (FreeBlockHeader *)trackedHeader);
    }
    else
    {
        trackedHeader->state = TrackedState::FreeUnshared;
        AddToListSorted(&regionHeader->trackedList, (FreeBlockHeader *)trackedHeader);
    }

#if DEBUG_BUILD
    DebugHistory::Entry entry;
    entry.type = DebugHistory::Entry::Type::Free;
    entry.address = allocation;
    AddDebugEntry(&regionHeader->debugHistory, entry);
#endif
}

void*
ReallocateTracked(MemoryRegion region, void* allocation, uintptr cBytesNew)
{
    Assert(region);

    void* result;
    if (!allocation)
    {
        result = AllocateTracked(region, cBytesNew);
    }
    else
    {
        TrackedBlockHeader * trackedHeader = (TrackedBlockHeader *)((u8 *)allocation - sizeof(TrackedBlockHeader));
        Assert(trackedHeader->state == TrackedState::Allocated);

        uintptr cBytesOld = trackedHeader->cBytes - sizeof(TrackedBlockHeader);
        if (cBytesOld >= cBytesNew)
        {
            result = allocation;
        }
        else
        {

            // TODO - Grow tracked regions from low to highto support possible cheap re-allocs by grabbing more
            //  from the shared block

            result = AllocateTracked(region, cBytesNew);

            CopyMemory(allocation, result, cBytesOld);
            FreeTrackedAllocation(region, allocation);
        }
    }

    return result;
}

void
FreeSubRegionAllocation(MemoryRegion patron, void* subregion)
{
    MemoryRegionHeader * patronHeader = patron;
    if (patronHeader)
    {
        bool isInRegion = (subregion >= patronHeader) && (subregion < (u8 *)patronHeader + patronHeader->cBytesBudget);
        if (isInRegion)
        {
            FreeTrackedAllocation(patronHeader, subregion);
        }
        else
        {
            // NOTE - We are probably better off just flagging the overflow region with if it
            //  was allocated by the root region calling out to the system. In that case, we
            //  could skip this recursion and directly call FreeFromSystem. (We'd still want
            //  to recurse in the non-system allocation case only if MEM_ALLOC::trackSubRegions)

            FreeSubRegionAllocation(patronHeader->patron, subregion);
        }
    }
    else
    {
        FreeFromSystem(subregion);
    }

}

void
FreeOverflowAllocations_(MemoryRegionHeader * region)
{
    OverflowHeader * overflow = region->overflow;
    while (overflow)
    {
        OverflowHeader * overflowNext = overflow->next;
        FreeSubRegionAllocation(region->patron, overflow);

        overflow = overflowNext;
    }
}

// @Cleanup - need these because core doesn't run hgen

bool EndMemoryRegion_(MemoryRegion region, bool unlink);

void
FreeChildAllocations_(MemoryRegionHeader * region)
{
    MemoryRegionHeader * child = region->first_child;
    while (child)
    {
        MemoryRegionHeader * childNext = child->next_sibling;
        EndMemoryRegion_(child, false /* unlink */); // Don't bother unlinking, since metadata is being de-allocated too
        child = childNext;
    }
}

bool
EndMemoryRegion_(MemoryRegion region, bool unlink)
{
    MemoryRegionHeader * regionHeader = region;
    FreeOverflowAllocations_(regionHeader);
    FreeChildAllocations_(regionHeader); // NOTE - This recurses back to EndMemoryRegion_

    if (unlink)
    {
        if (regionHeader->parent && (regionHeader->parent->first_child == region))
        {
            regionHeader->parent->first_child = regionHeader->next_sibling;
        }

        if (regionHeader->next_sibling)
        {
            regionHeader->next_sibling->prev_sibling = regionHeader->prev_sibling;
        }

        if (regionHeader->prev_sibling)
        {
            regionHeader->prev_sibling->next_sibling = regionHeader->next_sibling;
        }
    }

    FreeSubRegionAllocation(regionHeader->patron, region);

    // TODO - Maybe add some auditing and return false if it fails? Or just make this return void?
    return true;
}

bool
EndMemoryRegion(MemoryRegion region)
{
    bool unlink = true;
    bool result = EndMemoryRegion_(region, unlink);
    return result;
}

bool
ResetMemoryRegion(MemoryRegion region)
{
    FreeOverflowAllocations_(region);
    FreeChildAllocations_(region);

    MemoryRegionHeader * regionHeader = region;
    regionHeader->first_child = nullptr;
    regionHeader->overflow = nullptr;

    // TODO - Maybe add some auditing and return false if it fails? Or just make this return void?
    return true;
}

MemoryRegion
CreateSubRegion(MemoryRegion parent, MemoryRegion patron, uintptr cBytes)
{
    if (cBytes < MEM_ALLOC::cBytesMinimumRegion)
        return nullptr;

    MemoryRegionHeader * result = nullptr;

    // TODO - we might actually get a bigger region than asked for, but we don't really have a way of knowing...

    result = (MemoryRegionHeader *)AllocateTracked(patron, cBytes);
    *result = {};
    result->cBytesBudget = cBytes;
    result->sharedList = (FreeBlockHeader *)(result + 1);
    *result->sharedList = {};
    result->sharedList->cBytes = result->cBytesBudget - sizeof(MemoryRegionHeader);
    result->sharedList->state = TrackedState::FreeShared;
    result->patron = patron;
    result->parent = parent;
    result->next_sibling = parent->first_child;
    result->prev_sibling = nullptr;
    parent->first_child = result;

    if (result->next_sibling)
    {
        result->next_sibling->prev_sibling = result;
    }

    return result;
}

inline MemoryRegion
CreateSubRegion(MemoryRegion parentAndPatron, uintptr cBytes)
{
    MemoryRegion result = CreateSubRegion(parentAndPatron, parentAndPatron, cBytes);
    return result;
}

// --- Allocator for a single type. Freed values are recycled to service future allocations.

template <typename T>
struct RecycleAllocator
{
    union Slot
    {
        Slot* pNextRecycled;    // Valid if on recycled list. Also guarantees slots are at least 1 pointer big, in case someone tried to recycle-allocate something smaller than a pointer (which would be dumb, but hey)
        T item;                 // Valid if live.
    };

    Slot* recycleList;          // Free list. Using "recycle" nomenclature to match the type and function names.
    MemoryRegion memory;

#if DEBUG_BUILD
    int countLive;               // total # of slots live
    int countAvailableToRecycle; // total # of slots on recycled list
    int countRecycledTotal;      // total # of allocations serviced via recycling
    int countAllocatedTotal;     // total # of allocations serviced
#endif

    RecycleAllocator() = default;
    RecycleAllocator(MemoryRegion memory) { *this = {}; this->memory = memory; }
};

template <typename T>
T*
Allocate(
    RecycleAllocator<T> * alloc,
    CTZ clearToZero=CTZ::NIL)
{
    T* result;

    if (alloc->recycleList)
    {
        // @UB - Might be UB taking the address of one member of the union and then reading the other member...?
        //  But it makes sense, behaves as expected on MSVC, and is type-safe-ish
        result = &alloc->recycleList->item;
        alloc->recycleList = alloc->recycleList->pNextRecycled;

        if ((bool)clearToZero)
        {
            ZeroMemory(result, sizeof(RecycleAllocator<T>::Slot));
        }

#if DEBUG_BUILD
        alloc->countAvailableToRecycle--;
        alloc->countRecycledTotal++;
        Assert(alloc->countAvailableToRecycle >= 0);
#endif
    }
    else
    {
        result = (T*)Allocate(alloc->memory, sizeof(RecycleAllocator<T>::Slot), clearToZero);
    }

#if DEBUG_BUILD
    alloc->countAllocatedTotal++;
    alloc->countLive++;
#endif

    return result;
}

template <typename T>
 void
PreallocateRecycleListContiguous(
    RecycleAllocator<T> * alloc,
    int cntItemPreallocate,
    CTZ clearToZero=CTZ::NIL)
{
    auto * slots = (RecycleAllocator<T>::Slot*)Allocate(alloc->memory,
                                                         cntItemPreallocate * sizeof(RecycleAllocator<T>::Slot),
                                                         clearToZero);

    for (int iSlot = 0; iSlot < cntItemPreallocate; iSlot++)
    {
        auto * slot = slots + iSlot;
        auto * slotNext =
            (iSlot < cntItemPreallocate - 1) ?
            (slot + 1) :
            alloc->recycleList;

        slot->pNextRecycled = slotNext;
    }

    alloc->recycleList = slots;

#if DEBUG_BUILD
    alloc->countAvailableToRecycle += cntItemPreallocate;
#endif
}

template <typename T>
void
Recycle(RecycleAllocator<T> * alloc, T* item)
{
    auto * slot = (RecycleAllocator<T>::Slot*)item;

    slot->pNextRecycled = alloc->recycleList;
    alloc->recycleList = slot;

#if DEBUG_BUILD
    alloc->countAvailableToRecycle++;
    alloc->countLive--;
    Assert(alloc->countLive >= 0);
#endif
}
