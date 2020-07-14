#include "Game.h"
#include "Math.h"

internal void
DrawRectangle(GameOffscreenBuffer* buffer, vec2f min, vec2f max, vec3f color)
{
    s32 min_x = RoundF32ToS32(min.x);
    s32 min_y = RoundF32ToS32(min.y);
    s32 max_x = RoundF32ToS32(max.x);
    s32 max_y = RoundF32ToS32(max.y);
    
    if(min_x < 0)
    {
        min_x = 0;
    }
    if (min_y < 0)
    {
        min_y = 0;
    }
    if (max_x > buffer->width)
    {
        max_x = buffer->width;
    }
    if (max_y > buffer->height)
    {
        max_y = buffer->height;
    }
    
    u32 pixelColor = ((RoundF32ToU32(color.r * 255.0f) << 16) |
                      (RoundF32ToU32(color.g * 255.0f) << 8) |
                      (RoundF32ToU32(color.b * 255.0f) << 0));
    
    for (u32 y =  min_y; y < max_y; y++)
    {
        for (u32 x = min_x; x < max_x; x++)
        {
            ((u32 *) buffer->memory)[y * buffer->width + x] = pixelColor;
        }
    }
}


internal void
DrawBitmap(GameOffscreenBuffer* buffer, vec2f position, GameBitmap* bitmap)
{
    s32 min_x = RoundF32ToS32(position.x);
    s32 min_y = RoundF32ToU32(position.y);
    s32 max_x = min_x + bitmap->header.width;
    s32 max_y = min_y + bitmap->header.height;
    
    u32* at = (u32*)bitmap->buffer;
    
    if(min_x < 0)
    {
        min_x = 0;
    }
    if (min_y < 0)
    {
        min_y = 0;
    }
    if (max_x > buffer->width)
    {
        max_x = buffer->width;
    }
    if (max_y > buffer->height)
    {
        max_y = buffer->height;
    }
    
    u32 pixelColor = 0;
    
    for (u32 y =  min_y; y < max_y; y++)
    {
        for (u32 x = min_x; x < max_x; x++)
        {
            pixelColor = at[((y - min_y) * bitmap->header.width) + (x - min_x)];
            
            u8 r = (pixelColor >> 0) & 0xFF;
            u8 g = (pixelColor >> 8) & 0xFF;
            u8 b = (pixelColor >> 16) & 0xFF;
            u8 a = (pixelColor >> 24) & 0xFF;
            
            if(a)
            {
                pixelColor = ((a << 24) |
                              (r << 16) |
                              (g << 8) |
                              (b << 0));
                
                ((u32 *) buffer->memory)[y * buffer->width + x] = pixelColor;
            }
        }
    }
}



extern "C" __declspec(dllexport)
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    
    
    for(int i = 0; i < 1; i++)
    {
        DrawRectangle(buffer, {0.0f, 0.0f}, {1280.0f, 720.0f}, {1.0f, 0.0f, 0.0f});
    }
    
    
#if 0    
    GameState *gameState = (GameState *) gameMemory->permanentStorage;
    
    if (!gameMemory->isInitialized)
    {
        gameState->playerPos = {100.0f, 100.0f };
        gameState->playerPos1 = {500.0f, 100.0f };
        
        gameState->playerSpeed = 5.0f;
        
        gameState->rectangleColor = 155.0f;
        
        gameMemory->isInitialized = true;
    }
    
    if (gameState->rectangleColor > 255.0f)
    {
        gameState->rectangleColor = 0.0f;
    }
    else
    {
        gameState->rectangleColor += 10.0f;
    }
    
    if (gameState->input.leftPressed)
    {
        gameState->playerPos.x -= 5.0f;
    }
    else if (gameState->input.rightPressed)
    {
        real32 nextPlayerPos = gameState->playerPos.x + 200.0f + 10.0f;
        if (nextPlayerPos > gameState->playerPos1.x)
        {
            bool isCollided = true;
        }
        else
        {
            gameState->playerPos.x += 10.0f;
        }
    }
    else if (gameState->input.downPressed)
    {
    }
    else if (gameState->input.upPressed)
    {
    }
    
    gameState->input.leftPressed = false;
    gameState->input.rightPressed = false;
    gameState->input.upPressed = false;
    gameState->input.downPressed = false;
    
    DrawRectangle(buffer, {0.0f, 0.0f}, {1280.0f, 720.0f}, 0.0f, gameState->rectangleColor, 0.0f);
    DrawRectangle(buffer, gameState->playerPos, {gameState->playerPos.x + 200.0f, gameState->playerPos.y + 200.0f}, 255.0f, 0.0f, 0.0f);
    DrawRectangle(buffer, gameState->playerPos, {gameState->playerPos.x + 200.0f, gameState->playerPos.y + 200.0f}, 255.0f, 0.0f, 0.0f);
    DrawRectangle(buffer, gameState->playerPos1, {gameState->playerPos1.x + 200.0f, gameState->playerPos1.y + 200.0f}, 0.0f, 0.0f, 255.0f);
#endif
    
}
