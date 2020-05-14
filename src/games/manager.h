/*
@file

    manager.h

@purpose

    Manage games
*/

#pragma once

#include <unordered_map>
#include "base/base.h"

namespace retrogames
{

    class settings_t;

    class games_manager_t final
    {
        
    protected:



    private:

        // Our list of games (name, pointer)
        std::unordered_map<std::string, game_base_t*> games;

        // Pointer to our settings
        settings_t* settings;

        // Pointer to the current game
        game_base_t* current_game;

        // Pointer to pointers to default fonts
        ImFont** default_font_small, **default_font_big, **default_font_mid;

    public:

        /*
        @brief

            Constructor, saves the settings pointer
        */
        games_manager_t(settings_t* settings, ImFont** default_font_small, ImFont **default_font_mid, ImFont **default_font_big) :
            settings(settings),
            current_game(nullptr),
            default_font_small(default_font_small),
            default_font_mid(default_font_mid),
            default_font_big(default_font_big) {}

        /*
        @brief

            Adds a game
        */
        template<typename T> void add_game(const std::string& name, const std::string& version = "1.0", uint8_t* icon = nullptr)
        {
            auto& main_settings = settings->get_main_settings();

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)   
            // Create game-specific video settings
            settings->create(name + "_video_fps", main_settings.fps->get<uint32_t>());
            settings->create(name + "_video_fullscreen", main_settings.fullscreen->get<bool>());
            settings->create(name + "_video_vsync", main_settings.vsync->get<bool>());
            settings->create(name + "_video_resolution", main_settings.resolution->get<std::string>());
#endif
            settings->create(name + "_draw_frametime", main_settings.draw_frametime->get<bool>());
            settings->create(name + "_draw_fps", main_settings.draw_fps->get<bool>());
            settings->create(name + "_draw_playtime", main_settings.draw_playtime->get<bool>());
            settings->create(name + "_draw_position_alignment", main_settings.draw_position->get<std::string>());
            settings->create(name + "_lostfocus_timeout_time", main_settings.timeout_time->get<uint32_t>());

            games[name] = new T(settings, name, default_font_small, default_font_mid, default_font_big, version, icon);
        }

        /*
        @brief

            Selects a game
        */
        game_base_t* select_game(const std::string& name)
        {
            if (games.empty()) return nullptr;

            auto f = games.find(name);

            if (f == games.end()) return nullptr;

            return current_game = f->second;
        }

        /*
        @brief

            Gets the current game
        */
        game_base_t* get_current_game(void) { return current_game; }

        /*
        @brief

            Destructor, deletes all the games
        */
        ~games_manager_t()
        {
            for (auto& game : games) delete game.second;
        }

        /*
        @brief

            Exports the unordered_map for use in looping the games
        */
        const std::unordered_map<std::string, game_base_t*>& get_games(void) const { return games; }

    };

}