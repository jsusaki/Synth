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

    m_driver->Open();
    m_driver->Start();

    return true;
}

void AudioEngine::Update(f64 time)
{
    synth.Update(time);
}

void AudioEngine::Shutdown()
{
    std::printf("INFO: Audio device closed.\n");
    m_driver->Close();
}

std::vector<f32>& AudioEngine::ProcessOutputBlock(u32 frame_count)
{
    for (u32 frame = 0; frame < frame_count; frame++)
    {
        f32 mixed_output = 0.0f;
        for (auto& n : synth.notes)
        {
            f32 sample = 0.0f;
            bool note_finished = false;

            // if (n.channel == 0)
            sample = synth.GetOscillator("OSC1").GenerateSound(m_global_time, n, note_finished);
            mixed_output += sample * synth.m_master_volume;

            // If the note has finished playing, deactivate it
            if (note_finished && n.off > n.on)
                n.active = false;
        }

        mixed_output = std::clamp(mixed_output, -1.0f, 1.0f);

        // Update time
        m_global_time += m_time_per_sample;

        // Collect data
        synth.wave_data.wave.amplitude = synth.GetOscillator("OSC1").m_wave.amplitude;
        synth.wave_data.wave.frequency = synth.GetOscillator("OSC1").m_wave.frequency;
        synth.wave_data.waveform = synth.GetOscillator("OSC1").m_waveform;
        synth.wave_data.samples[frame] = mixed_output;
    }

    return synth.wave_data.samples;
}

const f64 AudioEngine::Time() const
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