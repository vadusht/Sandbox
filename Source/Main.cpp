#include <Windows.h>
#include <Windowsx.h>
#include "Game.h"
#include "Utils.h"
#include "Main.h"

struct Win32WindowDimension
{
    int width;
    int height;
};


struct Win32OffscreenBuffer 
{
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
};

global_variable Win32OffscreenBuffer globalBackbuffer;
global_variable bool running;

struct vec2f
{
    f32 x, y;
};

internal vec2f
operator+(vec2f left, vec2f right)
{
    vec2f result;
    
    result.x = left.x + right.x;
    result.y = left.y + right.y;
    
    return result;
}

union vec3f
{
    struct 
    {
        f32 x, y, z;
    };
    struct
    {
        f32 r, g, b;
    };
};


s32 RoundFloat32ToInt32(f32 value)
{
    s32 result = (s32)(value + 0.5f);
    return result;
}


u32 RoundFloat32ToUInt32(f32 value)
{
    u32 result = (u32)(value + 0.5f);
    return result;
}

#if 0
enum PNGChunk
{
    PNGChunk_IHDR = 0x52444849;
    PNGChunk_sRGB = 
        PNGChunk_IDAT = 
        PNGChunk_IEND = 
}
#endif 

#if 0
void ParsePNG(void* buffer, s64 size)
{
    //return;
    u8* pointer = (u8*)buffer;
    u64 header = *(u64*)pointer;
    pointer += sizeof(header);
    bool parsing = true;
    if(header == PNG_HEADER_BIG_ENDIAN)
    {
        while(parsing)
        {
            u32 chunk_length = *(u32*)pointer;
            UInt32ChangeEndianness(&chunk_length);
            pointer += sizeof(chunk_length);
            u32 chunk_type = *(u32*)pointer;
            pointer += sizeof(chunk_type);
            if(chunk_type == PNG_CHUNK_IHDR_BIG_ENDIAN)
            {
                u32 width = *(u32*)pointer;
                pointer += sizeof(width);
                
                u32 height = *(u32*)pointer;
                pointer += sizeof(height);
                
                u8 depth = *(u8*)pointer;
                pointer += sizeof(depth);
                
                u8 color_type = *(u8*)pointer;
                pointer += sizeof(color_type);
                
                u8 compression = *(u8*)pointer;
                pointer += sizeof(compression);
                
                u8 filter = *(u8*)pointer;
                pointer += sizeof(filter);
                
                u8 interlace = *(u8*)pointer;
                pointer += sizeof(interlace);
            }
            else if(chunk_type == PNG_CHUNK_IDAT_BIG_ENDIAN)
            { 
                u8 zlib_compression = *(u8*)pointer;
                pointer += sizeof(zlib_compression);
                
                u8 cm = zlib_compression & 0x0F;
                u8 cinfo = zlib_compression >> 4;
                
                u8 flags = *(u8*)pointer;
                pointer += sizeof(flags);
                
                u8 fcheck = flags & 0x0F;
                u8 fdict = flags & 0x10;
                u8 flevel = flags & 0xC0;
                
                int b = 20;
                
                //u8 check_value = *(u8*)pointer;
                //pointer += sizeof(check_value);
            }
            else
            {
                pointer += chunk_length;
            }
            u32 crc = *(u32*)pointer;
            pointer += sizeof(crc);
        }
    }
}
#endif

