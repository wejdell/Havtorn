// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EditorWindow.h"
#include "EditorManager.h"

namespace Havtorn
{
	CWindow::CWindow(const char* displayName, CEditorManager* manager, bool isEnabled)
		: DisplayName(displayName)
		, Manager(manager)
		, IsEnabled(isEnabled)
	{
	}
}
