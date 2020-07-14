#include <Windows.h>
#include <Windowsx.h>
#include "Game.h"
#include "Utils.h"
#include "Math.h"
#include "Platform.h"
#include <stdio.h>

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


internal Win32GameCode
Win32LoadGameCode() 
{
    Win32GameCode result = {};
    
    result.game_code_dll = LoadLibrary("Game.dll");
    if (result.game_code_dll) 
    {
        result.UpdateAndRender =
            (GameUpdateAndRenderFunc *)(GetProcAddress(result.game_code_dll, "GameUpdateAndRender"));
        
        result.valid = (result.UpdateAndRender != 0);
    }
    
    if (!result.valid)
    {
        result.UpdateAndRender = GameUpdateAndRenderStub;
    }
    
    return result;
};


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
    
    Win32ResizeDIBSection(&globalBackbuffer, 1280, 720);
    
    vec3f color = { 1.0f, 0.0f, 0.0f };
    vec2f pos = { 500.0f, 300.0f };
    vec2f size = { 450.0f, 450.0f };
    
    Input input;
    
    FileContents file_contents = Win32ReadEntireFile("Jump (32x32).rgba");
    
    
    Bitmap bitmap;
    
    if(file_contents.buffer)
    {
        bitmap.header = *(BitmapHeader*)file_contents.buffer;
        u8* at = (u8*)file_contents.buffer;
        at += sizeof(BitmapHeader);
        bitmap.buffer = at;
    }
    
    MMRESULT result = timeBeginPeriod(1);
    if(result != TIMERR_NOERROR)
    {
        Assert(false);
    }
    
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    
    u64 starting_time_stamp_counter = __rdtsc();
    
    f32 target_frames_per_second = 60.0f;
    f32 target_seconds_per_frame = 1.0f / target_frames_per_second;
    f32 target_milliseconds_per_frame = 1000.0f * target_seconds_per_frame;
    
    
    
    if(window)
    {
        Win32GameCode game = Win32LoadGameCode();
        
        GameOffscreenBuffer buffer = {};
        buffer.memory = globalBackbuffer.memory;
        buffer.width = globalBackbuffer.width;
        buffer.height = globalBackbuffer.height;
        buffer.pitch = globalBackbuffer.pitch;
        
        GameMemory game_memory = {};
        game_memory.permanent_storage_size = Megabytes(64);
        game_memory.transient_storage_size = Gigabytes(1);
        
        running = true;
        
        LARGE_INTEGER starting_time;
        QueryPerformanceCounter(&starting_time);
        
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
            
            
            
#if 0            
            DrawRectangle(&globalBackbuffer, {(f32)input.x_pos,(f32)input.y_pos}, {(f32)input.x_pos + 50.0f, (f32)input.y_pos + 50.0f}, {1.0f, 0.0f, 0.0f});
#endif
            
            //DrawBitmap(&globalBackbuffer, {200.0f, 300.0f}, &bitmap);
            //DrawBitmap(&globalBackbuffer, {500.0f, 500.0f}, &bitmap1);
            //DrawBitmap(&globalBackbuffer, pos, &bitmap);
            //DrawRectangle(&globalBackbuffer, pos, pos + size, color);
            
            Win32WindowDimension dimension = Win32GetWindowDimension(window);
            Win32DisplayBufferInWindow(device_context, 
                                       dimension.width, dimension.height,
                                       &globalBackbuffer);
            
            
            game.UpdateAndRender(&game_memory, &buffer);
            
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
            
            u64 ending_time_stamp_counter = __rdtsc();
            
            LARGE_INTEGER ending_time;
            QueryPerformanceCounter(&ending_time);
            
            u64 counter_elapsed = ending_time.QuadPart - starting_time.QuadPart;
            f32 milliseconds_per_frame = ((f32)counter_elapsed * 1000.0f) / (f32)frequency.QuadPart;
            
            if(milliseconds_per_frame < target_milliseconds_per_frame)
            {
                DWORD sleep_milliseconds = (DWORD)(target_milliseconds_per_frame - milliseconds_per_frame);
                Sleep(sleep_milliseconds);
            }
            else
            {
                OutputDebugString("We missed frame rate!!!\n");
            }
            
            QueryPerformanceCounter(&ending_time);
            
            counter_elapsed = ending_time.QuadPart - starting_time.QuadPart;
            milliseconds_per_frame = ((f32)counter_elapsed * 1000.0f) / (f32)frequency.QuadPart;
            u64 cycles_elapsed = ending_time_stamp_counter - starting_time_stamp_counter;
            f32 frames_per_second = 1000.0f / milliseconds_per_frame;
            
            char time_measurment[256] = {};
            sprintf(time_measurment, "%.02f ms/f, %.02f f/s, %llu c/f\n", milliseconds_per_frame, frames_per_second, cycles_elapsed);
            OutputDebugString(time_measurment);
            
            starting_time = ending_time;
            starting_time_stamp_counter = ending_time_stamp_counter;
        }
    }
    
    return 0;
}