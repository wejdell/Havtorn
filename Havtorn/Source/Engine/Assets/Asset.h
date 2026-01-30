// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include <set>
#include <variant>
#include "Assets/RuntimeAssetInclude.h"
#include "Assets/AssetReference.h"
#include "HexRune/HexRune.h"

namespace Havtorn
{
	typedef std::variant<std::monostate, SStaticMeshAsset, SSkeletalMeshAsset, SSkeletalAnimationAsset, STextureAsset, STextureCubeAsset, SGraphicsMaterialAsset, SSpriteAninmationClipAsset, HexRune::SScript*> SAssetData;
	struct SAsset
	{
		EAssetType Type = EAssetType::None;
		SAssetReference Reference;
		SSourceAssetData SourceData;

		std::set<U64> Requesters = {};
		SAssetData Data = std::monostate();

		const bool IsValid() const { return Reference.IsValid() && Type != EAssetType::None; }
	};
}