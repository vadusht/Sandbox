/* date = July 5th 2020 9:37 am */

#ifndef _UTILS_H
#define _UTILS_H

#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

#define ArrayCount(array) (sizeof((array)) / sizeof((array)[0]))

#define Kilobytes(value) ((value) * 1024)
#define Megabytes(value) (Kilobytes(value) * 1024)
#define Gigabytes(value) (Megabytes(value) * 1024)
#define Terabytes(value) (Gigabytes(value) * 1024)

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

typedef int8_t s8;
typedef int16_t s16;

#define Assert(statement) if(!(statement)) { *(u32*)0 = 0; } 

#define BIT(x) (1 << x)
#define BITS_IN_BYTE 8
#define BITS_IN_TWO_BYTES (2 * BITS_IN_BYTE)
#define BITS_IN_FOUR_BYTES (2 * BITS_IN_TWO_BYTES)

u32 GetBits(u32 value, u32 start, u32 end)
{
    Assert(end >= start);
    Assert((start < BITS_IN_FOUR_BYTES) && (end < BITS_IN_FOUR_BYTES));
    
    u32 result = 0;
    
    for(u32 bit_index = start; bit_index <= end; bit_index++)
    {
        result |= (value & BIT(bit_index));
    }
    
    result >>= start;
    
    return result;
}

u32 GetBits(u32 value, u32 position)
{
    return GetBits(value, position, position);
}

void ChangeEndianness(u32* value)
{
#if 0
    u32 byte0 = (*value & 0x000000FF) << 24;
    u32 byte1 = (*value & 0x0000FF00) << 8;
    u32 byte2 = (*value & 0x00FF0000) >> 8;
    u32 byte3 = (*value & 0xFF000000) >> 24;
    
    *value = byte0 | byte1 | byte2 | byte3;
#else
    *value = _byteswap_ulong(*value);
#endif
}

#endif //_UTILS_H
