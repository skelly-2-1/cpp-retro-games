# Makefile for cpp-retro-games

# Note: In order to compile for Windows, you need MinGW installed.

# Note 2: Compiling with Windows 10, using xinput may lead to the application not working on Windows 7 and below.
# If you want to disable xinput (and thus, joystick input), remove -lxinput to stop linking xinput
# and uncomment IMGUI_IMPL_WIN32_DISABLE_GAMEPAD in src/imgui/imgui_impl_win32.h.

# source directory
SRC_DIR := src

# object directory (subdirectories for Windows, Linux and NS will be created here)
OBJ_DIR := obj

# build directory (subdirectories for Windows, Linux and NS will be created here)
BUILD_DIR := bin

# compiler to use
CXX := g++

# NM to use
NM := gcc-nm

# output name (root folder name by default)
OUTPUT_NAME := $(notdir $(CURDIR))

# set our compiler flags for .cpp files
# note: you may want to include -Wall
CXXFLAGS := -g

# linker flags
LDFLAGS := 

# libraries
LDLIBS := 

# library directories
LDPATHS := 

# includes
INCLUDES := -I"$(CURDIR)/$(SRC_DIR)"

# error filter (see %DEVKITPRO%/libnx/switch_rules)
ERROR_FILTER := 

# check operating system
ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell uname 2>/dev/null || echo Unknown)
    detected_OS := $(patsubst CYGWIN%,Cygwin,$(detected_OS))
    detected_OS := $(patsubst MSYS%,MSYS,$(detected_OS))
    detected_OS := $(patsubst MINGW%,MSYS,$(detected_OS))
endif

# haven't tested other OS'es yet, so I'll just spew an error
ifneq ($(detected_OS),$(filter $(detected_OS),Windows Linux))
    $(error Unsupported operating system)
endif

# helper to turn text lowercase
lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

# grab the platform name
PLATFORM_NAME := $(call lc,$(detected_OS))

# check if we're targeting the Nintendo Switch
ifneq "$(findstring ns, $(MAKECMDGOALS))" ""
    # check if DEVKITPRO is installed (and in PATH)
    ifeq ($(strip $(DEVKITPRO)),)
		$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
	endif

	# change platform name (we don't want to compile for Windows or Linux anymore)
	# since we ran 'make ns'
    PLATFORM_NAME := ns

	# tell the compiler which platform we're on
    CXXFLAGS += -DPLATFORM_NS

	# using other compiler for NS
    CXX := aarch64-none-elf-$(CXX)

	# this too
    NM := aarch64-none-elf-$(NM)

	# flags we need for the compilation
    ARCH	:=	-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE
    CXXFLAGS += $(ARCH) -ffunction-sections -D__SWITCH__ -fno-rtti -fno-exceptions
    LDFLAGS += -specs=$(DEVKITPRO)/libnx/switch.specs -g $(ARCH) -Wl,-Map,$(OBJ_DIR)/$(PLATFORM_NAME)/$(OUTPUT_NAME).map

	# add the library directories of libnx
    LDPATHS += -L"$(DEVKITPRO)/portlibs/switch/lib" -L"$(DEVKITPRO)/libnx/lib"

	# add devkitpro include directories
    INCLUDES += -I"$(DEVKITPRO)/libnx/include" -I"$(DEVKITPRO)/portlibs/switch/include"

	# libraries needed for the Nintendo Switch
    LDLIBS += -lglad -lEGL -lglapi -ldrm_nouveau -lnx

	# also include the SDL2 library
    LDLIBS += `$(DEVKITPRO)/portlibs/switch/bin/aarch64-none-elf-pkg-config --libs SDL2_mixer`

	# taken from devkitpro
    ERROR_FILTER := 2>&1 | sed -e 's/\(.[a-zA-Z]\+\):\([0-9]\+\):/\1(\2):/g'

    CXXFLAGS += -MMD -MP

	# add the devkitpro tools directory to PATH to be able to use the programs it contains
    DEVKITPATH = $(shell echo "$(DEVKITPRO)" | sed -e 's/^\([a-zA-Z]\):/\/\1/')

    export PATH	:= $(DEVKITPATH)/tools/bin:$(DEVKITPATH)/devkitA64/bin:$(PATH)
