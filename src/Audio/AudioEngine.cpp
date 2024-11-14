#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include "AudioEngine.h"

AudioEngine::AudioEngine()
{

}

bool AudioEngine::Init(u32 sample_rate, u32 channels, u32 blocks, u32 block_samples)
{
    m_driver = std::make_unique<MiniAudio>(this);
    m_sample_rate = sample_rate;
    m_channels = channels;
    m_blocks = blocks;
    m_block_samples = block_samples;
    m_sample_per_time = f64(sample_rate);
    m_time_per_sample = 1.0 / f64(sample_rate);
    m_global_time = 0.0;

    m_driver->Open();
    m_driver->Start();

    return true;
}

void AudioEngine::Update(f64 time_step)
{
    synth.Update(time_step);
}

void AudioEngine::Shutdown()
{
    m_driver->Close();
}

// Called inside the: driver.FillOutputBuffer(void* pOutput, u32 frameCount)
std::vector<f64>& AudioEngine::ProcessOutputBlock(u32 frame_count)
{
    for (u32 frame = 0; frame < frame_count; frame++)
    {
        f64 mixed_output = 0.0;
        for (auto& n : synth.notes)
        {
            f64 sample = 0.0;
            bool note_finished = false;

            // Amplitude Envelope
            f64 amplitude = synth.m_amp_envelope.GenerateAmplitude(m_global_time, n.on, n.off);
            if (amplitude <= 0.0000001)
                note_finished = true;

            // Low Frequency Oscillator
            f64 lfo_output = synth.m_lfo.GenerateWave(m_global_time, synth.m_lfo.m_wave.amplitude, synth.m_lfo.m_wave.frequency);

            // Oscillators
            for (auto& [id, osc] : synth.oscillators)
            {
                // TODO: Frequency Modulation

                // Generate wave
                f64 sound = osc.GenerateWave(m_global_time - n.on, n);

                // Amplitude Modulation
                sound = (sound * (1.0 + lfo_output)) * amplitude;

                // Filter
                if (synth.vafilter) sound = synth.m_vafilter.FilterWave(sound);
                else                sound = synth.m_filter.FilterWave(sound);

                // Mix Oscillators
                sample += sound;
            }

            // Normalize
            sample /= static_cast<f64>(synth.oscillators.size());
            
            // Clamp
            sample = std::clamp(sample * synth.m_master_volume, -1.0, 1.0);

            // Mix all
            mixed_output += sample;

            // If the note has finished playing, deactivate it
            if (note_finished && n.off > n.on)
                n.active = false;
        }

        // Delay
        if (!synth.delay) mixed_output = synth.m_delay.Process(mixed_output);

        // Reverb
        if (!synth.reverb) mixed_output = synth.m_reverb.Process(mixed_output);

        // Equalizer
        if (!synth.eq) mixed_output = synth.m_eq.Process(mixed_output);

        mixed_output = std::clamp(mixed_output, -1.0, 1.0);
        synth.UpdateWaveData(frame, mixed_output);

        // Update time
        m_global_time += m_time_per_sample;
    }

    return synth.wave_data.samples;
}

const f64 AudioEngine::Timestep() const
{
    return m_global_time;
}

const u32 AudioEngine::SampleRate() const
{
    return m_sample_rate;
}

const u32 AudioEngine::Channels() const
{
    return m_channels;
}

const u32 AudioEngine::Blocks() const
{
    return m_blocks;
}

const u32 AudioEngine::BlockSamples() const
{
    return m_block_samples;
}

const std::vector<std::string> AudioEngine::GetOutputDeviceNames()
{
    return m_driver->GetOutputDevices();
}

void AudioEngine::SetOutputDevice(s32 index)
{
    m_driver->SetOutputDevice(index);
}

const s32 AudioEngine::GetOutputDevice()
{
    return m_driver->GetOutputDevice();
}