#include "Synthesizer.h"

Synthesizer::Synthesizer()
{
    // Create oscillator
    Wave wave = Wave(1.0f, note_to_freq(9));
    oscillators["OSC1"] = Oscillator(wave);

    // TODO: where do we get sample blocks from?
    wave_data.times.resize(441, 0.0f);
    wave_data.samples.resize(441, 0.0f);
}

void Synthesizer::Update(f64 time)
{
	notes.erase(std::remove_if(notes.begin(), notes.end(), [](const note& n) { return !n.active; }), notes.end());
}

void Synthesizer::PlayToggle()
{
    m_playing = !m_playing;
}

bool Synthesizer::IsPlaying()
{
    return m_playing;
}

void Synthesizer::SetMasterVolume(f32 volume)
{
    m_master_volume = volume;
}

f32 Synthesizer::GetMasterVolume()
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

WaveData& const Synthesizer::GetWaveData()
{
    return wave_data;
}

std::vector<note>& Synthesizer::GetNotes()
{
    return notes;
}