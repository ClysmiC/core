#pragma once

// @Cleanup hash utilities

// FNV-1a : http://www.isthe.com/chongo/tech/comp/fnv/
function u32
BuildHash(const void* pBytes, int cBytes, unsigned int runningHash)
{
    unsigned int result = runningHash;
    static const unsigned int s_fnvPrime = 16777619;

    for (int i = 0; i < cBytes; i++)
    {
        // @Slow - do this loop with uint64 to go faster?
        result ^= static_cast<const char*>(pBytes)[i];
        result *= s_fnvPrime;
    }

    return result;
}

function u32
BuildHashzstr(const char* zstr, unsigned int runningHash)
{
    unsigned int result = runningHash;
    unsigned int constexpr PRIME = 16777619;

    while (*zstr)
    {
        result ^= *zstr;
        result *= PRIME;
        zstr++;
    }

    return result;
}

function u32
StartHash(const void* pBytes=nullptr, int cBytes=0)
{
    static const unsigned int s_fnvOffsetBasis = 2166136261;
    return BuildHash(pBytes, cBytes, s_fnvOffsetBasis);
}

function u32
u64_hash(u64 const& value)
{
    u64 constexpr PRIME = 0x9E3779B97F4A7C15ull;
    u64 result = value;
    result ^= result >> 33;
    result *= PRIME;
    result ^= result >> 29;
    return (u32)result;
}

function u32
u32_hash(u32 const& value)
{
    u32 constexpr PRIME = 0x9E3779B9;
    u32 result = value;
    result ^= result >> 16;
    result *= PRIME;
    result ^= result >> 13;
    return result;
}

function u32
u16_hash(u16 const& value)
{
    u32 constexpr PRIME = 0x9E3779B9;
    u32 result = value;
    result ^= result >> 7;
    result *= PRIME;
    result ^= result >> 5;
    return result;
}

function u32
u8_hash(u8 const& value)
{
    u32 constexpr PRIME = 0x9E3779B9;
    u32 result = value;
    result ^= result >> 3;
    result *= PRIME;
    result ^= result >> 2;
    return result;
}

function u32
f32_hash(f32 const& value)
{
    u32 constexpr PRIME = 0x9E3779B9;

    u32 bits;
    mem_copy(&bits, &value, sizeof(f32));

    // Normalize -0.0 to 0.0
    if ((bits & 0x7FFFFFFF) == 0) bits = 0;

    // Treat all NaNs the same... maybe not that useful since we still use == for f32_eq(..)
    if ((bits & 0x7F800000) == 0x7F800000 && (bits & 0x007FFFFF) != 0)
    {
        bits = 0x7FC00000;
    }

    // Hashing
    bits ^= bits >> 16;
    bits *= PRIME;
    bits ^= bits >> 13;

    return bits;
}

inline bool u8_eq(u8 const& lhs, u8 const& rhs) { return lhs == rhs; }
inline bool u16_eq(u16 const& lhs, u16 const& rhs) { return lhs == rhs; }
inline bool u32_eq(u32 const& lhs, u32 const& rhs) { return lhs == rhs; }
inline bool u64_eq(u64 const& lhs, u64 const& rhs) { return lhs == rhs; }
inline bool f32_eq(f32 const& lhs, f32 const& rhs) { return lhs == rhs; }

// Identity hash function for ID keys
inline u32 u8_identity(u8 const& value) { return value; }
inline u32 u16_identity(u16 const& value) { return value; }
inline u32 u32_identity(u32 const& value) { return value; }

function unsigned int
StartHashzstr(const char* zstr)
{
    static const unsigned int s_fnvOffsetBasis = 2166136261;
    return BuildHashzstr(zstr, s_fnvOffsetBasis);
}

function unsigned int
CombineHash(unsigned int hash0, unsigned int hash1)
{
    return hash0 ^ 37 * hash1;
}



// --- Simple dictionary with linear probing

template <typename K, typename V>
struct Dict
{
    struct Kvp
    {
        static u32 constexpr HASH_UNOCCUPIED = 0;
        static u32 constexpr HASH_REMOVED = 1;
        static u32 constexpr HASH_VALID_MIN = 2;

