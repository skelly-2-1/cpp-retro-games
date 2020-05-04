#ifdef PLATFORM_LINUX

/*
@file

	glfw.h

@purpose

	Wrap ImGui and OpenGL/GLEW/GLFW functionality (Linux)
*/

#pragma once

#include <cstdint>
#include <string>
#include "misc/color.h"
#include "misc/area_size.h"
#include "misc/settings.h"

struct GLFWwindow;

namespace retrogames
{

	// So we don't have to include imgui/glfw headers here
	namespace internal
	{

		class glfw_imgui;

	}

	class imgui_wrapper_glfw_t final
	{

	protected:



	private:

		settings_t* settings;

		internal::glfw_imgui* imgui;

		bool imgui_created;
		bool render;

        std::string window_title;

        area_size_t window_size;

        bool vsync, fullscreen;

	public:

		/*
		@brief

			Constructor
		*/
		imgui_wrapper_glfw_t(settings_t* settings, const std::string& window_title, const area_size_t& size, bool vsync, bool fullscreen);

		/*
		@brief

			Destructor
		*/
		~imgui_wrapper_glfw_t();

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

			Initializes ImGui or GLFW
		*/
		bool initialize(bool glfw, std::string* error = nullptr);

		/*
		@brief

			Reinitializes GLFW/ImGui
		*/
		bool reinitialize(std::string* error = nullptr);

		/*
		@brief

			Shuts down GLFW and ImGui
		*/
		void shutdown(void);

        /*
        @brief

            Tells our caller if the window should close
        */
        bool should_close(void);

        /*
        @brief

            Polls events using GLFW
        */
        void poll_events(void);

		/*
    	@brief

        	Retrieves the GLFW window pointer
    	*/
    	GLFWwindow* get_glfw_window(void);

	};

}

#endif