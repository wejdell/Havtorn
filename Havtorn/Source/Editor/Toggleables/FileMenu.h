// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorToggleable.h"

namespace ImGui
{
	class CFileMenu : public CToggleable
	{
	public:
		CFileMenu(const char* displayName, Havtorn::CEditorManager* manager);
		~CFileMenu() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		const char* PopupName = "FileMenuPopup";
	};
}