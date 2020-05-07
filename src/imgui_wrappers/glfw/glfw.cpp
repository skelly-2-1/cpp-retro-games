#ifdef PLATFORM_LINUX

/*
@file

	glfw.h

@purpose

	Wrap ImGui and OpenGL/GLEW/GLFW functionality (Linux)
*/

#include "glfw.h"
#include "util/util.h"

// ImGui and GLFW/OGL3 implementation
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <stdio.h>
#include <string>

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

/*
@brief

    Error callback when an error occurs in GLFW
*/
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

// Internal class used by the wrapper
class retrogames::internal::glfw_imgui
{

protected:



private:

	GLFWwindow* glfw_window;

    bool initialized;

public:

    /*
    @brief

        Retrieves the GLFW window pointer
    */
    GLFWwindow* get_glfw_window(void) { return glfw_window; }

	/*
	@brief

		Constructor
	*/
	glfw_imgui() : glfw_window(nullptr), initialized(false)
	{

	}

	/*
	@brief

		Destructor
	*/
	~glfw_imgui()
	{
		if (!initialized) return;
	}

    /*
    @brief

        Sets up GLFW
    */
    bool setup(const std::string& window_title, area_size_t& size, bool vsync, bool fullscreen, std::string* error = nullptr, bool glfw_init = true)
    {
        if (initialized && glfw_init) return true;

        // Setup window
        if (glfw_init) glfwSetErrorCallback(glfw_error_callback);

        if (/*glfw_init && */!glfwInit())
        {
            if (error) *error = "glfwInit returned 0";

            return false;
        }

        // Check if the resolution we want to use is supported
        const auto& supported_resolutions = util::get_supported_resolutions(16, 9); // We only support 16:9 resolutions as of now!

        if (supported_resolutions.first.empty())
        {
            glfwTerminate();

            if (error) *error = "Failed to find supported resolutions";

            return false;
        }

        //if (fullscreen)
        //{
            bool found = false;

            for (const auto& resolution : supported_resolutions.first)
            {
                auto width = std::get<0>(resolution);
                auto height = std::get<1>(resolution);

                if (width == static_cast<uint16_t>(size.width) && height == static_cast<uint16_t>(size.height))
                {
                    found = true;

                    break;
                }
            }

            if (!found)
            {
                printf("Specified resolution not supported. Choosing biggest/native resolution!\n");
                printf("Supported resolutions:\n");

                for (const auto& resolution : supported_resolutions.first)
                {
                    printf("%ix%i\n", static_cast<int32_t>(std::get<0>(resolution)), static_cast<int32_t>(std::get<1>(resolution)));
                }

                auto& last_elem = supported_resolutions.first.at(supported_resolutions.first.size() - 1);

                size.width = static_cast<uint16_t>(std::get<0>(last_elem));
                size.height = static_cast<uint16_t>(std::get<1>(last_elem));
            }
        /*}
        else
        {
            if (!util::check_aspect_ratio(size))
            {
                glfwTerminate();

                if (error) *error = "Specified resolution is not 16:9";

                return false;
            }
        }*/

        if (!fullscreen)
        {
            auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

            if (size.width == static_cast<uint32_t>(mode->width) && size.height == static_cast<uint32_t>(mode->height))
            {
                glfwWindowHint(GLFW_RED_BITS, mode->redBits);
                glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
                glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
                glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
                
                fullscreen = true;
            }
        }

        // Disable double buffering when vsync is disabled
        if (!vsync) glfwWindowHint(GLFW_DOUBLEBUFFER, 0);

        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

        // Create window with graphics context
        glfw_window = glfwCreateWindow(size.width, size.height, window_title.c_str(), fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

        if (glfw_window == nullptr)
        {
            if (error) *error = "Failed to create a window using glfw (glfwCreateWindow returned nullptr)";

            return false;
        }

        glfwMakeContextCurrent(glfw_window);
        glfwSwapInterval(vsync ? 1 : 0); // Enable or disable vsync

        if (glfw_init)
        {
            // Initialize OpenGL loader
        #if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
            bool err = gl3wInit() != 0;
        #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
            bool err = glewInit() != GLEW_OK;
        #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
            bool err = gladLoadGL() == 0;
        #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
            bool err = false;
            glbinding::Binding::initialize();
        #elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
            bool err = false;
            glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
        #else
        #error No OpenGL loader found!
            bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
        #endif
            
            if (err)
            {
                if (error) *error = "OpenGL Loader failed to initialize";

                return false;
            }
        }

        initialized = true;

        return true;
    }

};

/*
@brief

	Constructor
*/
retrogames::imgui_wrapper_glfw_t::imgui_wrapper_glfw_t(settings_t* settings, const std::string& window_title, const area_size_t& size, bool vsync, bool fullscreen) :
    window_title(window_title),
    window_size(size),
    imgui_created(false),
    render(true),
    vsync(vsync),
    fullscreen(fullscreen),
    settings(settings),
    imgui(new internal::glfw_imgui()) {}

/*
@brief

	Destructor
*/
retrogames::imgui_wrapper_glfw_t::~imgui_wrapper_glfw_t()
{
    delete imgui;
}

/*
@brief

    Retrieves the GLFW window pointer
*/
GLFWwindow* retrogames::imgui_wrapper_glfw_t::get_glfw_window(void)
{
    return imgui->get_glfw_window();
}

/*
@brief

	Reinitializes GLFW/ImGui
*/
bool retrogames::imgui_wrapper_glfw_t::reinitialize(std::string* error/* = nullptr*/)
{
	if (!original_style_colors_set)
	{
		original_style_colors_set = true;

		memcpy(original_style_colors, ImGui::GetStyle().Colors, sizeof(original_style_colors));
	}

    // Shutdown everything needed
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    glfwDestroyWindow(imgui->get_glfw_window());
    glfwTerminate();

    // Now, re-initialize
    // First, set the new resolution_area because when we change
    // the resolution in the menu, all we do is modify the string,
    // which doesn't get used apart from initially setting the
    // resolution.
    auto& main_settings = settings->get_main_settings();

    main_settings.resolution_area = area_size_t(1280, 720); // 720p default
    {
        auto res = main_settings.resolution->get<std::string>();
        auto pos = res.find_first_of('x');

        if (pos != std::string::npos)
        {
            main_settings.resolution_area.width = std::stoi(res.substr(0, pos));
            main_settings.resolution_area.height = std::stoi(res.substr(pos + 1));
        }
    }

    // Now, re-initialize GLFW (create a new window etc)
    vsync = main_settings.vsync->get<bool>();
    fullscreen = main_settings.fullscreen->get<bool>();
    window_size = main_settings.resolution_area;

    if (!imgui->setup(window_title, main_settings.resolution_area, vsync, fullscreen, error, false)) return false;

    // GLFW re-initialized, re-initialize ImGui
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.IniFilename = nullptr; // Prevent ImGui from changing settings
    io.DisplaySize = ImVec2{static_cast<float>(main_settings.resolution_area.width), static_cast<float>(main_settings.resolution_area.height)};

	auto& style = ImGui::GetStyle();

	style = original_style;

	memcpy(style.Colors, original_style_colors, sizeof(original_style_colors));

	style.ScaleAllSizes(static_cast<float>(window_size.height) / 1080.f);

    // Decide GL+GLSL versions
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";

    // Setup Platform/Renderer bindings
    if (!ImGui_ImplGlfw_InitForOpenGL(imgui->get_glfw_window(), true))
    {
        if (error) *error = "ImGui_ImplGlfw_InitForOpenGL failed";

        return false;
    }

    if (!ImGui_ImplOpenGL3_Init(glsl_version))
    {
        if (error) *error = "ImGui_ImplOpenGL3_Init failed";

        return false;
    }

    return true;
}

/*
@brief

	Initializes ImGui or GLFW
*/
bool retrogames::imgui_wrapper_glfw_t::initialize(bool glfw, std::string* error/* = nullptr*/)
{
	// If we're only initializing glfw
	if (glfw)
    {
        if (imgui->setup(window_title, window_size, vsync, fullscreen, error))
        {
            settings->get_main_settings().resolution_area = window_size;
            settings->get_main_settings().resolution->set(std::to_string(window_size.width) + 'x' + std::to_string(window_size.height));

            return true;
        }

        return false;
    }

	// Check if we've already created the ImGui context
	if (imgui_created) return true;

	// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.IniFilename = nullptr; // Prevent ImGui from changing settings
    io.DisplaySize = ImVec2{static_cast<float>(window_size.width), static_cast<float>(window_size.height)};

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Decide GL+GLSL versions
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130"; 

    // Setup Platform/Renderer bindings
    if (!ImGui_ImplGlfw_InitForOpenGL(imgui->get_glfw_window(), true))
    {
        if (error) *error = "ImGui_ImplGlfw_InitForOpenGL failed";

        return false;
    }

    if (!ImGui_ImplOpenGL3_Init(glsl_version))
    {
        if (error) *error = "ImGui_ImplOpenGL3_Init failed";

        return false;
    }

    // ImGui has been initialized successfully
	imgui_created = true;

    // Save the original style
    auto& style = ImGui::GetStyle();

    original_style = style;

    // Scale the style by our resolution
    style.ScaleAllSizes(static_cast<float>(window_size.height) / 1080.f);

	// All done!
	return true;
}

/*
@brief

	Begins a frame
*/
bool retrogames::imgui_wrapper_glfw_t::begin_frame(void)
{
	// Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

	return true;
}

/*
@brief

	Ends a frame
*/
void retrogames::imgui_wrapper_glfw_t::end_frame(bool should_render, const color_t clear_color/* = color_t(20, 20, 20)*/)
{
	if (!render || !should_render)
	{
		// Otherwise ImGui will crash
		ImGui::EndFrame();
		ImGui::Render();

		// Don't re-draw the window (would be empty then)
		return;
	}

	ImGui::Render();

    int display_w, display_h;

    auto window = imgui->get_glfw_window();

    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(static_cast<float>(clear_color.r()) / 255.f, static_cast<float>(clear_color.g()) / 255.f, static_cast<float>(clear_color.b()) / 255.f, static_cast<float>(clear_color.a()) / 255.f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (vsync)
    {
        glfwSwapBuffers(window);
    }
    else
    {
        glFlush();
    }
}

/*
@brief

	Shuts down GLFW and ImGui
*/
void retrogames::imgui_wrapper_glfw_t::shutdown(void)
{
	if (!imgui_created) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();

    glfwDestroyWindow(imgui->get_glfw_window());
    glfwTerminate();

	imgui_created = false;
}

/*
@brief

    Tells our caller if the window should close
*/
bool retrogames::imgui_wrapper_glfw_t::should_close(void)
{
    return glfwWindowShouldClose(imgui->get_glfw_window()) != 0;
}

/*
@brief

    Polls events using GLFW
*/
void retrogames::imgui_wrapper_glfw_t::poll_events(void)
{
    glfwPollEvents();
}

#endif