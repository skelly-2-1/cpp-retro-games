/*
@file

    imgui_user.cpp

@purpose

    Own functions for easily implementing ImGui functionality
*/

#include "imgui_user.h"
#include <cmath>

/*
@brief

    Helper for tooltips, taken from the ImGui demo .cpp
*/
void ImGuiUser::help_marker(const std::string& desc)
{
     ImGui::TextDisabled("(?)");

    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

/*
@brief

    Draws a slider + input int given the range and description
*/
void ImGuiUser::inputslider_uint32_t(retrogames::cfgvalue_t* cfgvalue, const std::string& name, uint32_t max, uint32_t min/* = 0u*/, const std::string& desc/* = ""*/, float scaling/* = 1.f*/)
{
    ImGui::Text("%s:", name.c_str());

    if (!desc.empty())
    {
        ImGui::SameLine();

        help_marker(desc);
    }

    auto val = static_cast<int32_t>(cfgvalue->get<uint32_t>());
    auto avail_width = ImGui::GetContentRegionAvailWidth();
    auto input_width = std::ceil(avail_width / 7.f) * scaling;

    ImGui::PushItemWidth(input_width);
    ImGui::InputInt((std::string("##") + name).c_str(), &val);
    ImGui::PopItemWidth();
    ImGui::SameLine();

    auto slider_width = ImGui::GetContentRegionAvailWidth();

    ImGui::PushItemWidth(slider_width);
    ImGui::SliderInt((std::string("##") + name + "2").c_str(), &val, static_cast<int32_t>(min), static_cast<int32_t>(max));
    ImGui::PopItemWidth();

    val = std::max(val, static_cast<int32_t>(min));
    val = std::min(val, static_cast<int32_t>(max));

    cfgvalue->set<uint32_t>(static_cast<uint32_t>(val));
}

/*
@brief

    Draws a toggle button
*/
void ImGuiUser::toggle_button(retrogames::cfgvalue_t* cfgvalue, const std::string& name, const std::string& desc/* = ""*/)
{
    auto helplen = ImGui::CalcTextSize("(?)").x + ImGui::GetStyle().FramePadding.x + ImGui::GetStyle().ItemInnerSpacing.x;
    auto offlen = ImGui::CalcTextSize("OFF").x + ImGui::GetStyle().FramePadding.x + ImGui::GetStyle().ItemInnerSpacing.x;

    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0.f, .5f});
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));

    auto curval = cfgvalue->get<bool>();
    auto button_size = ImVec2(ImGui::GetContentRegionAvailWidth() - offlen, 0.f);

    if (!desc.empty()) button_size.x -= helplen;
    if (!curval) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    if (ImGui::Button(name.c_str(), button_size)) cfgvalue->set(!curval);
    if (!curval) ImGui::PopStyleColor();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    ImGui::SameLine();

    if (!curval) ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));

    ImGui::Text("%s", cfgvalue->get<bool>() ? "ON" : "OFF");

    if (!curval) ImGui::PopStyleColor();

    if (!desc.empty())
    {
        ImGui::SameLine();

        if (cfgvalue->get<bool>())
        {
            auto lendiff = ImGui::CalcTextSize("OFF").x - ImGui::CalcTextSize("ON").x;
            auto screen_pos = ImGui::GetCursorScreenPos();

            screen_pos.x += lendiff;

            ImGui::SetCursorScreenPos(screen_pos);
        }

        help_marker(desc);
    }
}

/*
@brief

    Adds a frame-height spacing
*/
void ImGuiUser::frame_height_spacing(uint8_t num/* = 1*/)
{
    for (uint8_t i = 0; i < num; i++)
    {
        auto cspos = ImGui::GetCursorScreenPos();

        ImGui::SetCursorScreenPos({cspos.x, cspos.y+ImGui::GetFrameHeight()});
    }
}

/*
@brief

    Converts a color_t to an ImGui color (ImVec4)
*/
ImVec4 ImGuiUser::color_to_imgui_color_vec4(const retrogames::color_t& color)
{
    return ImVec4{static_cast<float>(color.r()) / 255.f, static_cast<float>(color.g()) / 255.f, static_cast<float>(color.b()) / 255.f, static_cast<float>(color.a()) / 255.f};
}

/*
@brief

    Converts a color_t to an ImGui color (u32)
*/
ImU32 ImGuiUser::color_to_imgui_color_u32(const retrogames::color_t& color)
{
    return ImGui::GetColorU32(color_to_imgui_color_vec4(color));
}

/*
@brief

    Draws information on the screen
*/
void ImGuiUser::draw_info(const ImVec2& pos, std::string info)
{
    auto border_color = ImGui::GetStyleColorVec4(ImGuiCol_Border);
    auto text_color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    auto background_color = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
    auto size = ImVec2{ImGui::CalcTextSize(info.c_str()).x + ImGui::GetStyle().FramePadding.x * 2.f + ImGui::GetStyle().ItemSpacing.x,ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y * 2.f};
    auto pos_ = ImVec2{pos.x - size.x * .5f, pos.y - size.y * .5f};

    ImGui::SetNextWindowPos(pos_, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, background_color);
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleColor(ImGuiCol_Border, border_color);

    if (ImGui::Begin("##notification", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoInputs))
    {
        ImGui::TextUnformatted(info.c_str());
        ImGui::End();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);
}