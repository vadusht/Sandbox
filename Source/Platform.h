/* date = July 5th 2020 10:25 pm */

#ifndef _MAIN_H
#define _MAIN_H

#pragma pack(push, 1)

struct FileContents
{
    s64 size;
    void* buffer;
};

struct BitmapHeader
{
    u32 width;
    u32 height;
    u8 bytes_per_pixel;
};

struct Bitmap
{
    BitmapHeader header;
    void* buffer;
    s32 size;
};

#pragma pack(pop)

struct Win32GameCode
{
    HMODULE game_code_dll;
    GameUpdateAndRenderFunc* UpdateAndRender;
    
    b32 valid;
};

#endif //_MAIN_H
