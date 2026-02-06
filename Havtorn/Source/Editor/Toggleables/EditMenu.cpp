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
            if (GUI::BeginMenu("Editor Themes"))
            {
                F32 sz = GUI::GetTextLineHeight();
                for (U16 i = 0; i < static_cast<U16>(EEditorColorTheme::Count); i++)
                {
                    auto colorTheme = static_cast<EEditorColorTheme>(i);
                    std::string name = Manager->GetEditorColorThemeName(colorTheme).c_str();
                    SVector2<F32> cursorPos = GUI::GetCursorScreenPos();
                    SColor previewColor = Manager->GetEditorColorThemeRepColor(colorTheme);
                    GUI::AddRectFilled(cursorPos, SVector2<F32>(sz), previewColor);
					GUI::Dummy({ sz, sz });
                    GUI::SameLine();
                    if (GUI::MenuItem(name.c_str()))
                    {
                        Manager->SetEditorTheme(static_cast<EEditorColorTheme>(i));
                    }
                }
                GUI::EndMenu();
            }
        	
        	if (GUI::BeginMenu("Editor Preferences"))
            {
            	F32 sensitivity = Manager->GetEditorSensitivity();
            	if (GUI::DragFloat("Editor Sensitivity", sensitivity, 0.01f, 0.01f, 5.0f))
            	{
            		Manager->SetEditorSensitivity(sensitivity);
            	}
            	
            	GUI::EndMenu();
            }

			GUI::EndPopup();
		}
	}

	void CEditMenu::OnDisable()
	{
	}
}