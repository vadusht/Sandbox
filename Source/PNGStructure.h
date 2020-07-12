/* date = July 6th 2020 9:15 pm */

#ifndef _P_N_G_STRUCTURE_H
#define _P_N_G_STRUCTURE_H

#include "Utils.h"

global_variable u8 png_signature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

#pragma pack(push, 1)

struct PNGHeader
{
    u8 signature[8];
};

struct PNGChunkHeader
{
    u32 length;
    union
    {
        u32 type;
        u8 type_in_bytes[4];
    };
};

struct PNGChunkFooter
{
    u32 crc;
};

struct PNGChunkIHDR
{
    u32 width;
    u32 height;
    u8 bit_depth;
    u8 color_type;
    u8 compression_method;
    u8 filter_method;
    u8 interlace_method;
};

struct PNGChunkIDAT
{
    u8 zlib_compression_method;
    u8 additional_flags;
};

struct PNGData
{
    u32 width;
    u32 height;
    void* buffer;
    u8* at;
    u8 bytes_per_pixel;
};

enum PNGFilteringMethod
{
    Unfiltered = 0,
    Sub = 1,
    Up = 2,
    Average,
    Paeth,
};


#pragma pack(pop)

struct StreamingBuffer
{
    u8* at;
    s64 size;
    
    u32 bit_count;
    
    StreamingBuffer* next;
};

struct StreamingIDATChunk
{
    u8* at;
    s64 size;
    
    u32 bit_count;
    u16 bits;
    
    StreamingIDATChunk* next;
};

struct HuffmanCodes
{
    u16* code_lengths_repeat_count;
    u16* symbols;
};

#define MAX_LITERAL_CODES 286
#define MAX_DISTANCE_CODES 30
#define MAX_CODES (MAX_LITERAL_CODES + MAX_DISTANCE_CODES)

#endif //_P_N_G_STRUCTURE_H
