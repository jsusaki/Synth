#pragma once
#include "../../Core/Common.h"

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
        QUADRATIC,
        EXPONENTIAL,
    };

    Decay attack_phase;
    Decay decay_phase;
    Decay release_phase;

    f64 CalculateDecay(f64 normalized_time, f64 start_amplitude, Decay mode)
    {
        switch (mode)
        {
        case Decay::LINEAR:      return start_amplitude * normalized_time;
        case Decay::QUADRATIC:   return start_amplitude * std::pow(normalized_time, 2.0);
        case Decay::EXPONENTIAL: return start_amplitude * (1.0 - std::exp(-5.0 * normalized_time));
        default:                 return start_amplitude * normalized_time;
        }
    }

    Envelope()
    {
        attack_time       = 1.3;
        decay_time        = 1.0;
        sustain_amplitude = 0.8;
        release_time      = 1.5;
        start_amplitude   = 1.0;

        attack_phase  = Decay::LINEAR;
        decay_phase   = Decay::LINEAR;
        release_phase = Decay::LINEAR;
    }

    // TODO: State Machine Approach?
    f64 Amplitude(const f64 time_step, const f64 time_on, const f64 time_off)
    {
        f64 amplitude_output  = 0.0;
        f64 release_amplitude = 0.0;

        if (time_on > time_off) // Note is on
        {
            f64 lifetime = time_step - time_on;

            // Attack phase
            if (lifetime <= attack_time)
            {
                f64 nt = lifetime / attack_time;
                amplitude_output = CalculateDecay(nt, start_amplitude, attack_phase);
            }
            // Decay phase
            else if (lifetime > attack_time && lifetime <= (attack_time + decay_time))
            {
                f64 nt = (lifetime - attack_time) / decay_time;
                f64 a  = sustain_amplitude - start_amplitude;
                amplitude_output = CalculateDecay(nt, a, decay_phase) + start_amplitude;
            }
            // Sustain phase
            else if (lifetime > (attack_time + decay_time))
            {
                amplitude_output = sustain_amplitude;
            }

        }
        else // Note is off
        {
            f64 lifetime = time_off - time_on;

            // Release in attack
            if (lifetime <= attack_time)
            {
                f64 nt = lifetime / attack_time;
                release_amplitude = CalculateDecay(nt, start_amplitude, attack_phase);
            }
            // Release in decay
            else if (lifetime > attack_time && lifetime <= (attack_time + decay_time))
            {
                f64 nt = (lifetime - attack_time) / decay_time;
                f64 a  = (sustain_amplitude - start_amplitude);
                release_amplitude = CalculateDecay(nt, a, decay_phase) + start_amplitude;
            }
            // Release in sustain
            else if (lifetime > (attack_time + decay_time))
            {
                release_amplitude = sustain_amplitude;
            }

            // Release phase
            f64 nt = (time_step - time_off) / release_time;
            f64 a = 0.0 - release_amplitude;
            amplitude_output = CalculateDecay(nt, a, release_phase) + release_amplitude;
        }
        
        // Amplitude should not be negative
        amplitude_output = std::clamp(amplitude_output, 0.0, 1.0);
        return amplitude_output;
    }
};