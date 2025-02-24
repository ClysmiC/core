// IDEAS:
// - Make headers smaller by having a u32 offset for left/right instead of pointers. This limits allocations to 8gb per (mul u32 by 4 and enforce 4-byte alignment)
//
// TODO:
// - Alignment
// - Re-alloc in place
// - Audit for degenerate cases where things like overflow allocations are somehow too small to store an overflow + free header

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
debug_id_from_name(char const* name)
{
#if MEM_DEBUG_NAMES_ENABLE
    if (!name)
        return 0;

    char const* c = name;
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
debug_validate_left_and_right(Tracked_Block_Header* tracked)
{
#if BUILD_DEBUG
    Tracked_Block_Header* left = tracked ? tracked->left : nullptr;
    if (left)
    {
        ASSERT(left->right == tracked);
        ASSERT((u8*)left + left->byte_count == (u8*)tracked);

    }
    Tracked_Block_Header* right = tracked ? tracked->right : nullptr;
    if (right)
    {
        ASSERT(right->left == tracked);
        ASSERT((u8*)tracked + tracked->byte_count == (u8*)right);
    }
#endif
}

function void
debug_validate_prev_and_next(Free_Block_Header* free, bool allow_size_mismatch = false)
{
#if BUILD_DEBUG
    Free_Block_Header* prev = free ? free->prev : nullptr;
    if (prev)
    {
        ASSERT(prev->next == free);
        ASSERT(IMPLIES(!allow_size_mismatch, prev->byte_count >= free->byte_count));

    }
    Free_Block_Header* next = free ? free->next : nullptr;
    if (next)
    {
        ASSERT(next->prev == free);
        ASSERT(IMPLIES(!allow_size_mismatch, next->byte_count <= free->byte_count));
    }
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
        while (*c && i < ARRAY_LEN(region->name))
        {
            region->name[i] = *c;
            c++;
            i++;
        }
        region->name[ARRAY_LEN(region->name) - 1] = '\0';
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
function void* allocate(MEM::Region_Header* region, uintptr byte_count, CTZ ctz=CTZ::NO);
function void* allocate_tracked(MEM::Region_Header* region, uintptr byte_count, CTZ ctz=CTZ::NO);
function void free_tracked_allocation(MEM::Region_Header* region, void* allocation);
function bool mem_region_end(MEM::Region_Header* region);
function bool mem_region_reset(MEM::Region_Header* region);

namespace MEM
{

function void
resize_free_list_head(Free_Block_Header** ppHead, uintptr byte_count_new)
{
    Free_Block_Header* pHeadOrig = *ppHead;

    if (byte_count_new == 0)
    {
        // NOTE - 0 has special meaning. It means to remove the head node entirely.
        //  We don't bother updating the size

        *ppHead = pHeadOrig->next;
        if (*ppHead)
        {
            (*ppHead)->prev = nullptr;
        }

        debug_validate_prev_and_next(*ppHead);
    }
    else
    {
        ASSERT(byte_count_new > BYTE_COUNT::TOO_SMALL_TO_BOTHER_TRACKING);

        pHeadOrig->byte_count = byte_count_new;
        bool isHeadTooSmall = pHeadOrig->next && pHeadOrig->next->byte_count > pHeadOrig->byte_count;

        if (isHeadTooSmall)
        {
            // Set the head to the bigger node
            *ppHead = pHeadOrig->next;
            (*ppHead)->prev = nullptr;

            if (byte_count_new > 0)
            {
                ASSERT(byte_count_new > BYTE_COUNT::TOO_SMALL_TO_BOTHER_TRACKING);

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

                debug_validate_prev_and_next(pHeadOrig);
                debug_validate_prev_and_next(pHeadOrig->next);
                debug_validate_prev_and_next(pHeadOrig->prev);
            }

            debug_validate_prev_and_next(*ppHead);
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

    // --- Allocate an overflow region from our parent if shared block is too small.
    //      This memory is treated as a shared block.

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
        free->state = Tracked_State::FREE_SHARED;

        free->next = region_header->shared_list;
        if (free->next)
        {
            free->next->prev = free;
        }

        debug_validate_prev_and_next(free);
        debug_validate_prev_and_next(free->next);

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

        debug_validate_prev_and_next(*ppHead);
    }
    else
    {
        // TODO START HERE pItem->prev is pointing from 176 -> 160 even though 160 is allocated...
        // probably need to fix this at the point where we allocate + split 720 into 160 and 560

        if (pItem->prev)
        {
            pItem->prev->next = pItem->next;
        }

        if (pItem->next)
        {
            pItem->next->prev = pItem->prev;
        }

        debug_validate_prev_and_next(pItem->prev);
        debug_validate_prev_and_next(pItem->next);
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

    debug_validate_prev_and_next(pItem);
    debug_validate_prev_and_next(*ppNextToFix);
}

function void*
allocate_tracked_from_region(Region_Header* region, uintptr byte_count)
{
    // Make sure our tracked or free block is big enough!

    // HMM - Gracefully handle 0 byte allocation?
    byte_count += sizeof(Tracked_Block_Header);
    byte_count = max(byte_count, sizeof(Free_Block_Header)); // Make sure the allocation is big enough that if it gets freed, we can store a free block header in place

    Free_Block_Header* free = ensure_block_with_size(region, byte_count, AllocType::Tracked);

    bool is_free_block_from_tracked_list = (free == region->tracked_list);
    ASSERT(IMPLIES(!is_free_block_from_tracked_list, free == region->shared_list));

    // Split free block into the tracked allocation (left) and ...

    uintptr split_byte_count = free->byte_count - byte_count;
    if (split_byte_count <= BYTE_COUNT::TOO_SMALL_TO_BOTHER_TRACKING)
    {
        // ... nothing. Just lump the extra bytes with the returned tracked allocation.
        split_byte_count = 0;
    }

    Tracked_Block_Header* result_header = (Tracked_Block_Header*)free;
    void* result = (u8*)((Tracked_Block_Header*)result_header + 1);
    result_header->byte_count = free->byte_count - split_byte_count;
    result_header->state = Tracked_State::ALLOCATED;

    if (split_byte_count > 0)
    {
        // ... a smaller shared block (right)
        ASSERT(split_byte_count > BYTE_COUNT::TOO_SMALL_TO_BOTHER_TRACKING);

        Free_Block_Header* split = (Free_Block_Header*)((u8*)free + byte_count);
        split->byte_count = split_byte_count;
        split->left = result_header;
        split->right = result_header->right;
        split->state = is_free_block_from_tracked_list ?
                        Tracked_State::FREE_UNSHARED :
                        Tracked_State::FREE_SHARED;
        split->next = free->next;
        if (split->next)
        {
            split->next->prev = split;
        }
        split->prev = nullptr; ASSERT(!free->prev);

        result_header->right = split;
        if (split->right)
        {
            split->right->left = split;
        }

        // --- Update free list
        if (is_free_block_from_tracked_list)
        {
            region->tracked_list = split;
            resize_free_list_head(&region->tracked_list, split_byte_count);
        }
        else
        {
            region->shared_list = split;
            resize_free_list_head(&region->shared_list, split_byte_count);
        }

        debug_validate_prev_and_next(split);
    }
    else
    {
        // ... (nothing)

        // --- Update free list
        if (is_free_block_from_tracked_list)
        {
            resize_free_list_head(&region->tracked_list, 0);
            debug_validate_prev_and_next(region->tracked_list);
        }
        else
        {
            resize_free_list_head(&region->shared_list, 0);
            debug_validate_prev_and_next(region->shared_list);
        }
    }


    debug_validate_left_and_right(result_header);

    return result;
}

function void
free_tracked_allocation_from_region(Region_Header* region, void* allocation)
{
    if (!allocation)
        return;

    Tracked_Block_Header* tracked_header = (Tracked_Block_Header*)((u8*)allocation - sizeof(Tracked_Block_Header));
    ASSERT(tracked_header->state == Tracked_State::ALLOCATED);
    debug_validate_left_and_right(tracked_header);

    Tracked_Block_Header* left = tracked_header->left;
    Tracked_Block_Header* right = tracked_header->right;
    debug_validate_left_and_right(left);
    debug_validate_left_and_right(right);

    if (left && left->state == Tracked_State::FREE_UNSHARED)
    {
        ASSERT(left->right == tracked_header);

        // --- Merge w/ free left (unshared)

        left->byte_count += tracked_header->byte_count;
        left->right = tracked_header->right;

        if (left->right)
        {
            left->right->left = left;
        }

        // Remove for now, and...
        free_list_remove(&region->tracked_list, (Free_Block_Header*)left);
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
            free_list_remove(&region->tracked_list, (Free_Block_Header*)right);
        }
        else if (right->state == Tracked_State::FREE_SHARED)
        {
            merged_with_shared = true;

            // --- Merge w/ free right (shared)

            tracked_header->byte_count += right->byte_count;

            ASSERT(right->right == nullptr);     // right of shared is never tracked, by definition
            tracked_header->right = nullptr;

            // Remove for now. We'll add the merged result back in.
            free_list_remove(&region->shared_list, (Free_Block_Header*)right);
        }
    }

    if (merged_with_shared)
    {
        tracked_header->state = Tracked_State::FREE_SHARED;
        free_list_add(&region->shared_list, (Free_Block_Header*)tracked_header);
    }
    else
    {
        tracked_header->state = Tracked_State::FREE_UNSHARED;
        free_list_add(&region->tracked_list, (Free_Block_Header*)tracked_header);
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
    mem_region_reset(region);

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
allocate(Memory_Region region, uintptr byte_count, CTZ ctz)
{
    using namespace MEM;

    ASSERT(region);

    Region_Header* region_header = region;

    // --- Make sure our shared block is big enough!

    Free_Block_Header* shared = ensure_block_with_size(region_header, byte_count, AllocType::Untracked);
    ASSERT(shared == region_header->shared_list);
    ASSERT(shared->byte_count >= byte_count);

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
        debug_validate_prev_and_next(region_header->shared_list);

        result = shared;
    }
    else
    {
        // ... a smaller shared block (left)

        resize_free_list_head(&region_header->shared_list, split_byte_count);
        result = (u8*)shared + split_byte_count;

        debug_validate_prev_and_next(region_header->shared_list);
    }

    if ((bool)ctz)
    {
        mem_zero(result, byte_count);
    }

    return result;
}

template <typename T>
T*
allocate(
    Memory_Region region,
    CTZ ctz=CTZ::NO)
{
    T* result = (T*)allocate(region, sizeof(T), ctz);
    return result;
}

template <typename T>
T*
allocate_array(
    Memory_Region region,
    uintptr count,
    CTZ ctz=CTZ::NO)
{
    T* result = (T*)allocate(region, sizeof(T) * count, ctz);
    return result;
}

function void*
allocate_tracked(Memory_Region region, uintptr byte_count, CTZ ctz)
{
    void* result;
    if (region)
    {
        result = MEM::allocate_tracked_from_region(region, byte_count);
    }
    else
    {
        result = MEM::system_allocate(byte_count);
    }

    if ((bool)ctz)
    {
        mem_zero(result, byte_count);
    }

    return result;
}

template <typename T>
T*
allocate_tracked(
    Memory_Region region,
    CTZ ctz=CTZ::NO)
{
    T* result = (T*)allocate_tracked(region, sizeof(T), ctz);
    return result;
}

template <typename T>
T*
allocate_array_tracked(
    Memory_Region region,
    uintptr count,
    CTZ ctz=CTZ::NO)
{
    T* result = (T*)allocate_tracked(region, sizeof(T) * count, ctz);
    return result;
}

function void
free_tracked_allocation(Memory_Region region, void* allocation)
{
    if (region)
    {
        MEM::free_tracked_allocation_from_region(region, allocation);
    }
    else
    {
        MEM::system_free(allocation);
    }
}

function void*
reallocate_tracked(Memory_Region region, void* allocation, uintptr byte_count_new)
{
    using namespace MEM;

    void* result;
    if (!allocation)
    {
        result = allocate_tracked(region, byte_count_new);
    }
    else
    {
        Tracked_Block_Header* tracked_header = (Tracked_Block_Header*)((u8*)allocation - sizeof(Tracked_Block_Header));
        ASSERT(tracked_header->state == Tracked_State::ALLOCATED);
        debug_validate_left_and_right(tracked_header);

        uintptr byte_countOld = tracked_header->byte_count - sizeof(Tracked_Block_Header);
        if (byte_countOld >= byte_count_new)
        {
            result = allocation;
        }
        else
        {

            // TODO - Grow in place if possible

            result = allocate_tracked(region, byte_count_new);
            mem_copy(result, allocation, byte_countOld);

            free_tracked_allocation(region, allocation);
        }
    }

    return result;
}

template <typename T>
T*
reallocate_array_tracked(
    Memory_Region region,
    T* allocation,
    uintptr count_new)
{
    T* result = (T*)reallocate_tracked(region, allocation, sizeof(T) * count_new);
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
mem_region_init(
    u8* bytes,
    uintptr byte_count,
    Memory_Region parent,
    char const* debug_name = nullptr)
{
    using namespace MEM;

    byte_count = max(byte_count, BYTE_COUNT::MINIMUM_REGION);

    Region_Header* result = (Region_Header*)bytes;
    *result = {};
    result->byte_budget = byte_count;
    result->shared_list = (Free_Block_Header*)(result + 1);
    *result->shared_list = {};
    result->shared_list->byte_count = result->byte_budget - sizeof(Region_Header);
    result->shared_list->state = Tracked_State::FREE_SHARED;
    result->parent = parent;
    result->prev_sibling = nullptr;

    if (parent)
    {
        result->next_sibling = parent->first_child;
        if (result->next_sibling)
        {
            result->next_sibling->prev_sibling = result;
        }

        parent->first_child = result;
    }
    else
    {
        result->next_sibling = nullptr;
    }

    debug_region_name_set(result, debug_name);
    return result;
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
    mem_region_init((u8*)result, byte_count, parent, debug_name);

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

    // @Cleanup... both ctor and init
    Recycle_Allocator(Memory_Region memory) { *this = {}; this->memory = memory; }
};

template <typename T>
function void
recycle_allocator_init(Recycle_Allocator<T>* alloc, Memory_Region memory)
{
    *alloc = {};
    alloc->memory = memory;
}

template <typename T>
function T*
allocate(
    Recycle_Allocator<T> * alloc,
    CTZ ctz=CTZ::NO)
{
    T* result;

    if (alloc->recycleList)
    {
        // @UB - Might be UB taking the address of one member of the union and then reading the other member...?
        //  But it makes sense, behaves as expected on MSVC, and is type-safe-ish
        result = &alloc->recycleList->item;
        alloc->recycleList = alloc->recycleList->pNextRecycled;

        if ((bool)ctz)
        {
            mem_zero(result, sizeof(Recycle_Allocator<T>::Slot));
        }

#if BUILD_DEBUG
        alloc->countAvailableToRecycle--;
        alloc->countRecycledTotal++;
        ASSERT(alloc->countAvailableToRecycle >= 0);
#endif
    }
    else
    {
        result = (T*)allocate(alloc->memory, sizeof(Recycle_Allocator<T>::Slot), ctz);
    }

#if BUILD_DEBUG
    alloc->countallocatedTotal++;
    alloc->countLive++;
#endif

    return result;
}

template <typename T>
function void
PreallocateRecycleListContiguous(
    Recycle_Allocator<T> * alloc,
    int cntItemPreallocate,
    CTZ ctz=CTZ::NO)
{
    auto * slots = (Recycle_Allocator<T>::Slot*)allocate(alloc->memory,
                                                         cntItemPreallocate * sizeof(Recycle_Allocator<T>::Slot),
                                                         ctz);

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
function void
recycle(Recycle_Allocator<T>* alloc, T* item)
{
    auto * slot = (Recycle_Allocator<T>::Slot*)item;

    slot->pNextRecycled = alloc->recycleList;
    alloc->recycleList = slot;

#if BUILD_DEBUG
    alloc->countAvailableToRecycle++;
    alloc->countLive--;
    ASSERT(alloc->countLive >= 0);
#endif
}

#undef MEM_DEBUG_NAMES_ENABLE
