#ifdef PLATFORM_WINDOWS

/*
@file

    main_windows.cpp

@purpose

    Main entry point for our program (Windows)
*/

#include <Windows.h>
#include <unordered_map>
#include "misc/settings.h"
#include "misc/window.h"
#include "fpsmanager/fpsmanager.h"
#include "imgui_wrappers/dx9/dx9.h"
#include "imgui/imgui.h"
#include "main.h"
#include "util/util.h"
#include "snd/snd.h"

namespace retrogames
{

	// see snd/snd.h
	snd_t* snd = nullptr;

    // Needs to be global because of @window_procedure below otherwise not having access to it
	std::unique_ptr<imgui_wrapper_dx_t> imgui = nullptr;

	bool imgui_initialized = false;
	bool resetting_video_mode = false;

	/*
	@brief

		Our window procedure that captures all the Windows messages
	*/
	LRESULT CALLBACK window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    /*
    @brief

        Main entry point of our program (within the retrogames namespace)
    */
   	void main(void);

}

/*
@brief

	Turns a key wparam to an ImGuiKey
*/
static auto to_imgui_key = [](WPARAM wparam) -> ImGuiKey
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

	auto f = reverse_keymap.find(static_cast<int>(wparam));

	return f != reverse_keymap.end() ? f->second : ImGuiKey_COUNT;
};

/*
@brief

	Our window procedure that captures all the Windows messages
*/
LRESULT CALLBACK retrogames::window_procedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// Post a quit message if we receive WM_DESTROY
	if (msg == WM_DESTROY)
	{
		if (!resetting_video_mode)
		{
			PostQuitMessage(0);
		}
		else
		{
			resetting_video_mode = false;
		}

		return 0;
	}

	// Let ImGui handle window messages if it's initialized
	if (imgui_initialized && imgui->handle_message(hwnd, msg, wparam, lparam)) return 1;

	switch (msg)
	{
		case WM_SYSCOMMAND:
		{
			if ((wparam & 0xfff0) == SC_KEYMENU) return 0; // Disable ALT application menu

			break;
		}
		case WM_KEYDOWN: case WM_KEYUP:
		{
			if (msg == WM_KEYUP || !((static_cast<int>(lparam) >> 30) & 1)) // ignore repeat messages on WM_KEYDOWN
			{
				auto found_key = to_imgui_key(wparam);

				if (found_key != static_cast<ImGuiKey>(ImGuiKey_COUNT)) main_handle_key(msg == WM_KEYDOWN, found_key);
			}

			break;
		}
		default:
		{
			break;
		}
	}

	// Call the Windows message handler
	return DefWindowProcA(hwnd, msg, wparam, lparam);
}

/*
@brief

    Main program entry point (Windows)
*/
INT WINAPI WinMain(HINSTANCE,HINSTANCE,char*,int)
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
		MessageBoxA(nullptr, "Failed to initialize sound library", "cpp-retro-games", MB_ICONERROR | MB_SETFOREGROUND);

		return;
	}

	snd = &_snd;

	// Grab the wanted resolution from our settings
	auto resolution = main_settings.resolution_area;

	// Grab the main settings
	auto vsync = main_settings.vsync->get<bool>();
	auto fullscreen = main_settings.fullscreen->get<bool>();
	auto fps = main_settings.fps->get<uint32_t>();

	// Initialize the imgui object
	imgui = std::make_unique<imgui_wrapper_dx_t>(&settings, "cpp-retro-games", resolution, vsync, fullscreen);

	// Attempt to initialize DirectX
	std::string error;

	if (!imgui->initialize(true, &error, window_procedure))
	{
		MessageBoxA(nullptr, (std::string("Failed to initialize DirectX9. Terminating process.\n\nError: ") + error).c_str(), "cpp-retro-games", MB_ICONERROR | MB_SETFOREGROUND);

		return;
	}

	// Attempt to initialize ImGui
	if (!imgui->initialize(false, &error))
	{
		MessageBoxA(nullptr, (std::string("Failed to initialize DirectX. Terminating process.\n\nError: ") + error).c_str(), "cpp-retro-games", MB_ICONERROR | MB_SETFOREGROUND);

		return;
	}

	// Retrieve the window for own use
	auto window = imgui->get_window();

	// Tell our window that imgui has been initialized
	imgui_initialized = true;

    // Create the fpsmanager object (if vsync is disabled)
	std::unique_ptr<fpsmanager_t> fpsmanager;

	static bool vsync_enabled = vsync;

	if (!vsync_enabled) fpsmanager = std::make_unique<fpsmanager_t>(static_cast<uint16_t>(fps));

	// Call the main initialize function
	main_initialize(&settings);

	// This will be true if we changed video settings
	bool reset_video_settings = false;

	// Start our main loop
	MSG msg{};

	while (msg.message != WM_QUIT)
	{
		// Listen for messages, and if there are any, handle them (without continuing our main task)
		if (PeekMessageA(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageA(&msg);

			continue;
		}

		// Begin the frame
		auto render = imgui->begin_frame();

		// Draw
		auto should_render = render && window->is_in_foreground();
		auto should_exit = main_frame(should_render, reset_video_settings);

		// End the frame
		imgui->end_frame(should_render, color_t(40, 40, 40));

		// Exit if we should (if main_frame returns true)
		if (should_exit) break;

		// Limit our FPS
		if (!vsync_enabled) fpsmanager->run();

		// Reset video mode if we should
		if (reset_video_settings)
		{
			reset_video_settings = false;
			resetting_video_mode = true;

			if (!imgui->reinitialize(&error))
			{
				MessageBoxA(NULL, (std::string("Failed to reinitialize DirectX9/ImGui. Terminating process.\n\nError: ") + error).c_str(), "cpp-retro-games", MB_ICONERROR | MB_SETFOREGROUND);

				break;
			}

			window = imgui->get_window();

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

	// Thread finished, shut down ImGui and DirectX
	imgui->shutdown();

	// Save our settings
	settings.save();
}
#endif