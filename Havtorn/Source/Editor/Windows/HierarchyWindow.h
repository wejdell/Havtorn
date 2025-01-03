// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

namespace ImGui
{
	class CHierarchyWindow : public CWindow
	{
	public:
		CHierarchyWindow(const char* displayName, Havtorn::CEditorManager* manager);
		~CHierarchyWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		ImGuiTextFilter Filter = ImGuiTextFilter();
		Havtorn::I32 SelectedIndex = 0;
	};
}
