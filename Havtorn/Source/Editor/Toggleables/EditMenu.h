// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorToggleable.h"

namespace Havtorn
{
	class CEditMenu : public CToggleable
	{
	public:
		CEditMenu(const char* displayName, CEditorManager* manager);
		~CEditMenu() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		const char* PopupName = "EditMenuPopup";
	};
}
