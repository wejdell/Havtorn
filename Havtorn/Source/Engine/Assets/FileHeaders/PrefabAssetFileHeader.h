// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	class CScene;

	struct SPrefabFileHeader
	{
		EAssetType AssetType = EAssetType::Prefab;
		CScene* Scene = nullptr;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData, CScene* outScene);
	};
}
