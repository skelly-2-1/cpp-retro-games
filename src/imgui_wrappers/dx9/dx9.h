#ifdef PLATFORM_WINDOWS

/*
@file

	dx9.h

@purpose

	Wrap ImGui and DirectX 9 functionality (Windows)
*/

#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>
#include <memory>
#include "misc/color.h"
#include "misc/area_size.h"
#include "misc/settings.h"
#include "imgui/imgui.h"

namespace retrogames
{

	// So we don't have to include the headers here
	class window_t;

	// So we don't have to include imgui/directx headers here
	namespace internal
	{

		class dx_imgui;

	}

	class imgui_wrapper_dx_t final
	{

	protected:



	private:

		ImGuiStyle original_style;

		internal::dx_imgui* imgui;

		std::unique_ptr<window_t> window;

		bool imgui_created;
		bool render;
		bool vsync, fullscreen;

		std::string window_title;

        area_size_t window_size;

		settings_t* settings;

		WNDPROC proc;

	public:

		/*
		@brief

			Access our window from callers
		*/
		const window_t* get_window(void) const { return window.get(); }

		/*
		@brief

			Constructor
		*/
		imgui_wrapper_dx_t(settings_t* settings, const std::string& window_title, const area_size_t& size, bool vsync, bool fullscreen);

		/*
		@brief

			Destructor
		*/
		~imgui_wrapper_dx_t();

		/*
		@brief

			Begins a frame
		*/
		bool begin_frame(void);

		/*
		@brief

			Ends a frame
		*/
		void end_frame(bool should_render, const color_t clear_color = color_t(20, 20, 20));

		/*
		@brief

			Handles ImGui and returns whether to block input or not
		*/
		bool handle_message(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		/*
		@brief

			Initializes ImGui or DirectX
		*/
		bool initialize(bool directx, std::string* error = nullptr, WNDPROC proc = nullptr);

		/*
		@brief

			Resets the D3D9 device. If @width or @height is 0, they'll be ignored.
		*/
		void reset(uint32_t width = 0, uint32_t height = 0);

		/*
		@brief

			Shuts down DirectX and ImGui
		*/
		void shutdown(void);

		/*
		@brief

			Reinitializes DirectX/ImGui
		*/
		bool reinitialize(std::string* error = nullptr);

	};

}
#endif