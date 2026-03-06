// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EditorWindow.h"
#include "EditorManager.h"

namespace Havtorn
{
	CWindow::CWindow(const char* displayName, CEditorManager* manager, bool isEnabled)
		: Manager(manager)
		, IsEnabled(isEnabled)
		, DisplayName(displayName)
	{
	}

	void CWindow::UpdateState()
	{
		IsFocused = IsEnabled ? IsFocused : false;

		if (!WasEnabled && IsEnabled)
			OnEnable();
		if (WasEnabled && !IsEnabled)
			OnDisable();

		WasEnabled = IsEnabled;
	}
}
