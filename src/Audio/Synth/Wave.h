#pragma once
#include "../../Core/Common.h"
#include <algorithm>

struct Wave
{
    f32 amplitude;
    f32 frequency;

    Wave(f32 amp = 1.0f, f32 freq = 110.0f)
    {
        SetAmplitude(amp);
        SetFrequency(freq);
    }

    void SetAmplitude(f32 amp)  { amplitude = std::clamp(amp, -1.0f, 1.0f); }
    void SetFrequency(f32 freq) { frequency = std::clamp(freq, 0.0f, 20000.0f); }
};