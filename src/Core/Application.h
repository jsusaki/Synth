#pragma once

#include <print>
#include <memory>
#include <chrono>
#include <algorithm>
#include <mutex>

#include "Common.h"
#include "Random.h"
#include "Window.h"
#include "Input.h"
#include "../GUI/GUI.h"
#include "../GUI/Piano.h"
#include "../Audio/AudioEngine.h"

class Application
{
public:
    Application();

public: // Interface
    bool Init();
    bool Start();
    bool ShutDown();

private: // Main functions
    void Create();
    void ProcessInput();
    void Simulate(f32 dt);
    void Render();

private: // Helper functions
    void UpdateFrameTime();
    void ProcessNoteInput(f32 time, s32 key, u32 note_id, std::vector<note>& notes);

private: // Simulator variables
    // Timing
    std::chrono::time_point<std::chrono::system_clock> m_t1;
    std::chrono::time_point<std::chrono::system_clock> m_t2;
    u32 m_last_fps;
    u32 m_frame_count;
    f32 m_frame_timer;
    f32 m_accumulator;
    f32 m_delta_time;
    f32 m_elapsed_time;
    f32 m_last_elapsed_time;

    // Window
    Window window;

private: // Simulation variables
    AudioEngine audio;

    GUI gui;
    Piano piano;
    bool show = true;
};