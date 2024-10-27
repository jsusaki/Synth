#pragma once
#include "../../Core/Common.h"
#include <glfw3.h>

struct note
{
    s32 id = 0;     // Note in scale
    f64 on = 0.0;   // Time note was activated
    f64 off = 0.0;  // Time note was deactivated
    s32 channel = 0;
    bool active = false;
};

// https://pages.mtu.edu/~suits/NoteFreqCalcs.html
// f_n = f_0 * 2^1/12 ^ n
//static f64 note_freq(s32 note) { return 440.0f * std::pow(std::pow(2.0f, 1.0f/12.0f), note); } // A4 = 440Hz

// https://www.music.mcgill.ca/~gary/307/week1/node28.html
// MIDI 128 notes mapping formula: f_n = 440 * 2^ (n-69)/12
static f64 note_freq(s32 note) { return 440.0 * std::pow(2.0, (note - 69) / 12.0); } // A4 = 440Hz

#undef max
static u32 closest_note_from_frequency(f64 freq)
{
    f64 min_diff = std::numeric_limits<f64>::max();
    u32 closest_note_id = 0;
    for (s32 i = 0; i <= 127; i++)
    {
        f64 n_freq = note_freq(i);
        f64 diff = std::abs(freq - n_freq);
        if (diff < min_diff)
        {
            min_diff = diff;
            closest_note_id = i;
        }
    }
    return closest_note_id;
}

// MIDI note representation
static std::string note_name(u32 note)
{
    static const std::string tones[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    s32 tone      = note % 12;
    s32 octave    = note / 12;
    std::string n = tones[tone] + std::to_string(octave);
    return n;
}

enum class Note 
{
    C       = 0,
    C_SHARP = 1,
    D       = 2,
    D_SHARP = 3,
    E       = 4,
    F       = 5,
    F_SHARP = 6,
    G       = 7,
    G_SHARP = 8,
    A       = 9,
    A_SHARP = 10,
    B       = 11
};

static s32 note_id(Note note, s32 octave) { return octave * 12 + static_cast<s32>(note); }

static s32 scancode_to_note(s32 scancode)
{
    switch (scancode)
    {
    case GLFW_KEY_Z:         return note_id(Note::C,       3);
    case GLFW_KEY_S:         return note_id(Note::C_SHARP, 3);
    case GLFW_KEY_X:         return note_id(Note::D,       3);
    case GLFW_KEY_D:         return note_id(Note::D_SHARP, 3);
    case GLFW_KEY_C:         return note_id(Note::E,       3);
    case GLFW_KEY_V:         return note_id(Note::F,       3);
    case GLFW_KEY_G:         return note_id(Note::F_SHARP, 3);
    case GLFW_KEY_B:         return note_id(Note::G,       3);
    case GLFW_KEY_H:         return note_id(Note::G_SHARP, 3);
    case GLFW_KEY_N:         return note_id(Note::A,       3);
    case GLFW_KEY_J:         return note_id(Note::A_SHARP, 3);
    case GLFW_KEY_M:         return note_id(Note::B,       3);
    case GLFW_KEY_COMMA:     return note_id(Note::C,       4);
    case GLFW_KEY_L:         return note_id(Note::C_SHARP, 4);
    case GLFW_KEY_PERIOD:    return note_id(Note::D,       4);
    case GLFW_KEY_SEMICOLON: return note_id(Note::D_SHARP, 4);
    case GLFW_KEY_SLASH:     return note_id(Note::E,       4);
    }
}