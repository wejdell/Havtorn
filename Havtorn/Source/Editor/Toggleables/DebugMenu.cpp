// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "DebugMenu.h"
#include "EditorManager.h"
#include "Windows/RenderDebugWindow.h"

#include <Graphics/RenderManager.h>

#include <GUI.h>

namespace Havtorn
{
	CDebugMenu::CDebugMenu(const char* displayName, CEditorManager* manager)
		: CToggleable(displayName, manager)
	{
	}

	void CDebugMenu::OnEnable()
	{
	}

	void CDebugMenu::OnInspectorGUI()
	{
		if (GUI::Button(Name()))
			GUI::OpenPopup(PopupName);

		if (GUI::BeginPopup(PopupName))
		{
			if (GUI::MenuItem("Debug Rendering"))
			{
				Manager->GetEditorWindow<CRenderDebugWindow>()->SetEnabled(true);
			}

			if (GUI::MenuItem("Log Live Render Objects (Output)"))
			{
				Manager->GetRenderManager()->DebugLiveRenderObjects();
			}

			GUI::EndPopup();
		}
	}

	void CDebugMenu::OnDisable()
	{
	}
}