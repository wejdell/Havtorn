// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include <HavtornString.h>

namespace Havtorn 
{
	struct SSourceAssetData
	{
		// TODO.NW: This should be more flexible, and excluding data the asset doesn't need.
		// May want to use a variant as the parameter to ImportAsset instead

		EAssetType AssetType = EAssetType::None;
		CHavtornStaticString<128> SourcePath;
		CHavtornStaticString<128> AssetDependencyPath;
		F32 ImportScale = 1.0f;

		const bool IsValid() const { return SourcePath.Length() != 0; }
	};
}
