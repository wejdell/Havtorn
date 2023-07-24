// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "HelpMenu.h"
#include "EditorManager.h"

#include <imgui.h>

namespace ImGui
{
	CHelpMenu::CHelpMenu(const char* aName, Havtorn::CEditorManager* manager)
		: CToggleable(aName, manager)
	{
	}

	CHelpMenu::~CHelpMenu()
	{
	}

	void CHelpMenu::OnEnable()
	{
	}

	void CHelpMenu::OnInspectorGUI()
	{
		if (ImGui::Button(Name()))
		{

		}
	}

	void CHelpMenu::OnDisable()
	{
	}
}
