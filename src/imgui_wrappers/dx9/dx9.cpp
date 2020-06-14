#ifdef PLATFORM_WINDOWS
/*
@file

	dx9.cpp

@purpose

	Wrap ImGui and DirectX 9 functionality
*/

#include "dx9.h"
#include "misc/window.h"
#include "util/util.h"

// ImGui and DirectX9 implementation
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include <d3d9.h>

// defined in imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// defined in dx9.h
IDirect3DDevice9* retrogames::global_d3d9device = nullptr;

// internal class used by the wrapper
class retrogames::internal::dx_imgui
{

protected:



private:

	

public:

	// DX
	LPDIRECT3D9 d3d;
	LPDIRECT3DDEVICE9 d3ddevice;
	D3DPRESENT_PARAMETERS d3dpp;

	/*
	@brief

		Constructor
	*/
	dx_imgui() : d3d(nullptr), d3ddevice(nullptr), d3dpp({})
	{

	}

	/*
	@brief

		Releases all objects
	*/
	void release(void)
	{
		if (d3d != nullptr)
		{
			d3d->Release();
			d3d = nullptr;
		}

		if (d3ddevice != nullptr)
		{
			d3ddevice->Release();
			d3ddevice = nullptr;
		}
	}

	/*
	@brief

		Destructor
	*/
	~dx_imgui()
	{
		release();
	}

	/*
	@brief

		Initializes DirectX9
	*/
	bool create_d3d9_device(window_t* window, bool vsync, std::string* error = nullptr)
	{
		if ((d3d = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
		{
			if (error) *error = "Direct3DCreate9 failed";

			return false;
		}

		D3DDISPLAYMODE d3ddm{};

		auto hr = d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm);

		if (hr < 0)
		{
			if (error)
			{
				char fmt[256];

				sprintf_s(fmt, "LPDIRECT3D9::GetAdapterDisplayMode failed with error code: 0x%X", static_cast<DWORD>(hr));

				*error = fmt;
			}

			delete d3d;

			d3d = nullptr;

			return false;
		}

		auto hwnd = window->get_handle();

		// Create the D3DDevice
		memset(&d3dpp, 0, sizeof(d3dpp));

		d3dpp.Windowed = window->is_fullscreen() ? FALSE : TRUE;
		d3dpp.BackBufferCount = 1;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = d3ddm.Format;
		d3dpp.EnableAutoDepthStencil = TRUE;
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		d3dpp.BackBufferWidth = window->get_size().width;
		d3dpp.BackBufferHeight = window->get_size().height;
		//d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;

		if (d3dpp.Windowed == FALSE)
		{
			d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		}

		d3dpp.hDeviceWindow = hwnd;

		if (vsync)
		{
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
		}
		else
		{
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; // Present without vsync, maximum unthrottled framerate
		}

		hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3ddevice);

		if (hr < 0)
		{
			if (error)
			{
				char fmt[256];

				sprintf_s(fmt, "LPDIRECT3D9::CreateDevice failed with error code: 0x%X", static_cast<DWORD>(hr));

				*error = fmt;
			}

			d3d->Release();
			d3d = nullptr;

			if (d3ddevice != nullptr)
			{
				d3ddevice->Release();
				d3ddevice = nullptr;
			}

			return false;
		}

		global_d3d9device = d3ddevice;

		return true;
	}

	/*
	@brief

		Resets D3D9, terminates the process if it fails
	*/
	void reset_device(void)
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();

		auto hr = d3ddevice->Reset(&d3dpp);

		if (hr < 0)
		{
			char fmt[256];

			sprintf_s(fmt, "LPDIRECTD3DDEVICE9::Reset failed with error code: 0x%X", static_cast<DWORD>(hr));

			MessageBoxA(nullptr, fmt, "cpp-retro-games", MB_ICONERROR | MB_SETFOREGROUND);

			TerminateProcess(GetCurrentProcess(), EXIT_SUCCESS);
		}

		ImGui_ImplDX9_CreateDeviceObjects();
	}

};

/*
@brief

	Handles ImGui and returns whether to block input or not
*/
bool retrogames::imgui_wrapper_dx_t::handle_message(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) != 0;
}

