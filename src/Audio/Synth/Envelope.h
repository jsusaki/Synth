#pragma once

#include "../../Core/Common.h"

// Envelope ADSR: https://en.wikipedia.org/wiki/Envelope_(music)
// olc-synth: https://github.com/OneLoneCoder/synth/blob/master/main2.cpp
// earlevel engineering: https://www.earlevel.com/main/2013/06/03/envelope-generators-adsr-code/

struct Envelope
{
    f64 attack_time;       // time
    f64 decay_time;        // time
    f64 sustain_amplitude; // amplitude
    f64 release_time;      // time
    f64 start_amplitude;   // initial amplitude

    enum class Decay : u8
    {
        LINEAR,
        EXPONENTIAL,
        QUADRATIC,
    } decay_function;

    f64 CalculateDecay(f64 normalized_time, f64 start_amplitude, Decay mode)
    {
        switch (mode)
        {
        case Decay::LINEAR:      return start_amplitude * normalized_time;
        case Decay::EXPONENTIAL: return start_amplitude * (1.0 - std::exp(-5.0 * normalized_time));
        case Decay::QUADRATIC:   return start_amplitude * std::pow(normalized_time, 2.0);
        default:                 return start_amplitude * normalized_time;
        }
    }

    // TODO: State Machine Approach?
    f64 GenerateAmplitude(const f64 time_step, const f64 time_on, const f64 time_off)
    {
        f64 amplitude_output  = 0.0;
        f64 release_amplitude = 0.0;

        if (time_on > time_off) // Note on
        {
            f64 lifetime = time_step - time_on;

            // Attack phase
            if (lifetime <= attack_time)
            {
                f64 nt = lifetime / attack_time;
                amplitude_output = CalculateDecay(nt, start_amplitude, decay_function);
            }
            // Decay phase
            else if (lifetime > attack_time && lifetime <= (attack_time + decay_time))
            {
                f64 nt = (lifetime - attack_time) / decay_time;
                f64 a  = sustain_amplitude - start_amplitude;
                amplitude_output = CalculateDecay(nt, a, decay_function) + start_amplitude;
            }
            // Sustain phase
            else if (lifetime > (attack_time + decay_time))
            {
                amplitude_output = sustain_amplitude;
            }

        }
        else // Note off
        {
            f64 lifetime = time_off - time_on;

            // Release in attack
            if (lifetime <= attack_time)
            {
                f64 nt = lifetime / attack_time;
                release_amplitude = CalculateDecay(nt, start_amplitude, decay_function);
            }
            // Release in decay
            else if (lifetime > attack_time && lifetime <= (attack_time + decay_time))
            {
                f64 nt = (lifetime - attack_time) / decay_time;
                f64 a  = (sustain_amplitude - start_amplitude);
                release_amplitude = CalculateDecay(nt, a, decay_function) + start_amplitude;
            }
            // Release in sustain
            else if (lifetime > (attack_time + decay_time))
            {
                release_amplitude = sustain_amplitude;
            }

            // Release phase
            f64 nt = (time_step - time_off) / release_time;
            f64 a = 0.0 - release_amplitude;
            amplitude_output = CalculateDecay(nt, a, decay_function) + release_amplitude;
        }
        
        // Amplitude should not be negative
        amplitude_output = std::clamp(amplitude_output, 0.0, 1.0);
        return amplitude_output;
    }
};