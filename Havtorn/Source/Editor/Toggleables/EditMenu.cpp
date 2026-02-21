// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EditMenu.h"
#include "EditorManager.h"

#include <GUI.h>
#include <Color.h>

namespace Havtorn
{
	CEditMenu::CEditMenu(const char* displayName, CEditorManager* manager)
		: CToggleable(displayName, manager)
	{
	}

	CEditMenu::~CEditMenu()
	{
	}

	void CEditMenu::OnEnable()
	{
	}

	void CEditMenu::OnInspectorGUI()
	{
		if (GUI::Button(Name()))
			GUI::OpenPopup(PopupName);
		
        if (GUI::BeginPopup(PopupName))
		{
        	GUI::Separator();
        	
        	if(GUI::MenuItem("Editor Preferences"))
			{
				Manager->TogglePreferences();
			}

			GUI::EndPopup();
		}
	}

	void CEditMenu::OnDisable()
	{
	}
}