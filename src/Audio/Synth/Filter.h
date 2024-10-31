#pragma once

#include "../../Core/Common.h"
#include <algorithm>
#include <complex>
#include <vector>

// Filter: https://en.wikipedia.org/wiki/Filter_(signal_processing)
// MusicDSP Filters: https://www.musicdsp.org/en/latest/Filters/index.html
// DSP CPP Filters: https://github.com/dimtass/DSP-Cpp-filters

struct VAFilter
{
    enum class Type {
        LOW_PASS,
        BAND_PASS,
        HIGH_PASS,
        OFF
    } type;

    f64 frequency;
    f64 resonance;
    f64 sample_rate;

    // Coefficients
    f64 g = 0.0; // Gain
    f64 R = 0.0; // Damping
    f64 denom_inv = 0.0;

    f64 state_1 = 0.0;
    f64 state_2 = 0.0;

public:
    void CalcCoefs(f64 cutoff, f64 reso)
    {
        resonance = reso;
        frequency = cutoff;

        g = std::tan(PI * frequency * 1.0 / SAMPLE_RATE);
        R = std::min(1.0 - resonance, 0.999);
        denom_inv = 1.0 / (1.0 + (2.0 * R * g) + g * g);
    }

    void Reset()
    {
        state_1 = 0.0;
        state_2 = 0.0;
    }

    f64 FilterWave(f64 const& sample)
    {
        if (type == Type::OFF) return sample;

        f64 high_pass = (sample - (2.0 * R + g) * state_1 - state_2) * denom_inv;
        f64 band_pass = high_pass * g + state_1;
        f64 low_pass  = band_pass * g + state_2;

        state_1 = g * high_pass + band_pass;
        state_2 = g * band_pass + low_pass;

        switch (type)
        {
        case Type::LOW_PASS:  return low_pass;
        case Type::BAND_PASS: return band_pass;
        case Type::HIGH_PASS: return high_pass;
        }
    }

    f64 TransferFunction(f64 frequency)
    {
        f64 omega = 2.0 * PI * frequency / SAMPLE_RATE;
        f64 gain  = 0.0;

        switch (type)
        {
        case Type::LOW_PASS:
            gain = (g * g) / (1.0 + 2.0 * R * g + omega * omega);
            break;

        case Type::BAND_PASS:
            gain = (g * omega) / (omega * omega + 2.0 * R * omega + 1.0);
            break;

        case Type::HIGH_PASS:
            gain = 1.0 / (1.0 + 2.0 * R * g + omega * omega);
            break;

        case Type::OFF:
        default:
            gain = 1.0;
            break;
        }

        return gain;
    }
};

// Biquad Filter: https://en.wikipedia.org/wiki/Digital_biquad_filter
// DSP Filters: https://github.com/vinniefalco/DSPFilters
// sndfilters: https://github.com/velipso/sndfilter
// earlevel engineering: https://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
// BiQuadDesigner: https://arachnoid.com/BiQuadDesigner/index.html

//inline f64 db_to_volume(f64 dB) { return std::pow(10.0, dB / 20.0); }
inline f64 dB_to_volume(f64 dB) { return std::pow(10, 0.05 * dB); }
inline f64 volume_to_dB(f64 v) { return 20.0 * std::log10(v); }
inline f64 lerp(f64 a, f64 b, f64 t) { return a + (b - a) * t; }
inline s32 wrap(s32 value, s32 max)
{
    if (value < 0)    return value + max;
    if (value >= max) return value - max;
    return value;
}

