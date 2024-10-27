#pragma once

#include <functional>

#include "../../Core/Common.h"
#include "../../Core/Random.h"
#include "Wave.h"
#include "Note.h"


struct Oscillator
{
public:
    enum class Type
    {
        WAVE_SINE,
        WAVE_SQUARE,
        WAVE_TRIANGLE,
        WAVE_DIGI_SAWTOOTH,
        WAVE_ANLG_SAWTOOTH,
        NOISE_WHITE,
        CUSTOM,
    };

public:
    Oscillator(f64 volume = 1.0, s32 pitch = 0, Type waveform = Type::WAVE_SINE) 
        : m_wave(), m_volume(volume), m_pitch(pitch), m_waveform(waveform), m_output(0.0), m_custom(nullptr) {}

public:
    f64 GenerateWave(f64 time_step, note n)
    {
        m_wave.frequency = note_freq(n.id + m_pitch);
        f64 output = GenerateWave(time_step, m_wave.amplitude, m_wave.frequency);
        return output;
    }

    f64 GenerateWave(f64 time_step, f64 amp, f64 freq)
    {
        f64 phase = 2.0 * PI * freq * time_step;
 
        switch (m_waveform)
        {
        case Type::WAVE_SINE:
            m_output = amp * std::sin(phase);
            break;
        case Type::WAVE_SQUARE:
            m_output = amp * std::sin(phase) > 0 ? 1.0 : -1.0;
            break;
        case Type::WAVE_TRIANGLE:
            m_output = amp * std::asin(std::sin(phase));
            break;
        case Type::WAVE_DIGI_SAWTOOTH:
            m_output = amp * (2.0 / PI) * (freq * PI * fmod(time_step, 1.0 / freq) - (PI / 2.0));
            break;
        case Type::WAVE_ANLG_SAWTOOTH:
        {
            f64 N = 50.0;
            f64 acc = 0.0; 
            for (f64 n = 1.0; n < N; n++)
                acc += std::sin(phase * n) / n;
            m_output = acc * (2.0 / PI);
        } break;
        case Type::NOISE_WHITE:
            m_output = rand.normal(0.0, 1.0);
            break;
        case Type::CUSTOM:
            if (m_custom) m_output = m_custom(time_step);
            else          m_output = 0.0;
            break;
        default:
            m_output = 0.0;
        }

        f64 effective_volume = m_mute ? 0.0 : m_volume;
        m_output = std::clamp(m_output * effective_volume , -1.0, 1.0);

        return m_output;
    }

    void SetVolume(f64 amplitude) { m_volume = std::clamp(amplitude, 0.0, 1.0);  m_wave.SetAmplitude(amplitude); }
    void SetWaveform(Type w) { m_waveform = w; }

public:
    f64     m_volume;
    s32     m_pitch;
    f64     m_output;
    Type    m_waveform;
    Wave    m_wave;
    randf64 rand;
    std::function<f64(f64 time_step)> m_custom;
    bool m_mute = false;

    // Phase logic
    f64 phase_acc = 0.0;
    f64 phase = 0.0;
    f64 m_max_frequency = 20000.0;
};

static std::string wave_str(Oscillator::Type type)
{
    std::string n;
    switch (type)
    {
    case Oscillator::Type::WAVE_SINE:          n = "SINE";            break;
    case Oscillator::Type::WAVE_SQUARE:        n = "SQUARE";          break;
    case Oscillator::Type::WAVE_TRIANGLE:      n = "TRIANGLE";        break;
    case Oscillator::Type::WAVE_DIGI_SAWTOOTH: n = "SAWTOOTH";        break;
    case Oscillator::Type::WAVE_ANLG_SAWTOOTH: n = "ANALOG SAWTOOTH"; break;
    case Oscillator::Type::NOISE_WHITE:        n = "WHITE";           break;
    case Oscillator::Type::CUSTOM:             n = "CUSTOM";          break;
    }
    return n;
}


/*
// TODO: Do not work as intended

    f64 GenerateWavePhase(f64 time_step, f64 amp, f64 freq)
    {
        phase_inc = (2.0 * PI) * freq * (1.0 / SAMPLE_RATE);
        phase += phase_inc;

        if (phase > 2.0 * PI)
            phase -= 2.0 * PI;

        switch (m_waveform)
        {
        case Type::WAVE_SINE:
            m_output = amp * std::sin(phase);
            break;

        case Type::WAVE_SQUARE:
            m_output = amp * (std::sin(phase) > 0 ? 1.0 : -1.0);
            break;

        case Type::WAVE_TRIANGLE:
            m_output = amp * (2.0 / PI) * std::asin(std::sin(phase));
            break;

        case Type::WAVE_DIGI_SAWTOOTH:
            m_output = amp * (2.0 / PI) * (fmod(phase, 2.0 * PI) - (PI / 2.0));
            break;

        case Type::WAVE_ANLG_SAWTOOTH:
        {
            f64 N = 50.0;
            f64 acc = 0.0;
            for (f64 n = 1.0; n < N; ++n)
                acc += std::sin(phase * n) / n;
            m_output = amp * acc * (2.0 / PI);
        }
        break;

        case Type::NOISE_WHITE:
            m_output = rand.normal(0.0, 1.0);
            break;

        case Type::CUSTOM:
            if (m_custom)
                m_output = m_custom(phase);
            else
                m_output = 0.0;
            break;

        default:
            m_output = 0.0;
        }

        f64 effective_volume = m_mute ? 0.0 : m_volume;
        m_output = std::clamp(m_output * effective_volume, -1.0, 1.0);

        return m_output;
    }

*/