# cpp-retro-games
Retro games made by Steven Kelly in C++. Supports Windows, Linux and the Nintendo Switch.

# Games
* Snake
* Pingpong
* More games soon™

# Compiling
## Windows
* MinGW and DirectX SDK required

## Linux
* GLFW and an OpenGL Loader required

## Nintendo Switch
* ONLY works on Switches with custom firmware installed
* devkitpro is required with installed Switch support. Also needs EGL, glad and gladpi libraries installed

# Notes
* This is developed in Visual Studio Code. I've included my own .vscode directory, which may not work for you
* To change the target platform to build and test, change PLATFORM_X to PLATFORM_WINDOWS/LINUX/NS in .vscode/c_cpp_properties.json
* Only 16:9 resolutions are supported at the moment. I'll look into changing that in the future.

# Credits
* ocornut - [Dear ImGui](https://github.com/ocornut/imgui "Link to library")
* devkitpro/libnx - [compiler toolchain/SDK for the Switch](https://devkitpro.org/wiki/Getting_Started "Link to website")
* [RetroArch](https://www.retroarch.com/ "Link to website") - Design inspiration
* nlohmann - [JSON library](https://github.com/nlohmann/json "Link to library")
* Microsoft - DirectX SDK (Windows backend) + Win32API
* GLFW - Linux backend
* Probably various other things I forgot - feel free to contact me though

# Images
<img src="img/game-options.png">
<img src="img/snake.png">