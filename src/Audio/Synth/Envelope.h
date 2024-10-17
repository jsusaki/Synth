#pragma once
#include "../../Core/Common.h"

struct Envelope
{
    f32 attack_time;       // time
    f32 decay_time;        // time
    f32 sustain_amplitude; // amplitude
    f32 release_time;      // time
    f32 start_amplitude;   // initial amplitude

    enum class Decay : u8
    {
        LINEAR,
        QUADRATIC,
        EXPONENTIAL,
    };

    Decay attack_phase;
    Decay decay_phase;
    Decay release_phase;

    f32 CalculateDecay(f32 normalized_time, f32 start_amplitude, Decay mode)
    {
        switch (mode)
        {
        case Decay::LINEAR:      return start_amplitude * normalized_time;
        case Decay::EXPONENTIAL: return start_amplitude * (1.0f - std::expf(-5.0f * normalized_time));
        case Decay::QUADRATIC:   return start_amplitude * std::pow(normalized_time, 2);
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
    f32 Amplitude(const f32 time, const f32 time_on, const f32 time_off)
    {
        f32 amplitude_output  = 0.0;
        f32 release_amplitude = 0.0;

        if (time_on > time_off) // Note is on
        {
            f32 lifetime = time - time_on;

            // Attack phase
            if (lifetime <= attack_time)
            {
                f32 nt = lifetime / attack_time;
                amplitude_output = CalculateDecay(nt, start_amplitude, attack_phase);
            }
            // Decay phase
            else if (lifetime > attack_time && lifetime <= (attack_time + decay_time))
            {
                f32 nt = (lifetime - attack_time) / decay_time;
                f32 a  = sustain_amplitude - start_amplitude;
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
            f32 lifetime = time_off - time_on;

            // Release in attack
            if (lifetime <= attack_time)
            {
                f32 nt = lifetime / attack_time;
                release_amplitude = CalculateDecay(nt, start_amplitude, attack_phase);
            }
            // Release in decay
            else if (lifetime > attack_time && lifetime <= (attack_time + decay_time))
            {
                f32 nt = (lifetime - attack_time) / decay_time;
                f32 a  = (sustain_amplitude - start_amplitude);
                release_amplitude = CalculateDecay(nt, a, decay_phase) + start_amplitude;
            }
            // Release in sustain
            else if (lifetime > (attack_time + decay_time))
            {
                release_amplitude = sustain_amplitude;
            }

            // Release phase
            f32 nt = (time - time_off) / release_time;
            f32 a  = 0.0 - release_amplitude;
            amplitude_output = CalculateDecay(nt, a, release_phase) + release_amplitude;
        }
        
        // Amplitude should not be negative
        amplitude_output = std::clamp(amplitude_output, 0.0f, 1.0f);
        return amplitude_output;
    }
};