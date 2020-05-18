/*
@file

    imgui_user.h

@purpose

    Own functions for easily implementing ImGui functionality
*/

#pragma once

#include "imgui.h"
#include <string>
#include <cstdint>
#include "misc/cfgvalue.h"
#include "misc/color.h"

#define IMGUIUSER_CONCAT(a, b) a ## b
#define IMGUIUSER_WITH(decl)\
    for (bool __f = true; __f; ) \
    for (auto decl; __f; __f = false)
#define IMGUI_MODAL_POPUP(name, ...)\
    IMGUIUSER_WITH(IMGUIUSER_CONCAT(modal_, name) = ImGuiUser::modal_popup_t(#name, __VA_ARGS__))\
    if (IMGUIUSER_CONCAT(modal_, name).success())

namespace ImGuiUser
{

    // Tells us about the current modal popup ID (in case we want to stack them)
    extern uint8_t current_modal_popup_id;

    /*
    @brief

        Wrapper for ImGui's modal popups (just makes the code smaller)
    */
    struct modal_popup_t final
    {

        // Did we successfully open the dummy window?
        bool started_window;

        // Did we successfully open the popup?
        bool started_popup;

        // Do we want to darken the background?
        bool darkening;

        /*
        @brief

            Constructor
        */
        modal_popup_t(const char* name, bool darkening = false);

        /*
        @brief

            Destructor
        */
        ~modal_popup_t();

        /*
        @brief

            Tells the caller if we started the popup successfully or not
        */
        bool success(void) const { return started_popup; }

        /*
        @brief

            Closes the modal popup
        */
        void close(void) const { ImGui::CloseCurrentPopup(); }

    };

    /*
    @brief

        Helper for tooltips, taken from the ImGui demo .cpp
    */
    void help_marker(const std::string& desc);

    /*
    @brief

        Draws a slider + input int given the range and description
    */
    void inputslider_uint32_t(retrogames::cfgvalue_t* cfgvalue, const std::string& name, uint32_t max, uint32_t min = 0u, const std::string& desc = "", float scaling = 1.f);

    /*
    @brief

        Draws a slider + input float given the range and description
    */
   void inputslider_float(retrogames::cfgvalue_t* cfgvalue, const std::string& name, float max, float min = 0.f, const std::string& desc = "", float scaling = 1.f, float step = 1.f, float step_fast = 2.f, float power = 1.f, const char* format = "%.1f", int decimal_precision = 1);

    /*
    @brief

        Draws a toggle button
    */
    void toggle_button(retrogames::cfgvalue_t* cfgvalue, const std::string& name, const std::string& desc = "");

    /*
    @brief

        Adds a frame-height spacing
    */
    void frame_height_spacing(uint8_t num = 1);

    /*
    @brief

        Converts a color_t to an ImGui color (ImVec4)
    */
    ImVec4 color_to_imgui_color_vec4(const retrogames::color_t& color);
    
     /*
    @brief

        Converts a color_t to an ImGui color (u32)
    */
    ImU32 color_to_imgui_color_u32(const retrogames::color_t& color);

    /*
    @brief

        Draws information on the screen
    */
    void draw_info(const ImVec2& pos, std::string info);

}