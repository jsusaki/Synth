#include "Application.h"
#include <iostream>

Application::Application() : window("Sound Synthesizer", SCREEN_WIDTH, SCREEN_HEIGHT) 
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
    while (!window.ShouldClose())
    {
        // Poll events
        window.PollEvents();

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
            Simulate(m_elapsed_time);
            m_accumulator -= m_delta_time;
        }

        // Rendering pipeline
        Render();

        window.SwapBuffers();

        // Update Frame Time Info
        UpdateFrameTime();
    }

    return true;
}

bool Application::ShutDown()
{
    audio.Shutdown();
    gui.Shutdown();
    window.Close();
    return true;
}

void Application::Create()
{
    audio.Init(44100, 2, 8, SAMPLE_RATE/100);
    gui.Init(window.GetWindow());
}

// TODO: incorporate in audio control
void Application::ProcessNoteInput(f32 time, s32 key, u32 note_id, std::vector<note>& notes)
{
    Input& input = Input::Instance();

    // Check if note is already active

    auto noteFound = std::find_if(notes.begin(), notes.end(), [note_id](const note& n) { return n.id == note_id; });
    if (noteFound == notes.end())
    {
        if (input.IsKeyHeld(key))
        {
            // Note is not active, so create and add a new note
            note n;
            n.id = note_id;
            n.on = time;
            n.channel = 0;
            n.active = true;
            notes.emplace_back(n);

            // UI link
            piano.down(note_id, 1);
            //std::cout << note_id << std::endl;
        }
    }
    else
    {
        if (input.IsKeyHeld(key))
        {
            // Key is still held, so do nothing
            if (noteFound->off > noteFound->on)
            {
                // Key has been pressed again during release phase
                noteFound->on = time;
                noteFound->active = true;

                // UI link
                piano.down(noteFound->id, 1);
            }
        }
        else
        {
            if (noteFound->off < noteFound->on)
                noteFound->off = time;

            // UI link
            piano.up(noteFound->id);
            //std::cout << note_id << std::endl;
        }
    }
}

void Application::ProcessInput()
{
    // TODO: simplify this singleton to something
    Input& input = Input::Instance();

    // Close the window
    if (input.IsKeyPressed(GLFW_KEY_ESCAPE))
        window.SetShouldClose();

    if (input.IsKeyPressed(GLFW_KEY_SPACE))
    {
        audio.synth.PlayToggle();
        gui.Play(audio.synth.IsPlaying());
    }

    // Control variables
    static f32 master_volume = 0.5f;
    static s32 octave = 4*12;
    static f32 osc1_volume = 0.5f;

    f64 time = audio.Time();
    std::vector<note>& notes = audio.synth.GetNotes();

    // Synth Control
    // Keyboard Control
    ProcessNoteInput(time, GLFW_KEY_Z,        0+octave,    notes);
    ProcessNoteInput(time, GLFW_KEY_S,        1+octave,    notes);
    ProcessNoteInput(time, GLFW_KEY_X,        2+octave,    notes);
    ProcessNoteInput(time, GLFW_KEY_D,        3+octave,    notes);
    ProcessNoteInput(time, GLFW_KEY_C,        4+octave,    notes);
    ProcessNoteInput(time, GLFW_KEY_V,        5+octave,    notes);
    ProcessNoteInput(time, GLFW_KEY_G,        6+octave,    notes);
    ProcessNoteInput(time, GLFW_KEY_B,        7+octave,    notes);
    ProcessNoteInput(time, GLFW_KEY_H,        8+octave,    notes);
    ProcessNoteInput(time, GLFW_KEY_N,        9+octave,    notes);
    ProcessNoteInput(time, GLFW_KEY_J,        10+octave,   notes);
    ProcessNoteInput(time, GLFW_KEY_M,        11+octave,   notes);
    ProcessNoteInput(time, GLFW_KEY_COMMA,    0+octave+12, notes);
    ProcessNoteInput(time, GLFW_KEY_L,        1+octave+12, notes);
    ProcessNoteInput(time, GLFW_KEY_PERIOD,   2+octave+12, notes);
    ProcessNoteInput(time, GLFW_KEY_SEMICOLON,3+octave+12, notes);
    ProcessNoteInput(time, GLFW_KEY_SLASH,    4+octave+12, notes);

    // Pitch Control
    if (input.IsKeyPressed(GLFW_KEY_LEFT))  octave -= 12;
    if (input.IsKeyPressed(GLFW_KEY_RIGHT)) octave += 12;
    if (octave < 0) octave = 0;

    // Master Volume Control
    if (input.IsKeyPressed(GLFW_KEY_UP))    master_volume += 0.1f;
    if (input.IsKeyPressed(GLFW_KEY_DOWN))  master_volume -= 0.1f;
    audio.synth.SetMasterVolume(master_volume);

    // Oscillator 1 Control
    if (input.IsKeyPressed(GLFW_KEY_KP_ADD))       osc1_volume += 0.1f;
    if (input.IsKeyPressed(GLFW_KEY_KP_SUBTRACT))  osc1_volume -= 0.1f;
    audio.synth.GetOscillator("OSC1").SetVolume(osc1_volume);

    if (input.IsKeyPressed(GLFW_KEY_Q)) audio.synth.GetOscillator("OSC1").SetWaveform(Oscillator::Type::SINE);
    if (input.IsKeyPressed(GLFW_KEY_W)) audio.synth.GetOscillator("OSC1").SetWaveform(Oscillator::Type::SQUARE);
    if (input.IsKeyPressed(GLFW_KEY_E)) audio.synth.GetOscillator("OSC1").SetWaveform(Oscillator::Type::TRIANGLE);
    if (input.IsKeyPressed(GLFW_KEY_R)) audio.synth.GetOscillator("OSC1").SetWaveform(Oscillator::Type::SAWTOOTH);
    if (input.IsKeyPressed(GLFW_KEY_T)) notes.clear();



    if (!gui.IsWindowFocused())
    {
        if (input.IsButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {

        }
        if (input.IsButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE))
        {

        }
        if (input.IsButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
        {

        }
        if (input.GetMouseWheel() != 0)
        {
            input.ResetMouseWheel();
        }
    }

    // Update input state
    input.Update();
}

void Application::Simulate(f32 dt)
{
    audio.Update(dt);
    // Pass synth?
    gui.Display(audio.synth.GetWaveData(), audio.synth.GetOscillators(), audio.synth.GetMasterVolume(), audio.synth.GetNotes());
}

void Application::Render()
{
    window.Clear({ 25, 25, 25, 255 });

    // Enable rendering flags
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    piano.Render(&show);

    // Render GUI
    gui.Render();

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