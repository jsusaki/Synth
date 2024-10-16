#pragma once
#include "../../Core/Common.h"

struct Envelope
{
    f32 attack_time;       // time
    f32 decay_time;        // time
    f32 sustain_amplitude; // amplitude
    f32 release_time;      // time
    f32 start_amplitude;   // initial amplitude

    Envelope()
    {
        attack_time       = 1.3;
        decay_time        = 1.0;
        sustain_amplitude = 0.8;
        release_time      = 1.5;
        start_amplitude   = 1.0;
    }

    // TODO: State Machine?
    f32 Amplitude(const f32 time, const f32 time_on, const f32 time_off)
    {
        f32 amplitude_output = 0.0;
        f32 release_amplitude = 0.0;

        if (time_on > time_off) // Note is on
        {
            f32 lifetime = time - time_on;

            // Attack phase
            if (lifetime <= attack_time)
                amplitude_output = (lifetime / attack_time) * start_amplitude;
            // Decay phase
            else if (lifetime > attack_time && lifetime <= (attack_time + decay_time))
                amplitude_output = ((lifetime - attack_time) / decay_time) * (sustain_amplitude - start_amplitude) + start_amplitude;
            // Sustain phase
            else if (lifetime > (attack_time + decay_time))
                amplitude_output = sustain_amplitude;

        }
        else // Note is off
        {
            f32 lifetime = time_off - time_on;

            // Release in attack
            if (lifetime <= attack_time)
                release_amplitude = (lifetime / attack_time) * start_amplitude;
            // Release in decay
            else if (lifetime > attack_time && lifetime <= (attack_time + decay_time))
                release_amplitude = ((lifetime - attack_time) / decay_time) * (sustain_amplitude - start_amplitude) + start_amplitude;
            // Release in sustain
            else if (lifetime > (attack_time + decay_time))
                release_amplitude = sustain_amplitude;

            // Release phase
            amplitude_output = ((time - time_off) / release_time) * (0.0 - release_amplitude) + release_amplitude;
        }
        
        // Amplitude should not be negative
        amplitude_output = std::clamp(amplitude_output, 0.0f, 1.0f);
        return amplitude_output;
    }

};