// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "ViewMenu.h"
#include "EditorManager.h"

#include <imgui.h>

namespace ImGui
{
	CViewMenu::CViewMenu(const char* displayName, Havtorn::CEditorManager* manager)
		: CToggleable(displayName, manager)
	{
	}

	void CViewMenu::OnEnable()
	{
	}

	void CViewMenu::OnInspectorGUI()
	{
		if (ImGui::Button(Name()))
			ImGui::OpenPopup(PopupName);

		if (ImGui::BeginPopup(PopupName))
		{
			if(ImGui::MenuItem("Debug info"))
			{
				Manager->ToggleDebugInfo();
			}

			if(ImGui::MenuItem("Dear Imgui Demo"))
			{
				Manager->ToggleDemo();
			}

			ImGui::EndPopup();
		}
	}

	void CViewMenu::OnDisable()
	{
	}
}