// TODO - Replace this whole file. This implementation sucks and uses stdlib stuff instead of my libs

#pragma once

// @DependencyLeak
// HMM - Try deleting these to
#include <cstdint>
#include <cstdlib>
#include <cstring>

// Hash map (not thread safe)
//    Uses hopscotch hashing: http://mcg.cs.tau.ac.il/papers/disc2008-hopscotch.pdf

namespace AlsHash
{
static const int s_neighborhoodSize = 32;        // 'H' in the paper!

// Offset of the entry in a bucket from it's "ideal" bucket

static const uint8_t s_infoOffsetMask        = 0b00011111;

// 2 Most significant bits of the hash of the entry in this bucket. If these don't match a candidate key's hash,
//    no need to test the keys for equality (eliminates 75% of unnecessary equality checks)

static const uint8_t s_infoHashMsbs            = 0b11000000;

// 1 bit indicates whether the bucket is full or empty

static const uint8_t s_infoOccupiedMask        = 0b00100000;
}

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

template <typename K, typename V>
struct HashMap
{
    struct Bucket
    {
        uint8_t infoBits;

        K key;
        V value;
    };

    Bucket* pBuffer;
    int32_t cCapacity;

    uint32_t (*hashFn)(const K & key);
    bool (*equalFn)(const K & key0, const K & key1);
};

template <typename K, typename V>
function void
Clear(HashMap<K, V>* hashmap)
{

    for (int i = 0; i < hashmap->cCapacity; i++)
    {
        HashMap<K, V>::Bucket* bucket = hashmap->pBuffer + i;
        bucket->infoBits = 0;
    }
}

template <typename K, typename V>
struct HashMapIter
{
    const HashMap<K, V> * pHashmap;
    int iItem;

    const K * pKey;
    V * pValue;
};

template <typename K, typename V>
void IterNext(HashMapIter<K, V> * pIter)
{
    typedef HashMap<K, V> hm;

    bool hadNext = false;
    for (pIter->iItem += 1; pIter->iItem < pIter->pHashmap->cCapacity; pIter->iItem += 1)
    {
        hm::Bucket* pBucket = pIter->pHashmap->pBuffer + pIter->iItem;
        if (pBucket->infoBits & AlsHash::s_infoOccupiedMask)
        {
            pIter->pKey = &pBucket->key;
            pIter->pValue = &pBucket->value;
            hadNext = true;
            break;
        }
    }

    if (!hadNext)
    {
        pIter->pKey = nullptr;
        pIter->pValue = nullptr;
    }
}

template <typename K, typename V>
HashMapIter<K, V> Iter(const HashMap<K, V> & hashmap)
{
    HashMapIter<K, V> it;
    it.pHashmap = &hashmap;
    it.iItem = -1;

    IterNext(&it);

    return it;
}


