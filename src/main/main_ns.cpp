#ifdef PLATFORM_NS

/*
@file

    main_ns.cpp

@purpose

    Main entry point for our program (Nintendo Switch)
*/

#include <switch.h>
#include <sys/stat.h>
#include <string>
#include <unordered_map>
#include "fpsmanager/fpsmanager.h"
#include "imgui_wrappers/ns/ns.h"
#include "imgui/imgui.h"
#include "misc/settings.h"
#include "misc/macros.h"
#include "misc/trace.h"
#include "main/main.h"

// NXLINK support
#ifdef NS_ENABLE_NXLINK
static int s_nxlinkSock = -1;

static void initNxLink()
{
    if (R_FAILED(socketInitializeDefault()))
        return;

    s_nxlinkSock = nxlinkStdio();
    if (s_nxlinkSock >= 0)
        TRACE("printf output now goes to nxlink server");
    else
        socketExit();
}

static void deinitNxLink()
{
    if (s_nxlinkSock >= 0)
    {
        close(s_nxlinkSock);
        socketExit();
        s_nxlinkSock = -1;
    }
}

extern "C" void userAppInit()
{
    initNxLink();
}

extern "C" void userAppExit()
{
    deinitNxLink();
}
#endif

namespace retrogames
{

    // Variables
	std::unique_ptr<imgui_wrapper_opengl_t> imgui = nullptr;

	bool imgui_initialized = false;

    /*
    @brief

        Main entry point of our program (within the retrogames namespace)
    */
    void main(void);

    /*
    @brief

        Prints an error to the console, forces user to press + to exit
    */
    void show_error(const std::string& error);

}

/*
@brief

    Main entry point of our program
*/
int main(int argc, char* argv[])
{
    retrogames::main();

    return 0;
}

/*
@brief

    Prints an error to the console, forces user to press + to exit
*/
void retrogames::show_error(const std::string& error)
{
#ifndef NS_ENABLE_NXLINK
    consoleInit(NULL);

    printf((error + "\nPress PLUS to exit").c_str());

    while (appletMainLoop())
    {
        hidScanInput();

        if (hidKeysDown(CONTROLLER_P1_AUTO) & KEY_PLUS) break;

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
#else
    TRACE("%s", error.c_str());
#endif
}

void retrogames::main(void)
{
	// Create and load our settings
	settings_t settings("cpp-retro-games\\settings.json");

	auto& main_settings = settings.get_main_settings();

	// Create the config directory if it doesn't exist
	mkdir("cpp-retro-games", 0777);

    // Initialize the imgui object
	imgui = std::make_unique<imgui_wrapper_opengl_t>(&settings);

    // Attempt to initialize OpenGL (or the software renderer)
	std::string error;

	if (!imgui->initialize(true, &error)) return;

	// Attempt to initialize ImGui
	if (!imgui->initialize(false, &error)) return;

    // Grab the ImGui IO
    auto& io = ImGui::GetIO();

    // initialize main functions
	main_initialize(&settings);

    // should we reset video settings? (dummy in this case)
	bool reset_video_settings = false;

    // Begin our main application loop
	while (appletMainLoop())
	{
		// Scan for inputs
		hidScanInput();

        // Check what keys we are holding down
		auto keys = hidKeysHeld(CONTROLLER_P1_AUTO);

        static auto old_keys = keys;

        //if (keys & KEY_PLUS) break;

		// Map navigation inputs for ImGui (joystick input)
		memset(io.NavInputs, 0, sizeof(io.NavInputs));

		static auto map_button = [&io, &keys](ImGuiNavInput_ nav_input, HidControllerKeys key)
		{
			if (keys & key) io.NavInputs[nav_input] = 1.f;
		};

		map_button(ImGuiNavInput_Activate,    KEY_A);
		map_button(ImGuiNavInput_Cancel,      KEY_B);
		map_button(ImGuiNavInput_Menu,        KEY_Y);
		map_button(ImGuiNavInput_Input,       KEY_X);
		map_button(ImGuiNavInput_DpadLeft,    KEY_DLEFT);
		map_button(ImGuiNavInput_DpadRight,   KEY_DRIGHT);
		map_button(ImGuiNavInput_DpadUp,      KEY_DUP);
		map_button(ImGuiNavInput_DpadDown,    KEY_DDOWN);
		map_button(ImGuiNavInput_FocusPrev,   KEY_L);
		map_button(ImGuiNavInput_FocusNext,   KEY_R);
		map_button(ImGuiNavInput_TweakSlow,   KEY_L);
		map_button(ImGuiNavInput_TweakFast,   KEY_R);
		map_button(ImGuiNavInput_LStickLeft,  KEY_LSTICK_LEFT);
		map_button(ImGuiNavInput_LStickRight, KEY_LSTICK_RIGHT);
		map_button(ImGuiNavInput_LStickUp,    KEY_LSTICK_UP);
		map_button(ImGuiNavInput_LStickDown,  KEY_LSTICK_DOWN);

        // Also handle keys we need for our games
        if (old_keys != keys)
        {
            static auto handle_key = [&keys](HidControllerKeys key, ImGuiKey mapped_key)
            {
                if ((keys & key) && !(old_keys & key))
                {
                    // pressed: mapped_key
                    main_handle_key(true, mapped_key);
                }
                else if (!(keys & key) && (old_keys & key))
                {
                    // released: mapped_key
                    main_handle_key(false, mapped_key);
                }
            };

            // Arrow keys
            handle_key(KEY_DOWN, ImGuiKey_DownArrow);
            handle_key(KEY_UP, ImGuiKey_UpArrow);
            handle_key(KEY_LEFT, ImGuiKey_LeftArrow);
            handle_key(KEY_RIGHT, ImGuiKey_RightArrow);

            // Plus mapped to Escape (pause)
            handle_key(KEY_PLUS, ImGuiKey_Escape);

            old_keys = keys;
        }

		// Handle touch input
        static uint32_t prev_touchcount = 0;

		io.MouseDown[0] = false;

		auto touch_count = hidTouchCount();

		if (touch_count != prev_touchcount || keys & KEY_TOUCH)
		{
			prev_touchcount = touch_count;

            static touchPosition touch;

			hidTouchRead(&touch, touch_count - 1);

			if (touch.id == touch_count - 1)
			{
				io.MousePos = ImVec2(touch.px, touch.py);
				io.MouseDown[0] = true;
			}
		}

        // Begin the frame
		auto render = imgui->begin_frame();

		// Draw
		auto should_exit = main_frame(true, reset_video_settings);

		// End the frame
		imgui->end_frame(true, color_t(40, 40, 40));

		// Exit if we should (if game::draw returns true)
		if (should_exit) break;
    }

    imgui->shutdown();

    settings.save();
}

#endif