        u32 hash;
        K key;
        V value;
    };

    i32 max_load_factor;    // Percentage, 1-99
    i32 count;              // Valid item count
    i32 count_filled;       // Valid + removed item count. Need to remember removed items when linear probing.
    i32 capacity;           // Size of backing array. Grows when max_load_factor is exceeded.

    // User provided functions
    u32 (*key_hash)(K const& key);
    bool (*key_eq)(K const& key0, K const& key1);

    Memory_Region memory;
    Kvp* items;

    struct Iter
    {
        Kvp* cursor;
        Kvp* end;
        bool operator!=(Iter const& other) const { return cursor != other.cursor; }
        Kvp& operator*() { return *cursor; }
        Kvp* operator->() { return cursor; }
        Iter& operator++()
        {
            cursor++;
            while (cursor < end && cursor->hash < Kvp::HASH_VALID_MIN) cursor++;
            return *this;
        }
    };
    struct Const_Iter
    {
        Kvp const* cursor;
        Kvp const* end;
        bool operator!=(Const_Iter const& other) const { return cursor != other.cursor; }
        Kvp const& operator*() const { return *cursor; }
        Kvp const* operator->() const { return cursor; }
        Const_Iter& operator++()
        {
            cursor++;
            while (cursor < end && cursor->hash < Kvp::HASH_VALID_MIN) cursor++;
            return *this;
        }
    };

    Iter end() { return Iter{ items + capacity, items + capacity }; }
    Iter begin()
    {
        if (count == 0) return end();
        Iter result = Iter{ items, items + capacity };
        if (items[0].hash < Kvp::HASH_VALID_MIN) ++result; // overloaded ++ iterates until valid 
        return result;
    }
    Const_Iter end() const { return Const_Iter{ items + capacity, items + capacity }; }
    Const_Iter begin() const
    {
        if (count == 0) return end();
        Const_Iter result = Const_Iter{ items, items + capacity };
        if (items[0].hash < Kvp::HASH_VALID_MIN) ++result; // overloaded ++ iterates until valid 
        return result;
    }
};

// This function has no qualms about adding a duplicate key. Use wisely!
template <typename K, typename V>
function typename Dict<K, V>::Kvp*
dict_add_key_unchecked(Dict<K, V>* dict, K const& key)
{
    using Kvp = Dict<K, V>::Kvp;

    dict_ensure_capacity(dict, dict->count_filled + 1);

    u32 hash = dict->key_hash(key);
    if (hash < Kvp::HASH_VALID_MIN) hash += Kvp::HASH_VALID_MIN;

    u32 mask = dict->capacity - 1;
    i32 index = hash & mask;

    while (true)
    {
        if (dict->items[index].hash == Kvp::HASH_UNOCCUPIED)
            break;

        if (dict->items[index].hash == Kvp::HASH_REMOVED)
        {
            dict->count_filled--;    // remove the HASH_REMOVED, we'll increment this right back below
            break;
        }

        // HMM - consider quadratic probing with "triangular numbers"
        // https://fgiesen.wordpress.com/2015/02/22/triangular-numbers-mod-2n/
        // TODO - enforce power of 2 capacity and use & instead of %
        index = (index + 1) & mask;
    }

    auto* result = dict->items + index;
    result->hash = hash;
    result->key = key;
    dict->count++;
    dict->count_filled++;

    return result;
}

// This function has no qualms about adding a duplicate key. Use wisely!
template <typename K, typename V>
function void
dict_add_unchecked(Dict<K, V>* dict, K const& key, V const& value)
{
    using Kvp = Dict<K, V>::Kvp;
    Kvp* kvp = dict_add_key_unchecked(dict, key);
    kvp->value = value;
}

