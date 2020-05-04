# cpp-retro-games
Retro games made by Steven Kelly in C++. Supports Windows, Linux and the Nintendo Switch.

# Games
* Snake

# Compiling
## Windows
* MinGW and DirectX SDK required

## Linux
* GLFW and an OpenGL Loader required

## Nintendo Switch
* devkitpro is required with installed Switch support. Also needs EGL, glad and gladpi libraries installed.

# Notes
* This is developed in Visual Studio Code. I've included my own .vscode directory, which may not work for you.
* To change the target platform to build and test, change PLATFORM_X to PLATFORM_WINDOWS/LINUX/NS in .vscode/c_cpp_properties.json.