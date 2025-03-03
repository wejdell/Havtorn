// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "HelpMenu.h"
#include "EditorManager.h"

#include <GUI.h>

namespace Havtorn
{
	CHelpMenu::CHelpMenu(const char* displayName, CEditorManager* manager)
		: CToggleable(displayName, manager)
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
		if (GUI::Button(Name()))
		{

		}
	}

	void CHelpMenu::OnDisable()
	{
	}
}
