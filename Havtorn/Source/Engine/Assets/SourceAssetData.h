// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include <HavtornString.h>

namespace Havtorn 
{
	struct SSourceAssetData
	{
		EAssetType AssetType = EAssetType::None;
		CHavtornStaticString<128> SourcePath;
		CHavtornStaticString<128> AssetDependencyPath;
		F32 ImportScale = 1.0f;

		const bool IsValid() const { return SourcePath.Length() != 0; }
	};
}