template <typename K, typename V>
V * InsertNew(
    HashMap<K, V> * pHashmap,
    const K & key,
    K ** ppoKeyInTable=nullptr)        // Only really useful for BiHashMap... don't set this in most cases!
{
    typedef HashMap<K, V> hm;

    ASSERT(pHashmap->pBuffer);

    // H is the neighborhood size

    static const uint32_t H = AlsHash::s_neighborhoodSize;

    uint32_t hash = pHashmap->hashFn(key);
    uint32_t hashMsbs = (hash >> 24) & AlsHash::s_infoHashMsbs;

    // i is the ideal index for the key

    ASSERT(
        pHashmap->cCapacity != 0 &&
        (pHashmap->cCapacity & (pHashmap->cCapacity - 1)) == 0);  // Test that it is a power of 2

    // Mask out lower bits instead of modulus since we know capacity is power of 2

    uint32_t i = hash & (pHashmap->cCapacity - 1);

    hm::Bucket* pBucketIdeal = pHashmap->pBuffer + i;

    // j is the pre-modulo index of the empty bucket slot we are tracking

    uint32_t j = i;
    hm::Bucket* pBucketEmpty;

    // Linear probe to the first empty bucket.

    while (true)
    {
        hm::Bucket* pBucket = pHashmap->pBuffer + (j % pHashmap->cCapacity);

        if(!(pBucket->infoBits & AlsHash::s_infoOccupiedMask))
        {
            pBucketEmpty = pBucket;
            break;
        }
        else
        {
            // Check if this key is already in our table. If it is, just return the existing value

            if (((uint32_t)(pBucket->infoBits & AlsHash::s_infoHashMsbs) == hashMsbs) && pHashmap->equalFn(key, pBucket->key))
            {
                return &pBucket->value;
            }
        }

        if (j - i > H * 2)
        {
            // We looked way past neighborhood and still can't find an empty slot.
            //    Regrow/rehash.

            // Double in size!

            GrowHashmap(pHashmap, (pHashmap->cCapacity << 1));
            return InsertNew(pHashmap, key);
        }

        j++;
    }

    // If the empty bucket is too far away from the ideal index, find an
    //    "earlier" bucket that is allowed to move into the empty bucket, and
    //    use the "earlier" bucket's previous position as our new empty bucket.
    //    Repeat as many times as necessary.

    while (j - i >= H)
    {
        hm::Bucket* pBucketSwappable = nullptr;

        // k is the candidate to be swapped into j

        uint32_t k = j - (H - 1);
        int kIteration = 0;
        int dOffset = 0;

        while(k < j)
        {
            hm::Bucket* pBucketCandidate = pHashmap->pBuffer + (k % pHashmap->cCapacity);

            int candidateOffset = pBucketCandidate->infoBits & AlsHash::s_infoOffsetMask;
            if (candidateOffset <= kIteration)
            {
                pBucketSwappable = pBucketCandidate;
                dOffset = j - k;
                break;
            }

            k++;
            kIteration++;
        }

        if (!pBucketSwappable)
        {
            GrowHashmap(pHashmap, (pHashmap->cCapacity << 1));
            return InsertNew(pHashmap, key);
        }

        // Copy swappable bucket into the empty bucket and update the offset

        *pBucketEmpty = *pBucketSwappable;

        int newOffset = (pBucketSwappable->infoBits & AlsHash::s_infoOffsetMask )+ dOffset;
        ASSERT(newOffset < H);

        pBucketEmpty->infoBits &= ~AlsHash::s_infoOffsetMask;
        pBucketEmpty->infoBits |= newOffset;


        // Update pointer and index of empty bucket

        pBucketEmpty = pBucketSwappable;
        j = k;
    }

    int offset = j - i;
    pBucketEmpty->infoBits &= ~AlsHash::s_infoOffsetMask;
    pBucketEmpty->infoBits |= offset;

    pBucketEmpty->infoBits |= AlsHash::s_infoOccupiedMask;

    pBucketEmpty->infoBits &= ~AlsHash::s_infoHashMsbs;
    pBucketEmpty->infoBits |= hashMsbs;

    pBucketEmpty->key = key;

    if (ppoKeyInTable)
        *ppoKeyInTable = &pBucketEmpty->key;

    return &pBucketEmpty->value;
}

template <typename K, typename V>
void Insert(
    HashMap<K, V> * pHashmap,
    const K & key,
    const V & value,
    K ** ppoKeyInTable=nullptr)        // Only really useful for BiHashMap... don't set this in most cases!
{
    V * valueNew = InsertNew(pHashmap, key, ppoKeyInTable);
    *valueNew = value;
}

enum class HashOp : uint8_t
{
    NIL = 0,
    
    Lookup,
    Update,
    Remove,

    ENUM_COUNT
};

template <typename K, typename V>
function bool AlsHashHelper_(
    HashMap<K, V> * pHashmap,
    const K & key,
    HashOp hashopk,
    const V * pValueNew=nullptr,
    V ** ppoValueLookup=nullptr,
    V * poValueRemoved=nullptr,
    K ** ppoKeyFound=nullptr)        // Only really useful for BiHashMap... don't set this in most cases!
{
    typedef HashMap<K, V> hm;

    ASSERT(pHashmap->pBuffer);

    // H is the neighborhood size

    static const uint32_t H = AlsHash::s_neighborhoodSize;

    uint32_t hash = pHashmap->hashFn(key);
    uint32_t hashMsbs = (hash >> 24) & AlsHash::s_infoHashMsbs;

    // i is the ideal index for the key

    uint32_t i = hash % pHashmap->cCapacity;

    // Linear probe in the neighborhood.

    for (uint32_t iCandidate = i; iCandidate < i + H; iCandidate++)
    {
        hm::Bucket* pBucketCandidate = pHashmap->pBuffer + (iCandidate % pHashmap->cCapacity);

        // Check if candidate is occupied

        if(!(pBucketCandidate->infoBits & AlsHash::s_infoOccupiedMask))
        {
            continue;
        }

        // Check if candidate has offset we would expect

        int offset = iCandidate - i;
        if ((pBucketCandidate->infoBits & AlsHash::s_infoOffsetMask) != offset)
        {
            continue;
        }

        // Check if candidate hash matches our hash's most significant bits

        if ((uint32_t)(pBucketCandidate->infoBits & AlsHash::s_infoHashMsbs) != hashMsbs)
        {
            continue;
        }

        // Compare the keys!

        if (pHashmap->equalFn(key, pBucketCandidate->key))
        {
            if (ppoKeyFound)
            {
                *ppoKeyFound = &pBucketCandidate->key;
            }

            switch ((int)hashopk)
            {
                case HashOp::Lookup:
                {
                    if (ppoValueLookup)
                    {
                        *ppoValueLookup = &pBucketCandidate->value;
                    }

                    return true;
                }

                case HashOp::Update:
                {
                    pBucketCandidate->value = *pValueNew;
                    return true;
                }

                case HashOp::Remove:
                {
                    if (poValueRemoved)
                    {
                        *poValueRemoved = pBucketCandidate->value;
                    }

                    pBucketCandidate->infoBits &= ~AlsHash::s_infoOccupiedMask;
                    return true;
                }

                default:
                {
                    ASSERT(false);
                    return false;
                }
            }
        }
    }

    return false;
}

