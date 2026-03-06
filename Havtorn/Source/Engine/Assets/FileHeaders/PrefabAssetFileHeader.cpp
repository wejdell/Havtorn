// Copyright 2026 Team Havtorn. All Rights Reserved.

//#pragma once

#include "PrefabAssetFileHeader.h"

#include "Engine.h"
#include "Scene/Scene.h"
#include "Scene/World.h"

namespace Havtorn
{
	[[nodiscard]] U32 SPrefabFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += Scene->GetSize();
		return size;
	}
	
	void SPrefabFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		Scene->Serialize(toData, pointerPosition);
	}

	void SPrefabFileHeader::Deserialize(const char* fromData, CScene* outScene)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		outScene->Deserialize(fromData, pointerPosition);
	}
}
