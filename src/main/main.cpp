/*
@file

    main.cpp

@purpose

    Providing main functions without all the
    platform dependant code found in the other
    main_ files.
*/

#include "main.h"
#include "imgui/imgui.h"
#include "misc/color.h"
#include "mainmenu/mainmenu.h"

namespace retrogames
{

    static mainmenu_t mainmenu;

}

/*
@brief

    Called before any rendering is done
*/
void retrogames::main_initialize(settings_t* settings)
{
    mainmenu.initialize(settings);
}

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
bool retrogames::main_frame(bool should_render, bool& reset_video_mode)
{
    return mainmenu.run(should_render, reset_video_mode);
}

/*
@brief

    Handles key events
*/
void retrogames::main_handle_key(bool down, ImGuiKey key)
{
    mainmenu.handle_key(down, key);
}

/*
@brief

    Called when changing video mode
*/
void retrogames::main_reset(void)
{
    mainmenu.reset();
}