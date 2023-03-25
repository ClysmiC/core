// !SkipFile

// TODO - figure out why mem_util.h complains if I don't have this macro,
//  but other code complains about redifinition if I do...
//  I don't think I'm #including anything that should give us the CRT offsetof..
//  May /NODEFAULTLIB would help, but I think that is only a linker option?

// Using _ for now as a hack workaround
#define offsetof_(type, member) ((uintptr)&(((type*)0)->member))

#define ArrayLen(array) (sizeof(array) / sizeof((array)[0]))

#define Kilobytes(value) ((value) * 1024LL)
#define Megabytes(value) (Kilobytes(value) * 1024LL)
#define Gigabytes(value) (Megabytes(value) * 1024LL)
#define Terabytes(value) (Gigabytes(value) * 1024LL)

enum class Endianness : u8
{
    LITTLE = 0,
    NIL = 0,

    BIG
};

function uintptr
mem_align_offset(uintptr value, uintptr alignment)
{
    // NOTE - Alignment is required to be power of 2
    // TODO - runtime assert?
    
    uintptr alignMask = alignment - 1;
    uintptr alignOffset = (alignment - (value & alignMask)) & alignMask;
    return alignOffset;
}

function void
mem_zero(void* memory, uintptr cBytes)
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

#define ZeroArray(array) do { mem_zero((array), ArrayLen(array) * sizeof((array)[0])); } while (0)
#define FillArray(array, value) for (int i = 0; i < ArrayLen(array); i++) { (array)[i] = value; }

function void
mem_copy(void* dst, void* src, uintptr bytes)
{
    // NOTE - Does not try to handle overlapping src / dst
    // TODO - Copy multi-byte chunks for performance (like mem_zero does...)

    u8 * s = (u8 *)src;
    u8 * d = (u8 *)dst;
    for (uintptr i = 0; i < bytes; i++)
    {
        *d = *s;
        d++;
        s++;
    }
}

function void
mem_move(void* dst, void* src, uintptr bytes)
{
    // NOTE - Like CopyMemory, but handles overlapping src/dst
    // TODO - Copy multi-byte chunks for performance (like mem_zero does...)

    if (src >= dst)
    {
        mem_copy(dst, src, bytes);
    }
    else
    {
        u8 * s = (u8 *)src + bytes - 1;
        u8 * d = (u8 *)dst + bytes - 1;
        for (intptr i = bytes - 1; i >= 0; i--)
        {
            *d = *s;
            d--;
            s--;
        }
    }
}

function void
mem_set(void* dst, u8 value, uintptr bytes)
{
    // TODO - Copy multi-byte chunks for performance (like mem_zero does...)

    u8* cursor = (u8*)dst;
    while (bytes--)
    {
        *cursor = value;
        cursor++;
    }
}
