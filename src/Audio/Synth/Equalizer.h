#pragma once
#include "../../Core/Common.h"
#include "Filter.h"

static f64 log_interpolate(f64 a, f64 b, f64 t) 
{
    return std::pow(a, 1.0 - t) * std::pow(b, t);
}

static const f64 MAX_Q = 10.0;
static const s32 NUM_BANDS = 4;

struct Equalizer
{
    struct Band
    {
        s32 mode = 0;

        f64 frequency;
        f64 resonance = 0.1;
        f64 gain;

        BqFilter filter;

        Band(f64 freq) : frequency(freq) {}
    };

    Band bands[NUM_BANDS] = { 
        63.0, 
        294.0, 
        1363.0, 
        6324.0 
    };

    f64 Process(f64 sample)
    {
        f64 output;
        for (auto& band : bands) 
        {
            BqFilter::Type type;
            switch (band.mode) 
            {
            case 0: type = BqFilter::Type::LOW_PASS;   break;
            case 1: type = BqFilter::Type::HIGH_PASS;  break;
            case 2: type = BqFilter::Type::BAND_PASS;  break;
            case 3: type = BqFilter::Type::PEAK;       break;
            case 4: type = BqFilter::Type::NOTCH;      break;
            case 5: type = BqFilter::Type::LOW_SHELF;  break;
            case 6: type = BqFilter::Type::HIGH_SHELF; break;
            }

            band.filter.type = type;
            band.filter.CalcCoefs(band.frequency, band.resonance, band.gain);
        }

        for (auto& band : bands)
            output = band.filter.FilterWave(sample);

        return output;
    }
};