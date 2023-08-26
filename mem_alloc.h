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

using Fn_System_allocate = void* (*) (uintptr);
using Fn_System_Reallocate = void* (*) (void*, uintptr);
using Fn_System_Free = void (*) (void*);

Fn_System_allocate system_allocate = {};
Fn_System_Reallocate system_reallocate = {};
Fn_System_Free system_free = {};

enum class Tracked_State : u8
{
    FreeUnshared,
    FreeShared,
    allocated,
};

struct Tracked_Block_Header
{
    uintptr byte_count;             // Includes header
    Tracked_Block_Header* left;     // Physically adjacent in memory
    Tracked_Block_Header* right;    // ...
    Tracked_State state;
};

struct Free_Block_Header : Tracked_Block_Header
{
    Free_Block_Header* next;
    Free_Block_Header* prev;
};

struct Overflow_Header
{
    uintptr byte_count;             // Includes header
    Overflow_Header* next;
};

#define MEM_DEBUG_HISTORY_ENABLE 0

#if MEM_DEBUG_HISTORY_ENABLE
struct Debug_History
{
    struct Entry
    {
        enum Type
        {
            Free,
            allocate
        };

        Type type;
        void* address;
    };

    int iEntryNext;
    Entry entries[64];
};

void
debug_entry_add(Debug_History* history, Debug_History::Entry entry)
{
    history->entries[history->iEntryNext] = entry;
    history->iEntryNext++;
    history->iEntryNext %= ArrayLen(history->entries);
}
#endif

struct Region_Header
{
#if MEM_DEBUG_HISTORY_ENABLE
    Debug_History Debug_History;
#endif

    uintptr byte_countBudget;

    Region_Header* parent;
    Region_Header* first_child;
    Region_Header* next_sibling;
    Region_Header* prev_sibling;
    Overflow_Header* overflow;
    Free_Block_Header* trackedList;
    Free_Block_Header* sharedList;
};

namespace BYTE_COUNT
{
static constexpr uintptr TOO_SMALL_TO_BOTHER_TRACKING = sizeof(Free_Block_Header) + 63;
static constexpr uintptr MINIMUM_REGION = sizeof(Region_Header) + 256;
}

} // namespace MEM


// "Clear To Zero"?
enum class CTZ : u8
{
    NO = 0,
    NIL = 0,

    YES,
};

enum AllocType : u8
{
    Untracked,
    Tracked
};

// @Cleanup - forward declarations because core module doesn't run hgen
function void* allocate(MEM::Region_Header* region, uintptr byte_count, CTZ clearToZero=CTZ::NIL);
function void* allocate_tracked(MEM::Region_Header* region, uintptr byte_count, CTZ clearToZero=CTZ::NIL);
function void free_tracked_allocation(MEM::Region_Header* region, void* allocation);

