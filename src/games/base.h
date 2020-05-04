/*
@file

    base.h

@purpose

    Base of our games. All games must implement this class.
*/

#pragma once

#include <string>
#include <stdint.h>
#include "imgui/imgui.h"
#include "misc/settings.h"

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



    private:

        game_information_t game_info;

        settings_t* settings;

    public:

        /*
        @brief

            Constructor, we need the game information
        */
        game_base_t(const game_information_t& game_info, settings_t* settings) :
            game_info(game_info),
            settings(settings)
            {}

        /*
        @brief

            Resets information (when we re-start the game)
        */
        virtual void reset(settings_t* settings) = 0;

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
        virtual void draw_options(void) = 0;

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