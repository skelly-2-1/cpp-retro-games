/*
@file

    base.cpp

@purpose

    Base of our games. All games must implement this class.
*/

#include "base.h"
#include "imgui/imgui_user.h"
#include "imgui/imgui_internal.h"

/*
@brief

    Returns the time elapsed since a chrono::high_resolution_clock::time_point.
    Hours, minutes, seconds, milliseconds
*/
retrogames::game_base_t::playtime_t retrogames::game_base_t::get_playtime_elapsed(const std::chrono::high_resolution_clock::time_point& point)
{
    uint16_t time_elapsed_ms, time_elapsed_seconds, time_elapsed_minutes, time_elapsed_hours;

    auto total_time_elapsed_ns = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - point).count());
    auto total_time_elapsed_ms = total_time_elapsed_ns / 1000000.0; // to microseconds to milliseconds
    auto total_time_elapsed_seconds = total_time_elapsed_ms / 1000.0; // to seconds
    auto total_time_elapsed_minutes = total_time_elapsed_seconds / 60.0; // to minutes
    auto total_time_elapsed_hours = total_time_elapsed_minutes / 60.0; // to hours

    time_elapsed_hours = static_cast<uint16_t>(std::floor(total_time_elapsed_hours));
    time_elapsed_minutes = static_cast<uint16_t>(std::floor(total_time_elapsed_minutes - static_cast<double>(static_cast<uint64_t>(time_elapsed_hours) * 60)));
    time_elapsed_seconds = static_cast<uint16_t>(std::floor(total_time_elapsed_seconds - static_cast<double>(static_cast<uint64_t>(time_elapsed_minutes) * 60) - static_cast<double>(static_cast<uint64_t>(time_elapsed_hours) * 3600)));
    time_elapsed_ms = static_cast<uint16_t>(std::floor(total_time_elapsed_ms - static_cast<double>(static_cast<uint64_t>(time_elapsed_seconds) * 1000) - static_cast<double>(static_cast<uint64_t>(time_elapsed_minutes) * 60000) - static_cast<double>(static_cast<uint64_t>(time_elapsed_hours) * 3600000)));

    return { time_elapsed_hours, time_elapsed_minutes, time_elapsed_seconds, time_elapsed_ms };
}

/*
@brief

    Draws the pause menu
*/
bool retrogames::game_base_t::draw_pause_menu(ImFont* font)
{
    if (!do_draw_pause_menu) return false;

    bool result = false;

    IMGUI_MODAL_POPUP(pausemenu, true)
    {
        ImGuiUser::highlight_first_option_on_appearing();

        ImGui::TextUnformatted("Pause menu");
        ImGui::Separator();

        if (!confirming_main_menu)
        {
            auto button_size = ImVec2{ImGui::CalcTextSize("Back to main menu").x+ImGui::GetStyle().FramePadding.x*2.f,0.f};

            if (ImGui::Button("Continue",button_size)) unpause();
            if (ImGui::Button("Back to main menu",button_size)) confirming_main_menu = true;
        }
        else
        {
            ImGui::TextUnformatted("Are you sure?");

            auto button_size = ImVec2{((ImGui::CalcTextSize("Are you sure?").x+ImGui::GetStyle().FramePadding.x*2.f)*.5f)-ImGui::GetStyle().ItemInnerSpacing.x*2.f,0.f};

            if (ImGui::Button("Yes", button_size)) result = true;

            ImGui::SameLine();

            if (ImGui::Button("No", button_size)) confirming_main_menu = false;
        }
    }

    return result;
}

/*
@brief

    Base reset function, handles pause and timeout logic, also ends up calling
    the virtual method @reset
*/
void retrogames::game_base_t::base_reset(settings_t* settings, bool create_fonts)
{
    base_static_vars.reset(timeout_time);

    paused = false;
    confirming_main_menu = false;
    base_resolution_area = settings->get_main_settings().resolution_area;
    timeout_time = timeout_cfgvalue.get<uint32_t>();

    reset(settings, create_fonts);
}

