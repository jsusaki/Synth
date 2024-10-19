#include "Synthesizer.h"

Synthesizer::Synthesizer()
{
    // Synth Config
    m_master_volume = 0.2;
    m_max_frequency = 20000.0;

    oscillators["OSC1"] = Oscillator(0.8,  0, Oscillator::Type::WAVE_SINE);
    oscillators["OSC2"] = Oscillator(0.3, 12, Oscillator::Type::WAVE_ANLG_SAWTOOTH);
    oscillators["OSC3"] = Oscillator(0.1, 24, Oscillator::Type::WAVE_ANLG_SAWTOOTH);

    m_envelope = {
        .attack_time       = 0.6,
        .decay_time        = 1.0,
        .sustain_amplitude = 0.8,
        .release_time      = 1.5,
        .start_amplitude   = 1.0,
        .decay_function    = Envelope::Decay::EXPONENTIAL
    };

    m_filter = {
        .type             = Filter::Type::LOW_PASS,
        .cutoff_frequency = 1000.0,
        .resonance        = 0.7,
        .sample_rate      = SAMPLE_RATE,
    };

    wave_data.times.resize(SAMPLE_RATE/100, 0.0);
    wave_data.samples.resize(SAMPLE_RATE/100, 0.0);
}

f64 Synthesizer::Synthesize(f64 time_step, note n, bool& note_finished)
{
    // Envelope
    f64 envelope_amplitude = m_envelope.Amplitude(time_step, n.on, n.off);
    if (envelope_amplitude <= 0.0001)
        note_finished = true;
    // Oscillator
    // if (n.channel == 0)
    f64 sound_mixed = 0.0;
    for (auto& [id, osc] : oscillators)
    {
        // Generate wave
        f32 sound = osc.GenerateWave(time_step, n);

        // Filter
        //sound = m_filter.FilterWave(sound);

        // TODO: Low Frequency Oscillator

        // TODO:  Effects

        sound_mixed += sound;
    }

    f64 output = std::clamp(envelope_amplitude * sound_mixed * m_master_volume, -1.0, 1.0);

    return output;
}

void Synthesizer::Update(f64 time)
{
	notes.erase(std::remove_if(notes.begin(), notes.end(), [](const note& n) { return !n.active; }), notes.end());
}

void Synthesizer::TogglePlay()
{
    m_playing = !m_playing;
}

bool Synthesizer::IsPlaying()
{
    return m_playing;
}

void Synthesizer::SetMasterVolume(f64 volume)
{
    m_master_volume = volume;
}

f64 Synthesizer::GetMasterVolume()
{
    return m_master_volume;
}

Oscillator& Synthesizer::GetOscillator(std::string id)
{
    return oscillators[id];
}

std::unordered_map<std::string, Oscillator>& Synthesizer::GetOscillators()
{
    return oscillators;
}

const WaveData& Synthesizer::GetWaveData()
{
    return wave_data;
}

void Synthesizer::UpdateWaveData(u32 frame, f64 sample)
{
    wave_data.samples[frame] = sample;
}

std::vector<note>& Synthesizer::GetNotes()
{
    return notes;
}