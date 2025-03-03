// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "WindowMenu.h"
#include "EditorManager.h"

#include <GUI.h>

namespace Havtorn
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
		if (GUI::Button(Name()))
			GUI::OpenPopup(PopupName);

		if (GUI::BeginPopup(PopupName))
		{
			Havtorn::F32 viewportPadding = Manager->GetViewportPadding();
			if (GUI::DragFloat("Viewport Padding", &viewportPadding, 0.01f, 0.0f, 0.5f))
			{
				Manager->SetViewportPadding(viewportPadding);
			}

			GUI::EndPopup();
		}
	}

	void CWindowMenu::OnDisable()
	{
	}
}
