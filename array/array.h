#pragma once

//  Helper for iterating a packed array by pointer using for-each syntax.
//  usage:
//      for (Child* child : IterByPtr(children, child_count))

template <typename T>
struct IterByPtr
{
    T* items;
    int count;

    IterByPtr() = default;
    IterByPtr(T* items, int count)
    {
        this->items = items;
        this->count = count;
    }

    struct Ptr
    {
        // A pointer that returns itself when dereferenced (tricks C++ into iterating by pointer)

        T* ptr;

        Ptr() = default;
        explicit Ptr(T * ptr) { this->ptr = ptr; }
        T* operator*() { return ptr; }
        void operator++() { ptr++; }
        bool operator!=(Ptr const& other) { return ptr != other.ptr; }
    };

    Ptr begin() { return Ptr(items); }
    Ptr end()   { return Ptr(items + count); }
};



// --- Enum_Table
//  Array indexed by an enum. By default, Nil is an invalid index.
//  You need to use the DefineEnumOps macro to use an Enum_Table!

template <typename ENUM_KEY, typename T_VALUE, ENUM_KEY START=ENUM_KEY::NIL + 1>
struct Enum_Table
{
    T_VALUE items[ENUM_KEY::ENUM_COUNT - START];

    T_VALUE& operator[](ENUM_KEY index)
    {
#if ENUM_TABLE_BOUNDS_CHECK
        Assert(index >= START && index < ENUM_KEY::ENUM_COUNT);
#endif
        u64 offset_index = u64(index - START);
        return items[offset_index];
    }

    T_VALUE const& operator[](ENUM_KEY index) const
    {
#if ENUM_TABLE_BOUNDS_CHECK
        Assert(index >= START && index < ENUM_KEY::ENUM_COUNT);
#endif
        u64 offset_index = u64(index - START);
        return items[offset_index];
    }

    // HMM - This isn't an obvious overload, but it makes it work the same way as a raw ptr, which I like
    T_VALUE* operator+(ENUM_KEY index)
    {
#if ENUM_TABLE_BOUNDS_CHECK
        Assert(index >= START && index < ENUM_KEY::ENUM_COUNT);
#endif

        return &items[(int)(index - START)];
    }
};

template <typename ENUM_KEY, typename T_VALUE>
using Enum_Table_Allow_Nil = Enum_Table<ENUM_KEY, T_VALUE, ENUM_KEY::NIL>;

// TODO - DynEnum_Table -- growable enum table for indexing with unbounded ID enums



#include "slice.h"
#include "buffer.h"