/*
@brief

    Base draw function, handles pause and timeout logic, also ends up calling
    the virtual method @draw
*/
bool retrogames::game_base_t::base_draw(bool render)
{
    // When the game is minimized (render = false), offset the start_time by the duration between renders
    // (minimizing and opening)
    if (base_static_vars.last_render != render)
    {
        base_static_vars.last_render = render;

        if (!render)
        {
            base_static_vars.render_pause_begin = std::chrono::high_resolution_clock::now();
        }
        else
        {
            // (Re)Start the timeout timer
            base_static_vars.timeout_timer.stop();
            base_static_vars.timeout_timer.start();

            // When we have to draw again (after minimizing the game), offset the start_time by the time between the renders
            if (base_static_vars.start_timer.started()) base_static_vars.start_timer.offset_by_time(base_static_vars.render_pause_begin);
        }
    }

    // Pause and unpause the start timer if we minimize and open the game again
    if (base_static_vars.last_timeout_timer_started_needs_reset)
    {
        base_static_vars.last_timeout_timer_started_needs_reset = false;
        base_static_vars.last_timeout_timer_started = base_static_vars.timeout_timer.started();
    }

    if (base_static_vars.last_timeout_timer_started != base_static_vars.timeout_timer.started())
    {
        base_static_vars.last_timeout_timer_started = base_static_vars.timeout_timer.started();

        if (base_static_vars.start_timer.started())
        {
            if (!base_static_vars.timeout_timer.started() && base_static_vars.start_timer.paused() && !paused) base_static_vars.start_timer.unpause();
            else if (base_static_vars.timeout_timer.started() && !base_static_vars.start_timer.paused()) base_static_vars.start_timer.pause();
        }
    }

    if (render)
    {
        // Start the "start timer" if need be
        if (base_static_vars.start_timeout_timer)
        {
            base_static_vars.timeout_timer.start();
            base_static_vars.start_timeout_timer = false;
        }

        // Pause and unpause the start timer if we pause the game
        if (base_static_vars.last_pause_needs_reset)
        {
            base_static_vars.last_pause_needs_reset = false;
            base_static_vars.last_pause = paused;
        }

        if (base_static_vars.last_pause != paused)
        {
            base_static_vars.last_pause = paused;

            if (paused && !base_static_vars.start_timer.paused()) base_static_vars.start_timer.pause();
            else if (!paused/* && !base_static_vars.timeout_timer.started()*/) { base_static_vars.timeout_timer.stop(); base_static_vars.timeout_timer.start(); }//base_static_vars.start_timer.unpause();
        }

        // Stop the timeout timer if it exceeded the time
        if (base_static_vars.timeout_timer.started() && static_cast<uint8_t>(base_static_vars.timeout_timer.get_elapsed<std::chrono::seconds>().count()) >= base_static_vars.timeout_time) base_static_vars.timeout_timer.stop();

        // Set the start time if it didn't happen already
        if (!base_static_vars.start_timer.started() && !base_static_vars.timeout_timer.started()) base_static_vars.start_timer.start();

        // Set the playtime
        if (!paused && !base_static_vars.timeout_timer.started() && base_static_vars.start_timer.started() && !base_static_vars.start_timer.paused()) base_static_vars.playtime = get_playtime_elapsed(base_static_vars.start_timer.get_time_point());
        if (!base_static_vars.start_timer.started()) base_static_vars.playtime = {};
    }

    // Reset the flag indicating if we want to draw the pause menu
    do_draw_pause_menu = true;

    // Now, do any drawing that we need to do
    auto ret = draw(render);

    // Draw the timeout time if we're still in timeout
    if (base_static_vars.timeout_timer.started() && !paused && do_draw_pause_menu)
    {
        auto elapsed = static_cast<uint64_t>(base_static_vars.timeout_timer.get_elapsed().count());
        auto delay = static_cast<uint64_t>(static_cast<uint16_t>(static_cast<uint64_t>(timeout_time) * 1000));
        auto time_left = static_cast<uint8_t>(std::ceil((static_cast<double>(delay) - static_cast<double>(elapsed)) / 1000.));
        auto pos = ImVec2{static_cast<float>(base_resolution_area.width / 2),static_cast<float>(base_resolution_area.height/10)};
        auto timeout_text = std::string("Timeout: ") + std::to_string(time_left) + "s";
        
        ImGui::PushFont(get_default_font_small());
        ImGuiUser::draw_info(pos, timeout_text);
        ImGui::PopFont();
    }

    // Draw the FPS/frametime/playtime if wanted
    auto gamename = game_info.name;
    auto draw_fps = settings->get(gamename + "_draw_fps").get<bool>();
    auto draw_frametime = settings->get(gamename + "_draw_frametime").get<bool>();
    auto draw_playtime = settings->get(gamename + "_draw_playtime").get<bool>();

    if (draw_fps || draw_frametime || draw_playtime)
    {
        ImGui::PushFont(get_default_font_small());

        auto alignment = settings->get(gamename + "_draw_position_alignment").get<std::string>();

        std::string info;

        static char buffer[80];

        if (draw_fps) info += std::to_string(static_cast<uint32_t>(std::round(ImGui::GetIO().Framerate))) + "fps";

        if (draw_frametime)
        {
            auto frametime = ImGui::GetIO().DeltaTime * 1000.f;

            sprintf(buffer, "%.2fms", frametime);

            if (!info.empty()) info += " - ";

            info += buffer;
        }

        if (draw_playtime)
        {
            auto playtime = get_playtime();

            sprintf(buffer, "%02i:%02i:%02i:%03i", playtime.hours, playtime.minutes, playtime.seconds, playtime.milliseconds);

            if (!info.empty()) info += " - ";

            info += buffer;
        }

        ImVec2 draw_pos{};

        if (alignment.length() >= 7)
        {
            if (alignment.substr(alignment.length() - 5).compare("right") == 0) draw_pos.x = std::floor(static_cast<float>(base_resolution_area.width) - ImGui::CalcTextSize(info.c_str()).x);
            else if (alignment.substr(alignment.length() - 6).compare("center") == 0) draw_pos.x = std::floor(static_cast<float>(base_resolution_area.width) * .5f - ImGui::CalcTextSize(info.c_str()).x * .5f);

            if (alignment.substr(0, 6).compare("bottom") == 0) draw_pos.y = std::floor(static_cast<float>(base_resolution_area.height) - ImGui::GetFontSize());
        }

        ImGui::GetForegroundDrawList()->AddText(draw_pos, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled)), info.c_str());
        ImGui::PopFont();
    }

    return ret;
}