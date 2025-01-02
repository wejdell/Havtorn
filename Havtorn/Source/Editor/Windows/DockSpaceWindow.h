// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

namespace ImGui
{
	class CDockSpaceWindow : public CWindow
	{
	public:
		CDockSpaceWindow(const char* displayName, Havtorn::CEditorManager* manager);
		~CDockSpaceWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;
	};
}
