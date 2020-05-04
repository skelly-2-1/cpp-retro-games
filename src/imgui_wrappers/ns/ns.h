#ifdef PLATFORM_NS

/*
@file

	ns.h

@purpose

	Wrap ImGui and OpenGL/GLEW/GLFW functionality (NS)
*/

#pragma once

#include <cstdint>
#include <string>
#include "misc/color.h"
#include "misc/area_size.h"
#include "misc/settings.h"
#include "misc/macros.h"

#ifdef NS_IMGUI_SOFTWARE_RENDERING
#include <vector>
#include <switch.h>
#include "imgui/imgui_sw.h"
#endif

namespace retrogames
{

	class imgui_wrapper_opengl_t final
	{

	protected:



	private:

		bool gl_created;
		bool render;

        const settings_t* settings;

#ifdef NS_IMGUI_SOFTWARE_RENDERING
        // Pixel buffer for our renderer
	    std::vector<uint32_t> pixel_buffer;

        // Framebuffer
        Framebuffer fb;

        // Software rendering options
       	imgui_sw::SwOptions sw_options;
#endif

        /*
        @brief

            Sets up the ImGui IO variables the way they're wanted
        */
        void setup_io(void);

        /*
        @brief

            Sets up OpenGL
        */
        bool setup(std::string* error = nullptr);

	public:

		/*
		@brief

			Constructor
		*/
		imgui_wrapper_opengl_t(const settings_t* settings);

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

			Initializes ImGui or OpenGL
		*/
		bool initialize(bool opengl, std::string* error = nullptr);

		/*
		@brief

			Shuts down OpenGL and ImGui
		*/
		void shutdown(void);

	};

}

#endif