else
    # check if we're targeting emscripten
    ifneq "$(findstring emscripten, $(MAKECMDGOALS))" ""
        ##---------------------------------------------------------------------
        ## EMSCRIPTEN OPTIONS
        ##---------------------------------------------------------------------

        EMS += -s USE_SDL=2 -s WASM=1 -s USE_SDL_MIXER=2
        EMS += -s ALLOW_MEMORY_GROWTH=1
        EMS += -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=0
        EMS += -s ASSERTIONS=1

        # Uncomment next line to fix possible rendering bugs with Emscripten version older then 1.39.0 (https://github.com/ocornut/imgui/issues/2877)
        #EMS += -s BINARYEN_TRAP_MODE=clamp
        #EMS += -s SAFE_HEAP=1    ## Adds overhead

        # Emscripten allows preloading a file or folder to be accessible at runtime.
        # The Makefile for this example project suggests embedding the misc/fonts/ folder into our application, it will then be accessible as "/fonts"
        # See documentation for more details: https://emscripten.org/docs/porting/files/packaging_files.html
        # (Default value is 0. Set to 1 to enable file-system and include the misc/fonts/ folder as part of the build.)
        USE_FILE_SYSTEM ?= 1

        ifeq ($(USE_FILE_SYSTEM), 0)
            EMS += -s NO_FILESYSTEM=1 -DIMGUI_DISABLE_FILE_FUNCTIONS
        endif

        ifeq ($(USE_FILE_SYSTEM), 1)
        #    LDFLAGS += --no-heap-copy --preload-file ../../misc/fonts@/fonts
            LDFLAGS += --no-heap-copy -lidbfs.js
            EMS += -DIMGUI_DISABLE_FILE_FUNCTIONS
        endif

        # use the emscripten compiler
        CXX = em++

        # tell the compiler which platform we're on
        CXXFLAGS += -DPLATFORM_EMSCRIPTEN $(EMS) -Wformat -Os

        LDLIBS += $(EMS)
        #LDFLAGS += --shell-file shell_minimal.html

        # run emsdk_env.bat on Windows (requires EMSDK to be set persistently)
        ifeq ($(detected_OS),Windows)
            ifeq ($(strip $(EMSDK)),)
				$(error "Please set EMSDK in your environment. export EMSDK=<path to>/emsdk")
	        endif
		endif

		# change platform name (we don't want to compile for Windows or Linux anymore)
		# since we ran 'make emscripten'
        PLATFORM_NAME := emscripten
    else
        ifeq ($(detected_OS),Windows)
		    # Windows, tell the compiler which platform we're on
            CXXFLAGS += -DPLATFORM_WINDOWS

		    # check if the DirectX SDK is installed
            ifeq ($(strip $(DXSDK_DIR)),)
                $(error "Failed to find a valid DirectX SDK installation. Please set DXSDK_DIR in PATH")
		    endif

		    # include the DirectX SDK
            INCLUDES += -I"$(DXSDK_DIR)Include"

		    # tell the compiler where to include libraries for DirectX
            LDPATHS += -L"$(DXSDK_DIR)Lib\x64"

		    # include directx, gdi32 and xinput (xinput is optional: see note 2 at the top of this file)
            LDLIBS += -ld3d9 -ld3dx9 -lxinput -lgdi32

		    # add .exe to the output name
            OUTPUT_NAME := $(OUTPUT_NAME).exe

		    # stop the console from coming up in windows
            LDFLAGS += -mwindows
	    endif

        ifeq ($(detected_OS),Linux)
            CXXFLAGS += -DPLATFORM_LINUX

		    # include necessary libraries (opengl, glfw, etc.)
            LDLIBS += -lGL -lGLEW -lglfw3 -ldl -lX11 -lpthread
	    endif

	    # include SFML audio library (for both; windows and linux)
        LDLIBS += -lsfml-audio
	endif
