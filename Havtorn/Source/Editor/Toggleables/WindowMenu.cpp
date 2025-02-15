// Copyright 2022 Team Havtorn. All Rights Reserved.

//#include "hvpch.h"
#include "WindowMenu.h"
#include "EditorManager.h"
#include "Core/MathTypes/EngineMath.h"

//#include <imgui.h>

namespace ImGui
{
	CWindowMenu::CWindowMenu(const char* displayName, Havtorn::CEditorManager* manager)
		: CToggleable(displayName, manager)
	{
	}

	void CWindowMenu::OnEnable()
	{
	}

	void CWindowMenu::OnInspectorGUI()
	{
		if (ImGui::Button(Name()))
			ImGui::OpenPopup(PopupName);

		if (ImGui::BeginPopup(PopupName))
		{
			Havtorn::F32 viewportPadding = Manager->GetViewportPadding();
			if (ImGui::DragFloat("Viewport Padding", &viewportPadding, 0.01f, 0.0f, 0.5f))
			{
				Manager->SetViewportPadding(viewportPadding);
			}

			ImGui::EndPopup();
		}
	}

	void CWindowMenu::OnDisable()
	{
	}
}