// Grows the table so that it can store at least this many items within its load factor
template <typename K, typename V>
function void
dict_ensure_capacity(Dict<K, V>* dict, i32 capacity)
{
    using Kvp = Dict<K, V>::Kvp;

    // Save a handle to the old memory
    Slice<Kvp> old_items = slice_create(dict->items, dict->capacity);

    i32 new_capacity = max(dict->capacity, capacity);
    new_capacity = u32_ceil_power_of_2(new_capacity);

    while (new_capacity * dict->max_load_factor < capacity * 100)
    {
        new_capacity *= 2;
    }

    if (dict->capacity >= new_capacity)
        return;

    // Reset with bigger memory
    dict->items = (Kvp*)allocate_tracked(dict->memory, new_capacity * sizeof(Kvp));
    dict->count = 0;
    dict->count_filled = 0;
    dict->capacity = new_capacity;

    // Clear all slots to unoccupied
    for (Kvp* kvp: ByPtr(slice_create(dict->items, dict->capacity)))
    {
        kvp->hash = Kvp::HASH_UNOCCUPIED;
    }

    // Re-add old items
    for (Kvp const& kvp: old_items)
    {
        if (kvp.hash >= Kvp::HASH_VALID_MIN)
        {
            dict_add_unchecked(dict, kvp.key, kvp.value);
        }
    }

    // Free old memory
    ASSERT(IFF(old_items.items, old_items.count > 0));
    if (old_items.items)
    {
        free_tracked_allocation(dict->memory, old_items.items);
    }
}

template <typename K, typename V>
function typename Dict<K, V>::Kvp*
dict_find_kvp_ptr(Dict<K, V> const& dict, K const& key)
{
    using Kvp = Dict<K, V>::Kvp;

    if (dict.capacity <= 0)
    {
        ASSERT_FALSE_WARN;
        return nullptr;
    }

    u32 hash = dict.key_hash(key);
    if (hash < Kvp::HASH_VALID_MIN) hash += Kvp::HASH_VALID_MIN;

    u32 mask = dict.capacity - 1;
    i32 index = hash & mask;

    Kvp* result = nullptr;
    while (true)
    {
        if (dict.items[index].hash == Kvp::HASH_UNOCCUPIED)
            break;

        if (dict.items[index].hash == hash &&
            dict.key_eq(dict.items[index].key, key))
        {
            result = dict.items + index;
            break;
        }

        // HMM - consider quadratic probing with "triangular numbers"
        // https://fgiesen.wordpress.com/2015/02/22/triangular-numbers-mod-2n/
        // TODO - enforce power of 2 capacity and use & instead of %
        index = (index + 1) & mask;
    }

    return result;
}

template <typename K, typename V>
function V*
dict_find_ptr(Dict<K, V> const& dict, K const& key)
{
    V* result = nullptr;
    if (Dict<K, V>::Kvp* kvp = dict_find_kvp_ptr(dict, key))
    {
        result = &kvp->value;
    }
    return result;
}

template <typename K, typename V>
function V
dict_find(Dict<K, V> const& dict, K const& key, bool* success)
{
    if (V* result = dict_find_ptr(dict, key))
    {
        *success = true;
        return *result;
    }

    *success = false;
    return V{};
}

template <typename K, typename V>
function V*
dict_find_or_new(Dict<K, V>* dict, K const& key)
{
    if (V* value_ptr = dict_find_ptr(*dict, key))
    {
        return value_ptr;
    }
    else
    {
        dict_add_key_unchecked(dict, key);
    }
}

template <typename K, typename V>
function bool
dict_contains(Dict<K, V> const& dict, K const& key)
{
    bool result = (dict_find_ptr(dict, key) != nullptr);
    return result;
}

template <typename K, typename V>
function void
dict_set(Dict<K, V>* dict, K const& key, V const& value)
{
    if (V* value_ptr = dict_find_ptr(*dict, key))
    {
        *value_ptr = value;
    }
    else
    {
        dict_add_unchecked(dict, key, value);
    }
}

template <typename K, typename V>
function bool
dict_remove(Dict<K, V>* dict, K const& key)
{
    using Kvp = Dict<K, V>::Kvp;

    if (Kvp* kvp_ptr = dict_find_kvp_ptr(*dict, key))
    {
        kvp_ptr->hash = Kvp::HASH_REMOVED;
        dict->count--;
        // HMM - should we return the removed value?
        return true;
    }

    return false;
}