/*
@brief

	Constructor
*/
retrogames::imgui_wrapper_dx_t::imgui_wrapper_dx_t(settings_t* settings, const std::string& window_title, const area_size_t& size, bool vsync, bool fullscreen) :
	window_size(size),
	vsync(vsync),
	fullscreen(fullscreen),
	window_title(window_title),
	imgui_created(false),
	render(true),
	settings(settings),
	original_style_colors_set(false),
	imgui(new internal::dx_imgui()) {}

/*
@brief

	Reinitializes GLFW/ImGui
*/
bool retrogames::imgui_wrapper_dx_t::reinitialize(std::string* error/* = nullptr*/)
{
	if (!original_style_colors_set)
	{
		original_style_colors_set = true;

		memcpy(original_style_colors, ImGui::GetStyle().Colors, sizeof(original_style_colors));
	}

	shutdown();

	imgui->release();

	window.reset();

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

    // Now, re-initialize DirectX (create a new window etc)
    vsync = main_settings.vsync->get<bool>();
    fullscreen = main_settings.fullscreen->get<bool>();
    window_size = main_settings.resolution_area;

	if (!initialize(true, error, proc)) return false;
	if (!initialize(false, error)) return false;

	auto& style = ImGui::GetStyle();

	style = original_style;

	memcpy(style.Colors, original_style_colors, sizeof(original_style_colors));

	style.ScaleAllSizes(static_cast<float>(window_size.height) / 1080.f);

	return true;
}

/*
@brief

	Destructor
*/
retrogames::imgui_wrapper_dx_t::~imgui_wrapper_dx_t()
{
	delete imgui;
}

/*
@brief

	Initializes ImGui or DirectX
*/
bool retrogames::imgui_wrapper_dx_t::initialize(bool directx, std::string* error/* = nullptr*/, WNDPROC proc/* = nullptr*/)
{
	// If we're only initializing DirectX
	if (directx)
	{
		// Check if we have a valid window procedure
		if (proc == nullptr)
		{
			if (error) *error = "No window procedure specified";

			return false;
		}

		this->proc = proc;

		// Check if the resolution we want to use is supported
        const auto& supported_resolutions = util::get_supported_resolutions(16, 9); // We only support 16:9 resolutions as of now!

        if (supported_resolutions.first.empty())
        {
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

                if (width == static_cast<uint16_t>(window_size.width) && height == static_cast<uint16_t>(window_size.height))
                {
                    found = true;

                    break;
                }
            }

            if (!found)
            {
				std::string msg = "Specified resolution not supported. Choosing biggest/native resolution!\nSupported resolutions:\n";

				char buf[80];

                for (const auto& resolution : supported_resolutions.first)
                {
                    sprintf(buf, "\n%ix%i", static_cast<int32_t>(std::get<0>(resolution)), static_cast<int32_t>(std::get<1>(resolution)));

					msg += buf;
                }

				MessageBoxA(nullptr, msg.c_str(), "cpp-retro-games", MB_ICONWARNING | MB_SETFOREGROUND);

                auto& last_elem = supported_resolutions.first.at(supported_resolutions.first.size() - 1);

                window_size.width = static_cast<uint16_t>(std::get<0>(last_elem));
                window_size.height = static_cast<uint16_t>(std::get<1>(last_elem));
            }
        /*}
        else
        {
            if (!util::check_aspect_ratio(window_size))
            {
                if (error) *error = "Specified resolution is not 16:9";

                return false;
            }
        }*/

		// Set the window to fullscreen if we're running max resolution (to remove the borders)
		if (!fullscreen)
		{
			// Get the desktop resolution
			MONITORINFO monitorInfo = { 0 };
			monitorInfo.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY), &monitorInfo);

			if (window_size.width == static_cast<uint32_t>(monitorInfo.rcMonitor.right) && window_size.height == static_cast<uint32_t>(monitorInfo.rcMonitor.bottom)) fullscreen = true;
		}

		// Create a window
		window = std::make_unique<window_t>(window_size, window_title.c_str(), window_title.c_str(), fullscreen, proc);

		if (!window->create_window())
		{
			MessageBoxA(NULL, "Failed to create window!\n\nTerminating process...", "cpp-retro-games", MB_ICONERROR | MB_SETFOREGROUND);

			return false;
		}

		// Create the d3d9 device for the window to draw on
		if (!imgui->create_d3d9_device(window.get(), vsync, error)) return false;

		// Show the window
		window->show();

		// All done!
		return true;
	}

	// Check if we've already created the ImGui context
	if (imgui_created) return true;

	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.IniFilename = nullptr; // Prevent ImGui from changing settings

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	auto& style = ImGui::GetStyle();

	original_style = style;

	style.ScaleAllSizes(static_cast<float>(window_size.height) / 1080.f);

	// Setup Platform/Renderer bindings
	if (!ImGui_ImplWin32_Init(window->get_handle()))
	{
		if (error) *error = "ImGui_ImplWin32_Init returned false";

		return false;
	}

	if (!ImGui_ImplDX9_Init(imgui->d3ddevice))
	{
		if (error) *error = "ImGui_ImplDX9_Init returned false";

		return false;
	}

	imgui_created = true;

	// All done!
	return true;
}

