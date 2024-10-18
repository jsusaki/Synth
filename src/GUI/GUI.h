#pragma once

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

static std::pair<std::vector<f32>, std::vector<f32>> ComputeEnvelopeCurve(Envelope& envelope, f32 duration, s32 num_points)
{
    std::vector<f32> as;
    std::vector<f32> ts;
    ts.reserve(num_points);
    as.reserve(num_points);

    f32 time_step = 1 / SAMPLE_RATE;
    f32 note_on_time  = 0.0f;
    f32 note_off_time = 0.0f;
    f32 global_time   = 0.0f;

    for (s32 i = 0; i < num_points; i++)
    {
        f32 t = time_step * i;

        f32 amplitude = envelope.Amplitude(global_time, note_on_time, note_off_time);

        ts.push_back(i);
        as.push_back(amplitude);

        global_time += t;
    }

    return std::make_pair(ts, as);
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
            // Sound Synthesizer
            ImGui::Begin("Sound Synthesizer");
            {
                ImGui::SeparatorText("General");
                ImGui::Text("FPS: average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                ImGui::Text("Sample Rate (Hz): %.0f", SAMPLE_RATE);
                ImGui::Text("Channels: %d", CHANNELS);

                ImGui::Checkbox("Show ImGui Demo", &show_imgui_demo);
                ImGui::Checkbox("Show ImPlot Demo", &show_implot_demo);
                if (show_imgui_demo)  ImGui::ShowDemoWindow();
                if (show_implot_demo) ImPlot::ShowDemoWindow();
                ImGui::End();

            }
            
            // Volume Control
            ImGui::Begin("Mixer");
            {
                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
                SliderDouble("Master Volume", &synth.m_master_volume, 0.0, 1.0);
                SliderDouble("OSC 1 Volume", &synth.GetOscillator("OSC1").volume, 0.0, 1.0);
            }
            ImGui::End();

            // Envelope 
            ImVec2 slider_size(20, 300);
            ImVec2 plot_size(500, 300);
            ImGui::Begin("Envelope Generator");
            {
                ImGui::Text(" A   D   S   R");
                VSliderDouble("##A", slider_size, &synth.m_envelope.attack_time,       0.0, 10.0); ImGui::SameLine();
                VSliderDouble("##D", slider_size, &synth.m_envelope.decay_time,        0.0, 10.0); ImGui::SameLine();
                VSliderDouble("##S", slider_size, &synth.m_envelope.sustain_amplitude, 0.0, 1.0);  ImGui::SameLine();
                VSliderDouble("##R", slider_size, &synth.m_envelope.release_time,      0.0, 10.0); ImGui::SameLine();
            
                s32 num_points    = 1000;
                f32 time_step     = 1 / SAMPLE_RATE;
                f32 note_on_time  = 0.00001f;
                f32 note_off_time = 0.0f;
                f32 global_time   = 0.0f;

                std::vector<f32> as(num_points, 0.0f);
                std::vector<f32> ts(num_points, 0.0f);
                for (s32 i = 0; i < num_points; i++)
                {
                    f32 t = i * time_step;
                    if (i == 600) note_off_time = global_time;

                    f32 amplitude = synth.m_envelope.Amplitude(global_time, note_on_time, note_off_time);

                    ts[i] = i;
                    as[i] = amplitude;

                    global_time += t;
                }

                static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks;
                if (ImPlot::BeginPlot("ADSR Envelope", ImVec2(plot_size)))
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

            // Oscilloscope
            ImGui::Begin("Oscilloscope");
            {
                ImGui::Text("Waveform: %s",         wave_str(synth.wave_data.waveform).c_str());
                ImGui::Text("Amplitude: %.2f",      synth.wave_data.wave.amplitude);
                std::string f, s;
                for (auto& n : synth.notes)
                {
                    f += std::format("{:.2f} ", note_to_freq(n.id));
                    s += std::format("{} ", note_str(n.id));
                }
                ImGui::Text("Frequency (Hz): %s", f.c_str());
                ImGui::Text("Note            %s", s.c_str());

                const u32 sample_size = SAMPLE_RATE / 100;
                std::vector<f32> ts(sample_size, 0.0f);
                std::vector<f32> ss(sample_size, 0.0f);

                for (auto& [id, osc] : synth.oscillators)
                {
                    ts = std::vector<f32>(sample_size, 0.0f);
                    ss = std::vector<f32>(sample_size, 0.0f);

                    for (u32 i = 0; i < sample_size; i++)
                    {
                        ts[i] = i;
                        ss[i] = synth.wave_data.samples[i];
                    }
                }

                // TODO: make wave more visible
                static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoTickMarks;
                if (ImPlot::BeginPlot("##Waveform"))
                {
                    ImPlot::SetupAxes(nullptr, nullptr, flags | ImPlotAxisFlags_NoGridLines, flags);
                    ImPlot::SetupAxisLimits(ImAxis_X1, 0, sample_size, ImGuiCond_Always);
                    ImPlot::SetupAxisLimits(ImAxis_Y1, -1, 1);
                    ImPlot::PushStyleVar(ImPlotStyleVar_LineWeight, 3.0f);
                    ImPlot::PlotLine("Wave", ts.data(), ss.data(), ts.size());
                    ImPlot::PopStyleVar();
                    ImPlot::EndPlot();
                }
            }
            ImGui::End();

            // TODO: Waveform radio button, amplitude, frequency
            //ImGui::Begin("Oscillator 1");
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

private:
    ImGuiIO io;
    // demo reference
    bool show_imgui_demo = false;
    bool show_implot_demo = false;
    
    // Control
    bool play = true;
};