#pragma once
#include "../../Core/Common.h"

struct note
{
    s32 id = 0;    // Note in scale
    f64 on = 0.0;  // Time note was activated
    f64 off = 0.0;  // Time note was deactivated
    s32 channel = 0;
    bool active = false;
};

// https://pages.mtu.edu/~suits/NoteFreqCalcs.html
// f_n = f_0 * 2^1/12 ^ n
//static f32 note_to_freq(s32 note) { return 440.0f * std::pow(std::pow(2.0f, 1.0f/12.0f), note); } // A4 = 440Hz

// https://www.music.mcgill.ca/~gary/307/week1/node28.html
// MIDI 128 notes mapping formula: f_n = 440 * 2^ (n-69)/12
static f32 note_to_freq(s32 note) { return 440.0f * std::pow(2.0f, (note - 69) / 12.0f); } // A4 = 440Hz

#undef max
static u32 closest_note_from_frequency(f32 freq)
{
    f32 min_diff = std::numeric_limits<f32>::max();
    u32 closest_note_id = 0;
    for (s32 i = 0; i <= 127; i++)
    {
        f32 note_freq = note_to_freq(i);
        f32 diff = std::abs(freq - note_freq);
        if (diff < min_diff)
        {
            min_diff = diff;
            closest_note_id = i;
        }
    }

    return closest_note_id;
}

// MIDI note representation
static std::string note_str(u32 note)
{
    static const std::string note_names[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    std::string n = note_names[note % 12];
    s32 octave = (note / 12) - 1;
    n += std::to_string(octave);
    return n;
}
