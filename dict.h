#pragma once

// @Cleanup hash utilities

// FNV-1a : http://www.isthe.com/chongo/tech/comp/fnv/
function unsigned int
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

function unsigned int
BuildHashzstr(const char* zstr, unsigned int runningHash)
{
    unsigned int result = runningHash;
    static const unsigned int s_fnvPrime = 16777619;

    while (*zstr)
    {
        result ^= *zstr;
        result *= s_fnvPrime;
        zstr++;
    }

    return result;
}

function unsigned int
StartHash(const void* pBytes=nullptr, int cBytes=0)
{
    static const unsigned int s_fnvOffsetBasis = 2166136261;
    return BuildHash(pBytes, cBytes, s_fnvOffsetBasis);
}

// Use "id" hash functions for things like counters, ids, enums.
//  This is a bad hash function if used for generic integer data sets,
//  since data common divisors can lead to many collisions.
function u32 u16_id_hash(u16 const& id) { return (u32)id; }
function bool u16_eq(u16 const& lhs, u16 const& rhs) { return lhs == rhs; }
function u32 u32_id_hash(u32 const& id) { return id; }
function bool u32_eq(u32 const& lhs, u32 const& rhs) { return lhs == rhs; }

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
// Credit: JBlow
//  https://pastebin.com/xMUQXshn

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

// Adds the KVP to the table without testing existing keys for equality while probing.
//  i.e., This function has no qualms about adding a duplicate key. Use wisely!
template <typename K, typename V>
function void
dict_add_unchecked(Dict<K, V>* dict, K const& key, V const& value)
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

    dict->items[index] = { hash, key, value };
    dict->count++;
    dict->count_filled++;
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

    // Clear all new slots to unoccupied
    for (Kvp& kvp: slice_create(dict->items, dict->capacity))
    {
        kvp.hash = Kvp::HASH_UNOCCUPIED;
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
