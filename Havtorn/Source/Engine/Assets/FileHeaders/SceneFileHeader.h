// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/AssetFileHeaderBase.h"
#include "Scene/Scene.h"

namespace Havtorn
{
	struct SSceneFileHeader
	{
		SAssetFileHeaderBase HeaderBase = { .AssetType = EAssetType::Scene, .Version = 1 };
		std::string Name = "";
		CScene* Scene = nullptr;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition, CScene* outScene);
	};

	inline U32 SSceneFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(HeaderBase);
		size += GetDataSize(Name);
		size += Scene->GetSize();

		return size;
	}

	inline void SSceneFileHeader::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(HeaderBase, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		Scene->Serialize(toData, pointerPosition);
	}

	inline void SSceneFileHeader::Deserialize(const char* fromData, U64& pointerPosition, CScene* outScene)
	{
		DeserializeData(HeaderBase, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		outScene->Deserialize(fromData, pointerPosition);
	}
}
