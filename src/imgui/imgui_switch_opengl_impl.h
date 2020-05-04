#pragma once

#include "../misc/macros.h"

#ifndef IMGUI_SOFTWARE_RENDERING
#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif

#include "../misc/color.h"

struct ImDrawData;

IMGUI_IMPL_API bool     ImGui_ImplSwitch_Init();
IMGUI_IMPL_API void     ImGui_ImplSwitch_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplSwitch_NewFrame(void);
IMGUI_IMPL_API void     ImGui_ImplSwitch_RenderDrawData(ImDrawData* draw_data);
#endif