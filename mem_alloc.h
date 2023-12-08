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
    FREE_UNSHARED,
    FREE_SHARED,
    ALLOCATED,
};

struct Tracked_Block_Header
{
    uintptr byte_count;             // Includes header
    Tracked_Block_Header* left;     // Physically adjacent in memory, in same memory arena
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
    // Region_Header* patron;          // Which region ultimately allocated us?
};

#define MEM_DEBUG_NAMES_ENABLE 1

struct Region_Header
{
#if MEM_DEBUG_NAMES_ENABLE
    char name[8];
    u32 id;
#endif

    uintptr byte_budget;

    Region_Header* parent;
    Region_Header* first_child;
    Region_Header* next_sibling;
    Region_Header* prev_sibling;
    Overflow_Header* overflow;
    Free_Block_Header* tracked_list;
    Free_Block_Header* shared_list;
};

namespace BYTE_COUNT
{
static constexpr uintptr TOO_SMALL_TO_BOTHER_TRACKING = sizeof(Free_Block_Header) + 63;
static constexpr uintptr MINIMUM_REGION = sizeof(Region_Header) + 256;
}

function u32
debug_id_from_name(char const * name)
{
#if MEM_DEBUG_NAMES_ENABLE
    if (!name)
        return 0;

    char const * c = name;
    u32 constexpr magic = 2147463569;   // big prime number to "distribute" characters to 0-2^32 before x-oring for debug id
    u32 result = 0;
    while (*c)
    {
        result ^= (u32)(*c * magic);
        c++;
    }

    return result;
#else
    return 0;
#endif
}

