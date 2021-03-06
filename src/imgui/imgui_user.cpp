/*
@file

    imgui_user.cpp

@purpose

    Own functions for easily implementing ImGui functionality
*/

#include "imgui_user.h"
#include "imgui_internal.h"
#include <cmath>

uint8_t ImGuiUser::current_modal_popup_id = 0;

/*
@brief

    Constructor (from modal_popup_wrapper_t)
*/
ImGuiUser::modal_popup_t::modal_popup_t(const char* name, bool darkening/* = false*/) : started_popup(false), darkening(false)
{
    // Create a dummy window for the ImGui context (needed when we switch between windowed and fullscreen)
    // don't ask me why...
    if (current_modal_popup_id == 0 && !(started_window = ImGui::Begin((std::string("##no") + std::to_string(current_modal_popup_id++)).c_str(), nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoNavFocus))) return;

    if (!(this->darkening = darkening))
    {
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDarkening, { 0.f, 0.f, 0.f, 0.f });
        ImGui::PushStyleColor(ImGuiCol_ModalWindowDimBg, { 0.f, 0.f, 0.f, 0.f });
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::OpenPopup(name);

    started_popup = ImGui::BeginPopupModal(name, nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
}

/*
@brief

    Destructor (from modal_popup_wrapper_t)
*/
ImGuiUser::modal_popup_t::~modal_popup_t()
{
    current_modal_popup_id--;

    if (!started_window) return;
    if (started_popup) ImGui::EndPopup();
    if (!darkening) ImGui::PopStyleColor(2);

    ImGui::PopStyleVar(3);

    if (current_modal_popup_id == 0) ImGui::End();
}

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
void ImGuiUser::inputslider_uint32_t(retrogames::cfgvalue_t* cfgvalue, const std::string& name, uint32_t max, uint32_t min/* = 0u*/, const std::string& desc/* = ""*/, float scaling/* = 1.f*/, const char* format/* = "%d"*/)
{
    ImGui::Text("%s:", name.c_str());

    if (!desc.empty())
    {
        ImGui::SameLine();

        help_marker(desc);
    }

    auto val = static_cast<int32_t>(cfgvalue->get<uint32_t>());
    auto avail_width = ImGui::GetContentRegionAvailWidth();
    auto input_width = std::ceil(avail_width / 6.f) * scaling;

    ImGui::PushItemWidth(input_width);
    ImGui::InputInt((std::string("##") + name).c_str(), &val);
    ImGui::PopItemWidth();
    ImGui::SameLine();

    auto slider_width = ImGui::GetContentRegionAvailWidth();

    ImGui::PushItemWidth(slider_width);
    ImGui::SliderInt((std::string("##") + name + "2").c_str(), &val, static_cast<int32_t>(min), static_cast<int32_t>(max), format);
    ImGui::PopItemWidth();

    val = std::max(val, static_cast<int32_t>(min));
    val = std::min(val, static_cast<int32_t>(max));

    cfgvalue->set(static_cast<uint32_t>(val));
}

/*
@brief

    Draws a slider + input float given the range and description
*/
void ImGuiUser::inputslider_float(retrogames::cfgvalue_t* cfgvalue, const std::string& name, float max, float min/* = 0.f*/, const std::string& desc/* = ""*/, float scaling/* = 1.f*/, float step/* = 1.f*/, float step_fast/* = 2.f*/, float power/* = 1.f*/, const char* format/* = "%.1f"*/, int decimal_precision/* = 1*/)
{
    ImGui::Text("%s:", name.c_str());

    if (!desc.empty())
    {
        ImGui::SameLine();

        help_marker(desc);
    }

    auto val = cfgvalue->get<float>();
    auto avail_width = ImGui::GetContentRegionAvailWidth();
    auto input_width = std::ceil(avail_width / 6.f) * scaling;

    ImGui::PushItemWidth(input_width);
    ImGui::InputFloat((std::string("##") + name).c_str(), &val, step, step_fast, decimal_precision);
    ImGui::PopItemWidth();
    ImGui::SameLine();

    auto slider_width = ImGui::GetContentRegionAvailWidth();

    ImGui::PushItemWidth(slider_width);
    ImGui::SliderFloat((std::string("##") + name + "2").c_str(), &val, min, max, format, power);
    ImGui::PopItemWidth();

    val = std::max(val, min);
    val = std::min(val, max);

    cfgvalue->set(val);
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
    auto size = ImVec2{ImGui::CalcTextSize(info.c_str()).x+ImGui::GetStyle().FramePadding.x*2.f + ImGui::GetStyle().ItemSpacing.x*2.f,ImGui::GetFrameHeight()+ImGui::GetStyle().ItemSpacing.y*2.f};
    auto pos_ = pos;

    pos_.x -= size.x * .5f;
    pos_.y -= size.y * .5f;

    ImGui::GetForegroundDrawList()->AddRectFilled(pos_, ImVec2{pos_.x+size.x,pos_.y+size.y}, ImGui::GetColorU32(background_color));
    ImGui::GetForegroundDrawList()->AddRect(pos_, ImVec2{pos_.x+size.x,pos_.y+size.y}, ImGui::GetColorU32(border_color), 0.f, 0, 2.f);
    ImGui::GetForegroundDrawList()->AddText(ImVec2{pos_.x+ImGui::GetStyle().ItemSpacing.x+ImGui::GetStyle().FramePadding.x,pos_.y+ImGui::GetStyle().ItemSpacing.y+ImGui::GetStyle().FramePadding.y}, ImGui::GetColorU32(text_color), info.c_str());

    /*ImGui::SetNextWindowPos(ImVec2{pos.x - size.x * .5f, pos.y - size.y * .5f}, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
    ImGui::SetNextWindowFocus();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, background_color);
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::PushStyleColor(ImGuiCol_Border, border_color);

    if (ImGui::Begin("##notification", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
    {
        ImGui::TextUnformatted(info.c_str());
        ImGui::End();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);*/
}

/*
@brief

    Highlights the first option on an appearing window
*/
void ImGuiUser::highlight_first_option_on_appearing(void)
{
    auto& g = *ImGui::GetCurrentContext();

    if (g.CurrentWindow->Appearing)
    {
        g.NavDisableHighlight = false;
        g.NavDisableMouseHover = true;
    }
}