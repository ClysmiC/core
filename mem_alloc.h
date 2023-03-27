// IDEAS:
// - Make right side the bump side and left side the tracked side. This would make tracked blocks able to potentially re-alloc in place
// - Make headers smaller by having a u32 offset for left/right instead of pointers. This limits allocations to 8gb per (mul u32 by 4 and enforce 4-byte alignment)
//
// TODO:
// - Alignment
// - Audit for degenerate cases where things like overflow allocations are somehow too small to store an overflow + free header
// - Clear to Zero on allocations should be a separate call or compile-time template param?

namespace MEM
{
    using Fn_System_Allocate = void* (*) (uintptr);
    using Fn_System_Reallocate = void* (*) (void*, uintptr);
    using Fn_System_Free = void (*) (void*);

    Fn_System_Allocate system_allocate = {};
    Fn_System_Reallocate system_reallocate = {};
    Fn_System_Free system_free = {};
};

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

#define MEM_DEBUG_HISTORY_ENABLE 0

#if MEM_DEBUG_HISTORY_ENABLE
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

struct Memory_Region_Header
{
#if MEM_DEBUG_HISTORY_ENABLE
    DebugHistory debugHistory;
#endif

    uintptr cBytesBudget;

    Memory_Region_Header * parent;
    Memory_Region_Header * first_child;
    Memory_Region_Header * next_sibling;
    Memory_Region_Header * prev_sibling;
    OverflowHeader * overflow;
    FreeBlockHeader * trackedList;
    FreeBlockHeader * sharedList;
};

namespace MEM_ALLOC // @Cleanup - need to put this after struct definitions because of sizeof dependency...
{
static constexpr uintptr cBytesTooSmallToBotherTracking = sizeof(FreeBlockHeader) + 63;
static constexpr uintptr cBytesMinimumRegion = sizeof(Memory_Region_Header) + 256;
}

// "Opaque type" (but not really, due to unity build) provided by API
using Memory_Region = Memory_Region_Header*;

// "Clear To Zero"?
enum class CTZ : u8
{
    NO = 0,
    NIL = 0,

    YES,
};

// @Cleanup - need these because core doesn't run hgen
function void* Allocate(Memory_Region region, uintptr cBytes, CTZ clearToZero=CTZ::NIL);
function void* AllocateTracked(Memory_Region region, uintptr cBytes, CTZ clearToZero=CTZ::NIL);
function bool mem_region_end_(Memory_Region region, bool unlink);

enum AllocType : u8
{
    Untracked,
    Tracked
};

function void
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

function void*
AllocateFromSystem(uintptr cBytes)
{
    return MEM::system_allocate(cBytes);
}

function void
FreeFromSystem(void* allocation)
{
    MEM::system_free(allocation);
}

