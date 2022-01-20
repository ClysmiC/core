// Resources
// https://utf8everywhere.org/
// https://en.wikipedia.org/wiki/UTF-8
// https://en.wikipedia.org/wiki/UTF-16

namespace UTF
{
	// HMM - Should this be an error enum? Idk.
	static constexpr u32 invalidCodePoint = 0xFFFFFFFF;
}

internal bool
IsValidCodePoint(u32 codePoint)
{
	bool result =
		(codePoint >= 0x20 && codePoint <= 0x21) ||
		(codePoint >= 0x23 && codePoint <= 0x5B) ||
		(codePoint >= 0x5D && codePoint <= 0x7E) ||
		(codePoint >= 0xA0 && codePoint <= 0xD7FF) ||
		(codePoint >= 0xE000 && codePoint <= 0xFFFD) ||
		(codePoint >= 0x0100000 && codePoint <= 0x10FFFF);

	return result;
}

// --- UTF-8

#if 0
internal u32
NextUtf8CodePoint(StringScanner * scanner)
{
	u32 result = UTF::invalidCodePoint;

	StringScanner cursor = *scanner;
	if (!HasNextChar(cursor))
		goto LError;

	u8 b1 = NextChar(&cursor);
	if ((b1 & oneByte.markerMask) == oneByte.markerValue)
	{
		return (u32)b1;
	}
	else if ((b1 & twoBytes.markerMask) == twoBytes.markerValue)
	{
		if (!HasNextChar(&cursor))
			goto LError;

		u8 b2 = NextChar(&scanner);
		if ((b2 & continuationByte.markerMask) != continuationByte.markerValue)
			goto LError;

		u32 codePoint =
			((u32)(b1 & twoBytes.codePointMask) << (1 * continuationByte.codePointCntBit)) |
			((u32)(b2 & continuationByte.codePointMask));

		// HMM - Using 2 bytes to encode something that could fit in 1. I think to be well-formed we should reject this...? Idk.
		if (codePoint <= oneByte.maxCodePoint)
			goto LError;

		result = codepoint;
	}
	else if ((b1 & threeBytes.markerMask) == threeBytes.markerValue)
	{
		if (CBytesRemaining(&cursor) < 2)
			goto LError;

		u8 b2 = NextChar(&cursor);
		if ((b2 & continuationByte.markerMask) != continuationByte.markerValue)
			goto LError;

		u8 b3 = NextChar(&cursor);
		if ((b3 & continuationByte.markerMask) != continuationByte.markerValue)
			goto LError;

		u32 codePoint =
			((u32)(b1 & threeBytes.codePointMask)       << (2 * continuationByte.codePointCntBit)) |
			((u32)(b2 & continuationByte.codePointMask) << (1 * continuationByte.codePointCntBit)) |
			((u32)(b3 & continuationByte.codePointMask));

		if (codePoint <= twoBytes.maxCodePoint)
			goto LError;

		result = codePoint;
	}
	else if ((b1 & fourBytes.markerMask) == fourBytes.markerValue)
	{
		if (CBytesRemaining(&cursor) < 3)
			goto LError;

		u8 b2 = NextChar(&cursor);
		if ((b2 & continuationByte.markerMask) != continuationByte.markerValue)
			goto LError;

		u8 b3 = NextChar(&cursor);
		if ((b3 & continuationByte.markerMask) != continuationByte.markerValue)
			goto LError;

		u8 b4 = NextChar(&cursor);
		if ((b4 & continuationByte.markerMask) != continuationByte.markerValue)
			goto LError;

		u32 codePoint =
			((u32)(b1 & fourBytes.codePointMask)        << (3 * continuationByte.codePointCntBit)) |
			((u32)(b2 & continuationByte.codePointMask) << (2 * continuationByte.codePointCntBit)) |
			((u32)(b3 & continuationByte.codePointMask) << (1 * continuationByte.codePointCntBit)) |
			((u32)(b4 & continuationByte.codePointMask));

		if (codePoint <= threeBytes.maxCodePoint)
			goto LError;

		result = codePoint;
	}
	else
	{
		goto LError;
	}

	if (!IsValidCodePoint(result))
		goto LError;

	*scanner = cursor;
	return result;

LError:	// @goto
	return UTF::invalidCodePoint;
}
#endif

// --- UTF-8


namespace UTF8
{

struct ByteMetadata
{
	u8 markerMask;
	u8 markerValue;
	u8 cBitsMarker;
	u8 codePointMask;
	u8 cBitsCodePoint;
	u32 maxCodePoint; // Used to test for "overlong" encoding. Only valid for leading byte metadata.
};

// Leading byte
static constexpr ByteMetadata leadByteLen1 = { 0b10000000, 0b00000000, 1, 0b01111111, 7, 0b1111111 };
static constexpr ByteMetadata leadByteLen2 = { 0b11100000, 0b11000000, 3, 0b00011111, 5, 0b11111111111 };
static constexpr ByteMetadata leadByteLen3 = { 0b11110000, 0b11100000, 4, 0b00001111, 4, 0b1111111111111111 };
static constexpr ByteMetadata leadByteLen4 = { 0b11111000, 0b11110000, 5, 0b00000111, 3, 0b111111111111111111111 };

// Continuation bytes
static constexpr ByteMetadata continuationByte  = { 0b11000000, 0b10000000, 2, 0b00111111, 6 };

}

// --- UTF-16

