// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EditorToggleable.h"
#include "EditorManager.h"

namespace Havtorn
{
	CToggleable::CToggleable(const char* displayName, Havtorn::CEditorManager* manager)
		: Manager(manager)
		, DisplayName(displayName)
		, IsEnabled(false)
	{
	}
}
