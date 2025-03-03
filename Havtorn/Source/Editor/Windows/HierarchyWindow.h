// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <GUI.h>

namespace Havtorn
{
	class CHierarchyWindow : public CWindow
	{
	public:
		CHierarchyWindow(const char* displayName, CEditorManager* manager);
		~CHierarchyWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		SGuiTextFilter Filter = SGuiTextFilter();
		I32 SelectedIndex = 0;
	};
}
