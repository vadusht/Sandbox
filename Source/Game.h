#ifndef UNTITLED_GAME_H
#define UNTITLED_GAME_H

#include "Utils.h"

struct GameBitmapHeader
{
    u32 width;
    u32 height;
    u8 bytes_per_pixel;
};

struct GameBitmap
{
    GameBitmapHeader header;
    void* buffer;
    s32 size;
};

struct GameOffscreenBuffer
{
    void* memory;
    u32 width;
    u32 height;
    u32 pitch;
};

struct GameMemory
{
    b32 initialized;
    
    u64 permanent_storage_size;
    void* permanent_storage;
    
    u64 transient_storage_size;
    void* transient_storage;
};


struct Input
{
    bool left_pressed;
    bool right_pressed;
    bool up_pressed;
    bool down_pressed;
    int x_pos;
    int y_pos;
};

struct GameState
{
    Input input;
    
};

#define GAME_UPDATE_AND_RENDER(name) void name(GameMemory* game_memory, GameOffscreenBuffer* buffer)
typedef GAME_UPDATE_AND_RENDER(GameUpdateAndRenderFunc);
GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub)
{
    
}


#endif //UNTITLED_GAME_H
