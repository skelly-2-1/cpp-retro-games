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

// Needed for our settings
std::unordered_map<std::string, retrogames::setting_t<uint8_t>*>* retrogames::detail::settings_list = new std::unordered_map<std::string, setting_t<uint8_t>*>;

namespace retrogames
{

    // Variables
	std::unique_ptr<imgui_wrapper_opengl_t> imgui = nullptr;
	std::unique_ptr<settings_t> settings = nullptr;

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
	// Destructor takes care of deleting the detail::settings_list.
	auto settings = std::make_unique<retrogames::settings_t>("cpp-retro-games\\settings.json");

	settings->load();

	// Create the config directory if it doesn't exist
	mkdir("cpp-retro-games", 0777);

    // Initialize the imgui object
	imgui = std::make_unique<imgui_wrapper_opengl_t>(settings.get());

    // Attempt to initialize OpenGL (or the software renderer)
	std::string error;

	if (!imgui->initialize(true, &error)) return;

	// Attempt to initialize ImGui
	if (!imgui->initialize(false, &error)) return;

    // Demo
	bool show_demo_window = true;
    bool show_another_window = false;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Grab the ImGui IO
    auto& io = ImGui::GetIO();

    // Begin our main application loop
	while (appletMainLoop())
	{
		// Scan for inputs
		hidScanInput();

        // Check what keys we are holding down
		auto keys = hidKeysHeld(CONTROLLER_P1_AUTO);

        static auto old_keys = keys;

        if (keys & KEY_PLUS) break;

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

                }
                else if (!(keys & key) && (old_keys & key))
                {
                    // released: mapped_key

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
		auto should_render = render/* && window->is_in_foreground()*/;
		auto should_exit = /*snake->draw(should_render)*/false;

		// Demo
		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window) ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

		// End the frame
		imgui->end_frame(/*should_render*/true, color_t(40, 40, 40));

		// Exit if we should (if game::draw returns true)
		//if (should_exit) break;
    }

    imgui->shutdown();
}

#endif
