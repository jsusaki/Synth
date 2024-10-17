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
    std::printf("INFO: Audio device closed.\n");
    m_driver->Close();
}

std::vector<f32>& AudioEngine::ProcessOutputBlock(u32 frame_count)
{
    for (u32 frame = 0; frame < frame_count; frame++)
    {
        // TODO: clean up; synth note may go inside, and return a buffer of mixed outputs
        f32 mixed_output = 0.0f;
        for (auto& n : synth.notes)
        {
            f32 sample = 0.0f;
            bool note_finished = false;

            sample = synth.Synthesize(m_global_time, n, note_finished);
            mixed_output += sample;

            // If the note has finished playing, deactivate it
            if (note_finished && n.off > n.on)
                n.active = false;
        }
        
        // TODO: review mixing method: do we need to normalize?
        mixed_output = std::clamp(mixed_output, -1.0f, 1.0f);
        synth.UpdateWaveData(frame, mixed_output);

        // Update time
        m_global_time += m_time_per_sample;
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