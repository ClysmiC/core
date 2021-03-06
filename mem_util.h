// !SkipFile

enum class Endianness : u8
{
    Nil = 0,
    Little = Nil,
    Big
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
ZeroMemory(void * memory, int cBytes)
{
    bool isMemory8ByteAligned = (((uintptr)memory & 0x7) == 0);
    bool isCount8ByteMultiple = ((cBytes & 0x7) == 0);
    
    if (isMemory8ByteAligned && isCount8ByteMultiple)
    {
        // Faster. Clears 8 bytes at a time.

        u64 * pU64 = (u64 *)memory;
        int cntU64 = cBytes / 8;
        for (int iU64 = 0; iU64 < cntU64; iU64++)
        {
            *pU64 = 0;
            pU64++;
        }
    }
    else
    {
        // Slower. Clears 1 byte at a time.
    
        u8 * pByte = (u8 *)memory;
        for (int iByte = 0; iByte < cBytes; iByte++)
        {
            *pByte = 0;
            pByte++;
        }
    }
}

inline void
CopyMemory(void * src, void * dst, int cBytes)
{
    // NOTE - Args are reversed from C memcpy!
    // NOTE - Does not try to handle overlapping src / dst
    // TODO - Copy multi-byte chunks for performance (like ZeroMemory does...)
    // TODO - Make a "move" that does handle overlapping src / dst

    u8 * byteSrc = (u8 *)src;
    u8 * byteDst = (u8 *)dst;
    for (int iByte = 0; iByte < cBytes; iByte++)
    {
        *byteDst = *byteSrc;
        byteDst++;
        byteSrc++;
    }
}

inline void
MoveMemory(void * src, void * dst, int cBytes)
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
