// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorToggleable.h"

namespace Havtorn
{
	class CViewMenu final : public CToggleable
	{
	public:
		CViewMenu(const char* displayName, CEditorManager* manager);
		~CViewMenu() override = default;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		const char* PopupName = "ViewMenuPopup";
	};
}
