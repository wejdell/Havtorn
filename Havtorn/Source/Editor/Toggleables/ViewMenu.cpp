// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "ViewMenu.h"
#include "EditorManager.h"

#include <GUI.h>

namespace Havtorn
{
	CViewMenu::CViewMenu(const char* displayName, CEditorManager* manager)
		: CToggleable(displayName, manager)
	{
	}

	void CViewMenu::OnEnable()
	{
	}

	void CViewMenu::OnInspectorGUI()
	{
		if (GUI::Button(Name()))
			GUI::OpenPopup(PopupName);

		if (GUI::BeginPopup(PopupName))
		{
			if(GUI::MenuItem("Debug info"))
			{
				Manager->ToggleDebugInfo();
			}

			if(GUI::MenuItem("Dear Imgui Demo"))
			{
				Manager->ToggleDemo();
			}

			GUI::EndPopup();
		}
	}

	void CViewMenu::OnDisable()
	{
	}
}