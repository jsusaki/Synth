#include "Synthesizer.h"

Synthesizer::Synthesizer()
{
    // Create oscillator
    Wave wave = Wave(1.0, note_to_freq(9));
    oscillators["OSC1"] = Oscillator(wave);
    oscillators["OSC2"] = Oscillator(wave);
    oscillators["OSC3"] = Oscillator(wave);

    m_filter = Filter(SAMPLE_RATE);
    m_filter.Compute(Filter::Type::LOW_PASS, 1000.0, 0.7);

    // TODO: where do we get sample blocks from?
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
        f32 sound = osc.GenerateWave(time_step - n.on, n);

        // Filter
        sound = m_filter.FilterWave(sound);

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