function FreeBlockHeader *
EnsureBlockWithSize(Memory_Region_Header * regionHeader, uintptr cBytes, AllocType allocType)
{
    // NOTE - Caller is responsible for adding bytes for TrackedHeader if making a tracked allocation

    // Return tracked block if we've got one (and it's what was asked for)

    if (allocType == AllocType::Tracked &&
        regionHeader->trackedList &&
        regionHeader->trackedList->cBytes >= cBytes)
    {
        return regionHeader->trackedList;
    }

    // Allocate an overflow region from our parent if shared block is too small

    if (!regionHeader->sharedList ||
        regionHeader->sharedList->cBytes < cBytes)
    {
        // HMM - Maybe make regions have more control over how much they grow when they overflow?

        cBytes += sizeof(OverflowHeader);
        cBytes = max(cBytes, regionHeader->cBytesBudget);

        OverflowHeader * overflowHeader;
        if (regionHeader->parent)
        {
            overflowHeader = (OverflowHeader *)AllocateTracked(regionHeader->parent, cBytes);
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

function void*
Allocate(Memory_Region region, uintptr cBytes, CTZ clearToZero)
{
    // HMM - Gracefully handle 0 byte allocation?

    Assert(region);

    void* result;

    Memory_Region_Header * regionHeader = region;

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

        mem_move(split, shared, sizeof(FreeBlockHeader));
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
        mem_zero(result, cBytes);
    }

    return result;
}

template <typename T>
T*
Allocate(
    Memory_Region region,
    CTZ clearToZero=CTZ::NIL)
{
    T* result = (T*)Allocate(region, sizeof(T), clearToZero);
    return result;
}

template <typename T>
T*
AllocateArray(
    Memory_Region region,
    uintptr count,
    CTZ clearToZero=CTZ::NIL)
{
    T* result = (T*)Allocate(region, sizeof(T) * count, clearToZero);
    return result;
}

function void*
AllocateTracked(Memory_Region region, uintptr cBytes, CTZ clearToZero)
{
    // HMM - Gracefully handle 0 byte allocation?

    Assert(region);

    void* result;

    Memory_Region_Header * regionHeader = region;

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
        mem_zero(result, cBytesOrig);
    }

#if MEM_DEBUG_HISTORY_ENABLE
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
    Memory_Region region,
    CTZ clearToZero=CTZ::NIL)
{
    T* result = (T*)AllocateTracked(region, sizeof(T), clearToZero);
    return result;
}

function void
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

function void
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
        pItem->prev = (FreeBlockHeader *)((u8 *)ppNextToFix - offsetof_(FreeBlockHeader, next));
    }

    pItem->next = *ppNextToFix;
    if (pItem->next)
    {
        pItem->next->prev = pItem;
    }

    *ppNextToFix = pItem;
}

function void
FreeTrackedAllocation(Memory_Region region, void* allocation)
{
    Assert(region);

    Memory_Region_Header * regionHeader = region;
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

#if MEM_DEBUG_HISTORY_ENABLE
    DebugHistory::Entry entry;
    entry.type = DebugHistory::Entry::Type::Free;
    entry.address = allocation;
    AddDebugEntry(&regionHeader->debugHistory, entry);
#endif
}

function void*
ReallocateTracked(Memory_Region region, void* allocation, uintptr cBytesNew)
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

            mem_copy(result, allocation, cBytesOld);
            FreeTrackedAllocation(region, allocation);
        }
    }

    return result;
}

function void
FreeSubRegionAllocation(Memory_Region parent, void* subregion)
{
    Memory_Region_Header * parentHeader = parent;
    if (parentHeader)
    {
        bool isInRegion = (subregion >= parentHeader) && (subregion < (u8 *)parentHeader + parentHeader->cBytesBudget);
        if (isInRegion)
        {
            FreeTrackedAllocation(parentHeader, subregion);
        }
        else
        {
            // TODO - this doesn't work, because the region is a tracked allocation which means comes from the
            //  right end of the overflow region. +1 reason to put tracked allocations on the left and untracked on the right.
            //  Then, I think this will work
#if 0
            // Check if the subregion itself was an overflow allocation in the parent.
            // If so, we want to the deallocation to include the overflow header.
            OverflowHeader * overflow = parentHeader->overflow;
            while (overflow)
            {
                // @Slow, maybe we should store this on the Memory_Region header rather than walking this list?
                if (overflow == ((OverflowHeader*)subregion) - 1)
                {
                    FreeSubRegionAllocation(parentHeader->parent, overflow);
                    break;
                }

                overflow = overflow->next;
            }

            if (bool not_found = !overflow)
#endif
            {
                FreeSubRegionAllocation(parentHeader->parent, subregion);
            }
        }
    }
    else
    {
        FreeFromSystem(subregion);
    }

}

function void
FreeOverflowAllocations_(Memory_Region_Header * region)
{
    OverflowHeader * overflow = region->overflow;
    while (overflow)
    {
        OverflowHeader * overflowNext = overflow->next;
        FreeSubRegionAllocation(region->parent, overflow);

        overflow = overflowNext;
    }
}

function void
FreeChildAllocations_(Memory_Region_Header * region)
{
    Memory_Region_Header * child = region->first_child;
    while (child)
    {
        Memory_Region_Header * childNext = child->next_sibling;
        mem_region_end_(child, false /* unlink */); // Don't bother unlinking, since metadata is being de-allocated too
        child = childNext;
    }
}

