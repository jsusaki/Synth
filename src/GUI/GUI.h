#pragma once

#include <complex>
#include <algorithm>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"
#include "imgui-knobs.h"

#include "../Core/Common.h"
#include "../Audio/Synth/Synthesizer.h"

struct ScrollingBuffer 
{
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;

    ScrollingBuffer(int max_size = 8192) 
    {
        MaxSize = max_size;
        Offset = 0;
        Data.reserve(MaxSize);
    }

    void AddPoint(float x, float y) 
    {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x, y));
        else 
        {
            Data[Offset] = ImVec2(x, y);
            Offset = (Offset + 1) % MaxSize;
        }
    }

    void Erase() 
    {
        if (Data.size() > 0) 
        {
            Data.shrink(0);
            Offset = 0;
        }
    }
};

static bool SliderDouble(const char* label, double* v, double v_min, double v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
{
    return ImGui::SliderScalar(label, ImGuiDataType_Double, v, &v_min, &v_max, format, flags);
}

static bool VSliderDouble(const char* label, const ImVec2& size, double* v, double v_min, double v_max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
{
    return ImGui::VSliderScalar(label, size, ImGuiDataType_Double, v, &v_min, &v_max, format, flags);
}

class GUI
{
public:
    GUI() {}

public:
    void Init(GLFWwindow* window)
	{
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
        ImGui::SetNextWindowFocus();
        // Setup ImPlot context
        ImPlot::CreateContext();
	}

    void Display(Synthesizer& synth, AudioEngine &audio)
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            // Info
            General(synth, audio);   

            // Oscillator
            static s32 wf1 = static_cast<s32>(synth.GetOscillator("OSC1").m_waveform);
            static s32 wf2 = static_cast<s32>(synth.GetOscillator("OSC2").m_waveform);
            static s32 wf3 = static_cast<s32>(synth.GetOscillator("OSC3").m_waveform);
            static bool mute1 = false, mute2 = false, mute3 = false;
            DefaultOscillator(synth.GetOscillator("OSC1"), "OSC1", wf1, mute1);
            DefaultOscillator(synth.GetOscillator("OSC2"), "OSC2", wf2, mute2);
            DefaultOscillator(synth.GetOscillator("OSC3"), "OSC3", wf3, mute3);

            // Oscilloscope
            Oscilloscope(synth);

            // Note Info
            Notes(synth);

            // Amplitude Envelope
            static s32 decay_function = static_cast<s32>(synth.m_amp_envelope.decay_function);
            Envelope(synth.m_amp_envelope, decay_function, "Amplitude Envelope");

            // TODO: Filter Envelope

            // Volume
            Mixer(synth);

            // Filter
            Filter(synth);
        
            // Low Frequency Oscillator
            static s32 wflfo    = static_cast<s32>(synth.m_lfo.m_waveform);
            static bool mutelfo = false;
            LowFrequencyOscillator(synth.m_lfo, "LFO", wflfo, mutelfo);

            // Delay
            DelayEffect(synth);

            // Reverb
            ReverbEffect(synth);

            // Equalizer
            Eq(synth);
        }
    }

    void Render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Shutdown()
    {
        ImPlot::DestroyContext();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    bool IsWindowFocused() const { return ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow); }
    void Play(bool p) { play = p; }

    void General(Synthesizer& synth, AudioEngine& audio)
    {
        ImGui::Begin("Sound Synthesizer");
        {
            ImGui::Text("FPS: average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("Sample Rate (Hz): %.0f", SAMPLE_RATE);
            ImGui::Text("Channels: %d", CHANNELS);

            // Select Output Device
            static s32 selected_idx = audio.GetOutputDevice();
            std::vector<std::string> device_names = audio.GetOutputDeviceNames();
            std::string combo_preview_value = device_names[selected_idx];

            if (ImGui::BeginCombo("Output Device", combo_preview_value.c_str()))
            {
                for (s32 n = 0; n < device_names.size(); n++)
                {
                    const bool is_selected = (selected_idx == n);
                    if (ImGui::Selectable(device_names[n].c_str(), is_selected))
                    {
                        selected_idx = n;
                        audio.SetOutputDevice(selected_idx);
                    }
                    if (is_selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::Checkbox("Show ImGui Demo", &show_imgui_demo);
            ImGui::Checkbox("Show ImPlot Demo", &show_implot_demo);
            if (show_imgui_demo)  ImGui::ShowDemoWindow();
            if (show_implot_demo) ImPlot::ShowDemoWindow();
        }
        ImGui::End();
    }

    void Notes(Synthesizer& synth)
    {
        ImGui::Begin("Notes");
        {
            std::string f, s;
            for (auto& n : synth.notes)
            {
                f += std::format("{:.2f} ", note_freq(n.id));
                s += std::format("{} ", note_name(n.id));
            }

            ImGui::Text("Frequency (Hz): %s", f.c_str());
            ImGui::Text("Note            %s", s.c_str());
            ImGui::Text("Notes           %d", synth.notes.size());
        }
        ImGui::End();
    }

    void Oscilloscope(Synthesizer& synth)
    {
        ImGui::Begin("Oscilloscope");
        {
            const u32 sample_size = SAMPLE_RATE / 100;
            std::vector<f64> ts(sample_size, 0.0);
            std::vector<f64> ss(sample_size, 0.0);

            for (u32 i = 0; i < sample_size; i++)
            {
                ts[i] = i;
                ss[i] = synth.wave_data.samples[i];
            }

            // TODO: make wave more visible
            ImVec2 plot_size(500, 300);
            static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks;
            if (ImPlot::BeginPlot("Waveform", plot_size))
            {
                ImPlot::SetupAxes(nullptr, "Amplitude", flags | ImPlotAxisFlags_NoGridLines, ImPlotAxisFlags_NoTickMarks);
                ImPlot::SetupAxisLimits(ImAxis_X1, 0.0, sample_size, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -1.0, 1.0);
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 3.0f);
                ImPlot::PlotLine("##Wave", ts.data(), ss.data(), ts.size());
                ImPlot::PopStyleVar();
                ImPlot::EndPlot();
            }
        }
        ImGui::End();
    }

    void Mixer(Synthesizer& synth)
    {
        ImGui::Begin("Mixer");
        {
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
            SliderDouble("Master Volume", &synth.m_master_volume, 0.0, 1.0);
        }
        ImGui::End();
    }

    void Envelope(Envelope& env,s32& decay_function, std::string name)
    {
        ImGui::Begin(name.c_str());
        {
            ImGui::Text(" A   D   S   R"); ImGui::SameLine();
            ImGui::BeginGroup();           ImGui::SameLine();
            ImGui::RadioButton("LINEAR", &decay_function, static_cast<s32>(Envelope::Decay::LINEAR));      ImGui::SameLine();
            ImGui::RadioButton("EXPO",   &decay_function, static_cast<s32>(Envelope::Decay::EXPONENTIAL)); ImGui::SameLine();
            ImGui::RadioButton("QUAD",   &decay_function, static_cast<s32>(Envelope::Decay::QUADRATIC));
            ImGui::EndGroup();

            ImVec2 slider_size(20, 300);
            VSliderDouble("##A", slider_size, &env.attack_time, 0.01, 10.0, "%.2f");      ImGui::SameLine();
            VSliderDouble("##D", slider_size, &env.decay_time,  0.01, 10.0, "%.2f");      ImGui::SameLine();
            VSliderDouble("##S", slider_size, &env.sustain_amplitude, 0.01, 1.0, "%.2f"); ImGui::SameLine();
            VSliderDouble("##R", slider_size, &env.release_time, 0.01, 10.0, "%.2f");     ImGui::SameLine();

            env.decay_function = static_cast<Envelope::Decay>(decay_function);

            // Compute ADSR parametric curve
            const s32 sample_size = 1000;
            f64 time_step = 1 / SAMPLE_RATE;
            f64 note_on_time = 0.00001;
            f64 note_off_time = 0.0;
            f64 global_time = 0.0;

            std::vector<f64> as(sample_size, 0.0);
            std::vector<f64> ts(sample_size, 0.0);
            for (s32 i = 0; i < sample_size; i++)
            {
                f64 t = i * time_step;
                if (i == 600) note_off_time = global_time;

                f64 amplitude = env.GenerateAmplitude(global_time, note_on_time, note_off_time);

                ts[i] = i;
                as[i] = amplitude;

                global_time += t;
            }

            static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks;
            if (ImPlot::BeginPlot("ADSR Envelope", ImVec2(500, 300)))
            {
                ImPlot::SetupAxes(nullptr, "Amplitude", flags | ImPlotAxisFlags_NoGridLines, ImPlotAxisFlags_NoTickMarks);
                ImPlot::SetupAxisLimits(ImAxis_X1, 0, sample_size, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 3.0f);
                ImPlot::PlotLine("##ADSR Curve", ts.data(), as.data(), ts.size());
                ImPlot::PopStyleVar();
                ImPlot::EndPlot();
            }
        }
        ImGui::End();
    }

    void DefaultOscillator(Oscillator& osc, std::string label, s32& waveform, bool& mute)
    {
        ImVec2 osc_slider_size(20, 150);
        ImGui::Begin(label.c_str());
        {
            ImGui::Text(" P   V"); ImGui::SameLine();

            ImGui::Checkbox("Mute", &osc.m_mute);

            ImGui::VSliderInt("##P", osc_slider_size, &osc.m_pitch,  -24, 48);  ImGui::SameLine();
            VSliderDouble("##V",     osc_slider_size, &osc.m_volume, 0.0, 1.0); ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::RadioButton("SINE",     &waveform, static_cast<s32>(Oscillator::Type::WAVE_SINE));
            ImGui::RadioButton("SQUARE",   &waveform, static_cast<s32>(Oscillator::Type::WAVE_SQUARE));
            ImGui::RadioButton("TRIANGLE", &waveform, static_cast<s32>(Oscillator::Type::WAVE_TRIANGLE));
            ImGui::RadioButton("DIGI SAW", &waveform, static_cast<s32>(Oscillator::Type::WAVE_DIGI_SAWTOOTH));
            ImGui::RadioButton("ANLG SAW", &waveform, static_cast<s32>(Oscillator::Type::WAVE_ANLG_SAWTOOTH));
            ImGui::RadioButton("WHITE",    &waveform, static_cast<s32>(Oscillator::Type::NOISE_WHITE));
            ImGui::EndGroup();

            osc.m_waveform = static_cast<Oscillator::Type>(waveform);
        }
        ImGui::End();
    }

    void LowFrequencyOscillator(Oscillator& osc, std::string label, s32& waveform, bool& mute)
    {
        ImVec2 osc_slider_size(20, 150);
        ImGui::Begin(label.c_str());
        {
            ImGui::Text(" P   V   F   A"); ImGui::SameLine();

            ImGui::Checkbox("Mute", &osc.m_mute);

            ImGui::VSliderInt("##P", osc_slider_size, &osc.m_pitch, -24, 48);            ImGui::SameLine();
            VSliderDouble("##V",     osc_slider_size, &osc.m_volume, 0.0, 1.0);          ImGui::SameLine();
            VSliderDouble("##F",     osc_slider_size, &osc.m_wave.frequency, 0.0, 20.0); ImGui::SameLine();
            VSliderDouble("##A",     osc_slider_size, &osc.m_wave.amplitude, 0.0, 0.01); ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::RadioButton("SINE",     &waveform, static_cast<s32>(Oscillator::Type::WAVE_SINE));
            ImGui::RadioButton("SQUARE",   &waveform, static_cast<s32>(Oscillator::Type::WAVE_SQUARE));
            ImGui::RadioButton("TRIANGLE", &waveform, static_cast<s32>(Oscillator::Type::WAVE_TRIANGLE));
            ImGui::RadioButton("DIGI SAW", &waveform, static_cast<s32>(Oscillator::Type::WAVE_DIGI_SAWTOOTH));
            ImGui::RadioButton("ANLG SAW", &waveform, static_cast<s32>(Oscillator::Type::WAVE_ANLG_SAWTOOTH));
            ImGui::RadioButton("WHITE",    &waveform, static_cast<s32>(Oscillator::Type::NOISE_WHITE));
            ImGui::EndGroup();

            osc.m_waveform = static_cast<Oscillator::Type>(waveform);
        }
        ImGui::End();
    }

    void Filter(Synthesizer& synth)
    {
        ImGui::Begin("Filter Type");
        {
            static s32 filter = synth.vafilter;
            ImGui::RadioButton("BIQUAD", &filter, 0); ImGui::SameLine();
            ImGui::RadioButton("VA", &filter, 1);
            synth.vafilter = static_cast<bool>(filter);
        }
        ImGui::End();

        // Filter
        if (synth.vafilter)
            VAFilter(synth);
        else
            BqFilter(synth);
    }

    void BqFilter(Synthesizer& synth)
    {
        ImGui::Begin("Biquad Filter");
        {
            static f64 resonance = 0.5;
            static f64 cutoff_freq = 2000.0;
            static s32 filter_type = static_cast<s32>(synth.m_filter.type);

            // Store previous values
            static f64 prev_resonance = resonance;
            static f64 prev_cutoff_freq = cutoff_freq;
            static s32 prev_filter_type = filter_type;

            ImGui::Text("   C     R   "); ImGui::SameLine();
            ImGui::BeginGroup();   ImGui::SameLine();
            ImGui::RadioButton("LPF",  &filter_type, static_cast<s32>(BqFilter::Type::LOW_PASS));   ImGui::SameLine();
            ImGui::RadioButton("HPF",  &filter_type, static_cast<s32>(BqFilter::Type::HIGH_PASS));  ImGui::SameLine();
            ImGui::RadioButton("BPF",  &filter_type, static_cast<s32>(BqFilter::Type::BAND_PASS));  ImGui::SameLine();
            //ImGui::RadioButton("APF",  &filter_type, static_cast<s32>(BqFilter::Type::ALL_PASS));   ImGui::SameLine();
            //ImGui::RadioButton("PEAK", &filter_type, static_cast<s32>(BqFilter::Type::PEAK));       ImGui::SameLine();
            //ImGui::RadioButton("NOTCH",&filter_type, static_cast<s32>(BqFilter::Type::NOTCH));      ImGui::SameLine();
            //ImGui::RadioButton("LSF",  &filter_type, static_cast<s32>(BqFilter::Type::LOW_SHELF));  ImGui::SameLine();
            //ImGui::RadioButton("HSF",  &filter_type, static_cast<s32>(BqFilter::Type::HIGH_SHELF)); ImGui::SameLine();
            ImGui::RadioButton("OFF",  &filter_type, static_cast<s32>(BqFilter::Type::OFF)); ImGui::SameLine();
            ImGui::EndGroup();

            // TODO: Knobs
            //ImGuiKnobs::KnobDouble("F", &cutoff_freq, 20.0, SAMPLE_RATE/2.0, 100.0f, "%.2fHz", ImGuiKnobVariant_Tick);
            VSliderDouble("##F", ImVec2(40, 300), &cutoff_freq, 20.0, SAMPLE_RATE / 2.0); ImGui::SameLine();
            VSliderDouble("##R", ImVec2(20, 300), &resonance, 0.0, 10.0, "%.2f");         ImGui::SameLine();

            if (resonance != prev_resonance || cutoff_freq != prev_cutoff_freq || filter_type != prev_filter_type)
            {
                synth.m_filter.type = static_cast<BqFilter::Type>(filter_type);
                synth.m_filter.CalcCoefs(cutoff_freq, resonance);

                // Update previous values
                prev_resonance   = resonance;
                prev_cutoff_freq = cutoff_freq;
                prev_filter_type = filter_type;
            }

            // Plot Frequency Response: Amplitude Response
            f64 nyquist_freq = SAMPLE_RATE / 2.0;
            const s32 sample_size = 1000;
            std::vector<f64> frequencies(sample_size, 0.0); // Hz
            std::vector<f64> magnitudes(sample_size, 0.0);  // dB
            for (s32 i = 0; i < sample_size; i++)
            {
                f64 freq = (static_cast<f64>(i) / sample_size) * nyquist_freq;
                frequencies[i] = freq;

                f64 mag = synth.m_filter.TransferFunction(freq);
                if (mag > 0.0) magnitudes[i] = volume_to_dB(mag);
                else           magnitudes[i] = -100.0;
            }

            if (ImPlot::BeginPlot("Amplitude Response", ImVec2(500, 300)))
            {
                ImPlot::SetupAxes("Frequency (Hz)", "Gain (dB)", 0, ImPlotAxisFlags_AutoFit);
                ImPlot::SetupAxisLimits(ImAxis_X1, 20.0, nyquist_freq, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -100, 10, ImGuiCond_Always); // Y-axis limits for dB (-100 dB to 10 dB)
                //ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);           // Logarithmic scale frequency (Hz)

                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 3.0f);
                ImPlot::PlotLine("##Filter Frequency", frequencies.data(), magnitudes.data(), frequencies.size());
                ImPlot::PopStyleVar();
                ImPlot::EndPlot();
            }
        }
        ImGui::End();
    }

    void VAFilter(Synthesizer& synth)
    {
        ImGui::Begin("VA Filter");
        {
            static f64 resonance = 0.5;
            static f64 cutoff_freq = 2000.0;
            static s32 filter_type = static_cast<s32>(synth.m_filter.type);

            // Store previous values
            static f64 prev_resonance = resonance;
            static f64 prev_cutoff_freq = cutoff_freq;
            static s32 prev_filter_type = filter_type;

            ImGui::Text("   C     R   "); ImGui::SameLine();
            ImGui::BeginGroup();          ImGui::SameLine();
            ImGui::RadioButton("LPF",  &filter_type, static_cast<s32>(VAFilter::Type::LOW_PASS));   ImGui::SameLine();
            ImGui::RadioButton("HPF",  &filter_type, static_cast<s32>(VAFilter::Type::HIGH_PASS));  ImGui::SameLine();
            ImGui::RadioButton("BPF",  &filter_type, static_cast<s32>(VAFilter::Type::BAND_PASS));  ImGui::SameLine();
            ImGui::RadioButton("OFF",  &filter_type, static_cast<s32>(VAFilter::Type::OFF));        ImGui::SameLine();
            ImGui::EndGroup();

            // TODO: Knobs
            //ImGuiKnobs::KnobDouble("F", &cutoff_freq, 20.0, SAMPLE_RATE/2.0, 100.0f, "%.2fHz", ImGuiKnobVariant_Tick);
            VSliderDouble("##F", ImVec2(40, 300), &cutoff_freq, 20.0, SAMPLE_RATE / 2.0); ImGui::SameLine();
            VSliderDouble("##R", ImVec2(20, 300), &resonance, 0.0, 1.0, "%.2f");          ImGui::SameLine();

            if (resonance != prev_resonance || cutoff_freq != prev_cutoff_freq || filter_type != prev_filter_type)
            {
                synth.m_vafilter.type = static_cast<VAFilter::Type>(filter_type);
                synth.m_vafilter.CalcCoefs(cutoff_freq, resonance);

                // Update previous values
                prev_resonance = resonance;
                prev_cutoff_freq = cutoff_freq;
                prev_filter_type = filter_type;
            }

            // Plot Frequency Response: Amplitude Response
            f64 nyquist_freq = SAMPLE_RATE / 2.0;
            const s32 sample_size = 1000;
            std::vector<f64> frequencies(sample_size, 0.0); // Hz
            std::vector<f64> magnitudes(sample_size, 0.0);  // dB
            for (s32 i = 0; i < sample_size; i++)
            {
                f64 freq = (static_cast<f64>(i) / sample_size) * nyquist_freq;
                frequencies[i] = freq;

                f64 mag = synth.m_vafilter.TransferFunction(freq);
                if (mag > 0.0) magnitudes[i] = volume_to_dB(mag);
                else           magnitudes[i] = -100.0;
            }

            static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks;
            if (ImPlot::BeginPlot("Amplitude Response", ImVec2(500, 300)))
            {
                ImPlot::SetupAxes("Frequency (Hz)", "Gain (dB)", 0, ImPlotAxisFlags_AutoFit);
                ImPlot::SetupAxisLimits(ImAxis_X1, 20.0, nyquist_freq, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -100, 10, ImGuiCond_Always); // Y-axis limits for dB (-100 dB to 10 dB)
                //ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);         // Logarithmic scale frequency (Hz)

                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 3.0f);
                ImPlot::PlotLine("##Filter Frequency", frequencies.data(), magnitudes.data(), frequencies.size());
                ImPlot::PopStyleVar();
                ImPlot::EndPlot();
            }
        }
        ImGui::End();
    }


    void ReverbEffect(Synthesizer& synth)
    {
        ImGui::Begin("Reverb");
        {

            static f64 room   = 1.0;
            static f64 spread = 0.2;
            static f64 damp   = 0.2;
            static f64 decay  = 1.0;
            static f64 dry    = 0.0;
            static f64 wet    = 0.0;

            static f64 prev_room = room;
            static f64 prev_spread = spread;
            static f64 prev_damp = damp;
            static f64 prev_decay = decay;
            static f64 prev_dry = dry;
            static f64 prev_wet = wet;

            ImVec2 osc_slider_size(20, 150);
            ImGui::Text("RM  SPR DMP DCY DRY WET"); ImGui::SameLine();
            ImGui::Checkbox("Mute", &synth.reverb); 
            VSliderDouble("##R",  osc_slider_size, &room,   0.1, 10.0); ImGui::SameLine();
            VSliderDouble("##S",  osc_slider_size, &spread, 0.1, 1.0);  ImGui::SameLine();
            VSliderDouble("##DA", osc_slider_size, &damp,   0.0, 1.0);  ImGui::SameLine();
            VSliderDouble("##DC", osc_slider_size, &decay,  0.1, 1.0);  ImGui::SameLine();
            VSliderDouble("##DR", osc_slider_size, &dry,   -60.0, 0.0); ImGui::SameLine();
            VSliderDouble("##WT", osc_slider_size, &wet,   -60.0, 0.0); ImGui::SameLine();

            if (room != prev_room || spread != prev_spread || damp != prev_damp || decay != prev_decay || dry != prev_dry || wet != prev_wet)
            {
                synth.m_reverb.room   = room;
                synth.m_reverb.spread = spread;
                synth.m_reverb.damp   = damp;
                synth.m_reverb.decay  = decay;
                synth.m_reverb.dry    = dry;
                synth.m_reverb.wet    = wet;

                synth.m_reverb.ComputeFilterDelays();

                prev_room   = room;
                prev_spread = spread;
                prev_damp   = damp;
                prev_decay  = decay;
                prev_dry    = dry;
                prev_wet    = wet;
            }
        }
        ImGui::End();
    }

    void DelayEffect(Synthesizer& synth)
    {
        ImGui::Begin("Delay");
        {
            ImVec2 osc_slider_size(20, 150);
            ImGui::Text("B  BPB BPM FDBK"); ImGui::SameLine();
            ImGui::Checkbox("Mute", &synth.delay);
            ImGui::VSliderInt("##B",   osc_slider_size, &synth.m_delay.beat, 1, 16);         ImGui::SameLine();
            ImGui::VSliderInt("##BPB", osc_slider_size, &synth.m_delay.beat_per_bar, 1, 8); ImGui::SameLine();
            ImGui::VSliderInt("##BPM", osc_slider_size, &synth.m_delay.bpm, 40, 200);        ImGui::SameLine();
            VSliderDouble("##F",       osc_slider_size, &synth.m_delay.feedback, 0.0, 1.0);
        }
        ImGui::End();
    }

    void Eq(Synthesizer& synth)
    {
        ImGui::Begin("Equalizer");
        {
            static constexpr char const* mode_names[] = {
                "Low Pass",
                "High Pass",
                "Band Pass",
                "Peak",
                "Notch",
                "Low Shelf",
                "High Shelf"
            };

            // Custom ticks for the X-axis (Frequency in Hz)
            static constexpr double x_ticks[] = {
                20.0,   30.0,   40.0,   60.0,   80.0,   100.0,
                200.0,  300.0,  400.0,  600.0,  800.0,  1000.0,
                2000.0, 3000.0, 4000.0, 6000.0, 8000.0, 10000.0,
                20000.0
            };
            static constexpr char const* tick_labels[] = {
                "20",  "30",  "40",  "60",  "80",  "100",
                "200", "300", "400", "600", "800", "1K",
                "2K",  "3K",  "4K",  "6K",  "8K",  "10K",
                "20K"
            };

            // Graphical Band controls
            static constexpr s32 N = 4*1024;
            std::vector<f64> frequencies(N, 0.0); // Hz
            std::vector<f64> magnitudes(N, 0.0);  // dB
            for (s32 i = 0; i < N; i++) 
            {
                f64 freq = log_interpolate(20.0, 20000.0, (i + 0.5) / N);
                f64 sqrt_phi = std::sin(PI * freq * (1.0 / SAMPLE_RATE));
                f64 phi = sqrt_phi * sqrt_phi;
                f64 response = 0.0;

                for (s32 b = 0; b < NUM_BANDS; b++)
                {
                    const auto& band = synth.m_eq.bands[b];
                    f64 b0 = band.filter.b0;
                    f64 b1 = band.filter.b1;
                    f64 b2 = band.filter.b2;
                    f64 a1 = band.filter.a1;
                    f64 a2 = band.filter.a2;

                    f64 b012 = 0.5 *  (b0 + b1 + b2);
                    f64 a012 = 0.5 * (1.0 + a1 + a2);

                    f64 numerator   = b012 * b012 - phi * (4.0 * b0 * b2 * (1.0 - phi) + b1 * (b0 + b2));
                    f64 denominator = a012 * a012 - phi * (4.0 * a2 * (1.0 - phi) + a1 * (1.0 + a2));
                    response += 10.0 * (std::log10(numerator) - std::log10(denominator));
                }

                frequencies[i] = freq;
                magnitudes[i] = response;
            }

            ImVec2 space = ImVec2(600, 300);
            if (ImPlot::BeginPlot("EQ", space))
            {
                ImPlot::SetupAxes("Frequency (Hz)", "Gain (dB)", 0, ImPlotAxisFlags_AutoFit);
                ImPlot::SetupAxisLimits(ImAxis_X1, 20.0, 20000.0, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -18, 18, ImGuiCond_Always); // Y-axis limits for dB (-18 dB to 18 dB)
                ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Log10);          // Logarithmic scale frequency (Hz)
                ImPlot::SetupAxisTicks(ImAxis_X1, x_ticks, 19, tick_labels);
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 3.0f);


                ImPlot::PlotLine("", frequencies.data(), magnitudes.data(), frequencies.size());

                // Band EQ drag point
                char label[32] = {};
                char popup_label[32] = {};
                for (s32 b = 0; b < NUM_BANDS; b++) 
                {
                    auto& band = synth.m_eq.bands[b];
                    sprintf_s(label, "#%i", b);

                    ImPlot::DragPoint(b, &band.frequency, &band.gain, ImVec4(0, 0.9, 0, 1));

                    // Resonance Control
                    if (ImGui::IsItemHovered()) 
                        band.resonance = std::clamp(band.resonance - 0.1 * ImGui::GetIO().MouseWheel, 0.0, MAX_Q);

                    ImPlot::PlotText(label, band.frequency, band.gain, ImVec2(0, -12));
                }

                ImPlot::PopStyleVar();
                ImPlot::EndPlot();
            }
            ImGui::SameLine();
            static s32 mode = 0;
            for (s32 b = 0; b < NUM_BANDS; b++)
            {
                Equalizer::Band& band = synth.m_eq.bands[b];
                band.mode = mode;
            }

            ImGui::BeginGroup();
            ImGui::Checkbox("Mute",     &synth.eq);
            ImGui::RadioButton("LPF",   &mode, 0);
            ImGui::RadioButton("HPF",   &mode, 2);
            ImGui::RadioButton("BPF",   &mode, 1);
            ImGui::RadioButton("PEAK",  &mode, 3);
            ImGui::RadioButton("NOTCH", &mode, 4);
            ImGui::RadioButton("LSF",   &mode, 5);
            ImGui::RadioButton("HSF",   &mode, 6);
            ImGui::EndGroup();

            for (s32 b = 0; b < NUM_BANDS; b++)
            {
                Equalizer::Band& band = synth.m_eq.bands[b];
                ImGui::BeginGroup();
                ImGui::Text("F: %.1f", band.frequency);
                ImGui::Text("R: %.1f", band.resonance); 
                ImGui::Text("G: %.1f", band.gain);      
                ImGui::EndGroup(); 
                ImGui::SameLine();
            }
        }
        ImGui::End();
    }


private:
    ImGuiIO io;
    bool show_imgui_demo  = false;
    bool show_implot_demo = false;
    
    // Control
    bool play = true;
};