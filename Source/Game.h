#ifndef UNTITLED_GAME_H
#define UNTITLED_GAME_H

#include "Utils.h"

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

#endif //UNTITLED_GAME_H
