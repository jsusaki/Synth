#include "Application.h"
#include <iostream>

Application::Application() : m_window("Sound Synthesizer", SCREEN_WIDTH, SCREEN_HEIGHT) 
{

}

bool Application::Init()
{
    // Create resources
    Create();

    // Time
    m_t1 = std::chrono::system_clock::now();
    m_t2 = std::chrono::system_clock::now();
    m_last_fps = 0;
    m_frame_timer = 1.0f;
    m_frame_count = 0;
    m_accumulator = 0.0f;
    m_delta_time = 1.0f / 60.0f;
    m_elapsed_time = 0.0f;
    m_last_elapsed_time = 0.0f;

    return true;
}

bool Application::Start()
{
    while (!m_window.ShouldClose())
    {
        // Poll events
        m_window.PollEvents();

        // Handle timing
        m_t2 = std::chrono::system_clock::now();
        std::chrono::duration<f32> elapsed_time = m_t2 - m_t1;
        m_t1 = m_t2;

        // Compute elapsed time
        m_elapsed_time = elapsed_time.count();
        m_last_elapsed_time = m_elapsed_time;

        // Handle input
        ProcessInput();

        // Fixed Time Update
        m_accumulator += m_delta_time;
        while (m_accumulator >= m_delta_time)
        {
            Update(m_elapsed_time);
            m_accumulator -= m_delta_time;
        }

        // Rendering pipeline
        Render();

        m_window.SwapBuffers();

        // Update Frame Time Info
        UpdateFrameTime();
    }

    return true;
}

bool Application::ShutDown()
{
    m_audio.Shutdown();
    m_gui.Shutdown();
    m_window.Close();
    return true;
}

void Application::Create()
{
    m_audio.Init(44100, 2, 8, SAMPLE_RATE/100);

    // How do we link synth + application?
    m_gui.Init(m_window.GetWindow());
}


void Application::ProcessInput()
{
    // TODO: simplify this singleton to something
    Input& input = Input::Instance();

    // Close the window
    if (input.IsKeyPressed(GLFW_KEY_ESCAPE))
        m_window.SetShouldClose();

    if (input.IsKeyPressed(GLFW_KEY_SPACE))
    {
        //m_audio.synth.TogglePlay();
        //m_gui.Play(m_audio.synth.IsPlaying());
    }

    f64 time_step = m_audio.Timestep();
    m_audio.synth.ProcessInput(time_step);

    /*
    // TODO: mouse piano press
    if (!m_gui.IsWindowFocused())
    {
        if (input.IsButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {}
        if (input.IsButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {}
        if (input.IsButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {}
        if (input.GetMouseWheel() != 0) { input.ResetMouseWheel(); }
    }
    */

    // Update input state
    input.Update();
}

void Application::Update(f32 dt)
{
    m_audio.Update(1.0/SAMPLE_RATE);
    m_gui.Display(m_audio.synth);
}

void Application::Render()
{
    m_window.Clear({ 25, 25, 25, 255 });

    // Enable rendering flags
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    // Render GUI
    m_audio.synth.Render();
    m_gui.Render();
}

void Application::UpdateFrameTime()
{
    m_frame_timer += m_elapsed_time;
    m_frame_count++;
    if (m_frame_timer >= 1.0f)
    {
        m_last_fps = m_frame_count;
        m_frame_timer -= 1.0f;
        //std::printf("INFO: Frame Time: %.4f FPS: %d\n", m_elapsed_time, m_frame_count);
        m_frame_count = 0;
    }
}