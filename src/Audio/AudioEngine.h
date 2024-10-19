#pragma once

#include <mutex>
#include <cmath>
#include <string>
#include <vector>
#include <limits>
#include <memory>
#include <algorithm>
#include <functional>
#include <unordered_map>

#include "miniaudio.h"

#include "../Core/Common.h"
#include "Driver/AudioDriver.h"
#include "Synth/Synthesizer.h"


class AudioEngine
{
public:
    AudioEngine();
    friend class MiniAudio;

public: // Synthesizers, synthesizers, synthesizers
    Synthesizer synth;

public: // Audio Engine Interface
    bool Init(u32 sample_rate = 44100, u32 channels = 1, u32 blocks = 8, u32 block_samples = 512);
    void Update(f64 time);
    void Shutdown();

public: // Accessors 
    const f64 Timestep() const;
    const u32 SampleRate() const;
    const u32 Channels() const;
    const u32 Blocks() const;
    const u32 BlockSamples() const;

private: // Audio Engine Internal
    u32 m_sample_rate     = 44100;
    u32 m_channels        = 1;
    u32 m_blocks          = 8;
    u32 m_block_samples   = m_sample_rate / 100;
    f64 m_sample_per_time = 44100.0;
    f64 m_time_per_sample = 1.0 / 44100.0;
    f64 m_global_time     = 0.0;

private: // Audio Driver Internal
    // Generate samples for FillOutputBuffer in AudioDriver
    std::vector<f64>& ProcessOutputBlock(u32 frame_count);
    // Audio Driver
    std::unique_ptr<AudioDriver> m_driver;
};