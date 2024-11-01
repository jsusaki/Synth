#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <algorithm>

#include "miniaudio.h"

#include "../../Core/Common.h"

class AudioEngine;

class AudioDriver
{
public:
    AudioDriver(AudioEngine* host);
    virtual ~AudioDriver();

public:
    virtual bool Open();
    virtual void Close();
    virtual bool Start();
    virtual void Stop();

    virtual void EnumerateOutputDevices();
    virtual std::vector<std::string> GetOutputDevices();
    virtual const s32 GetOutputDevice();
    virtual void SetOutputDevice(s32 index);

protected:
    AudioEngine* m_host = nullptr;

    std::vector<std::string> m_output_devices;
    std::string m_current_device;
};


class MiniAudio : public AudioDriver
{
public:
    MiniAudio(AudioEngine* host);
    ~MiniAudio();

public:
    bool Open() override;
    void Close() override;
    bool Start() override;
    void Stop() override;

public:
    void EnumerateOutputDevices() override;
    void SetOutputDevice(s32 index) override;
    const s32 GetOutputDevice() override;

public:
    void FillOutputBuffer(void* pOutput, u32 frameCount);

private: // miniaudio specific implementations
    static void MiniAudio_Callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

    ma_mutex m_mutex; 
    ma_context m_context;
    ma_context_config m_context_config;
    ma_device m_device;
    ma_device_config m_device_config;

    ma_device_info* m_playback_device_infos;
    ma_uint32 m_playback_device_count;

    ma_device_info* m_capture_device_infos;
    ma_uint32 m_capture_device_count;

};