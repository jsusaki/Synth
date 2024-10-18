#pragma once

#include "../../Core/Common.h"
#include "Wave.h"
#include "Note.h"
#include "Envelope.h"

#include "../../../ref/ADSR/ADSR.h"

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
        ANALOG_SAWTOOTH,
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
    f64 GenerateWave(f64 time_step, note n)
    {
        SetNote(n.id);

        switch (m_waveform)
        {
        case Type::SINE:
            m_output = m_wave.amplitude * std::sin(2.0 * PI * m_wave.frequency * time_step);
            break;
        case Type::SQUARE:
            m_output = m_wave.amplitude * std::sin(2.0 * PI * m_wave.frequency * time_step) > 0 ? 1.0 : -1.0;
            break;
        case Type::TRIANGLE:
            m_output = m_wave.amplitude * std::asin(std::sin(2.0 * PI * m_wave.frequency * time_step));
            break;
        case Type::SAWTOOTH:
            m_output = m_wave.amplitude * (2.0 / PI) * (m_wave.frequency * PI * fmod(time_step, 1.0f / m_wave.frequency) - (PI / 2.0f));
            break;
        case Type::ANALOG_SAWTOOTH:
        {
            f64 acc = 0.0;
            for (f64 n = 1.0; n < 50.0; n++)
                acc += (std::sin(2.0 * PI * m_wave.frequency * time_step * n)) / n;
            m_output = acc * (2.0 / PI);
        } break;
        default:
            m_output = 0.0;
        }

        m_output = std::clamp(m_output * volume, -1.0, 1.0);

        return m_output;
    }

    // TODO: Do not work as intended
    f64 GenerateWavePhase(f64 time_step, note n)
    {
        SetNote(n.id);

        //m_phase_acc += m_wave.frequency * time_step;
        m_phase_acc += m_wave.frequency / SAMPLE_RATE;
        if (m_phase_acc >= 2.0) m_phase_acc -= 2.0;

        switch (m_waveform)
        {
        case Type::SINE:
            m_output = m_wave.amplitude * std::sin(m_phase_acc * 2.0 * PI);
            break;
        case Type::SQUARE:
            m_output = m_wave.amplitude * (m_phase_acc >= 1.0) ? 1.0 : -1.0;
            break;
        case Type::TRIANGLE:
            m_output = m_wave.amplitude * (m_phase_acc < 1.0) ? (m_phase_acc * 0.5) : (1.0 - m_phase_acc * 0.5);
            break;
        case Type::SAWTOOTH:
            m_output = m_wave.amplitude * (m_phase_acc - 1.0) * 2.0;
            break;
        default:
            m_output = 0.0;
        }

        m_output = std::clamp(m_output * volume, -1.0, 1.0);

        return m_output;
    }

    void SetNote(s32 id) { m_wave.SetFrequency(note_to_freq(id)); }
    void SetVolume(f64 amplitude) { volume = std::clamp(amplitude, 0.0, 1.0);  m_wave.SetAmplitude(amplitude); }
    void SetWaveform(Type w) { m_waveform = w; }

public:
    f64 volume      = 1.0;
    f64 m_output    = 0.0;
    Type m_waveform = Type::SINE;
    Wave m_wave;
    //Noise m_noise_type = WHITE;

    f64 m_phase_acc = 0.0f;
    f64 m_max_frequency = 20000.0;

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
    case Oscillator::Type::ANALOG_SAWTOOTH: n = "ANALOG SAWTOOTH"; break;
    }
    return n;
}