internal FileContents 
Win32ReadEntireFile(char* filename)
{
    FileContents result;
    HANDLE handle = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, 0, 
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(handle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER size;
        BOOL success = GetFileSizeEx(handle, &size);
        if(success)
        {
            DWORD bytes_to_read = size.QuadPart;
            DWORD bytes_read = 0;
            void* buffer = VirtualAlloc(0, bytes_to_read, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            success = ReadFile(handle, buffer, bytes_to_read, &bytes_read, 0);
            if(success && (bytes_to_read == bytes_read))
            {
                result.buffer = buffer;
                result.size = bytes_to_read;
            }
            else
            {
                VirtualFree(buffer, 0, MEM_RELEASE);
            }
        }
    }
    
    return result;
}


internal void
DrawBitmap(Win32OffscreenBuffer* buffer, vec2f position, Bitmap* bitmap)
{
    s32 min_x = RoundFloat32ToInt32(position.x);
    s32 min_y = RoundFloat32ToInt32(position.y);
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

internal void
DrawRectangle(Win32OffscreenBuffer* buffer, vec2f min, vec2f max, vec3f color)
{
    s32 min_x = RoundFloat32ToInt32(min.x);
    s32 min_y = RoundFloat32ToInt32(min.y);
    s32 max_x = RoundFloat32ToInt32(max.x);
    s32 max_y = RoundFloat32ToInt32(max.y);
    
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
    
    u32 pixelColor = ((RoundFloat32ToUInt32(color.r * 255.0f) << 16) |
                      (RoundFloat32ToUInt32(color.g * 255.0f) << 8) |
                      (RoundFloat32ToUInt32(color.b * 255.0f) << 0));
    
    for (u32 y =  min_y; y < max_y; y++)
    {
        for (u32 x = min_x; x < max_x; x++)
        {
            ((u32 *) buffer->memory)[y * buffer->width + x] = pixelColor;
        }
    }
}


internal void
Win32ResizeDIBSection(Win32OffscreenBuffer* buffer, int width, int height) 
{
    if (buffer->memory) 
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }
    
    buffer->width = width;
    buffer->height = height;
    
    int bytesPerPixel = 4;
    
    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;
    
    int bitmapMemorySize = buffer->width * buffer->height * bytesPerPixel;
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = buffer->width * bytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(HDC device_context, 
                           int window_width, int window_height, Win32OffscreenBuffer* buffer) 
{
    StretchDIBits(device_context,
                  0, 0, window_width, window_height,
                  0, 0, buffer->width, buffer->height,
                  buffer->memory, &buffer->info,
                  DIB_RGB_COLORS, SRCCOPY);
}

internal Win32WindowDimension
Win32GetWindowDimension(HWND window) 
{
    Win32WindowDimension result;
    
    RECT client_rect;
    GetClientRect(window, &client_rect);
    result.width = client_rect.right - client_rect.left;
    result.height = client_rect.bottom - client_rect.top;
    
    return result;
}

internal void
ClearInput(Input* input)
{
    input->right_pressed = false;
    input->left_pressed = false;
    input->up_pressed = false;
    input->down_pressed = false;
}

internal void
Win32ProcessInput(MSG* message, Input* input)
{
    //bool key_was_up = (message->lParam & BIT(30)) == 0;
    bool key_is_down = (message->lParam & BIT(31)) == 0;
    bool alt_is_down = (message->lParam & BIT(29)) != 0;
    
    ClearInput(input);
    
    if(message->message == WM_MOUSEMOVE)
    {
        input->x_pos = GET_X_LPARAM(message->lParam);
        input->y_pos = GET_Y_LPARAM(message->lParam);
    }
    
    if(key_is_down)
    {
        if(message->wParam == VK_RIGHT)
        {
            input->right_pressed = true;
        }
        if(message->wParam == VK_LEFT)
        {
            input->left_pressed = true;
        }
        if(message->wParam == VK_UP)
        {
            input->up_pressed = true;
        }
        if(message->wParam == VK_DOWN)
        {
            input->down_pressed = true;
        }
        
        if(message->wParam == VK_F4 &&
           alt_is_down)
        {
            running = false;
        }
    }
}

LRESULT CALLBACK 
Win32WindowProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) 
{
    LRESULT result = 0;
    
    switch(message) 
    {
        case WM_PAINT: 
        {
            PAINTSTRUCT paint_struct;
            HDC device_context = BeginPaint(window, &paint_struct);
            StretchDIBits(device_context,
                          0, 0, globalBackbuffer.width, globalBackbuffer.height,
                          0, 0, globalBackbuffer.width, globalBackbuffer.height,
                          globalBackbuffer.memory, &globalBackbuffer.info, DIB_RGB_COLORS, SRCCOPY);
            FillRect(device_context, &paint_struct.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
            EndPaint(window, &paint_struct);
            break;
        }
        
        case WM_DESTROY: 
        {
            PostQuitMessage(0);
            break;
        }
        
        default: 
        {
            return DefWindowProc(window, message, w_param, l_param);
        }
    }
    
    return result;
}

int WINAPI
WinMain(HINSTANCE instance, HINSTANCE prev_instance, 
        LPSTR cmd_line, int cmd_show)
{
    WNDCLASS window_class = {};
    
    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = Win32WindowProc;
    window_class.hInstance = instance;
    window_class.lpszClassName = "WindowClass";
    
    RegisterClass(&window_class);
    
    HWND window = CreateWindowEx(0, window_class.lpszClassName, "Window", 
                                 WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 0, 0, instance, 0);
    
    HDC device_context = GetDC(window);
    
    Win32ResizeDIBSection(&globalBackbuffer, 640, 427);
    
    vec3f color = { 1.0f, 0.0f, 0.0f };
    vec2f pos = { 500.0f, 300.0f };
    vec2f size = { 450.0f, 450.0f };
    
    Input input;
    
    FileContents file_contents = Win32ReadEntireFile("Jump (32x32).rgba");
    
    Bitmap bitmap;
    
    bitmap.header = *(BitmapHeader*)file_contents.buffer;
    u8* at = (u8*)file_contents.buffer;
    at += sizeof(BitmapHeader);
    bitmap.buffer = at;
    
    if(file_contents.buffer)
    {
        
    }
    
    if(window)
    {
        running = true;
        while(running)
        {
            MSG message = {};
            while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
            {
                if(message.message == WM_KEYDOWN ||
                   message.message == WM_SYSKEYDOWN || 
                   message.message == WM_KEYUP ||
                   message.message == WM_SYSKEYUP ||
                   message.message == WM_MOUSEMOVE)
                {
                    
                    Win32ProcessInput(&message, &input);
                }
                else
                {
                    if(message.message == WM_QUIT)
                    {
                        running = false;
                    }
                    
                    TranslateMessage(&message);
                    DispatchMessage(&message);
                }
            }
            
            
            DrawRectangle(&globalBackbuffer, {0.0f, 0.0f}, {1280.0f, 720.0f}, {1.0f, 1.0f, 0.0f});
            
#if 0            
            DrawRectangle(&globalBackbuffer, {(f32)input.x_pos,(f32)input.y_pos}, {(f32)input.x_pos + 50.0f, (f32)input.y_pos + 50.0f}, {1.0f, 0.0f, 0.0f});
#endif
            
            //DrawBitmap(&globalBackbuffer, {200.0f, 300.0f}, &bitmap);
            //DrawBitmap(&globalBackbuffer, {500.0f, 500.0f}, &bitmap1);
            DrawBitmap(&globalBackbuffer, {0.0f, 0.0f}, &bitmap);
            //DrawRectangle(&globalBackbuffer, pos, pos + size, color);
            
            Win32WindowDimension dimension = Win32GetWindowDimension(window);
            Win32DisplayBufferInWindow(device_context, 
                                       dimension.width, dimension.height,
                                       &globalBackbuffer);
            
            
            if(input.left_pressed)
            {
                pos.x -= 1.0f;
            }
            else if(input.right_pressed)
            {
                pos.x += 1.0f;
            }
            else if(input.up_pressed)
            {
                pos.y -= 1.0f;
            }
            else if(input.down_pressed)
            {
                pos.y += 1.0f;
            }
        }
    }
    
    return 0;
}