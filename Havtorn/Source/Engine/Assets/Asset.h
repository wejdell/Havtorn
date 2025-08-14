// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	struct SAsset
	{
		EAssetType Type = EAssetType::None;
		std::string AssetPath = "";
		std::string SourcePath = "";
		U64 UID = 0;

		const bool IsValid() const { return UID != 0 && Type != EAssetType::None && AssetPath != ""; }
	};
}
