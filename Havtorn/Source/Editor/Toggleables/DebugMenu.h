// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorToggleable.h"

namespace Havtorn
{
	class CDebugMenu final : public CToggleable
	{
	public:
		CDebugMenu(const char* displayName, CEditorManager* manager);
		~CDebugMenu() override = default;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
		const char* PopupName = "DebugMenuPopup";
	};
}
