/*
@file

    main.h

@purpose

    Exporting of all the main.cpp's functions
*/

#pragma once

typedef int ImGuiKey;

namespace retrogames
{

    class settings_t;

    /*
    @brief

        Called before any rendering is done
    */
    void main_initialize(settings_t* settings);

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
    bool main_frame(bool should_render, bool& reset_video_mode);

    /*
    @brief

        Handles key events
    */
    void main_handle_key(bool down, ImGuiKey key);

    /*
    @brief

        Called when changing video mode
    */
    void main_reset(void);

}