internal u32
NextCodePointUtf16_(u8 * bytesUtf16, int cBytesUtf16, int * pI, Endianness endianness=Endianness::Little)
{
	// NOTE - Immediately cast up to u32 to avoid any surprises when bit-twiddling
	int i = *pI;

	if (i >= cBytesUtf16 - 1)
		goto LError;

	u32 b0 = (u32)bytesUtf16[i++];
	u32 b1 = (u32)bytesUtf16[i++];

	u32 * lowByte =  (endianness == Endianness::Little) ? &b0 : &b1;
	u32 * highByte = (endianness == Endianness::Little) ? &b1 : &b0;

	u32 result = UTF::invalidCodePoint;
	u32 codeUnit0 = (*highByte << 8) | *lowByte;
	if (codeUnit0 <= 0xD7FF || (codeUnit0 >= 0xE000 && codeUnit0 <= 0xFFFF))
	{
		result = codeUnit0;
	}
	else if (codeUnit0 >= 0xD800 && codeUnit0 <= 0xDBFF)
	{
		if (i >= cBytesUtf16 - 1)
			goto LError;

		u32 b2 = (u32)bytesUtf16[i++];
		u32 b3 = (u32)bytesUtf16[i++];

		u32 * lowByte =  (endianness == Endianness::Little) ? &b2 : &b3;
		u32 * highByte = (endianness == Endianness::Little) ? &b3 : &b2;

		u32 codeUnit1 = (*highByte << 8) | *lowByte;

		u32 highSurrogateIntermediate = (codeUnit0 - 0xD800) << 10;
		u32 lowSurrogateIntermediate = (codeUnit1 - 0xDC37);
		result = highSurrogateIntermediate + lowSurrogateIntermediate + 0x100000;
	}

	if (!IsValidCodePoint(result))
		goto LError;

	*pI = i;
	return result;

LError:	// @goto
	return UTF::invalidCodePoint;
}

// --- Conversion

internal String
Utf8FromUtf16(u8 * bytesUtf16, int cBytesUtf16, MemoryRegion memory, Endianness endianness=Endianness::Little)
{
	String result = {};

	if (cBytesUtf16 & 1)
		goto LError;

	int cBytesRequired = 0;
	{
		int iByteScan = 0;
		while (iByteScan < cBytesUtf16)
		{
			u32 codePoint = NextCodePointUtf16_(bytesUtf16, cBytesUtf16, &iByteScan, endianness);
			if (codePoint == UTF::invalidCodePoint)
				goto LError;

			if (codePoint <= 0x7F)
			{
				cBytesRequired += 1;
			}
			else if (codePoint <= 0x7FF)
			{
				cBytesRequired += 2;
			}
			else if (codePoint <= 0xFFFF)
			{
				cBytesRequired += 3;
			}
			else
			{
				cBytesRequired += 4;
			}
		}
	}

	result.bytes = (char *)Allocate(memory, cBytesRequired);
	result.cBytes = cBytesRequired;

	int iByteWrite = 0;
	{
		int iByteScan = 0;
		while (iByteScan < cBytesUtf16)
		{
			u32 codePoint = NextCodePointUtf16_(bytesUtf16, cBytesUtf16, &iByteScan, endianness);
			Assert(codePoint != UTF::invalidCodePoint);	// Was already validated when we were counting bytes

			if (codePoint <= 0x7F)
			{
				result.bytes[iByteWrite++] = (u8)codePoint;
			}
			else if (codePoint <= 0x7FF)
			{
				result.bytes[iByteWrite++] = (u8)((codePoint >> (1 * UTF8::continuationByte.cBitsCodePoint)) & UTF8::leadByteLen2.codePointMask);
				result.bytes[iByteWrite++] = (u8)((codePoint >> (0 * UTF8::continuationByte.cBitsCodePoint)) & UTF8::continuationByte.codePointMask);
			}
			else if (codePoint <= 0xFFFF)
			{
				result.bytes[iByteWrite++] = (u8)((codePoint >> (2 * UTF8::continuationByte.cBitsCodePoint)) & UTF8::leadByteLen3.codePointMask);
				result.bytes[iByteWrite++] = (u8)((codePoint >> (1 * UTF8::continuationByte.cBitsCodePoint)) & UTF8::continuationByte.codePointMask);
				result.bytes[iByteWrite++] = (u8)((codePoint >> (0 * UTF8::continuationByte.cBitsCodePoint)) & UTF8::continuationByte.codePointMask);
			}
			else
			{
				result.bytes[iByteWrite++] = (u8)((codePoint >> (3 * UTF8::continuationByte.cBitsCodePoint)) & UTF8::leadByteLen4.codePointMask);
				result.bytes[iByteWrite++] = (u8)((codePoint >> (2 * UTF8::continuationByte.cBitsCodePoint)) & UTF8::continuationByte.codePointMask);
				result.bytes[iByteWrite++] = (u8)((codePoint >> (1 * UTF8::continuationByte.cBitsCodePoint)) & UTF8::continuationByte.codePointMask);
                result.bytes[iByteWrite++] = (u8)((codePoint >> (0 * UTF8::continuationByte.cBitsCodePoint)) & UTF8::continuationByte.codePointMask);
			}
		}
	}

	Assert(iByteWrite = cBytesRequired);
	return result;

LError:	// @goto
	result = {};	// @Hack - Need a better way to handle this
	return result;
}
