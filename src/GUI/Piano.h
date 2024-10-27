// Based on: https://github.com/shric/midi
#pragma once
#include <vector>
#include "../Core/Common.h"

class Piano 
{
public:
    void up(s32 key);
    void down(s32 key, s32 velocity);
    void Render();
    std::vector<s32> current_notes();
public:
    s32 key_states[256] = { 0 };
};