template <typename K, typename V>
function Dict<K, V>
dict_create(
    Memory_Region memory,
    u32 (*key_hash)(K const& key),
    bool (*key_eq)(K const& key0, K const& key1),
    i32 starting_capacity=16)
{
    starting_capacity = max(starting_capacity, 16);

    Dict<K, V> result = {};
    result.max_load_factor = 70;
    result.key_hash = key_hash;
    result.key_eq = key_eq;
    result.memory = memory;
    // HMM - maybe we shouldn't allocate in create(..). Wait until we have an item instead?
    dict_ensure_capacity(&result, starting_capacity);
    return result;
}

template <typename K, typename V>
function void
dict_reset(Dict<K, V>* dict)
{
    using Kvp = Dict<K, V>::Kvp;

    dict->count = 0;
    dict->count_filled = 0;
    for (int i = 0; i < dict->capacity; i++)
    {
        dict->items[i].hash = Kvp::HASH_UNOCCUPIED;
    }
}

//  --- Bi-directional dict (all values are unique)
// template <typename K, typename V>
// function void
// struct Dict_Unique
// {
//     // Values are const, because they are keys in the complimentary dict.
//     //  Modifying them would break change the hash calculation for the complimentary dict's lookups.
//     Dict<K, V const*> forward;
//     Dict<V, K const*> reverse;
// };

// template <typename K, typename V>
// function void
// dict_unique_fixup_forward(Dict_Unique<K, V>* dict)
// {
//     using Kvp_Forward = typename Dict<K, V*>::Kvp;
//     using Kvp_Reverse = typename Dict<V, K*>::Kvp;

//     // Iterate over reverse, since it's currently all valid
//     for (Kvp_Reverse const& kvp_reverse: dict->reverse)
//     {
//         // @Slow - lots of lookups needed to do this fixup. There's probably a better
//         //  design for Dict_Unique. Something like Dict<Key_Or_Value, int> that would let
//         //  us fixup both in one pass without doing a bunch of lookups. Consider implementing
//         //  that and then profiling both to see if it's noticable.
//         Kvp_Forward* kvp_forward = dict_find_kvp_ptr(dict->forward, *kvp_reverse.value);
//         ASSERT(kvp_reverse);

//         // Fix-up the V*
//         kvp_forward->value = &kvp_reverse->key;
//     }
// }

// template <typename K, typename V>
// function void
// dict_unique_fixup_reverse(Dict_Unique<K, V>* dict)
// {
//     using Kvp_Forward = typename Dict<K, V*>::Kvp;
//     using Kvp_Reverse = typename Dict<V, K*>::Kvp;

//     // Iterate over forward, since it's currently all valid
//     for (Kvp_Forward const& kvp_forward: dict->forward)
//     {
//         // @Slow - See note in dict_unique_fixup_forward(..)
//         Kvp_Reverse* kvp_reverse = dict_find_kvp_ptr(dict->reverse, *kvp_forward.value);
//         ASSERT(kvp_reverse);

//         // Fix-up the K*
//         kvp_reverse->value = &kvp_forward->key; // is this line const-correct?
//     }
// }

// template <typename K, typename V>
// function bool
// dict_unique_try_set(Dict_Unique<K, V>* dict, K const& key, V const& value)
// {
//     using Kvp_Forward = typename Dict<K, V*>::Kvp;
//     using Kvp_Reverse = typename Dict<V, K*>::Kvp;

//     // If the key or value aren't unique, do nothing
//     if (dict_contains(&dict->forward, key) || dict_contains(&dict->reverse, value))
//     {
//         // HMM - may make sense to return true if key and value already exist
//         //  and are mapped to each other... but for now I'm assuming the
//         //  caller will only set the mapping once.
//         return false;
//     }

//     Kvp_Forward* kvp_forward;
//     bool is_forward_rehashed;
//     {
//         // Add key
//         i32 old_capacity = dict->forward.capacity;
//         kvp_forward = dict_add_key_unchecked(&dict->forward, key);

