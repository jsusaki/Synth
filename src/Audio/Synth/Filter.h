#pragma once

#include "../../Core/Common.h"
#include <algorithm>
#include <complex>

// Filter: https://en.wikipedia.org/wiki/Filter_(signal_processing)
// Biquad Filter: https://en.wikipedia.org/wiki/Digital_biquad_filter
// MusicDSP Filters: https://www.musicdsp.org/en/latest/Filters/index.html
// DSP CPP Filters: https://github.com/dimtass/DSP-Cpp-filters
// DSP Filters: https://github.com/vinniefalco/DSPFilters
// sndfilters: https://github.com/velipso/sndfilter
// earlevel engineering: https://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
// BiQuadDesigner: https://arachnoid.com/BiQuadDesigner/index.html

inline f64 db_to_linear(f64 x) { return std::pow(10.0, x / 20.0); }

struct Filter
{
    enum class Type 
    { 
        LOW_PASS, 
        HIGH_PASS, 
        BAND_PASS,
        ALL_PASS,
        PEAK,
        NOTCH,
        LOW_SHELF,
        HIGH_SHELF,

    } type;

    f64 frequency;
    f64 resonance;
    f64 sample_rate;
    f64 output;

    // Coefficients
    f64 b0 = 1.0f, b1 = 1.0f, b2 = 1.0f, a1 = 1.0f, a2 = 1.0f;

    f64 x1 = { }, x2 = { };
    f64 y1 = { }, y2 = { };

    void ComputeCoefficients(f64 freq, f64 reso, f64 gain_db = 0.0)
    {
        f64 omega = 2.0 * PI * freq * 1.0 / SAMPLE_RATE;
        f64 sin_omega = std::sin(omega);
        f64 cos_omega = std::cos(omega);

        f64 gain = db_to_linear(0.5f * gain_db);

        f64 alpha = sin_omega / (2.0f * std::max(reso, 0.001));
        f64 beta = std::sqrt(2.0f * gain);

        f64 a0 = 1.0f;

        switch (type)
        {
        case Type::LOW_PASS:
            b0 = (1.0f - cos_omega) * 0.5f;
            b1 = (1.0f - cos_omega);
            b2 = (1.0f - cos_omega) * 0.5f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;

        case Type::HIGH_PASS:
            b0 = (1.0f + cos_omega) * 0.5f;
            b1 = -(1.0f + cos_omega);
            b2 = (1.0f + cos_omega) * 0.5f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;

        case Type::BAND_PASS:
            b0 = alpha;
            b1 = 0.0f;
            b2 = -alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;            
            break;

        case Type::ALL_PASS:
            b0 = 1.0f - alpha;
            b1 = -2.0f * cos_omega;
            b2 = 1.0f + alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;

        case Type::PEAK:
            b0 = 1.0f + (alpha * gain);
            b1 = -2.0f * cos_omega;
            b2 = 1.0f - (alpha * gain);
            a0 = 1.0f + (alpha / gain);
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - (alpha / gain);
            break;

        case Type::NOTCH:
            b0 = 1.0f;
            b1 = -2.0f * cos_omega;
            b2 = 1.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;

        case Type::LOW_SHELF:
            b0 = gain * ((gain + 1.0f) - (gain - 1.0f) * cos_omega + beta * sin_omega);
            b1 = 2.0f * gain * ((gain - 1.0f) - (gain + 1.0f) * cos_omega);
            b2 = gain * ((gain + 1.0f) - (gain - 1.0f) * cos_omega - beta * sin_omega);
            a0 = (gain + 1.0f) + (gain - 1.0f) * cos_omega + beta * sin_omega;
            a1 = -2.0f * ((gain - 1.0f) + (gain + 1.0f) * cos_omega);
            a2 = (gain + 1.0f) + (gain - 1.0f) * cos_omega - beta * sin_omega;            
            break;

        case Type::HIGH_SHELF:
            b0 = gain * ((gain + 1.0f) + (gain - 1.0f) * cos_omega + beta * sin_omega);
            b1 = -2.0f * gain * ((gain - 1.0f) + (gain + 1.0f) * cos_omega);
            b2 = gain * ((gain + 1.0f) + (gain - 1.0f) * cos_omega - beta * sin_omega);
            a0 = (gain + 1.0f) - (gain - 1.0f) * cos_omega + beta * sin_omega;
            a1 = 2.0f * ((gain - 1.0f) - (gain + 1.0f) * cos_omega);
            a2 = (gain + 1.0f) - (gain - 1.0f) * cos_omega - beta * sin_omega;
            break;
        }

        // Normalize
        b0 /= a0;
        b1 /= a0;
        b2 /= a0;
        a1 /= a0;
        a2 /= a0;
    }

    void Reset() 
    {
        x1 = { };
        x2 = { };
        y1 = { };
        y2 = { };
    }

    f64 FilterWave(f64 const& x) {
        f64 y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;

        return y;
    }
};