#include "Synthesizer.h"

Synthesizer::Synthesizer()
{
    // Synth Config
    m_master_volume = 0.2;
    m_max_frequency = 20000.0;

    // TODO: struct Instrument
    // Oscillator
    oscillators["OSC1"] = Oscillator(0.8,  0, Oscillator::Type::WAVE_SINE);
    oscillators["OSC2"] = Oscillator(0.3, 12, Oscillator::Type::WAVE_ANLG_SAWTOOTH);
    oscillators["OSC3"] = Oscillator(0.1, 24, Oscillator::Type::WAVE_ANLG_SAWTOOTH);

    // LFO
    m_lfo = Oscillator(0.01, 0, Oscillator::Type::WAVE_SINE);
    m_lfo.m_wave.frequency = 5.0;
    m_lfo.m_wave.amplitude = 0.01;

    // ADSR
    m_amp_envelope = {
        .attack_time       = 0.3,
        .decay_time        = 1.0,
        .sustain_amplitude = 0.8,
        .release_time      = 2.5,
        .start_amplitude   = 1.0,
        .decay_function    = Envelope::Decay::EXPONENTIAL
    };

    // Filter
    m_filter = {
        .type             = BqFilter::Type::LOW_PASS,
        .frequency        = 2000.0,
        .resonance        = 0.5,
        .sample_rate      = SAMPLE_RATE,
    };
    m_filter.CalcCoefs(2000.0, 0.5);

    m_vafilter = {
        .type             = VAFilter::Type::LOW_PASS,
        .frequency        = 2000.0,
        .resonance        = 0.5,
        .sample_rate      = SAMPLE_RATE,
    };
    m_vafilter.CalcCoefs(2000.0, 0.5);

    // Delay
    m_delay.tempo = 120; // BPM
    m_delay.steps = 3;
    m_delay.feedback = 0.7;
    m_delay.offset = 0;

    // Reverb
    m_reverb.room   = 1.0;
    m_reverb.spread = 0.2;
    m_reverb.damp   = 0.2;
    m_reverb.decay  = 1.0;
    m_reverb.dry    = 0.0;
    m_reverb.wet    = 0.0;

    m_reverb.ComputeFilterDelays();

    // Data
    wave_data.times.resize(SAMPLE_RATE/100, 0.0);
    wave_data.samples.resize(SAMPLE_RATE/100, 0.0);
}

f64 Synthesizer::Synthesize(f64 time_step, note n, bool& note_finished)
{

}

void Synthesizer::ProcessNoteInput(f64 time, s32 key, s32 note_id)
{
    Input& input = Input::Instance();

    // Check if note is already active
    auto note_found = std::find_if(notes.begin(), notes.end(), [note_id](const note& n) { return n.id == note_id; });
    if (note_found == notes.end())
    {
        if (input.IsKeyHeld(key))
        {
            // Note is not active, so create and add a new note
            note n;
            n.id = note_id;
            n.on = time;
            n.off = -1.0;
            n.channel = 0;
            n.active = true;
            notes.push_back(n);

            // UI link
            m_piano.down(note_id, 1);
            //std::cout << note_id << std::endl;
        }
    }
    else
    {
        if (input.IsKeyHeld(key))
        {
            // Key is still held, so do nothing
            if (note_found->off > note_found->on)
            {
                // Key has been pressed again during release phase
                note_found->on = time;
                note_found->active = true;

                // UI link
                m_piano.down(note_found->id, 1);
            }
        }
        else
        {
            if (note_found->off < note_found->on)
                note_found->off = time;

            // UI link
            m_piano.up(note_found->id);
            //std::cout << note_id << std::endl;
        }
    }
}

void Synthesizer::ProcessInput(f64 time)
{
    Input& input = Input::Instance();

    // Control variables
    static s32 octave = 4 * 12;

    // Synth Control
    // Keyboard Control
    // TODO: integrate press and release in synth
    ProcessNoteInput(time, GLFW_KEY_Z, 0 + octave);
    ProcessNoteInput(time, GLFW_KEY_S, 1 + octave);
    ProcessNoteInput(time, GLFW_KEY_X, 2 + octave);
    ProcessNoteInput(time, GLFW_KEY_D, 3 + octave);
    ProcessNoteInput(time, GLFW_KEY_C, 4 + octave);
    ProcessNoteInput(time, GLFW_KEY_V, 5 + octave);
    ProcessNoteInput(time, GLFW_KEY_G, 6 + octave);
    ProcessNoteInput(time, GLFW_KEY_B, 7 + octave);
    ProcessNoteInput(time, GLFW_KEY_H, 8 + octave);
    ProcessNoteInput(time, GLFW_KEY_N, 9 + octave);
    ProcessNoteInput(time, GLFW_KEY_J, 10 + octave);
    ProcessNoteInput(time, GLFW_KEY_M, 11 + octave);
    ProcessNoteInput(time, GLFW_KEY_COMMA, 0 + octave + 12);
    ProcessNoteInput(time, GLFW_KEY_L, 1 + octave + 12);
    ProcessNoteInput(time, GLFW_KEY_PERIOD, 2 + octave + 12);
    ProcessNoteInput(time, GLFW_KEY_SEMICOLON, 3 + octave + 12);
    ProcessNoteInput(time, GLFW_KEY_SLASH, 4 + octave + 12);

    // Pitch Control
    if (input.IsKeyPressed(GLFW_KEY_LEFT))  octave -= 12;
    if (input.IsKeyPressed(GLFW_KEY_RIGHT)) octave += 12;
    if (octave < 0) octave = 0;

    if (input.IsKeyPressed(GLFW_KEY_TAB)) notes.clear();
}

void Synthesizer::Update(f64 time)
{
	notes.erase(std::remove_if(notes.begin(), notes.end(), [](const note& n) { return !n.active; }), notes.end());
}

void Synthesizer::Render()
{
    // Render Midi keyboard
    m_piano.Render();
}

void Synthesizer::TogglePlay()
{
    m_playing = !m_playing;
}

bool Synthesizer::IsPlaying()
{
    return m_playing;
}

void Synthesizer::SetMasterVolume(f64 volume)
{
    m_master_volume = volume;
}

f64 Synthesizer::GetMasterVolume()
{
    return m_master_volume;
}

Oscillator& Synthesizer::GetOscillator(std::string id)
{
    return oscillators[id];
}

std::unordered_map<std::string, Oscillator>& Synthesizer::GetOscillators()
{
    return oscillators;
}

const WaveData& Synthesizer::GetWaveData()
{
    return wave_data;
}

void Synthesizer::UpdateWaveData(u32 frame, f64 sample)
{
    wave_data.samples[frame] = sample;
}

std::vector<note>& Synthesizer::GetNotes()
{
    return notes;
}