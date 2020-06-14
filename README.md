# cpp-retro-games
Retro games made by Steven Kelly in C++. Supports Windows, Linux, Emscripten (web) and the Nintendo Switch.

# Web version
* The web version is available at https://skelly-2-1.github.io/cpp-retro-games/cpp-retro-games.html for you to try out
* Otherwise, binaries are available in the release section

# Games
* Snake
* Pingpong
* More games soon™

# Compiling
## Windows
* MinGW, DirectX SDK and SFML required

## Linux
* GLFW, SFML and an OpenGL Loader required

## Nintendo Switch
* ONLY works on Switches with custom firmware installed
* devkitpro is required with installed Switch support. Also needs SDL2, EGL, glad and gladpi libraries installed

## Emscripten
* Needs the emscripten SDK installed
* Building via Visual Studio not fully supported yet, use the command line (make emscripten)
* Touchscreen not supported (yet), only keyboard + mouse

# Notes
* This is developed in Visual Studio Code. I've included my own .vscode directory, which may not work for you
* To change the target platform to build and test, change PLATFORM_X to PLATFORM_WINDOWS/LINUX/NS in .vscode/c_cpp_properties.json
* Only 16:9 resolutions are supported at the moment. I'll look into changing that in the future.
* The emscripten (web) version of this is available at https://skelly-2-1.github.io/cpp-retro-games/cpp-retro-games.html

# Credits
* ocornut - [Dear ImGui](https://github.com/ocornut/imgui "Link to library")
* devkitpro/libnx - [compiler toolchain/SDK for the Switch](https://devkitpro.org/wiki/Getting_Started "Link to website")
* [RetroArch](https://www.retroarch.com/ "Link to website") - Design inspiration
* nlohmann - [JSON library](https://github.com/nlohmann/json "Link to library")
* chelle19 - [Ding sound (freesound.org)](https://freesound.org/people/chelle19/sounds/320201/ "Link to sound")
* crisstanza - [Pause sound (freesound.org)](https://freesound.org/people/crisstanza/sounds/167127/ "Link to sound")
* InspectorJ - [Chewing, Breadstick, Single, E.wav (freesound.org)](https://freesound.org/people/InspectorJ/sounds/429598/ "Link to sound")
* Microsoft - DirectX SDK (Windows backend) + Win32API
* GLFW - Linux backend
* SFML - Sound library for Windows and Linux
* SDL2 - SDL2 - Sound library for the Switch and Emscripten
* emscripten - made this possible on the web
* Probably various other things I forgot - feel free to contact me though

# Images
<img src="img/game-options.png">
<img src="img/snake.png">