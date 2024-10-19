#pragma once

#include "../../Core/Common.h"

// Filter: https://en.wikipedia.org/wiki/Filter_(signal_processing)
// Biquad Filter: https://en.wikipedia.org/wiki/Digital_biquad_filter
// MusicDSP Filters: https://www.musicdsp.org/en/latest/Filters/index.html
// DSP CPP Filters: https://github.com/dimtass/DSP-Cpp-filters
// DSP Filters: https://github.com/vinniefalco/DSPFilters
// sndfilters: https://github.com/velipso/sndfilter

// earlevel engineering: https://www.earlevel.com/main/2012/11/26/biquad-c-source-code/

struct Filter
{
    Filter(f64 sr = SAMPLE_RATE)
    {
        sample_rate = sr;
    }

    enum class Type 
    { 
        LOW_PASS, 
        HIGH_PASS, 
        BAND_PASS 
    };

    Type type = Type::LOW_PASS;
    f64 cutoff_frequency = 1000.0;  // Hz
    f64 resonance = 0.7;            // Resonance (Q)
    f64 sample_rate = 44100.0;

    // Coefficients

    void Compute(Type filter_type, f64 cutoff, f64 reso)
    {
        type = filter_type;
        cutoff_frequency = cutoff;
        resonance = reso;

        switch (type)
        {
        case Type::LOW_PASS:
            break;

        case Type::HIGH_PASS:
            break;

        case Type::BAND_PASS:
            break;
        }
    }

    f64 FilterWave(f64 input)
    {
        f64 output = 0.0;
        return output;
    }
};