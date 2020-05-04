#ifdef PLATFORM_NS

/*
@file

	ns.cpp

@purpose

	Wrap ImGui and OpenGL/GLEW/GLFW functionality (NS)
*/

#include "ns.h"
#include "imgui/imgui.h"
#include <chrono>
#include <glad/glad.h>  // glad library (OpenGL loader)

#ifndef NS_IMGUI_SOFTWARE_RENDERING
#include <switch.h>
#include "imgui/imgui_switch_opengl_impl.h"
#else
#include "imgui/imgui_sw.h"
#endif

/*
@brief

    Sets up OpenGL
*/
bool retrogames::imgui_wrapper_opengl_t::setup(std::string* error/* = nullptr*/)
{
	if (gl_created) return true;

	// Setup ImGui context
    IMGUI_CHECKVERSION();

	ImGui::CreateContext();

    setup_io();

#ifndef NS_IMGUI_SOFTWARE_RENDERING
    if (!ImGui_ImplSwitch_Init())
    {
        if (error) *error = "ImGui_ImplSwitch_Init returned false";

        return false;
    }
#else
    // Retrieve the default window
    auto win = nwindowGetDefault();

	// Create a framebuffer to render to
    framebufferCreate(&fb, win, FB_WIDTH, FB_HEIGHT, PIXEL_FORMAT_RGBA_8888, 2); // 2 = double-buffered
    framebufferMakeLinear(&fb);

    // Bind software renderer to ImGui
    imgui_sw::bind_imgui_painting();
	imgui_sw::make_style_fast();
#endif

	gl_created = true;

    return true;
}

/*
@brief

    Constructor
*/
retrogames::imgui_wrapper_opengl_t::imgui_wrapper_opengl_t(const settings_t* settings) :
	gl_created(false),
    render(true),
    settings(settings)
{
#ifdef NS_IMGUI_SOFTWARE_RENDERING
	pixel_buffer.resize(FB_WIDTH * FB_HEIGHT, 0);
#endif
}

/*
@brief

	Initializes ImGui or GLFW
*/
bool retrogames::imgui_wrapper_opengl_t::initialize(bool opengl, std::string* error/* = nullptr*/)
{
	// If we're only initializing opengl
	if (opengl) return setup(error);

	// Nothing to be done here, since we have to initialize ImGui before initializing the renderer (see @setup)
	return true;
}

/*
@brief

	Begins a frame
*/
bool retrogames::imgui_wrapper_opengl_t::begin_frame(void)
{
    // Determine frametime (this will also tell us the FPS in the end)
	static bool g_time_set = false;
	static std::chrono::high_resolution_clock::time_point g_time;

    if (!g_time_set)
	{
	    g_time = std::chrono::high_resolution_clock::now();
		g_time_set = true;
	}

	auto current_time = std::chrono::high_resolution_clock::now();
	auto delta_time_ns = current_time - g_time;
	auto& io = ImGui::GetIO();

	io.DeltaTime = static_cast<float>(static_cast<double>(delta_time_ns.count()) / 1000000000.); // Delta time is in seconds, so divide nanoseconds by 1000000000

	g_time = current_time;

    // Start the new frame (Renderer)
#ifndef NS_IMGUI_SOFTWARE_RENDERING
	ImGui_ImplSwitch_NewFrame();
#endif

    // Start the new frame (ImGui)
	ImGui::NewFrame();

	return true;
}

/*
@brief

	Ends a frame
*/
void retrogames::imgui_wrapper_opengl_t::end_frame(bool should_render, const color_t clear_color/* = color_t(20, 20, 20)*/)
{
	if (!render || !should_render)
	{
		// Otherwise ImGui will crash
		ImGui::EndFrame();
		ImGui::Render();

		// Don't re-draw the window (would be empty then)
		return;
	}

	ImGui::EndFrame();
    ImGui::Render();

#ifdef NS_IMGUI_SOFTWARE_RENDERING
	static auto create_rgba = [](uint8_t r, uint8_t g, uint8_t b, uint8_t a) -> uint32_t
	{
		return ((r & 0xff) << 24) + ((g & 0xff) << 16) + ((b & 0xff) << 8) + (a & 0xff);
	};

	std::fill_n(pixel_buffer.data(), FB_WIDTH * FB_HEIGHT, 0x19191919);
	//std::fill_n(pixel_buffer.data(), FB_WIDTH * FB_HEIGHT, 0x19191919u);
	//std::fill_n(pixel_buffer.data(), FB_WIDTH * FB_HEIGHT, create_rgba(clear_color.r(), clear_color.g(), clear_color.b(), clear_color.a()));

	// Render using software rendering
	paint_imgui(ImGui::GetDrawData(), pixel_buffer.data(), FB_WIDTH, FB_HEIGHT, sw_options);
#else
	// Render using OpenGL
    glClearColor(static_cast<float>(clear_color.r()) / 255.f, static_cast<float>(clear_color.g()) / 255.f, static_cast<float>(clear_color.b()) / 255.f, static_cast<float>(clear_color.a()) / 255.f);
    glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplSwitch_RenderDrawData(ImGui::GetDrawData());
#endif

#ifdef NS_IMGUI_SOFTWARE_RENDERING
	// Set our framebuffer pixels to our software rendered pixels
	u32 stride = 0;

	auto framebuf = (u32*) framebufferBegin(&fb, &stride);

	for (int y = 0; y < FB_HEIGHT; y++)
	{
		for (int x = 0; x < FB_WIDTH; x++)
		{
			framebuf[y * stride / sizeof(uint32_t) + x] = pixel_buffer.data()[y * FB_WIDTH + x];
		}
	}

	// Apply the framebuffer to screen
	framebufferEnd(&fb);
#endif
}

/*
@brief

	Shuts down OpenGL and ImGui
*/
void retrogames::imgui_wrapper_opengl_t::shutdown(void)
{
	if (!gl_created) return;

#ifndef NS_IMGUI_SOFTWARE_RENDERING
	// Shut down the OpenGL renderer
	ImGui_ImplSwitch_Shutdown();
#else
	// Clean up software rendering textures
	imgui_sw::unbind_imgui_painting();
#endif

	// Done using ImGui, destroy its context
	ImGui::DestroyContext();
}

/*
@brief

    Sets up the ImGui IO variables the way they're wanted
*/
void retrogames::imgui_wrapper_opengl_t::setup_io(void)
{
    auto& io = ImGui::GetIO();

    io.IniFilename = nullptr;
    io.FontGlobalScale = 2.0;
    io.DisplaySize = ImVec2((float)FB_WIDTH, (float)FB_HEIGHT);
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
}
#endif