/*
@file

    macros.h

@purpose

    Defines we need across multiple files. Kind of like preprocessor macros, except we need to include this file
    everywhere we need those defines.
*/

#pragma once

// Version of this program
#define CPP_RETRO_GAMES_VERSION "1.0"

// If defined and the settings system encounters an error, an error
// will be spewed instead of just exiting
#define SETTINGS_ENABLE_DEBUGGING

#ifdef PLATFORM_NS
// If defined, software rendering will be used for ImGui (ONLY for the NS) instead of
// OpenGL. OpenGL doesn't seem to work on the Yuzu emulator for some reason,
// hence why this exists. Testing can be done on Yuzu via software rendering,
// and the final product will have this commented (so it uses OpenGL).
//#define NS_IMGUI_SOFTWARE_RENDERING

// For debugging purposes, nxlink can be enabled
#define NS_ENABLE_NXLINK

// Define the desired framebuffer resolution (here we set it to 720p).
#define FB_WIDTH  1280
#define FB_HEIGHT 720
#endif