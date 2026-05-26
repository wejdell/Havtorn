// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/AssetFileHeaderBase.h"
#include "Engine.h"
#include "Scene/Scene.h"
#include "Scene/World.h"

namespace Havtorn
{
	class CScene;

	struct SPrefabFileHeader
	{
		SAssetFileHeaderBase HeaderBase = { .AssetType = EAssetType::Prefab, .Version = 1 };
		std::string Name = "";
		CScene* Scene = nullptr;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData, CScene* outScene);
	};

	inline [[nodiscard]] U32 SPrefabFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(HeaderBase);
		size += GetDataSize(Name);
		size += Scene->GetSize();
		return size;
	}

	inline void SPrefabFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(HeaderBase, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		Scene->Serialize(toData, pointerPosition);
	}

	inline void SPrefabFileHeader::Deserialize(const char* fromData, CScene* outScene)
	{
		U64 pointerPosition = 0;
		DeserializeData(HeaderBase, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		outScene->Deserialize(fromData, pointerPosition);
		outScene->SceneName = Name;
	}
}