struct BqFilter
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
        OFF,

    } type;

    f64 frequency;
    f64 resonance;
    f64 sample_rate;

    // Coefficients
    f64 b0 = 1.0, b1 = 1.0, b2 = 1.0, a1 = 1.0, a2 = 1.0;
    f64 x1 = 0.0, x2 = 0.0;
    f64 y1 = 0.0, y2 = 0.0;

    f64 output;

    void CalcCoefs(f64 cutoff, f64 reso, f64 gain_db = 0.0)
    {
        resonance = reso;
        frequency = cutoff;

        f64 omega = 2.0 * PI * frequency * 1.0 / SAMPLE_RATE;
        f64 sin_omega = std::sin(omega);
        f64 cos_omega = std::cos(omega);

        f64 gain = dB_to_volume(0.5 * gain_db);

        f64 alpha = sin_omega / (2.0 * std::max(reso, 0.001));
        f64 beta  = std::sqrt(2.0 * gain);

        f64 a0 = 1.0f;

        switch (type)
        {
        case Type::LOW_PASS:
            b0 = (1.0 - cos_omega) * 0.5;
            b1 = (1.0 - cos_omega);
            b2 = (1.0 - cos_omega) * 0.5;
            a0 =  1.0 + alpha;
            a1 = -2.0 * cos_omega;
            a2 =  1.0 - alpha;
            break;

        case Type::HIGH_PASS:
            b0 =  (1.0 + cos_omega) * 0.5;
            b1 = -(1.0 + cos_omega);
            b2 =  (1.0 + cos_omega) * 0.5;
            a0 =   1.0 + alpha;
            a1 =  -2.0 * cos_omega;
            a2 =   1.0 - alpha;
            break;

        case Type::BAND_PASS:
            b0 =  alpha;
            b1 =  0.0;
            b2 = -alpha;
            a0 =  1.0 + alpha;
            a1 = -2.0 * cos_omega;
            a2 =  1.0 - alpha;            
            break;

        case Type::ALL_PASS:
            b0 =  1.0 - alpha;
            b1 = -2.0 * cos_omega;
            b2 =  1.0 + alpha;
            a0 =  1.0 + alpha;
            a1 = -2.0 * cos_omega;
            a2 =  1.0 - alpha;
            break;

        case Type::PEAK:
            b0 =  1.0 + (alpha * gain);
            b1 = -2.0 * cos_omega;
            b2 =  1.0 - (alpha * gain);
            a0 =  1.0 + (alpha / gain);
            a1 = -2.0 * cos_omega;
            a2 =  1.0 - (alpha / gain);
            break;

        case Type::NOTCH:
            b0 =  1.0;
            b1 = -2.0 * cos_omega;
            b2 =  1.0;
            a0 =  1.0 + alpha;
            a1 = -2.0 * cos_omega;
            a2 =  1.0 - alpha;
            break;

        case Type::LOW_SHELF:
            b0 =       gain * ((gain + 1.0) - (gain - 1.0) * cos_omega + beta * sin_omega);
            b1 = 2.0 * gain * ((gain - 1.0) - (gain + 1.0) * cos_omega);
            b2 =       gain * ((gain + 1.0) - (gain - 1.0) * cos_omega - beta * sin_omega);
            a0 =               (gain + 1.0) + (gain - 1.0) * cos_omega + beta * sin_omega;
            a1 = -2.0       * ((gain - 1.0) + (gain + 1.0) * cos_omega);
            a2 =               (gain + 1.0) + (gain - 1.0) * cos_omega - beta * sin_omega;            
            break;

        case Type::HIGH_SHELF:
            b0 =         gain * ((gain + 1.0) + (gain - 1.0) * cos_omega + beta * sin_omega);
            b1 = -2.0 *  gain * ((gain - 1.0) + (gain + 1.0) * cos_omega);
            b2 =         gain * ((gain + 1.0) + (gain - 1.0) * cos_omega - beta * sin_omega);
            a0 =                 (gain + 1.0) - (gain - 1.0) * cos_omega + beta * sin_omega;
            a1 = 2.0          * ((gain - 1.0) - (gain + 1.0) * cos_omega);
            a2 =                 (gain + 1.0) - (gain - 1.0) * cos_omega - beta * sin_omega;
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
        x1 = 0.0;
        x2 = 0.0;
        y1 = 0.0;
        y2 = 0.0;
    }

    f64 FilterWave(f64 const& x) 
    {
        if (type == Type::OFF) return x;

        f64 y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;

        return y;
    }

    f64 TransferFunction(f64 freq)
    {
        f64 omega     = 2.0 * PI * freq / sample_rate;
        f64 cos_omega = std::cos(omega);
        f64 sin_omega = std::sin(omega);

        std::complex<f64> z1 = std::polar(1.0, -omega);
        std::complex<f64> z2 = std::polar(1.0, -2.0 * omega);

        std::complex<f64> numerator   = b0 + b1 * z1 + b2 * z2;
        std::complex<f64> denominator = 1.0 + a1 * z1 + a2 * z2;
        std::complex<f64> gain        = numerator / denominator;

        return std::abs(gain);
    }
};


// For reverb effect
struct CombFilter 
{
    std::vector<f64> history;
    s32 history_offset = 0;
    f64 feedback = 0.0;
    f64 damp = 0.0;

    f64 state = 0.0;

    void SetDelay(s32 delay_in_samples) 
    {
        history.resize(delay_in_samples);
        history_offset %= history.size();
    }

    f64 Process(const f64 x)
    {
        f64 y = history[history_offset];
        state = lerp(y, state, damp);

        history[history_offset] = x + feedback * state;
        history_offset = wrap(history_offset + 1, history.size());

        return y;
    }
};

struct AllPassFilter 
{
    std::vector<f64> history;
    s32 history_offset = 0;
    f64 feedback = 0.0;

    void SetDelay(s32 delay_in_samples) 
    {
        history.resize(delay_in_samples);
        history_offset %= history.size();
    }

    f64 Process(const f64 x)
    {
        f64 old = history[history_offset];
        f64 y = -x + old;

        history[history_offset] = x + feedback * old;
        history_offset = wrap(history_offset + 1, history.size());

        return y;
    }
};