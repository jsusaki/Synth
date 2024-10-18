#pragma once
#include "../../Core/Common.h"
#include <algorithm>

struct Wave
{
    f64 amplitude;
    f64 frequency;

    Wave(f64 amp = 1.0, f64 freq = 110.0)
    {
        SetAmplitude(amp);
        SetFrequency(freq);
    }

    void SetAmplitude(f64 amp)  { amplitude = std::clamp(amp, -1.0, 1.0); }
    void SetFrequency(f64 freq) { frequency = std::clamp(freq, 0.0, 20000.0); }
};