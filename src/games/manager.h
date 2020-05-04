/*
@file

    manager.h

@purpose

    Manage games
*/

#pragma once

#include <unordered_map>
#include "base.h"

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

    public:

        /*
        @brief

            Constructor, saves the settings pointer
        */
        games_manager_t(settings_t* settings) : settings(settings), current_game(nullptr) {}

        /*
        @brief

            Adds a game
        */
        template<typename T> void add_game(const std::string& name, const std::string& version = "1.0", uint8_t* icon = nullptr)
        {
#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)   
            // Create game-specific video settings
            settings->create(name + "_video_fps", settings->get_main_settings().fps->get<uint32_t>());
            settings->create(name + "_video_fullscreen", settings->get_main_settings().fullscreen->get<bool>());
            settings->create(name + "_video_vsync", settings->get_main_settings().vsync->get<bool>());
            settings->create(name + "_video_resolution", settings->get_main_settings().resolution->get<std::string>());
#endif

            games[name] = new T(settings, name, version, icon);
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