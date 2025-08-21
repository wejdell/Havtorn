// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/RuntimeAssetDeclarations.h"

#include <set>

namespace Havtorn
{
	struct SSourceAssetData
	{
		EAssetType AssetType = EAssetType::None;
		std::string SourcePath = "";

		std::string AssetDependencyPath = "";
		F32 ImportScale = 1.0f;

		const bool IsValid() const { return SourcePath != ""; }
	};

	struct SAsset
	{
		EAssetType Type = EAssetType::None;
		U64 UID = 0;
		std::string AssetPath = "";
		SSourceAssetData SourceData;

		std::set<U64> References = {};
		SAssetData Data = std::monostate();

		const bool IsValid() const { return UID != 0 && Type != EAssetType::None && AssetPath != ""; }
	};
}
