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

namespace ImGuiUser
{

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

        Draws a toggle button
    */
    void toggle_button(retrogames::cfgvalue_t* cfgvalue, const std::string& name, const std::string& desc = "");

}