function bool
mem_region_end_(Memory_Region region, bool unlink)
{
    Memory_Region_Header * regionHeader = region;
    FreeOverflowAllocations_(regionHeader);
    FreeChildAllocations_(regionHeader); // NOTE - This recurses back to mem_region_end_

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

    FreeSubRegionAllocation(regionHeader->parent, region);

    // TODO - Maybe add some auditing and return false if it fails? Or just make this return void?
    return true;
}

function bool
mem_region_end(Memory_Region region)
{
    bool unlink = true;
    bool result = mem_region_end_(region, unlink);
    return result;
}

function bool
mem_region_reset(Memory_Region region)
{
    FreeOverflowAllocations_(region);
    FreeChildAllocations_(region);

    Memory_Region_Header * regionHeader = region;
    regionHeader->first_child = nullptr;
    regionHeader->overflow = nullptr;

    // TODO - Maybe add some auditing and return false if it fails? Or just make this return void?
    return true;
}

// Initialize a root memory initially backed by the provided memory.
// If a an allocation would "overflow" a memory region, it will use MEM::system_allocate(..) to get more.
function Memory_Region
mem_region_root_begin(u8 * bytes, uintptr cBytes)
{
    cBytes = max(cBytes, MEM_ALLOC::cBytesMinimumRegion);

    Memory_Region_Header * header = (Memory_Region_Header *)bytes;
    *header = {};
    header->cBytesBudget = cBytes;
    header->sharedList = (FreeBlockHeader *)(header + 1);
    *header->sharedList = {};
    header->sharedList->cBytes = header->cBytesBudget - sizeof(Memory_Region_Header);
    header->sharedList->state = TrackedState::FreeShared;
    return header;
}

// Initialize a sub-region within the provided parent region.
// If parent is nullptr, initializes a root memory region by calling MEM::system_allocate(..)
function Memory_Region
mem_region_begin(Memory_Region parent, uintptr cBytes)
{
    cBytes = max(cBytes, MEM_ALLOC::cBytesMinimumRegion);

    Memory_Region_Header * result = nullptr;
    if (parent)
    {
        result = (Memory_Region_Header *)AllocateTracked(parent, cBytes);
    }
    else
    {
        result = (Memory_Region_Header *)MEM::system_allocate(cBytes);
    }

    *result = {};
    result->cBytesBudget = cBytes;
    result->sharedList = (FreeBlockHeader *)(result + 1);
    *result->sharedList = {};
    result->sharedList->cBytes = result->cBytesBudget - sizeof(Memory_Region_Header);
    result->sharedList->state = TrackedState::FreeShared;
    result->parent = parent;
    result->next_sibling = parent->first_child;
    result->prev_sibling = nullptr;

    if (parent)
    {
        parent->first_child = result;
    }

    if (result->next_sibling)
    {
        result->next_sibling->prev_sibling = result;
    }

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
    Memory_Region memory;

#if BUILD_DEBUG
    int countLive;               // total # of slots live
    int countAvailableToRecycle; // total # of slots on recycled list
    int countRecycledTotal;      // total # of allocations serviced via recycling
    int countAllocatedTotal;     // total # of allocations serviced
#endif

    RecycleAllocator() = default;
    RecycleAllocator(Memory_Region memory) { *this = {}; this->memory = memory; }
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
            mem_zero(result, sizeof(RecycleAllocator<T>::Slot));
        }

#if BUILD_DEBUG
        alloc->countAvailableToRecycle--;
        alloc->countRecycledTotal++;
        Assert(alloc->countAvailableToRecycle >= 0);
#endif
    }
    else
    {
        result = (T*)Allocate(alloc->memory, sizeof(RecycleAllocator<T>::Slot), clearToZero);
    }

#if BUILD_DEBUG
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

#if BUILD_DEBUG
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

#if BUILD_DEBUG
    alloc->countAvailableToRecycle++;
    alloc->countLive--;
    Assert(alloc->countLive >= 0);
#endif
}

#undef MEM_DEBUG_HISTORY_ENABLE