endif

# set our target
TARGET := $(BUILD_DIR)/$(PLATFORM_NAME)/$(OUTPUT_NAME)

# helper to recursively find files
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# find all .cpp files in the $(SRC_DIR) directory (including subdirectories)
SRC_FILES := $(call rwildcard,$(SRC_DIR)/,*.cpp)

# grab a list of object files using the source files above
OBJ_FILES := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/$(PLATFORM_NAME)/%,$(SRC_FILES:.cpp=.o))

# final build of windows and linux
$(TARGET): $(OBJ_FILES)
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/$(PLATFORM_NAME)
	@echo building ... $(TARGET)
	@$(CXX) $(LDFLAGS) $(OBJ_FILES) $(LDPATHS) $(LDLIBS) -o $@
	@echo built ... $(TARGET)
	@cp -r dep/$(PLATFORM_NAME)/* $(BUILD_DIR)/$(PLATFORM_NAME)

# final build of emscripten
$(TARGET).html: $(OBJ_FILES)
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/$(PLATFORM_NAME)
	@echo building ... $(TARGET).html
	@$(CXX) $(LDFLAGS) $(OBJ_FILES) $(LDPATHS) $(LDLIBS) -o $@
	@echo built ... $(TARGET).html
	@cp -r dep/$(PLATFORM_NAME)/* $(BUILD_DIR)/$(PLATFORM_NAME)

# object files (ns)
$(OBJ_DIR)/ns/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -MF $(OBJ_DIR)/$(PLATFORM_NAME)/$*.d $(INCLUDES) -c -o $@ $< $(ERROR_FILTER)

# object files (windows)
$(OBJ_DIR)/windows/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $< $(ERROR_FILTER)

# object files (linux)
$(OBJ_DIR)/linux/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $< $(ERROR_FILTER)

# object files (emscripten)
$(OBJ_DIR)/emscripten/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $< $(ERROR_FILTER)

# 'make ns' option will call this, which will build the object
# files like normal and call the target .elf and .nro
.PHONY: ns
.SILENT: ns
ns: $(OBJ_FILES) $(TARGET).elf $(TARGET).nro

# 'make emscripten' option will call this
.PHONY: emscripten
.SILENT: emscripten
emscripten: setup_emscripten $(OBJ_FILES) $(TARGET).html

.SILENT: setup_emscripten
.PHONY: setup_emscripten
setup_emscripten:
	{ \
		set -e ;\
		SRC="$$BASH_SOURCE" ;\
		if [ "$$SRC" = "" ]; then\
			SRC="$$0" ;\
		fi ;\
		CURDIR="$$(pwd)" ;\
		cd "$$(dirname "$$SRC")" ;\
		unset SRC ;\
		tmpfile=`mktemp` || exit 1 ;\
		EMSDK_BASH=1 $(EMSDK)/./emsdk construct_env $$tmpfile ;\
		. $$tmpfile ;\
		rm -f $$tmpfile ;\
    }

# clean: simply remove the whole obj and bin/build directory
.PHONY: clean
.SILENT: clean
clean:
	rm -r -f $(OBJ_DIR)
	rm -r -f $(BUILD_DIR)

# taken from %DEVKITPRO%/libnx/switch_rules and modified a bit
.PHONY: $(TARGET).elf
$(TARGET).elf:
	@echo building ... $(TARGET).elf
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/$(PLATFORM_NAME)
	@$(CXX) $(LDFLAGS) $(OBJ_FILES) $(LDPATHS) $(LDLIBS) -o $@
	@$(NM) -CSn $@ > $(OBJ_DIR)/$(PLATFORM_NAME)/$(OUTPUT_NAME).lst

.PHONY: $(TARGET).nro
$(TARGET).nro:
	@rm -f $(TARGET).nro
	@echo building ... $(TARGET).nro
	@elf2nro $(TARGET).elf $(TARGET).nro
	@rm -f $(TARGET).elf
	@echo built ... $(TARGET).nro