// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorToggleable.h"

namespace Havtorn
{
	class CHelpMenu : public CToggleable
	{
	public:
		CHelpMenu(const char* displayName, CEditorManager* manager);
		~CHelpMenu() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		const char* PopupName = "HelpMenuPopup";
	};
}
