// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include <HavtornString.h>

namespace Havtorn
{
	struct SAudioClipSettings
	{
		bool IsSpatialized = true;
		bool IsLooping = false;
	};

	struct SAudioClipSourceData
	{
		SAudioClipSettings Settings;
	};
}
