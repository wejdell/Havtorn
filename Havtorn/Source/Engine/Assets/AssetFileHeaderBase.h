// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	struct SAssetFileHeaderBase
	{
		EAssetType AssetType = EAssetType::None;
		U16 Version = 0;
	};
}
