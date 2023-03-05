// Copyright 2022 Team Havtorn. All Rights Reserved.
#pragma once

#include <Core/imgui.h>
#include <Core/ImGuizmo/ImGuizmo.h>
#include "ImGuiManager.h"

#ifndef CROSS_PROJECT_IMGUI_SETUP
#define CROSS_PROJECT_IMGUI_SETUP()														    \
		{																					\
			ImGuiMemAllocFunc memAlloc;														\
			ImGuiMemFreeFunc memFree;														\
			void* userData = nullptr;														\
			Havtorn::GImGuiManager::GetAllocatorFunctions(&memAlloc, &memFree, &userData);	\
			ImGui::SetAllocatorFunctions(memAlloc, memFree, userData);						\
			ImGui::SetCurrentContext(Havtorn::GImGuiManager::GetContext());					\
			ImGuizmo::SetImGuiContext(Havtorn::GImGuiManager::GetContext());				\
		}						
#endif // !CROSS_PROJECT_IMGUI_SETUP
