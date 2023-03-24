// !SkipFile

enum class Endianness : u8
{
    LITTLE = 0,
    NIL = 0,

    BIG
};

inline uintptr
GetAlignmentOffset(uintptr value, uintptr alignment)
{
    // NOTE - Alignment is required to be power of 2
    
    uintptr alignMask = alignment - 1;
    uintptr alignOffset = (alignment - (value & alignMask)) & alignMask;
    return alignOffset;
}

inline void
ZeroMemory(void* memory, uintptr cBytes)
{
    bool isMemory8ByteAligned = (((uintptr)memory & 0x7) == 0);
    bool isCount8ByteMultiple = ((cBytes & 0x7) == 0);
    
    if (isMemory8ByteAligned && isCount8ByteMultiple)
    {
        // Faster. Clears 8 bytes at a time.

        u64 * pU64 = (u64 *)memory;
        uintptr cntU64 = cBytes / 8;
        for (uintptr iU64 = 0; iU64 < cntU64; iU64++)
        {
            *pU64 = 0;
            pU64++;
        }
    }
    else
    {
        // Slower. Clears 1 byte at a time.
    
        u8 * pByte = (u8 *)memory;
        for (uintptr iByte = 0; iByte < cBytes; iByte++)
        {
            *pByte = 0;
            pByte++;
        }
    }
}

#define ZeroArray(array) do { ZeroMemory((array), ArrayLen(array) * sizeof((array)[0])); } while (0)
#define FillArray(array, value) for (int i = 0; i < ArrayLen(array); i++) { (array)[i] = value; }

inline void
CopyMemory(void* src, void* dst, uintptr cBytes)
{
    // NOTE - Args are reversed from C memcpy!
    // NOTE - Does not try to handle overlapping src / dst
    // TODO - Copy multi-byte chunks for performance (like ZeroMemory does...)
    // TODO - Make a "move" that does handle overlapping src / dst

    u8 * byteSrc = (u8 *)src;
    u8 * byteDst = (u8 *)dst;
    for (uintptr iByte = 0; iByte < cBytes; iByte++)
    {
        *byteDst = *byteSrc;
        byteDst++;
        byteSrc++;
    }
}

template <typename T>
inline void
CopyStruct(T* src, T* dst)
{
    CopyMemory(src, dst, sizeof(*src));
}

inline void
MoveMemory(void* src, void* dst, int cBytes)
{
    // NOTE - Like CopyMemory, but handles overlapping src/dst
    if (src >= dst)
    {
        CopyMemory(src, dst, cBytes);
    }
    else
    {
        u8 * byteSrc = (u8 *)src + cBytes - 1;
        u8 * byteDst = (u8 *)dst + cBytes - 1;
        for (int iByte = cBytes - 1; iByte >= 0; iByte--)
        {
            *byteDst = *byteSrc;
            byteDst--;
            byteSrc--;
        }
    }
}
