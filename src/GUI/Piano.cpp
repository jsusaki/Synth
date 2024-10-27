#include "../lib/imgui/imgui.h"

#include "Piano.h"

static const std::string key_names[18] = {"Z", "S", "X", "D", "C", "V", "G", "B", "H", "N", "J", "M", ",", "L", ".", ";", "\\"};

static bool has_black(s32 key) 
{
    return (!((key - 1) % 7 == 0 || (key - 1) % 7 == 3) && key != 51);
}

void Piano::up(s32 key) 
{
    key_states[key] = 0;
}

void Piano::down(s32 key, s32 velocity)
{
    key_states[key] = velocity;
}

void Piano::Render() 
{
    ImU32 Black = IM_COL32(0, 0, 0, 255);
    ImU32 White = IM_COL32(255, 255, 255, 255);
    ImU32 Gray  = IM_COL32(128, 128, 128, 192);

    ImGui::Begin("MIDI Keyboard");
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 p = ImGui::GetCursorScreenPos();
        s32 width = 20;

        ImFont* font = ImGui::GetFont();

        s32 cur_key = 21;
        for (s32 key = 0; key < 52; key++)
        {
            ImU32 col = White;
            if (key_states[cur_key]) col = Gray;

            // Draw white key
            ImVec2 key_pos_min = ImVec2(p.x + key * width, p.y);
            ImVec2 key_pos_max = ImVec2(p.x + key * width + width, p.y + 120);
            draw_list->AddRectFilled(key_pos_min, key_pos_max, col, 0, ImDrawFlags_RoundCornersAll);
            draw_list->AddRect(key_pos_min, key_pos_max, Black, 0, ImDrawFlags_RoundCornersAll);

            if (cur_key >= 48 && cur_key < 66)
            {
                const char* key_label = key_names[(cur_key - 12) % 18].c_str();
                ImVec2 text_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, key_label);
                ImVec2 text_pos = ImVec2(key_pos_min.x + (width - text_size.x) / 2, key_pos_min.y + (120 - text_size.y) / 2 + 40);
                draw_list->AddText(text_pos, Black, key_label);
            }

            cur_key++;
            if (has_black(key)) cur_key++;
        }

        cur_key = 22;
        for (int key = 0; key < 52; key++)
        {
            if (has_black(key))
            {
                ImU32 col = Black;
                if (key_states[cur_key]) col = Gray;

                // Draw black key
                ImVec2 black_key_pos_min = ImVec2(p.x + key * width + width * 3 / 4, p.y);
                ImVec2 black_key_pos_max = ImVec2(p.x + key * width + width * 5 / 4 + 1, p.y + 80);
                draw_list->AddRectFilled(black_key_pos_min, black_key_pos_max, col, 0, ImDrawFlags_RoundCornersAll);
                draw_list->AddRect(black_key_pos_min, black_key_pos_max, Black, 0, ImDrawFlags_RoundCornersAll);

                if (cur_key >= 48 && cur_key < 66)
                {
                    const char* key_label = key_names[(cur_key - 12) % 18].c_str();
                    ImVec2 text_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, 0.0f, key_label);
                    ImVec2 text_pos = ImVec2(black_key_pos_min.x + (width * 0.5f - text_size.x) / 2, black_key_pos_min.y + (80 - text_size.y) / 2 + 20);
                    draw_list->AddText(text_pos, White, key_label);
                }

                cur_key += 2;
            }
            else
            {
                cur_key++;
            }
        }
    }
    ImGui::End();
}

std::vector<int> Piano::current_notes() 
{
    std::vector<int> result;
    for (int i = 0; i < 256; i++) 
    {
        if (key_states[i]) 
            result.push_back(i);

    }
    return result;
}