function void
debug_region_name_set(MEM::Region_Header * region, char const * name)
{
#if MEM_DEBUG_NAMES_ENABLE
    if (name)
    {
        char const * c = name;
        int i = 0;
        while (*c && i < ArrayLen(region->name))
        {
            region->name[i] = *c;
            c++;
            i++;
        }
        region->name[ArrayLen(region->name) - 1] = '\0';
        region->id = debug_id_from_name(name);
    }
    else
    {
        ZeroArray(region->name);
        region->id = 0;
    }
#endif
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
        uintptr byte_count_orig_debug = pHeadOrig->byte_count; // NOTE - only used for an assert

        pHeadOrig->byte_count = byte_countNew;
        bool isHeadTooSmall = pHeadOrig->next && pHeadOrig->next->byte_count > pHeadOrig->byte_count;

        if (isHeadTooSmall)
        {
            Assert(byte_countNew < byte_count_orig_debug);

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
ensure_block_with_size(Region_Header* region_header, uintptr byte_count, AllocType allocType)
{
    // --- Return tracked block if we've got one big enough (and we asked for tracked)

    if (allocType == AllocType::Tracked &&
        region_header->tracked_list &&
        region_header->tracked_list->byte_count >= byte_count)
    {
        return region_header->tracked_list;
    }

    // --- Allocate an overflow region from our parent if shared block is too small

    if (!region_header->shared_list ||
        region_header->shared_list->byte_count < byte_count)
    {
        // HMM - Maybe make regions have more control over how much they grow when they overflow?
        byte_count += sizeof(Overflow_Header);
        byte_count = max(byte_count, region_header->byte_budget);

        Overflow_Header* overflow = (Overflow_Header*)allocate_tracked(region_header->parent, byte_count);

        // --- Maintain overflow list

        overflow->byte_count = byte_count;
        overflow->next = region_header->overflow;
        region_header->overflow = overflow;

        // --- Initialize free-block and make it the head of the free list

        Free_Block_Header* free = (Free_Block_Header*)(overflow + 1);
        *free = {};
        free->byte_count = byte_count - sizeof(Overflow_Header);

        free->next = region_header->shared_list;
        if (free->next)
        {
            free->next->prev = free;
        }

        region_header->shared_list = free;
    }

    // --- Give the shared block

    return region_header->shared_list;
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

function void*
allocate_tracked_unchecked(Region_Header* region, uintptr byte_count)
{
    Assert(region);

    Region_Header* region_header = region;

    // Make sure our tracked or free block is big enough!

    // HMM - Gracefully handle 0 byte allocation?
    byte_count += sizeof(Tracked_Block_Header);
    byte_count = max(byte_count, sizeof(Free_Block_Header)); // Make sure the allocation is big enough that if it gets freed, we can store a free block header in place

    Free_Block_Header* free = ensure_block_with_size(region_header, byte_count, AllocType::Tracked);

    bool is_free_block_from_tracked_list = (free == region_header->tracked_list);
    Assert(Implies(!is_free_block_from_tracked_list, free == region_header->shared_list));

    // Split free block into the tracked allocation (left) and ...

    // If we would've split off something that is smaller than the metadata we need to do tracking,
    //  just lump it in with the memory we give back to the user. Otherwise it'd fragment our left/right merging.

    Tracked_Block_Header* result_header = (Tracked_Block_Header*)free;
    void* result = (u8*)((Tracked_Block_Header*)free + 1);

    uintptr split_byte_count = free->byte_count - byte_count;
    if (split_byte_count <= BYTE_COUNT::TOO_SMALL_TO_BOTHER_TRACKING)
    {
        // ... nothing! The remaining block is too small to fit anything of use.
        byte_count = free->byte_count;

        if (is_free_block_from_tracked_list)
        {
            resize_free_list_head(&region_header->tracked_list, 0);
        }
        else
        {
            resize_free_list_head(&region_header->shared_list, 0);
        }

    }
    else
    {
        // ... a smaller shared block (right)


        Free_Block_Header* split = (Free_Block_Header*)((u8*)free + byte_count);
        mem_move(split, free, sizeof(Free_Block_Header));

        result_header->right = split;
        split->left = result_header;

        if (is_free_block_from_tracked_list)
        {
            region_header->tracked_list = split;
            resize_free_list_head(&region_header->tracked_list, split_byte_count);
        }
        else
        {
            region_header->shared_list = split;
            resize_free_list_head(&region_header->shared_list, split_byte_count);
        }
    }

    result_header->byte_count = byte_count;
    result_header->state = Tracked_State::ALLOCATED;

    if (result_header->left)
    {
        Assert(result_header->left->right == result_header);  // @Temp if this triggers, then free/shared block isn't counted in left/right?
        // result_header->left->right = result_header;
    }

    return result;
}

function void
free_tracked_allocation_unchecked(Region_Header* region, void* allocation)
{
    if (region->id == 2150138239)
    {
        bool brk = true;
    }

    Assert(region);

    Region_Header* region_header = region;
    Tracked_Block_Header* tracked_header = (Tracked_Block_Header*)((u8*)allocation - sizeof(Tracked_Block_Header));
    Assert(tracked_header->state == Tracked_State::ALLOCATED);

    Tracked_Block_Header* left = tracked_header->left;
    Tracked_Block_Header* right = tracked_header->right;
    if (left && left->state == Tracked_State::FREE_UNSHARED)
    {
        // --- Merge w/ free left (unshared)

        left->byte_count += tracked_header->byte_count;
        left->right = tracked_header->right;

        if (left->right)
        {
            left->right->left = left;
        }

        // Remove for now, and...
        free_list_remove(&region_header->tracked_list, (Free_Block_Header*)left);
        // ... consider the merged result the new "tracked header" that we will add back in.
        tracked_header = left;
    }

    bool merged_with_shared = false;

    if (right)
    {
        if (right->state == Tracked_State::FREE_UNSHARED)
        {
            // --- Merge w/ free right (unshared)

            tracked_header->byte_count += right->byte_count;

            tracked_header->right = right->right;
            if (tracked_header->right)
            {
                tracked_header->right->left = tracked_header;
            }

            // Remove for now. We'll add the merged result back in.
            free_list_remove(&region_header->tracked_list, (Free_Block_Header*)right);
        }
        else if (right->state == Tracked_State::FREE_SHARED)
        {
            merged_with_shared = true;

            // --- Merge w/ free right (shared)

            tracked_header->byte_count += right->byte_count;

            Assert(right->right == nullptr);     // right of shared is never tracked, by definition
            tracked_header->right = nullptr;

            // Remove for now. We'll add the merged result back in.
            free_list_remove(&region_header->shared_list, (Free_Block_Header*)right);
        }
    }

    if (merged_with_shared)
    {
        tracked_header->state = Tracked_State::FREE_SHARED;
        free_list_add(&region_header->shared_list, (Free_Block_Header*)tracked_header);
    }
    else
    {
        tracked_header->state = Tracked_State::FREE_UNSHARED;
        free_list_add(&region_header->tracked_list, (Free_Block_Header*)tracked_header);
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
        mem_region_end(child, false /* unlink */); // Don't bother unlinking, since we are clobbering this list anyways
        child = childNext;
    }

    region->first_child = nullptr;
}

function bool
mem_region_end(Region_Header* region, bool unlink)
{
    // Clean up children before freeing overflow regions,
    //  since a child could live in an overflow region.
    free_child_allocations(region); // NOTE - this can recurse back to mem_region_end(..)
    
    // --- Free overflow allocations
    {
        Overflow_Header* overflow = region->overflow;
        while (overflow)
        {
            Overflow_Header* overflowNext = overflow->next;
            free_tracked_allocation(region->parent, overflow);
            overflow = overflowNext;
        }

        region->overflow = nullptr;
    }

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

    free_tracked_allocation(region->parent, region);

    // TODO - Maybe add some auditing and return false if it fails? Or just make this return void?
    return true;
}

} // namespace MEM

// "Opaque type"
using Memory_Region = MEM::Region_Header*;

function void*
allocate(Memory_Region region, uintptr byte_count, CTZ clearToZero)
{
    using namespace MEM;

    Assert(region);

    Region_Header* region_header = region;

    // --- Make sure our shared block is big enough!

    Free_Block_Header* shared = ensure_block_with_size(region_header, byte_count, AllocType::Untracked);
    Assert(shared == region_header->shared_list);
    Assert(shared->byte_count >= byte_count);

    // --- Split shared block into the untracked allocation (right) and ...

    void* result;

    // HMM - Gracefully handle 0 byte allocation?
    uintptr split_byte_count = shared->byte_count - byte_count;
    if (split_byte_count <= BYTE_COUNT::TOO_SMALL_TO_BOTHER_TRACKING)
    {
        // ... nothing! The remaining block is too small to fit anything of use

        if (shared->left)
        {
            shared->left->right = nullptr;
        }

        resize_free_list_head(&region_header->shared_list, 0);

        result = shared;
    }
    else
    {
        // ... a smaller shared block (left)

        resize_free_list_head(&region_header->shared_list, split_byte_count);
        result = (u8*)shared + split_byte_count;
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
    void* result;
    if (region)
    {
        result = MEM::allocate_tracked_unchecked(region, byte_count);
    }
    else
    {
        result = MEM::system_allocate(byte_count);
    }

    if ((bool)clearToZero)
    {
        mem_zero(result, byte_count);
    }

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
    if (region)
    {
#if 1
        MEM::free_tracked_allocation_unchecked(region, allocation);
#else
        bool isInRegion = (subregion >= parent) && (subregion < (u8*)parent + parent->byte_budget);
        if (isInRegion)
        {
            free_tracked_allocation(parent, subregion);
        }
        else
        {
            // TODO - this doesn't work, because the region is a tracked allocation which means comes from the
            //  right end of the overflow region. +1 reason to put tracked allocations on the left and untracked on the right.
            //  Then, I think this will work
            // Check if the subregion itself was an overflow allocation in the parent.
            // If so, we want to the deallocation to include the overflow header.
            Overflow_Header* overflow = parent->overflow;
            while (overflow)
            {
                // @Slow, maybe we should store this on the Memory_Region header rather than walking this list?
                if (overflow == ((Overflow_Header*)subregion) - 1)
                {
                    free_subregion_allocation(parent->parent, overflow);
                    break;
                }

                overflow = overflow->next;
            }

            bool not_found = !overflow;
            if (not_found)
            {
                // Recurse
                free_subregion_allocation(parent->parent, subregion);
            }
        }
#endif
    }
    else
    {
        MEM::system_free(allocation);
    }
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
        Tracked_Block_Header* tracked_header = (Tracked_Block_Header*)((u8*)allocation - sizeof(Tracked_Block_Header));
        Assert(tracked_header->state == Tracked_State::ALLOCATED);

        uintptr byte_countOld = tracked_header->byte_count - sizeof(Tracked_Block_Header);
        if (byte_countOld >= byte_countNew)
        {
            result = allocation;
        }
        else
        {

            // TODO - Grow in place if possible

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

    Region_Header* header = region;

    if (header->id == debug_id_from_name("cnsl-L"))
    {
        bool brk = true;
    }

    // Clean up children before freeing overflow regions,
    //  since a child could live in an overflow region.
    free_child_allocations(region);
    
    // --- Free overflow allocations
    {
        Overflow_Header* overflow = region->overflow;
        while (overflow)
        {
            Overflow_Header* overflowNext = overflow->next;
            free_tracked_allocation(region->parent, overflow);
            overflow = overflowNext;
        }

        region->overflow = nullptr;
    }

    region->tracked_list = nullptr;
    region->shared_list = (Free_Block_Header*)(header + 1);
    *region->shared_list = {};
    region->shared_list->byte_count = header->byte_budget - sizeof(Region_Header);
    region->shared_list->state = Tracked_State::FREE_SHARED;

    // TODO - Maybe add some auditing and return false if it fails? Or just make this return void?
    return true;
}

// Initialize a root memory initially backed by the provided memory.
// If a an allocation would "overflow" a memory region, it will use MEM::system_allocate(..) to get more.
function Memory_Region
mem_region_root_begin(u8* bytes, uintptr byte_count, char const* debug_name = nullptr)
{
    using namespace MEM;

    byte_count = max(byte_count, BYTE_COUNT::MINIMUM_REGION);

    Region_Header* header = (Region_Header*)bytes;
    *header = {};
    header->byte_budget = byte_count;
    header->shared_list = (Free_Block_Header*)(header + 1);
    *header->shared_list = {};
    header->shared_list->byte_count = header->byte_budget - sizeof(Region_Header);
    header->shared_list->state = Tracked_State::FREE_SHARED;
    debug_region_name_set(header, debug_name);
    return header;
}

// Initialize a sub-region within the provided parent region.
// If parent is nullptr, initializes a root memory region by calling MEM::system_allocate(..)
function Memory_Region
mem_region_begin(Memory_Region parent, uintptr byte_count, char const* debug_name = nullptr)
{
    using namespace MEM;

    byte_count = max(byte_count, BYTE_COUNT::MINIMUM_REGION);

    Region_Header* result = nullptr;
    result = (Region_Header*)allocate_tracked(parent, byte_count);

    *result = {};
    result->byte_budget = byte_count;
    result->shared_list = (Free_Block_Header*)(result + 1);
    *result->shared_list = {};
    result->shared_list->byte_count = result->byte_budget - sizeof(Region_Header);
    result->shared_list->state = Tracked_State::FREE_SHARED;
    result->parent = parent;
    result->next_sibling = parent->first_child;
    result->prev_sibling = nullptr;
    debug_region_name_set(result, debug_name);

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

#undef MEM_DEBUG_NAMES_ENABLE
