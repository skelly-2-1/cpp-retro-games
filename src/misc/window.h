#ifdef PLATFORM_WINDOWS
#pragma once

#include <Windows.h>
#include <string>
#include "misc/area_size.h"

namespace retrogames
{

    class window_t final
    {

    protected:



    private:

        area_size_t size;

        WNDCLASSEXA wc;

        std::string class_name, title;

        HINSTANCE instance;

        HWND hwnd;

        bool window_created, window_shown, fullscreen;

    public:

        /*
        @brief

            Constructor
        */
        window_t(const area_size_t& size, const std::string& class_name, const std::string& title, bool fullscreen, WNDPROC proc) :
            size(size),
            class_name(class_name),
            instance(GetModuleHandle(NULL)),
            hwnd(NULL),
            title(title),
            window_created(false),
            window_shown(false),
            fullscreen(fullscreen)
        {
            ZeroMemory(&wc, sizeof(WNDCLASSEX));

            wc.cbSize = sizeof(WNDCLASSEX);
            wc.lpfnWndProc = proc;
            wc.hInstance = instance;
            wc.lpszClassName = class_name.c_str();

            RegisterClassExA(&wc); // TODO: Add error checking?
        }

        /*
        @brief

            Destructor, cleans everything up
        */
        ~window_t()
        {
            if (hwnd == NULL) return;
            
            DestroyWindow(hwnd);

            UnregisterClassA(class_name.c_str(), instance);
        }

        /*
        @brief

            Create our window with the settings given
        */
        bool create_window(bool show = false)
        {
            // Check if the window already exists
            if (hwnd != NULL) return true;

            // Calculate the client size
            RECT rect{};

            // The window style we want to use
            DWORD window_style = (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME) ^ WS_MAXIMIZEBOX;

            if (!fullscreen)
            {
                rect.bottom = size.height;
                rect.right = size.width;

                if (!AdjustWindowRect(&rect, window_style, FALSE)) return false;
            }
            else
            {
                rect.right = size.width;
                rect.bottom = size.height;
            }

            // If not, create it!
            if ((hwnd = CreateWindowExA(
                0,                                                          // Optional window styles.
                class_name.c_str(),                                         // Window class
                title.c_str(),                                              // Window text
                window_style,                                               // Window style

                // Position
                CW_USEDEFAULT, CW_USEDEFAULT,

                // Size
                rect.right - rect.left, rect.bottom - rect.top,

                NULL,       // Parent window
                NULL,       // Menu
                instance,   // Instance handle
                NULL        // Additional application data
            )) == NULL) // Checks if CreateWindowExA succeeds or not
            {
                UnregisterClassA(class_name.c_str(), instance);

                return false;
            }

            if (show) this->show();

            return true;
        }

        /*
        @brief

            Check if the window is active (in the foreground)
        */
        bool is_in_foreground() const
        {
            return (GetForegroundWindow() == hwnd);
        }

        /*
        @brief

            Gets the raw window handle
        */
        HWND get_handle(void) const { return hwnd; }

        /*
        @brief
            
            Shows the window
        */
        void show(void)
        {
            ShowWindow(hwnd, SW_SHOWDEFAULT);

            UpdateWindow(hwnd);

            window_shown = true;
        }

        /*
        @brief

            Gets the client size of the window
        */
        const area_size_t& get_size(void) const { return size; }

        /*
        @brief

            Tells the caller if the window is fullscreen or not
        */
        const bool& is_fullscreen(void) const { return fullscreen; }

    };

}
#endif