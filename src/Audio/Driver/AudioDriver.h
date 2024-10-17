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

    virtual std::vector<std::string> EnumerateOutputDevices();

protected:
    AudioEngine* m_host = nullptr;
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

    void FillOutputBuffer(void* pOutput, u32 frameCount);

private: // miniaudio specific implementations
    static void MiniAudio_Callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

    ma_mutex m_mutex; 
    ma_context m_context;
    ma_context_config m_context_config;
    ma_device m_device;
    ma_device_config m_device_config;
};