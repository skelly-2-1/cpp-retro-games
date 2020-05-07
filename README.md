# cpp-retro-games
Retro games made by Steven Kelly in C++. Supports Windows, Linux and the Nintendo Switch.

# Games
* Snake
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

# Credits
* ocornut - Dear ImGui
* devkitpro/libnx - compiler toolchain/SDK for the Switch
* RetroArch - Design inspiration
* nlohmann - JSON library
* Microsoft - DirectX SDK (Windows backend) + Win32API
* GLFW - Linux backend
* Probably various other things I forgot - feel free to contact me though

# Images
<img src="img/game-options.png">
<img src="img/snake.png">