//         is_forward_rehashed = (dict->forward.capacity != old_capacity);
//         if (is_forward_rehashed)
//         {
//             // Fixup the reverse dict to point to the rehashed keys
//             dict_unique_fixup_reverse(dict);
//         }
//     }

//     Kvp_Reverse* kvp_reverse;
//     {
//         // Add value
//         i32 old_capacity = dict->reverse.capacity;
//         kvp_reverse = dict_add_key_unchecked(&dict->reverse, value);

//         bool is_reverse_rehashed = (dict->reverse.capacity != old_capacity);

//         // Both dicts are expected to grow in lockstep. Technically it should still work if they don't, but why wouldn't they?
//         ASSERT_WARN(is_forward_rehashed == is_reverse_rehashed);

//         if (is_reverse_rehashed)
//         {
//             // Fixup the forward dict to point to the rehashed values
//             dict_unique_fixup_forward(dict);
//         }
//     }

//     kvp_forward->value = &kvp_reverse->key;
//     kvp_reverse->value = &kvp_forward->key;

//     ASSERT(kvp_forward->value);
//     ASSERT(kvp_reverse->value);
//     return true;
// }

// template <typename K, typename V>
// function bool
// dict_unique_remove_by_key(Dict_Unique<K, V>* dict, K const& key)
// {
//     V const** value = dict_find_ptr(dict->forward, key);

//     if (value)
//     {
//         ASSERT(*value);

//         VERIFY(dict_remove(&dict->forward, key));
//         VERIFY(dict_remove(&dict->reverse, **value));
//         return true;
//     }

//     return false;
// }

// template <typename K, typename V>
// function bool
// dict_unique_remove_by_value(Dict_Unique<K, V>* dict, V const& value)
// {
//     K const** key = dict_find_ptr(dict->reverse, value);

//     if (key)
//     {
//         ASSERT(*key);

//         VERIFY(dict_remove(&dict->forward, **key));
//         VERIFY(dict_remove(&dict->reverse, value));
//         return true;
//     }
//     return false;
// }

// // Finds a value by key
// template <typename K, typename V>
// function V const*
// dict_unique_find_by_key(Dict_Unique<K, V> const& dict, K const& key)
// {
//     V const** found = dict_find_ptr(dict.forward, key);
//     V const* result = (found) ? *found : nullptr;
//     return result;
// }

// // Finds a key by value
// template <typename K, typename V>
// function K const*
// dict_unique_find_by_value(Dict_Unique<K, V> const& dict, V const& value)
// {
//     K const** found = dict_find_ptr(dict.reverse, value);
//     K const* result = (found) ? *found : nullptr;
//     return result;
// }

// template <typename K, typename V>
// function Dict_Unique<K, V>
// dict_unique_create(
//     Memory_Region memory,
//     u32 (*key_hash)(K const& key),
//     bool (*key_eq)(K const& key0, K const& key1),
//     u32 (*value_hash)(V const& value),
//     bool (*value_eq)(V const& value0, 1 const& value1)
//     i32 starting_capacity=16)
// {
//     starting_capacity = max(starting_capacity, 16);

//     using Kov = typename Dict_Unique<K, V>::Key_Or_Value;

//     Dict_Unique<K, V> result = {};
//     result.dict = dict_create<K, V const*>(memory, key_hash, key_eq, starting_capacity);
//     result.reverse = dict_create<V, K const*>(memory, value_hash, value_eq, starting_capacity);
//     return result;
// }

// template <typename K, typename V>
// function void
// dict_unique_reset(Dict_Unique<K, V>* dict)
// {
//     dict_reset(&dict->forward);
//     dict_reset(&dict->reverse);
// }



// Convenience wrapper for when you don't care about the values. Most functionality is
//  still exposed through the dict(..) API. The idiom is to set the key's value to
//  true to add it to the set.
template<typename K>
using Set = Dict<K, bool>;

template <typename K>
function Set<K>
set_create(
    Memory_Region memory,
    u32 (*key_hash)(K const& key),
    bool (*key_eq)(K const& key0, K const& key1),
    i32 starting_capacity=16)
{
    Set<K> result = dict_create<K, bool>(memory, key_hash, key_eq, starting_capacity);
    return result;
}
