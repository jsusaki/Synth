#include "AudioDriver.h"
#include "../AudioEngine.h"

AudioDriver::AudioDriver(AudioEngine* host) : m_host(host)
{
}

AudioDriver::~AudioDriver()
{
}

bool AudioDriver::Open()
{
    return false;
}

void AudioDriver::Close()
{

}

bool AudioDriver::Start()
{
    return false;
}

void AudioDriver::Stop()
{

}

void AudioDriver::EnumerateOutputDevices()
{

}

std::vector<std::string> AudioDriver::GetOutputDevices()
{
    return m_output_devices;
}

const s32 AudioDriver::GetOutputDevice()
{
    return 0;
}

void AudioDriver::SetOutputDevice(s32 index)
{

}

// miniaudio backend
MiniAudio::MiniAudio(AudioEngine* host) : AudioDriver(host)
{

}

MiniAudio::~MiniAudio()
{

}

bool MiniAudio::Open()
{
    // Initialize audio context
    m_context_config = ma_context_config_init();
    if (ma_context_init(nullptr, 0, &m_context_config, &m_context) != MA_SUCCESS)
    {
        std::printf("ERROR: Failed to initialize context");
        ma_context_uninit(&m_context);
        return false;
    }

    // Initialize audio device
    m_device_config = ma_device_config_init(ma_device_type_playback);
    m_device_config.playback.format = ma_format_f32;
    m_device_config.playback.channels = m_host->Channels();
    m_device_config.sampleRate = m_host->SampleRate();
    m_device_config.dataCallback = MiniAudio_Callback;
    m_device_config.pUserData = this;

    if (ma_device_init(&m_context, &m_device_config, &m_device) != MA_SUCCESS)
    {
        std::printf("ERROR: Failed to initialize playback device.\n");
        ma_device_uninit(&m_device);
        ma_context_uninit(&m_context);
        return false;
    }
    std::printf("INFO: Audio device initialized\n");
    std::printf("INFO: Device: %s\n", m_device.playback.name);
    std::printf("INFO: Backend: miniaudio | %s\n", ma_get_backend_name(m_context.backend));
    std::printf("INFO: Format:        %s -> %s\n", ma_get_format_name(m_device.playback.format), ma_get_format_name(m_device.playback.internalFormat));
    std::printf("INFO: Channels:      %d -> %d\n", m_device.playback.channels, m_device.playback.internalChannels);
    std::printf("INFO: Sample rate:   %d Hz-> %d Hz\n", m_device.sampleRate, m_device.playback.internalSampleRate);
    std::printf("INFO: Periods size:  %d\n", m_device.playback.internalPeriodSizeInFrames * m_device.playback.internalPeriods);
    std::printf("INFO: Block Size:    %d\n", m_host->Blocks());
    std::printf("INFO: Samples per Block:  %d\n", m_host->BlockSamples());

    if (ma_mutex_init(&m_mutex) != MA_SUCCESS)
    {
        std::printf("ERROR: Failed to initialize mutex.\n");
        ma_mutex_uninit(&m_mutex);
        ma_device_uninit(&m_device);
        ma_context_uninit(&m_context);
        return false;
    }
    
    // Store the default device name
    m_current_device = m_device.playback.name;

    EnumerateOutputDevices();

    return true;
}

void MiniAudio::Close()
{
    std::printf("INFO: Audio device closed.\n");
    ma_mutex_uninit(&m_mutex);
    ma_device_uninit(&m_device);
    ma_context_uninit(&m_context);
}

bool MiniAudio::Start()
{
    if (ma_mutex_init(&m_mutex) != MA_SUCCESS)
    {
        printf("ERROR: Failed create mutex for mixing.\n");
        ma_device_uninit(&m_device);
        ma_context_uninit(&m_context);
        return false;
    }

    if (ma_device_start(&m_device) != MA_SUCCESS)
    {
        printf("ERROR: Failed to start playback device.\n");
        ma_device_uninit(&m_device);
        ma_context_uninit(&m_context);
        return false;
    }

    return true;
}

void MiniAudio::Stop()
{
    ma_mutex_uninit(&m_mutex);
    ma_device_uninit(&m_device);
    ma_context_uninit(&m_context);
}

void MiniAudio::EnumerateOutputDevices()
{
    m_output_devices.clear();
    std::printf("INFO: Enumerating Devices\n");
    if (ma_context_get_devices(&m_context, &m_playback_device_infos, &m_playback_device_count, &m_capture_device_infos, &m_capture_device_count) != MA_SUCCESS)
    {
        ma_device_uninit(&m_device);
        ma_context_uninit(&m_context);
    }

    for (ma_uint32 i = 0; i < m_playback_device_count; i++)
    {
        std::string device_name = m_playback_device_infos[i].name;
        m_output_devices.push_back(device_name);
        std::printf("INFO:   Device ID: %d %s\n", i, device_name.c_str());
    }
}

void MiniAudio::SetOutputDevice(s32 index)
{
    if (index < 0 || index > m_output_devices.size())
    {
        std::printf("ERROR:    There is no device\n");
        return;
    }

    m_device_config.playback.pDeviceID = &m_playback_device_infos[index].id;
    std::string device_name = m_output_devices[index];
    std::printf("INFO:   Selected Output Device: %d %s\n", index, device_name.c_str());

    if (ma_device_init(&m_context, &m_device_config, &m_device) != MA_SUCCESS) 
    {
        printf("ERROR: Failed to select playback device.\n");
        ma_device_uninit(&m_device);
        ma_context_uninit(&m_context);
    }
}

const s32 MiniAudio::GetOutputDevice()
{
    auto it = std::find(m_output_devices.begin(), m_output_devices.end(), m_current_device);
    return (it != m_output_devices.end()) ? std::distance(m_output_devices.begin(), it) : 0;
}

void MiniAudio::FillOutputBuffer(void* pOutput, u32 frameCount)
{
    // Generate mixed samples for sound card
    std::vector<f64>& mixed_outputs = m_host->ProcessOutputBlock(frameCount);
    u32 channels = m_host->Channels();

    // Fill output buffer with mixed samples for each channel, interleaved
    f32* pFramesOutF32 = (f32*)pOutput;
    for (u32 frame = 0; frame < frameCount; frame++)
    {
        f32 mixed_output = static_cast<f32>(mixed_outputs[frame]);
        for (u32 channel = 0; channel < channels; channel++)
            pFramesOutF32[frame * channels + channel] = mixed_output;
    }
}

void MiniAudio::MiniAudio_Callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    MiniAudio* driver = (MiniAudio*)pDevice->pUserData;
    driver->FillOutputBuffer(pOutput, frameCount);
}