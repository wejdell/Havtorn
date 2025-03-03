// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EditMenu.h"
#include "EditorManager.h"

#include <GUI.h>

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
                    ImVec2 p = GUI::GetCursorScreenPos();
                    auto previewColor = Manager->GetEditorColorThemeRepColor(colorTheme);
                    GUI::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), GUI::ColorConvertFloat4ToU32(previewColor));
                    GUI::Dummy(ImVec2(sz, sz));
                    GUI::SameLine();
                    if (GUI::MenuItem(name.c_str()))
                    {
                        Manager->SetEditorTheme(static_cast<EEditorColorTheme>(i));
                    }
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