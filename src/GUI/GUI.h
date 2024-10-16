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

    void Display(WaveData& wave_data, std::unordered_map<std::string, Oscillator>& oscs, f32 master_volume, std::vector<note>& notes)
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
                ImGui::SliderFloat("Master Volume", &master_volume, 0.0f, 1.0f);
                ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.25f);
                ImGui::SliderFloat("OSC 1 Volume", &oscs["OSC1"].volume, 0.0f, 1.0f);
            }
            ImGui::End();

            // Envelope
            ImVec2 size(20, 160);
            ImGui::Begin("Envelope");
            {
                ImGui::Text(" A   D   S   R");
                ImGui::VSliderFloat("##A", size, &oscs["OSC1"].m_envelope.attack_time,       0.0f, 10.0f); ImGui::SameLine();
                ImGui::VSliderFloat("##D", size, &oscs["OSC1"].m_envelope.decay_time,        0.0f, 10.0f); ImGui::SameLine();
                ImGui::VSliderFloat("##S", size, &oscs["OSC1"].m_envelope.sustain_amplitude, 0.0f, 1.0f);  ImGui::SameLine();
                ImGui::VSliderFloat("##R", size, &oscs["OSC1"].m_envelope.release_time,      0.0f, 10.0f);
            }
            ImGui::End();

            // Oscilloscope
            ImGui::Begin("Oscilloscope");
            {
                ImGui::Text("Waveform: %s",         wave_str(wave_data.waveform).c_str());
                ImGui::Text("Amplitude: %.2f",      wave_data.wave.amplitude);
                ImGui::Text("Frequency (Hz): %.2f", wave_data.wave.frequency);

                std::string s;
                for (auto& n : notes)
                    s += std::format("{} ", note_str(n.id));
                ImGui::Text("Note %s", s.c_str());

                const u32 sample_size = SAMPLE_RATE / 100;
                std::vector<f32> ts(sample_size, 0.0f);
                std::vector<f32> ss(sample_size, 0.0f);

                for (auto& [id, osc] : oscs)
                {
                    ts = std::vector<f32>(sample_size, 0.0f);
                    ss = std::vector<f32>(sample_size, 0.0f);

                    for (u32 i = 0; i < sample_size; i++)
                    {
                        ts[i] = i;
                        ss[i] = wave_data.samples[i];
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