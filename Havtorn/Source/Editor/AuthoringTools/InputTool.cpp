// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "InputTool.h"
#include "EditorManager.h"

namespace Havtorn
{
	CInputTool::CInputTool(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager, false)
	{
	}

	void CInputTool::OnEnable()
	{
	}

	void CInputTool::OnInspectorGUI()
	{
		if (!GUI::Begin(Name(), &IsEnabled))
		{
			GUI::End();
			return;
		}

		EGUITableFlags tableFlags = EGUITableFlags::Resizable | EGUITableFlags::Borders;
		if (GUI::BeginTable("InputColumns", 2, STATIC_I32(tableFlags)))
		{
			GUI::TableNextColumn();
			GUI::Selectable("Actions");
			const I32 actioncount = 5;
			std::vector<const char*> names = { "Jump", "Crouch", "Move", "Pause", "Interact" };
			static I32 selectedIndex = -1;

			for (U32 i = 0; i < actioncount; i++)
			{
				if (GUI::TreeNode(names[i]))
				{
					selectedIndex = i;
					GUI::SameLine();
					if (GUI::Button("Add"))
					{
						
					}
					GUI::Text("<No Binding>");
					GUI::TreePop();
				}
			}

			GUI::TableNextColumn();
			GUI::Selectable("Properties");

			GUI::EndTable();
		}
		GUI::End();
	}

	void CInputTool::OnDisable()
	{
	}

	void CInputTool::OpenInputAsset(SEditorAssetRepresentation* asset)
	{
		asset;
		IsEnabled = true;
	}
}
