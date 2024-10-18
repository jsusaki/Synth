#include "Synthesizer.h"

Synthesizer::Synthesizer()
{
    // Create oscillator
    Wave wave = Wave(1.0, note_to_freq(9));
    oscillators["OSC1"] = Oscillator(wave);

    // TODO: where do we get sample blocks from?
    wave_data.times.resize(SAMPLE_RATE/100, 0.0);
    wave_data.samples.resize(SAMPLE_RATE/100, 0.0);

#ifdef ENVELOPE_TEST
    // Configure ADSR
    m_adsr.SetAttackRate(1.5   * SAMPLE_RATE);
    m_adsr.SetDecayRate(0.8    * SAMPLE_RATE);
    m_adsr.SetSustainLevel(1.0 * SAMPLE_RATE);
    m_adsr.SetReleaseRate(1.3  * SAMPLE_RATE);
#endif    
}

f64 Synthesizer::Synthesize(f64 time_step, note n, bool& note_finished)
{
#ifdef ENVELOPE_TEST
    if (n.on > n.off) m_adsr.Gate(1);
    else              m_adsr.Gate(0);
    f64 envelope_amplitude = m_adsr.Process();
#else
    // Envelope
    f64 envelope_amplitude = m_envelope.Amplitude(time_step, n.on, n.off);
    if (envelope_amplitude <= 0.0) note_finished = true;
#endif    

    // Oscillator
    // if (n.channel == 0)
    f64 sound = oscillators["OSC1"].GenerateWave(time_step - n.on, n);

    // Low Frequency Oscillator

    // Filter

    // Effects

    f64 output = std::clamp(envelope_amplitude * sound * m_master_volume, -1.0, 1.0);

    return output;
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

WaveData& const Synthesizer::GetWaveData()
{
    return wave_data;
}

void Synthesizer::UpdateWaveData(u32 frame, f64 sample)
{
    // Collect data
    wave_data.wave.amplitude = GetOscillator("OSC1").m_wave.amplitude;
    wave_data.wave.frequency = GetOscillator("OSC1").m_wave.frequency;
    wave_data.waveform       = GetOscillator("OSC1").m_waveform;
    wave_data.samples[frame] = sample;
}

std::vector<note>& Synthesizer::GetNotes()
{
    return notes;
}