/*
@brief

	Resets the D3D9 device. If @width or @height is 0, they'll be ignored.
*/
void retrogames::imgui_wrapper_dx_t::reset(uint32_t width/* = 0*/, uint32_t height/* = 0*/)
{
	if (width != 0) imgui->d3dpp.BackBufferWidth = width;
	if (height != 0) imgui->d3dpp.BackBufferHeight = height;

	imgui->reset_device();
}

/*
@brief

	Begins a frame
*/
bool retrogames::imgui_wrapper_dx_t::begin_frame(void)
{
	// Test if we need to reset the D3D9 device or we lost it
	auto hr = imgui->d3ddevice->TestCooperativeLevel();

	if (hr == D3DERR_DEVICELOST)
	{
		// Start a new frame (otherwise ImGui will crash)
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		render = false;

		// Shut down ImGui (otherwise it will crash)
		//shutdown();

		return false;
	}
	else if (hr == D3DERR_DEVICENOTRESET)
	{
		reset();

		render = true;

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		/*// Re-initialize
		std::string error;

		if (!initialize(false, window, &error))
		{
			MessageBoxA(NULL, (std::string("Re-initializing of ImGui failed, error:\n\n") + error).c_str(), "cpp-retro-games", MB_ICONERROR | MB_SETFOREGROUND);

			TerminateProcess(GetCurrentProcess(), EXIT_SUCCESS);
		}*/

		return true;
	}
	else if (hr == D3DERR_DRIVERINTERNALERROR)
	{
		MessageBoxA(NULL, "Fatal error:\n\nLPDIRECT3DDEVICE9::TestCooperativeLevel() returned D3DERR_DRIVERINTERNALERROR", "cpp-retro-games", MB_ICONERROR | MB_SETFOREGROUND);

		TerminateProcess(GetCurrentProcess(), EXIT_SUCCESS);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	return true;
}

/*
@brief

	Ends a frame
*/
void retrogames::imgui_wrapper_dx_t::end_frame(bool should_render, const color_t clear_color/* = color_t(20, 20, 20)*/)
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

	imgui->d3ddevice->SetRenderState(D3DRS_ZENABLE, false);
	imgui->d3ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	imgui->d3ddevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

	auto clear_col_dx = D3DCOLOR_RGBA((int)(static_cast<float>(clear_color.r())), (int)(static_cast<float>(clear_color.g())), (int)(static_cast<float>(clear_color.b())), (int)(static_cast<float>(clear_color.a())));

	imgui->d3ddevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.f, 0);

	if (imgui->d3ddevice->BeginScene() >= 0)
	{
		ImGui::Render();

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		imgui->d3ddevice->EndScene();
	}

	auto result = imgui->d3ddevice->Present(NULL, NULL, NULL, NULL);

	// Handle loss of D3D9 device
	//if (result == D3DERR_DEVICELOST && imgui->d3ddevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) reset();
}

/*
@brief

	Shuts down DirectX and ImGui
*/
void retrogames::imgui_wrapper_dx_t::shutdown(void)
{
	if (!imgui_created) return;

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	imgui_created = false;
}
#endif