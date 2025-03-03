// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

namespace Havtorn
{
	class CDockSpaceWindow : public CWindow
	{
	public:
		CDockSpaceWindow(const char* displayName, CEditorManager* manager);
		~CDockSpaceWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;
	};
}
