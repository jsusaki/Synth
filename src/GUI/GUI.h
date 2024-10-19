#pragma once

#include <complex>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include "../Core/Common.h"
#include "../Audio/AudioEngine.h"

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

    void Display(Synthesizer& synth)
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        {
            // Info
            General(synth);   

            // Oscillator Control
            static s32 wf1 = static_cast<s32>(synth.GetOscillator("OSC1").m_waveform);
            static s32 wf2 = static_cast<s32>(synth.GetOscillator("OSC2").m_waveform);
            static s32 wf3 = static_cast<s32>(synth.GetOscillator("OSC3").m_waveform);
            static bool mute1 = false, mute2 = false, mute3 = false;
            Oscillator(synth.GetOscillator("OSC1"), "OSC1", wf1, mute1);
            Oscillator(synth.GetOscillator("OSC2"), "OSC2", wf2, mute2);
            Oscillator(synth.GetOscillator("OSC3"), "OSC3", wf3, mute3);

            // Oscilloscope Control
            Oscilloscope(synth);

            // Envelope Control
            static s32 decay_function = static_cast<s32>(synth.m_envelope.decay_function);
            Envelope(synth, decay_function);

            // Volume Control
            Mixer(synth);

            // Filter Control
            Filter(synth);
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

    void General(Synthesizer& synth)
    {
        ImGui::Begin("Sound Synthesizer");
        {
            ImGui::SeparatorText("General");
            ImGui::Text("FPS: average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("Sample Rate (Hz): %.0f", SAMPLE_RATE);
            ImGui::Text("Channels: %d", CHANNELS);
            // TODO: List Devices and select 

            ImGui::Checkbox("Show ImGui Demo", &show_imgui_demo);
            ImGui::Checkbox("Show ImPlot Demo", &show_implot_demo);
            if (show_imgui_demo)  ImGui::ShowDemoWindow();
            if (show_implot_demo) ImPlot::ShowDemoWindow();
        }
        ImGui::End();
    }

    void Oscilloscope(Synthesizer& synth)
    {
        ImGui::Begin("Oscilloscope");
        {
            std::string f, s;
            for (auto& n : synth.notes)
            {
                f += std::format("{:.2f} ", note_to_freq(n.id));
                s += std::format("{} ",     note_str(n.id));
            }

            ImGui::Text("Frequency (Hz): %s", f.c_str());
            ImGui::Text("Note            %s", s.c_str());

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
                ImPlot::SetupAxes(nullptr, nullptr, flags | ImPlotAxisFlags_NoGridLines, flags);
                ImPlot::SetupAxisLimits(ImAxis_X1, 0, sample_size, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -1, 1);
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

    void Envelope(Synthesizer& synth,s32& decay_function)
    {
        ImVec2 slider_size(20, 300);
        ImVec2 plot_size(500, 300);
        ImGui::Begin("Envelope Generator");
        {
            ImGui::Text(" A   D   S   R"); ImGui::SameLine();
            ImGui::BeginGroup(); ImGui::SameLine();
            ImGui::RadioButton("LINEAR", &decay_function, static_cast<s32>(Envelope::Decay::LINEAR));      ImGui::SameLine();
            ImGui::RadioButton("EXPO",   &decay_function, static_cast<s32>(Envelope::Decay::EXPONENTIAL)); ImGui::SameLine();
            ImGui::RadioButton("QUAD",   &decay_function, static_cast<s32>(Envelope::Decay::QUADRATIC));
            ImGui::EndGroup();

            VSliderDouble("##A", slider_size, &synth.m_envelope.attack_time, 0.0, 10.0);      ImGui::SameLine();
            VSliderDouble("##D", slider_size, &synth.m_envelope.decay_time, 0.0, 10.0);       ImGui::SameLine();
            VSliderDouble("##S", slider_size, &synth.m_envelope.sustain_amplitude, 0.0, 1.0); ImGui::SameLine();
            VSliderDouble("##R", slider_size, &synth.m_envelope.release_time, 0.0, 10.0);     ImGui::SameLine();

            synth.m_envelope.decay_function = static_cast<Envelope::Decay>(decay_function);

            // Compute ADSR parametric curve
            s32 num_points = 1000;
            f64 time_step = 1 / SAMPLE_RATE;
            f64 note_on_time = 0.00001;
            f64 note_off_time = 0.0f;
            f64 global_time = 0.0f;

            std::vector<f64> as(num_points, 0.0);
            std::vector<f64> ts(num_points, 0.0);
            for (s32 i = 0; i < num_points; i++)
            {
                f64 t = i * time_step;
                if (i == 600) note_off_time = global_time;

                f64 amplitude = synth.m_envelope.Amplitude(global_time, note_on_time, note_off_time);

                ts[i] = i;
                as[i] = amplitude;

                global_time += t;
            }

            static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks;
            if (ImPlot::BeginPlot("ADSR Envelope", plot_size))
            {
                ImPlot::SetupAxes(nullptr, nullptr, flags | ImPlotAxisFlags_NoGridLines, ImPlotAxisFlags_NoTickMarks);
                ImPlot::SetupAxisLimits(ImAxis_X1, 0, num_points, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);
                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 3.0f);
                ImPlot::PlotLine("##ADSR Curve", ts.data(), as.data(), ts.size());
                ImPlot::EndPlot();
            }
        }
        ImGui::End();
    }

    void Oscillator(Oscillator& osc, std::string label, s32& waveform, bool& mute)
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

    void Filter(Synthesizer& synth)
    {
        ImVec2 slider_size(40, 300);
        ImGui::Begin("Filter");
        {
            static f64 resonance = 0.7;
            static f64 cutoff_freq = 10000.0;
            //static s32 filter_type = static_cast<s32>(synth.m_filter.type);
            static s32 filter_type = static_cast<s32>(synth.m_vafilter.type);

            // Store previous values
            static f64 prev_resonance = resonance;
            static f64 prev_cutoff_freq = cutoff_freq;
            static s32 prev_filter_type = filter_type;

            ImGui::Text(" C   R"); ImGui::SameLine();
            ImGui::BeginGroup();   ImGui::SameLine();
            ImGui::RadioButton("LPF",  &filter_type, static_cast<s32>(VAFilter::Type::LOW_PASS));   ImGui::SameLine();
            ImGui::RadioButton("HPF",  &filter_type, static_cast<s32>(VAFilter::Type::HIGH_PASS));  ImGui::SameLine();
            ImGui::RadioButton("BPF",  &filter_type, static_cast<s32>(VAFilter::Type::BAND_PASS));  ImGui::SameLine();
            ImGui::RadioButton("OFF",  &filter_type, static_cast<s32>(VAFilter::Type::OFF));        ImGui::SameLine();

            //ImGui::RadioButton("LPF",  &filter_type, static_cast<s32>(Filter::Type::LOW_PASS));   ImGui::SameLine();
            //ImGui::RadioButton("HPF",  &filter_type, static_cast<s32>(Filter::Type::HIGH_PASS));  ImGui::SameLine();
            //ImGui::RadioButton("BPF",  &filter_type, static_cast<s32>(Filter::Type::BAND_PASS));  ImGui::SameLine();
            //ImGui::RadioButton("APF",  &filter_type, static_cast<s32>(Filter::Type::ALL_PASS));   ImGui::SameLine();
            //ImGui::RadioButton("PEAK", &filter_type, static_cast<s32>(Filter::Type::PEAK));       ImGui::SameLine();
            //ImGui::RadioButton("NOTCH",&filter_type, static_cast<s32>(Filter::Type::NOTCH));      ImGui::SameLine();
            //ImGui::RadioButton("LSF",  &filter_type, static_cast<s32>(Filter::Type::LOW_SHELF));  ImGui::SameLine();
            //ImGui::RadioButton("HSF",  &filter_type, static_cast<s32>(Filter::Type::HIGH_SHELF)); ImGui::SameLine();
            ImGui::EndGroup();

            VSliderDouble("##F", slider_size, &cutoff_freq, 0.0, SAMPLE_RATE / 2.0); ImGui::SameLine();
            VSliderDouble("##R", slider_size, &resonance, 0.0, 2.0);                 ImGui::SameLine();

            if (resonance != prev_resonance || cutoff_freq != prev_cutoff_freq || filter_type != prev_filter_type)
            {
                synth.m_vafilter.type = static_cast<VAFilter::Type>(filter_type);

                //synth.m_filter.CalcCoefs(cutoff_freq, resonance);
                synth.m_vafilter.CalcCoefs(cutoff_freq, resonance);

                // Update previous values
                prev_resonance   = resonance;
                prev_cutoff_freq = cutoff_freq;
                prev_filter_type = filter_type;
            }


            // TODO: Plot Frequency and Magnitude
            //s32 num_points  = 1000;
            f64 sample_rate = 44100.0;
            f64 freq_step = (sample_rate / 2);
            s64 sample_size = 20000;
            std::vector<f64> frequencies(20000, 0.0);
            std::vector<f64> magnitudes(sample_size, 0.0);
            for (s32 i = 0; i < sample_size; i++)
            {
                f64 freq = i * freq_step;
                frequencies[i] = freq;
            }

            ImVec2 plot_size(500, 300);
            static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks;
            if (ImPlot::BeginPlot("Filter Frequency Response", ImVec2(plot_size)))
            {
                ImPlot::SetupAxes(nullptr, nullptr, flags | ImPlotAxisFlags_NoGridLines, ImPlotAxisFlags_NoTickMarks);
                ImPlot::SetupAxisLimits(ImAxis_X1, 0, sample_rate / 2, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);

                ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 3.0f);
                ImPlot::PlotLine("##Frequency Response", frequencies.data(), magnitudes.data(), frequencies.size());
                ImPlot::EndPlot();
            }
        }
        ImGui::End();
    }

private:
    ImGuiIO io;
    bool show_imgui_demo = false;
    bool show_implot_demo = false;
    
    // Control
    bool play = true;
};