#ifdef PLATFORM_LINUX

/*
@file

    main_linux.cpp

@purpose

    Main entry point for our program (Linux)
*/

#include <GLFW/glfw3.h>
#include "misc/window.h"
#include "fpsmanager/fpsmanager.h"
#include "imgui_wrappers/glfw/glfw.h"
#include "imgui/imgui.h"
#include "main.h"
#include "snd/snd.h"

namespace retrogames
{

	// see snd/snd.h
	snd_t* snd = nullptr;

    // Variables
	std::unique_ptr<imgui_wrapper_glfw_t> imgui = nullptr;

	bool imgui_initialized = false;

    /*
    @brief

        Main entry point of our program (within the retrogames namespace)
    */
   	void main(void);

	/*
	@brief

		Will tell us if our GLFW window is focused or not
	*/
	bool window_focused = true;

	GLFWwindow* glfw_window = nullptr;

   	void window_focus_callback(GLFWwindow* window, int focused)
	{
		// better be safe than sorry
		if (window != glfw_window) return;
		
		window_focused = focused != 0;
	}

	/*
	@brief

		Turns a glfw key to an ImGuiKey
	*/
	static auto to_imgui_key = [](int key) -> ImGuiKey
	{
		static bool reverse_keymap_init = true;
		static std::unordered_map<int, ImGuiKey> reverse_keymap;

		if (reverse_keymap_init)
		{
			reverse_keymap_init = false;

			auto& io = ImGui::GetIO();

			for (uint8_t i = 0; i < static_cast<uint8_t>(ImGuiKey_COUNT); i++)
			{
				auto key = static_cast<ImGuiKey>(i);
				auto raw_key = io.KeyMap[i];

				reverse_keymap[raw_key] = key;
			}
		}

		if (reverse_keymap.empty()) return ImGuiKey_COUNT;

		auto f = reverse_keymap.find(static_cast<int>(key));

		return f != reverse_keymap.end() ? f->second : ImGuiKey_COUNT;
	};

	/*
	@brief

		Handles GLFW key functions
	*/
	GLFWkeyfun previous_key_callback = nullptr;

	void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (previous_key_callback != nullptr) previous_key_callback(window, key, scancode, action, mods);
		if (action != GLFW_PRESS && action != GLFW_RELEASE) return;

		auto found_key = to_imgui_key(key);

		if (found_key != static_cast<ImGuiKey>(ImGuiKey_COUNT)) main_handle_key(action == GLFW_PRESS, found_key);
	}

}

/*
@brief

    Main program entry point (Linux)
*/
int main(void)
{
    retrogames::main();

    return 0;
}

/*
@brief

    Main entry point of our program (within the retrogames namespace)
*/
void retrogames::main(void)
{
	// Create and load our settings
	settings_t settings("settings.json");

	auto& main_settings = settings.get_main_settings();

	// load the sound library
	snd_t _snd;

	if (!_snd.initialize())
	{
		fprintf(stderr, "Failed to initialize sound library\n");

		return;
	}

	snd = &_snd;

	// Grab the main settings
	auto vsync = main_settings.vsync->get<bool>();
	auto fullscreen = main_settings.fullscreen->get<bool>();
	auto fps = main_settings.fps->get<uint32_t>();

	// Initialize the imgui object
	imgui = std::make_unique<imgui_wrapper_glfw_t>(&settings, "cpp-retro-games", main_settings.resolution_area, vsync, fullscreen);
	
	// Attempt to initialize GLFW
	std::string error;

	if (!imgui->initialize(true, &error))
	{
        fprintf(stderr, (std::string("Failed to initialize GLFW. Terminating process.\n\nError: ") + error).c_str());

		return;
	}

	// Attempt to initialize ImGui
	if (!imgui->initialize(false, &error))
	{
        fprintf(stderr, (std::string("Failed to initialize ImGui. Terminating process.\n\nError: ") + error).c_str());

		return;
	}

	// Listen for focus events so we know if our window is focused or not
	glfw_window = imgui->get_glfw_window();

	glfwSetWindowFocusCallback(glfw_window, window_focus_callback);

	window_focused = glfwGetWindowAttrib(glfw_window, GLFW_FOCUSED);

	// Also set the key callback so we can listen for keys
	previous_key_callback = glfwSetKeyCallback(glfw_window, glfw_key_callback);

    // Create the fpsmanager object (if vsync is disabled)
	std::unique_ptr<fpsmanager_t> fpsmanager;

	static bool vsync_enabled = vsync;

	if (!vsync_enabled) fpsmanager = std::make_unique<fpsmanager_t>(static_cast<uint16_t>(fps));

	// initialize main functions
	main_initialize(&settings);

	// should we reset video settings?
	bool reset_video_settings = false;

    // Main loop
    while (!imgui->should_close())
    {
        // Poll window events
		imgui->poll_events();

		// Begin the frame
		auto render = imgui->begin_frame();

		// Draw
		auto should_render = render && window_focused;
		auto should_exit = main_frame(should_render, reset_video_settings);

		// End the frame
		imgui->end_frame(should_render, color_t(40, 40, 40));

		// Exit if we should
		if (should_exit) break;

		// Limit our FPS
		if (!vsync_enabled) fpsmanager->run();

		// Reset video mode if we should
		if (reset_video_settings)
		{
			reset_video_settings = false;

			// Attempt to re-initialize GLFW
			if (!imgui->reinitialize(&error))
			{
				fprintf(stderr, (std::string("Failed to initialize GLFW/ImGui. Exiting process.\n\nError: ") + error).c_str());

				break;
			}

			// Listen for focus events so we know if our window is focused or not
			glfw_window = imgui->get_glfw_window();

			glfwSetWindowFocusCallback(glfw_window, window_focus_callback);

			window_focused = glfwGetWindowAttrib(glfw_window, GLFW_FOCUSED);

			// Also set the key callback so we can listen for keys
			previous_key_callback = glfwSetKeyCallback(glfw_window, glfw_key_callback);

			// Tell anything else that the video mode changed
			main_reset();

			// Reset the FPS manager
			auto old_vsync_enabled = vsync_enabled;
			
			vsync_enabled = settings.get_main_settings().vsync->get<bool>();

			if (vsync_enabled && !old_vsync_enabled)
			{
				fpsmanager.reset();
			}
			else if (!vsync_enabled && old_vsync_enabled)
			{
				fpsmanager.reset(new fpsmanager_t(static_cast<uint16_t>(settings.get_main_settings().fps->get<uint32_t>())));
			}
			else if (!vsync_enabled)
			{
				fpsmanager.reset(new fpsmanager_t(static_cast<uint16_t>(settings.get_main_settings().fps->get<uint32_t>())));
			}
		}
	}

	// Thread finished, shut down ImGui and GLFW
	imgui->shutdown();

	// Save our settings
	settings.save();
}
#endif