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
void ImGuiUser::inputslider_uint32_t(retrogames::cfgvalue_t* cfgvalue, const std::string& name, uint32_t max, uint32_t min/* = 0u*/, const std::string& desc/* = ""*/)
{
    ImGui::Text("%s:", name.c_str());

    if (!desc.empty())
    {
        ImGui::SameLine();

        help_marker(desc);
    }

    auto val = static_cast<int32_t>(cfgvalue->get<uint32_t>());
    auto avail_width = ImGui::GetContentRegionAvailWidth();
    auto input_width = std::ceil(avail_width / 7.f);

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