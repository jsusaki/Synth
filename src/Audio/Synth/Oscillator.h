#pragma once
#include "../../Core/Common.h"
#include "Wave.h"
#include "Note.h"
#include "Envelope.h"

// Multi function oscillator
struct Oscillator
{
public:
    Oscillator() {}
    Oscillator(Wave w) : m_wave(w) {}

public:
    enum class Type
    {
        SINE,
        SQUARE,
        TRIANGLE,
        SAWTOOTH,
    };

    // TODO: Add Noises
    enum Noise
    {
        WHITE,
        PINK,
        BROWNIAN
    };

public:
    // TODO: custom function

    f32 GenerateWave(f32 time, note n)
    {
        SetNote(n.id);

        switch (m_waveform)
        {
        case Type::SINE:
            m_output = m_wave.amplitude * std::sin(2.0f * PI * m_wave.frequency * time);
            break;
        case Type::SQUARE:
            m_output = m_wave.amplitude * std::sin(2.0f * PI * m_wave.frequency * time) > 0 ? 1.0 : -1.0;
            break;
        case Type::TRIANGLE:
            m_output = m_wave.amplitude * std::asin(std::sin(2.0f * PI * m_wave.frequency * time));
            break;
        case Type::SAWTOOTH:
            m_output = m_wave.amplitude * (2.0f / PI) * (m_wave.frequency * PI * fmod(time, 1.0f / m_wave.frequency) - (PI / 2.0f));
            break;
        default:
            m_output = 0.0;
        }

        return m_output;
    }

    f32 GenerateSound(f32 time, note n, bool& note_finished)
    {
        f32 envAmp = m_envelope.Amplitude(time, n.on, n.off);
        if (envAmp <= 0.0) note_finished = true;
        f32 sound  = GenerateWave(time - n.on, n);

        f32 output = std::clamp(envAmp * sound * volume, -1.0f, 1.0f);
        return output;
    }

    void SetNote(s32 id) { m_wave.SetFrequency(note_to_freq(id)); }
    void SetVolume(f32 amplitude) { volume = std::clamp(amplitude, 0.0f, 1.0f);  m_wave.SetAmplitude(amplitude); }
    void SetWaveform(Type w) { m_waveform = w; }

public:
    Type m_waveform = Type::SINE;
    Wave m_wave;
    f32 m_output = 0.0;
    f32 volume   = 1.0;
    Envelope m_envelope;

    //Noise m_noise_type = WHITE;
};

static std::string wave_str(Oscillator::Type type)
{
    std::string n;
    switch (type)
    {
    case Oscillator::Type::SINE:     n = "SINE";     break;
    case Oscillator::Type::SQUARE:   n = "SQUARE";   break;
    case Oscillator::Type::TRIANGLE: n = "TRIANGLE"; break;
    case Oscillator::Type::SAWTOOTH: n = "SAWTOOTH"; break;
    }
    return n;
}