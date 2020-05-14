/*
@file

    mainmenu.h

@purpose

    Handling of our main menu
*/

typedef int ImGuiKey;

#include <deque>
#include <memory>
#include "games/manager.h"
#include "imgui/imgui.h"
#include "misc/area_size.h"

namespace retrogames
{

    class settings_t;

    class mainmenu_t final
    {

    protected:



    private:

        std::unique_ptr<games_manager_t> games_manager;

        game_base_t* selected_game;

        ImFont *default_font_small, *default_font_big, *default_font_mid;

        settings_t* settings;

        bool game_running;
        bool reset_game;

        float global_scaling;

    public:

        /*
        @brief

            Called on reset
        */
        void reset(void);

        /*
        @brief

            Creates needed fonts
        */
        void create_fonts(void);

        /*
        @brief

            Constructor
        */
        mainmenu_t() : selected_game(nullptr), games_manager(nullptr), game_running(false), global_scaling(1.f), reset_game(false) {}

        /*
        @brief

            Called before any rendering is done
        */
        void initialize(settings_t* settings);

        /*
        @brief

            Runs every frame, we do all required rendering here

        @params

            @should_render:     True if we should render, false if not.
                                This would be the case if the window
                                isn't in the foreground, for example.
                                You can still handle anything other
                                than drawing in the meanwhile though.
            @reset_video_mode:  Returns information to the caller if
                                the video mode should be reset
                                (when we changed settings)

        @return

            True if the application should exit, false if not
        */
        bool run(bool should_render, bool& reset_video_mode);

        /*
        @brief

            Handles key events
        */
        void handle_key(bool down, ImGuiKey key);

    };

}