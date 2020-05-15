/*
@file

    base.h

@purpose

    Base of our games. All games must implement this class.
*/

#pragma once

#include <string>
#include <chrono>
#include <stdint.h>
#include "imgui/imgui.h"
#include "misc/settings.h"
#include "misc/timer.h"
#include "imgui/imgui.h"

namespace retrogames
{

    struct game_information_t final
    {

        std::string name;
        std::string version;

        uint8_t* icon;

        game_information_t(const game_information_t& other) :
            name(other.name),
            version(other.version),
            icon(other.icon)
            {}

        game_information_t(const std::string& name, const std::string& version = "1.0", uint8_t* icon = nullptr) : 
            name(name),
            version(version),
            icon(icon)
            {}

        static game_information_t create(const std::string& name, const std::string& version = "1.0", uint8_t* icon = nullptr)
        {
            return game_information_t(name, version, icon);
        }

    };

    class game_base_t
    {

    protected:



    public:

        struct playtime_t final
        {

            uint16_t hours, minutes, seconds, milliseconds;

        };

    private:

        // Static vars
        struct base_static_vars_t final
        {

            playtime_t playtime;

            std::chrono::high_resolution_clock::time_point render_pause_begin;

            uint8_t timeout_time;

            timer_t timeout_timer, start_timer;

            bool last_pause, last_pause_needs_reset;
            bool last_render;
            bool last_timeout_timer_started, last_timeout_timer_started_needs_reset;
            bool start_timeout_timer;

            void reset(uint8_t timeout_time)
            {
                last_pause = false;
                last_pause_needs_reset = true;
                last_render = true;
                last_timeout_timer_started = false;
                last_timeout_timer_started_needs_reset = true;
                start_timeout_timer = true;

                this->timeout_time = timeout_time;

                timeout_timer.stop();
                timeout_timer.start();

                start_timer.stop();

                playtime = {};
            }

            base_static_vars_t(uint8_t timeout_time) { reset(timeout_time); }

        };

        base_static_vars_t base_static_vars;

        bool paused;

        game_information_t game_info;

        settings_t* settings;

        uint8_t timeout_time;

        ImFont** default_font_small, **default_font_big, **default_font_mid;

        cfgvalue_t& timeout_cfgvalue;

    public:

        enum class PAUSE_STATE : uint8_t
        {

            PAUSE_STATE_NONE,
            PAUSE_STATE_PAUSED,
            PAUSE_STATE_TIMEOUT,
            PAUSE_STATE_NOFOCUS

        };

    private:

        bool confirming_main_menu;

        PAUSE_STATE pause_state;

        area_size_t base_resolution_area;

    public:

        /*
        @brief

            Sets the timeout time
        */
        void set_lostfocus_timeout_time(uint32_t timeout_time) { this->timeout_time = static_cast<uint8_t>(timeout_time); }

        /*
        @brief

            Gets the small default font
        */
        ImFont* get_default_font_small(void) { return *default_font_small; }

        /*
        @brief

            Gets the mid default font
        */
        ImFont* get_default_font_mid(void) { return *default_font_mid; }

        /*
        @brief

            Gets the big default font
        */
        ImFont* get_default_font_big(void) { return *default_font_big; }

        /*
        @brief

            Draws the pause menu
        */
        bool draw_pause_menu(ImFont* font);

        /*
        @brief

            Pauses the game
        */
        void pause(void) { paused = true; }

        /*
        @brief

            Unpauses the game
        */
        void unpause(void)
        {
            paused = false;
        }

        /*
        @brief

            Toggles pause
        */
        void toggle_pause(void) { paused = !paused; }

        /*
        @brief

            Returns if we're paused
        */
        bool is_paused(void) const { return paused; }

        /*
        @brief

            Returns if we're good to go (not paused/in timeout/lost focus)
        */
        bool can_continue(void) const { return !paused && !is_in_timeout(); }

        /*
        @brief

            Returns if we're in timeout
        */
        bool is_in_timeout(void) const { return base_static_vars.timeout_timer.started(); }

        /*
        @brief

            Starts timeout
        */
        void start_timeout(void) { base_static_vars.timeout_timer.stop(); base_static_vars.timeout_timer.start(); }

        /*
        @brief

            Resets the playtime
        */
        void reset_playtime(void) { base_static_vars.playtime = {}; base_static_vars.start_timer.stop(); }

        /*
        @brief

            Returns the time elapsed since a chrono::high_resolution_clock::time_point.
            Hours, minutes, seconds, milliseconds
        */
        playtime_t get_playtime_elapsed(const std::chrono::high_resolution_clock::time_point& point);

        /*
        @brief

            Retrieves the current total playtime
        */
        playtime_t get_playtime(void) const { return base_static_vars.playtime; }

    public:

        /*
        @brief

            Constructor
        */
        game_base_t(const game_information_t& game_info, settings_t* settings, ImFont** default_font_small, ImFont** default_font_mid, ImFont** default_font_big, uint8_t timeout_time = 3) :
            game_info(game_info),
            settings(settings),
            pause_state(PAUSE_STATE::PAUSE_STATE_NONE),
            base_resolution_area(settings->get_main_settings().resolution_area),
            confirming_main_menu(false),
            timeout_time(timeout_time),
            base_static_vars(timeout_time),
            paused(false),
            default_font_small(default_font_small),
            default_font_mid(default_font_mid),
            default_font_big(default_font_big),
            timeout_cfgvalue(settings->get(game_info.name + "_lostfocus_timeout_time"))
            {}

        /*
        @brief

            Base draw function, handles pause and timeout logic, also ends up calling
            the virtual method @draw
        */
        bool base_draw(bool render);

        /*
        @brief

            Base reset function, handles pause and timeout logic, also ends up calling
            the virtual method @reset
        */
        void base_reset(settings_t* settings, bool create_fonts);

        /*
        @brief

            Resets information (when we re-start the game)
        */
        virtual void reset(settings_t* settings, bool create_fonts) = 0;

        /*
        @brief

            Callback for drawing.

        @notes

            Drawing should only be done if @render is true, otherwise
            only handle non-drawing functions.
        */
        virtual bool draw(bool render) = 0;

        /*
        @brief

            Handles keypress/keyrelease events
        */
        virtual void handle_key(ImGuiKey key, bool pressed) = 0;

        /*
        @brief

            Draws the options menu
        */
        virtual void draw_options(float scaling) = 0;

        /*
        @brief

            Draws controls
        */
        virtual void draw_controls(float scaling) = 0;

        /*
        @brief

            Draws some information
        */
        virtual void draw_information(float scaling) = 0;

        /*
        @brief

            Returns the game information to the caller
        */
        const game_information_t& get_information(void) const { return game_info; }

        /*
        @brief

            Returns the settings for the game to use
        */
        settings_t* get_settings(void) { return settings; }

    };

}