namespace MEM
{

function void
resize_free_list_head(Free_Block_Header** ppHead, uintptr byte_countNew)
{
    Free_Block_Header* pHeadOrig = *ppHead;

    if (byte_countNew == 0)
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
        Assert(byte_countNew > BYTE_COUNT::TOO_SMALL_TO_BOTHER_TRACKING);
        uintptr byte_countOrig_debug = pHeadOrig->byte_count; // NOTE - only used for an assert

        pHeadOrig->byte_count = byte_countNew;
        bool isHeadTooSmall = pHeadOrig->next && pHeadOrig->next->byte_count > pHeadOrig->byte_count;

        if (isHeadTooSmall)
        {
            Assert(byte_countNew < byte_countOrig_debug);

            // Set the head to the bigger node
            *ppHead = pHeadOrig->next;
            (*ppHead)->prev = nullptr;

            if (byte_countNew > 0)
            {
                Assert(byte_countNew > BYTE_COUNT::TOO_SMALL_TO_BOTHER_TRACKING);

                // Find the node that should point to the old head
                Free_Block_Header* biggerThanOrig = *ppHead;
                while (biggerThanOrig->next &&
                       biggerThanOrig->next->byte_count > pHeadOrig->byte_count)
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

function Free_Block_Header*
ensure_block_with_size(Region_Header* regionHeader, uintptr byte_count, AllocType allocType)
{
    // --- Return tracked block if we've got one (and it's what was asked for)

    if (allocType == AllocType::Tracked &&
        regionHeader->trackedList &&
        regionHeader->trackedList->byte_count >= byte_count)
    {
        return regionHeader->trackedList;
    }

    // --- allocate an overflow region from our parent if shared block is too small

    if (!regionHeader->sharedList ||
        regionHeader->sharedList->byte_count < byte_count)
    {
        // HMM - Maybe make regions have more control over how much they grow when they overflow?

        byte_count += sizeof(Overflow_Header);
        byte_count = max(byte_count, regionHeader->byte_countBudget);

        Overflow_Header* overflow;
        if (regionHeader->parent)
        {
            overflow = (Overflow_Header*)allocate_tracked(regionHeader->parent, byte_count);
        }
        else
        {
            overflow = (Overflow_Header*)MEM::system_allocate(byte_count);
        }

        // --- Maintain overflow list

        overflow->byte_count = byte_count;
        overflow->next = regionHeader->overflow;
        regionHeader->overflow = overflow;

        // --- Initialize free-block and make it the head of the free list

        Free_Block_Header* free = (Free_Block_Header*)(overflow + 1);
        *free = {};
        free->byte_count = byte_count - sizeof(Overflow_Header);

        free->next = regionHeader->sharedList;
        if (free->next)
        {
            free->next->prev = free;
        }

        regionHeader->sharedList = free;
    }

    return regionHeader->sharedList;
}

function void
free_list_remove(Free_Block_Header** ppHead, Free_Block_Header* pItem)
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
free_list_add(Free_Block_Header** ppHead, Free_Block_Header* pItem)
{
    Free_Block_Header** ppNextToFix = ppHead;
    while ((*ppNextToFix) &&
           (*ppNextToFix)->byte_count > pItem->byte_count)
    {
        ppNextToFix = &(*ppNextToFix)->next;
    }

    if (ppNextToFix == ppHead)
    {
        pItem->prev = nullptr;
    }
    else
    {
        pItem->prev = (Free_Block_Header*)((u8*)ppNextToFix - offsetof_(Free_Block_Header, next));
    }

    pItem->next = *ppNextToFix;
    if (pItem->next)
    {
        pItem->next->prev = pItem;
    }

    *ppNextToFix = pItem;
}

function void
free_subregion_allocation(Region_Header* parent, void* subregion)
{
    if (parent)
    {
        bool isInRegion = (subregion >= parent) && (subregion < (u8*)parent + parent->byte_countBudget);
        if (isInRegion)
        {
            free_tracked_allocation(parent, subregion);
        }
        else
        {
            // TODO - this doesn't work, because the region is a tracked allocation which means comes from the
            //  right end of the overflow region. +1 reason to put tracked allocations on the left and untracked on the right.
            //  Then, I think this will work
#if 0
            // Check if the subregion itself was an overflow allocation in the parent.
            // If so, we want to the deallocation to include the overflow header.
            Overflow_Header* overflow = parent->overflow;
            while (overflow)
            {
                // @Slow, maybe we should store this on the Memory_Region header rather than walking this list?
                if (overflow == ((Overflow_Header*)subregion) - 1)
                {
                    FreeSubRegionAllocation(parent->parent, overflow);
                    break;
                }

                overflow = overflow->next;
            }

            if (bool not_found = !overflow)
#endif
            {
                // Recurse
                free_subregion_allocation(parent->parent, subregion);
            }
        }
    }
    else
    {
        system_free(subregion);
    }

}

function void
free_overflow_allocations(Region_Header* region)
{
    Overflow_Header* overflow = region->overflow;
    while (overflow)
    {
        Overflow_Header* overflowNext = overflow->next;
        free_subregion_allocation(region->parent, overflow);

        overflow = overflowNext;
    }
}

function bool mem_region_end(Region_Header* region, bool unlink);

function void
free_child_allocations(Region_Header* region)
{
    Region_Header* child = region->first_child;
    while (child)
    {
        Region_Header* childNext = child->next_sibling;
        mem_region_end(child, false /* unlink */); // Don't bother unlinking, since metadata is being de-allocated too
        child = childNext;
    }
}

function bool
mem_region_end(Region_Header* region, bool unlink)
{
    free_overflow_allocations(region);
    free_child_allocations(region); // NOTE - This recurses back to mem_region_end(..)

    if (unlink)
    {
        if (region->parent && (region->parent->first_child == region))
        {
            region->parent->first_child = region->next_sibling;
        }

        if (region->next_sibling)
        {
            region->next_sibling->prev_sibling = region->prev_sibling;
        }

        if (region->prev_sibling)
        {
            region->prev_sibling->next_sibling = region->next_sibling;
        }
    }

    free_subregion_allocation(region->parent, region);

    // TODO - Maybe add some auditing and return false if it fails? Or just make this return void?
    return true;
}

} // namespace MEM

// "Opaque type"
using Memory_Region = MEM::Region_Header*;

// TODO - remove these non-namespace'd wrappers?
function void*
allocate_from_system(uintptr byte_count)
{
    return MEM::system_allocate(byte_count);
}

function void
free_from_system(void* allocation)
{
    MEM::system_free(allocation);
}

function void*
allocate(Memory_Region region, uintptr byte_count, CTZ clearToZero)
{
    using namespace MEM;

    Assert(region);

    Region_Header* regionHeader = region;

    // --- Make sure our shared block is big enough!

    Free_Block_Header* shared = ensure_block_with_size(regionHeader, byte_count, AllocType::Untracked);
    Assert(shared == regionHeader->sharedList);

    // --- Split shared block into the untracked allocation (left) and ...

    void* result = shared;

    Tracked_Block_Header* rightOfShared = shared->right;

    // HMM - Gracefully handle 0 byte allocation?
    uintptr splitbyte_count = shared->byte_count - byte_count;
    if (splitbyte_count > BYTE_COUNT::TOO_SMALL_TO_BOTHER_TRACKING)
    {
        // ... a smaller shared block (right)

        Free_Block_Header* split = (Free_Block_Header*)((u8*)shared + byte_count);

        // @Slow - Checking these conditions for what should be a simple bump allocate.
        //  This is more reason to grow the shared portion from high to low.
        //  It'd move this cost into allocate_tracked, where we are more willing to
        //  pay allocation overhead costs

        if (shared->prev)
        {
            shared->prev->next = split;
        }

        if (shared->next)
        {
            shared->next->prev = split;
        }

        mem_move(split, shared, sizeof(Free_Block_Header));
        split->left = nullptr;
        if (rightOfShared)
        {
            rightOfShared->left = split;
        }

        regionHeader->sharedList = split;
        resize_free_list_head(&regionHeader->sharedList, splitbyte_count);
    }
    else
    {
        // ... nothing! The remaining block is too small to fit anything of use

        if (rightOfShared)
        {
            rightOfShared->left = nullptr;
        }

        resize_free_list_head(&regionHeader->sharedList, 0); // 0 removes node altogether
    }

    if ((bool)clearToZero)
    {
        mem_zero(result, byte_count);
    }

    return result;
}

template <typename T>
T*
allocate(
    Memory_Region region,
    CTZ clearToZero=CTZ::NIL)
{
    T* result = (T*)allocate(region, sizeof(T), clearToZero);
    return result;
}

template <typename T>
T*
allocate_array(
    Memory_Region region,
    uintptr count,
    CTZ clearToZero=CTZ::NIL)
{
    T* result = (T*)allocate(region, sizeof(T) * count, clearToZero);
    return result;
}

function void*
allocate_tracked(Memory_Region region, uintptr byte_count, CTZ clearToZero)
{
    using namespace MEM;

    Assert(region);

    Region_Header* regionHeader = region;

    // Make sure our tracked or free block is big enough!

    // HMM - Gracefully handle 0 byte allocation?
    uintptr byte_countOrig = byte_count;
    byte_count += sizeof(Tracked_Block_Header);
    byte_count = max(byte_count, sizeof(Free_Block_Header)); // Make sure the allocation is big enough that if it gets freed, we can store a free block header in place

    Free_Block_Header* free = ensure_block_with_size(regionHeader, byte_count, AllocType::Tracked);
    Tracked_Block_Header* rightOfFree = free->right;
    Tracked_Block_Header* leftOfFree = free->left;

    bool isFreeBlockFromTrackedList = (free == regionHeader->trackedList);
    Assert(Implies(!isFreeBlockFromTrackedList, free == regionHeader->sharedList));

    // Split free block into the tracked allocation (right) and ...

    // If we would've split off something
    //  that is smaller than the metadata we need to do tracking, just lump it in with the
    //  memory we give back to the user. Otherwise it'd fragment our left/right merging.

    uintptr splitbyte_count = free->byte_count - byte_count;
    if (splitbyte_count <= BYTE_COUNT::TOO_SMALL_TO_BOTHER_TRACKING)
    {
        splitbyte_count = 0;
        byte_count = free->byte_count; // HMM - Maybe we should have a way to tell the user that they got a little extra memory?
    }

    Tracked_Block_Header* resultHeader = (Tracked_Block_Header*)((u8*)free + splitbyte_count);
    resultHeader->byte_count = byte_count;
    resultHeader->right = free->right;
    resultHeader->state = Tracked_State::allocated;

    if (resultHeader->right)
    {
        resultHeader->right->left = resultHeader;
    }

    void* result = (u8*)(resultHeader + 1);

    if (splitbyte_count > BYTE_COUNT::TOO_SMALL_TO_BOTHER_TRACKING)
    {
        // ... a smaller shared block (left)

        Free_Block_Header* split = free;
        split->right = resultHeader;
        resultHeader->left = split;

        if (isFreeBlockFromTrackedList)
        {
            resize_free_list_head(&regionHeader->trackedList, splitbyte_count);
        }
        else
        {
            resize_free_list_head(&regionHeader->sharedList, splitbyte_count);
        }
    }
    else
    {
        Assert(splitbyte_count == 0);

        // ... nothing! The remaining block is too small to fit anything of use.

        if (isFreeBlockFromTrackedList)
        {
            resize_free_list_head(&regionHeader->trackedList, 0); // 0 removes node altogether
        }
        else
        {
            resize_free_list_head(&regionHeader->sharedList, 0); // 0 removes node altogether
        }
    }

    // @Slow - Should be a separate call or compile-time template param?
    if (clearToZero != CTZ::NIL)
    {
        mem_zero(result, byte_countOrig);
    }

#if MEM_DEBUG_HISTORY_ENABLE
    Debug_History::Entry entry;
    entry.type = Debug_History::Entry::Type::allocate;
    entry.address = result;
    AddDebugEntry(&regionHeader->Debug_History, entry);
#endif

    return result;
}

template <typename T>
T*
allocate_tracked(
    Memory_Region region,
    CTZ clearToZero=CTZ::NIL)
{
    T* result = (T*)allocate_tracked(region, sizeof(T), clearToZero);
    return result;
}

template <typename T>
T*
allocate_array_tracked(
    Memory_Region region,
    uintptr count,
    CTZ clearToZero=CTZ::NIL)
{
    T* result = (T*)allocate_tracked(region, sizeof(T) * count, clearToZero);
    return result;
}

function void
free_tracked_allocation(Memory_Region region, void* allocation)
{
    using namespace MEM;

    Assert(region);

    Region_Header* regionHeader = region;
    Tracked_Block_Header* trackedHeader = (Tracked_Block_Header*)((u8*)allocation - sizeof(Tracked_Block_Header));
    Assert(trackedHeader->state == Tracked_State::allocated);

    Tracked_Block_Header* left = trackedHeader->left;
    Tracked_Block_Header* right = trackedHeader->right;
    if (right && right->state == Tracked_State::FreeUnshared)
    {
        // Merge w/ free right

        trackedHeader->byte_count += right->byte_count;

        trackedHeader->right = right->right;
        if (trackedHeader->right)
        {
            trackedHeader->right->left = trackedHeader;
        }

        free_list_remove(&regionHeader->trackedList, (Free_Block_Header*)right);
    }

    bool mergedIntoShared = false;

    if (left && left->state == Tracked_State::FreeUnshared)
    {
        // Merge w/ free left

        left->byte_count += trackedHeader->byte_count;

        left->right = trackedHeader->right;
        if (left->right)
        {
            left->right->left = left;
        }

        free_list_remove(&regionHeader->trackedList, (Free_Block_Header*)left);

        trackedHeader = left;
    }
    else if (left && left->state == Tracked_State::FreeShared)
    {
        // Merge w/ shared left

        left->byte_count += trackedHeader->byte_count;

        left->right = trackedHeader->right;
        if (left->right)
        {
            left->right->left = left;
        }

        free_list_remove(&regionHeader->sharedList, (Free_Block_Header*)left);

        trackedHeader = left;
        mergedIntoShared = true;
    }

    if (mergedIntoShared)
    {
        trackedHeader->state = Tracked_State::FreeShared;
        free_list_add(&regionHeader->sharedList, (Free_Block_Header*)trackedHeader);
    }
    else
    {
        trackedHeader->state = Tracked_State::FreeUnshared;
        free_list_add(&regionHeader->trackedList, (Free_Block_Header*)trackedHeader);
    }

#if MEM_DEBUG_HISTORY_ENABLE
    Debug_History::Entry entry;
    entry.type = Debug_History::Entry::Type::Free;
    entry.address = allocation;
    AddDebugEntry(&regionHeader->Debug_History, entry);
#endif
}

function void*
reallocate_tracked(Memory_Region region, void* allocation, uintptr byte_countNew)
{
    using namespace MEM;

    Assert(region);

    void* result;
    if (!allocation)
    {
        result = allocate_tracked(region, byte_countNew);
    }
    else
    {
        Tracked_Block_Header* trackedHeader = (Tracked_Block_Header*)((u8*)allocation - sizeof(Tracked_Block_Header));
        Assert(trackedHeader->state == Tracked_State::allocated);

        uintptr byte_countOld = trackedHeader->byte_count - sizeof(Tracked_Block_Header);
        if (byte_countOld >= byte_countNew)
        {
            result = allocation;
        }
        else
        {

            // TODO - Grow tracked regions from low to highto support possible cheap re-allocs by grabbing more
            //  from the shared block

            result = allocate_tracked(region, byte_countNew);

            mem_copy(result, allocation, byte_countOld);
            free_tracked_allocation(region, allocation);
        }
    }

    return result;
}

function bool
mem_region_end(Memory_Region region)
{
    bool unlink = true;
    bool result = MEM::mem_region_end(region, unlink);
    return result;
}

function bool
mem_region_reset(Memory_Region region)
{
    using namespace MEM;

    free_overflow_allocations(region);
    free_child_allocations(region);

    Region_Header* header = region;
    header->first_child = nullptr;
    header->overflow = nullptr;

    // TODO - Maybe add some auditing and return false if it fails? Or just make this return void?
    return true;
}

// Initialize a root memory initially backed by the provided memory.
// If a an allocation would "overflow" a memory region, it will use MEM::system_allocate(..) to get more.
function Memory_Region
mem_region_root_begin(u8* bytes, uintptr byte_count)
{
    using namespace MEM;

    byte_count = max(byte_count, BYTE_COUNT::MINIMUM_REGION);

    Region_Header* header = (Region_Header*)bytes;
    *header = {};
    header->byte_countBudget = byte_count;
    header->sharedList = (Free_Block_Header*)(header + 1);
    *header->sharedList = {};
    header->sharedList->byte_count = header->byte_countBudget - sizeof(Region_Header);
    header->sharedList->state = Tracked_State::FreeShared;
    return header;
}

// Initialize a sub-region within the provided parent region.
// If parent is nullptr, initializes a root memory region by calling MEM::system_allocate(..)
function Memory_Region
mem_region_begin(Memory_Region parent, uintptr byte_count)
{
    using namespace MEM;

    byte_count = max(byte_count, BYTE_COUNT::MINIMUM_REGION);

    Region_Header* result = nullptr;
    if (parent)
    {
        result = (Region_Header*)allocate_tracked(parent, byte_count);
    }
    else
    {
        result = (Region_Header*)MEM::system_allocate(byte_count);
    }

    *result = {};
    result->byte_countBudget = byte_count;
    result->sharedList = (Free_Block_Header*)(result + 1);
    *result->sharedList = {};
    result->sharedList->byte_count = result->byte_countBudget - sizeof(Region_Header);
    result->sharedList->state = Tracked_State::FreeShared;
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
struct Recycle_Allocator
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
    int countallocatedTotal;     // total # of allocations serviced
#endif

    Recycle_Allocator() = default;
    Recycle_Allocator(Memory_Region memory) { *this = {}; this->memory = memory; }
};

template <typename T>
T*
allocate(
    Recycle_Allocator<T> * alloc,
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
            mem_zero(result, sizeof(Recycle_Allocator<T>::Slot));
        }

#if BUILD_DEBUG
        alloc->countAvailableToRecycle--;
        alloc->countRecycledTotal++;
        Assert(alloc->countAvailableToRecycle >= 0);
#endif
    }
    else
    {
        result = (T*)allocate(alloc->memory, sizeof(Recycle_Allocator<T>::Slot), clearToZero);
    }

#if BUILD_DEBUG
    alloc->countallocatedTotal++;
    alloc->countLive++;
#endif

    return result;
}

template <typename T>
void
PreallocateRecycleListContiguous(
    Recycle_Allocator<T> * alloc,
    int cntItemPreallocate,
    CTZ clearToZero=CTZ::NIL)
{
    auto * slots = (Recycle_Allocator<T>::Slot*)allocate(alloc->memory,
                                                         cntItemPreallocate * sizeof(Recycle_Allocator<T>::Slot),
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
recycle(Recycle_Allocator<T>* alloc, T* item)
{
    auto * slot = (Recycle_Allocator<T>::Slot*)item;

    slot->pNextRecycled = alloc->recycleList;
    alloc->recycleList = slot;

#if BUILD_DEBUG
    alloc->countAvailableToRecycle++;
    alloc->countLive--;
    Assert(alloc->countLive >= 0);
#endif
}

#undef MEM_DEBUG_HISTORY_ENABLE