template <typename K, typename V>
V * Lookup(
    const HashMap<K, V> & pHashmap,
    const K & key,
    K ** ppoKeyFound=nullptr)        // Only really useful for BiHashMap... don't set this in most cases!
{
    V * pResult = nullptr;

    AlsHashHelper_(
        const_cast<HashMap<K, V> *>(&pHashmap),   // Helper is a more general function so the parameter can't be const, but the lookup code path maintains constness
        key,
        HashOp::Lookup,
        static_cast<const V*>(nullptr),        // Update
        &pResult,                            // Lookup
        static_cast<V *>(nullptr),            // Remove
        ppoKeyFound                            // Address of found key
        );

    return pResult;
}

template <typename K, typename V>
bool Remove(
    HashMap<K, V> * pHashmap,
    const K & key,
    V * poValueRemoved=nullptr)
{
    return AlsHashHelper_(
        pHashmap,
        key,
        HashOp::Remove,
        static_cast<const V*>(nullptr),    // Update
        static_cast<V*>(nullptr),        // Lookup
        poValueRemoved,                    // Remove
        static_cast<K **>(nullptr)        // Address of found key
        );
}

template <typename K, typename V>
bool Update(
    HashMap<K, V> * pHashmap,
    const K & key,
    const V & value)
{
    return AlsHashHelper_(
        pHashmap,
        key,
        HashOp::Update,
        &value,                        // Update
        static_cast<V*>(nullptr),    // Lookup
        static_cast<V*>(nullptr)    // Remove
        static_cast<K **>(nullptr)    // Address of found key
        );
}

template <typename K, typename V>
void GrowHashmap(
    HashMap<K, V> * pHashmap,
    int newCapacity)
{
    typedef HashMap<K, V> hm;

    ASSERT(newCapacity > pHashmap->cCapacity);

    // Store pointer to old buffer so we can copy from it

    hm::Bucket* pBufferOld = pHashmap->pBuffer;
    int cCapacityOld = pHashmap->cCapacity;

    // Create new buffer and set it and assign it to the hashmap

    unsigned int cBytesNewBuffer = newCapacity * sizeof(hm::Bucket);
    pHashmap->pBuffer = (hm::Bucket*)malloc(cBytesNewBuffer);
    memset(pHashmap->pBuffer, 0, cBytesNewBuffer);

    pHashmap->cCapacity = newCapacity;

    // Deal with old buffer!

    if (pBufferOld)
    {
        // Rehash into new buffer

        for (int i = 0; i < cCapacityOld; i++)
        {
            hm::Bucket* pBucket = pBufferOld + i;

            if (pBucket->infoBits & AlsHash::s_infoOccupiedMask)
            {
                Insert(pHashmap, pBucket->key, pBucket->value);
            }
        }

        // Free old buffer

        free(pBufferOld);
    }
}

template <typename K, typename V>
void Init(
    HashMap<K, V> * pHashmap,
    uint32_t (*hashFn)(const K & key),
    bool (*equalFn)(const K & key0, const K & key1),
    unsigned int startingCapacity=AlsHash::s_neighborhoodSize)
{
    // Starting capacity can't be smaller than neighborhood size

    if (startingCapacity < AlsHash::s_neighborhoodSize)
    {
        startingCapacity = AlsHash::s_neighborhoodSize;
    }

    // Round startingCapacity to power of 2

    unsigned int power = 2;
    while (power <= startingCapacity)
        power <<= 1;

    startingCapacity = power;

    pHashmap->pBuffer = nullptr;
    pHashmap->cCapacity = 0;
    pHashmap->hashFn = hashFn;
    pHashmap->equalFn = equalFn;

    GrowHashmap(pHashmap, startingCapacity);
}

template <typename K, typename V>
void Dispose(HashMap<K, V> * pHashmap)
{
    if (pHashmap->pBuffer) free(pHashmap->pBuffer);
    pHashmap->pBuffer = nullptr;
}
