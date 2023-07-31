// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorToggleable.h"

namespace ImGui
{
	class CWindowMenu : public CToggleable
	{
	public:
		CWindowMenu(const char* displayName, Havtorn::CEditorManager* manager);
		~CWindowMenu() override = default;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		const char* PopupName = "WindowMenuPopup";
	};
}
