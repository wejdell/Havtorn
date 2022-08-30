// Copyright 2022 Team Havtorn. All Rights Reserved.
#pragma once

#include <Core/imgui.h>
#include "WindowHandler.h"

#ifndef CROSS_DLL_IMGUI_SETUP
#define CROSS_DLL_IMGUI_SETUP(windowHandler)												\
		{																					\
			ImGuiMemAllocFunc memAlloc;														\
			ImGuiMemFreeFunc memFree;														\
			void* userData = nullptr;														\
			windowHandler->GetImGuiAllocatorFunctions(&memAlloc, &memFree, &userData);		\
			ImGui::SetAllocatorFunctions(memAlloc, memFree, userData);						\
			ImGui::SetCurrentContext(windowHandler->GetImGuiContext());						\
		}						
#endif // !CROSS_DLL_IMGUI_SETUP
