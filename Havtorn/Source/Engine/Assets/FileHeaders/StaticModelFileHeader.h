// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Assets/SourceAssetData.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	struct SStaticModelFileHeader
	{
		EAssetType AssetType = EAssetType::StaticMesh;
		std::string Name;
		// TODO.NW: Remove all UIDs, should not be needed anymore
		U32 UID = 0;
		SSourceAssetData SourceData;
		U8 NumberOfMaterials = 0;
		U32 NumberOfMeshes = 0;
		std::vector<SStaticMesh> Meshes;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SStaticModelFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += GetDataSize(UID);
		size += GetDataSize(SourceData);
		size += GetDataSize(NumberOfMaterials);
		size += GetDataSize(NumberOfMeshes);

		for (auto& mesh : Meshes)
		{
			size += GetDataSize(mesh.Name);
			size += GetDataSize(mesh.Vertices);
			size += GetDataSize(mesh.Indices);
			size += GetDataSize(mesh.MaterialIndex);
		}
		return size;
	}

	inline void SStaticModelFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);
		SerializeData(SourceData, toData, pointerPosition);
		SerializeData(NumberOfMaterials, toData, pointerPosition);
		SerializeData(NumberOfMeshes, toData, pointerPosition);

		for (auto& mesh : Meshes)
		{
			SerializeData(mesh.Name, toData, pointerPosition);
			SerializeData(mesh.Vertices, toData, pointerPosition);
			SerializeData(mesh.Indices, toData, pointerPosition);
			SerializeData(mesh.MaterialIndex, toData, pointerPosition);
		}
	}

	inline void SStaticModelFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(UID, fromData, pointerPosition);
		DeserializeData(SourceData, fromData, pointerPosition);
		DeserializeData(NumberOfMaterials, fromData, pointerPosition);
		DeserializeData(NumberOfMeshes, fromData, pointerPosition);

		Meshes.reserve(NumberOfMeshes);
		for (U16 i = 0; i < NumberOfMeshes; i++)
		{
			Meshes.emplace_back();
			DeserializeData(Meshes.back().Name, fromData, pointerPosition);
			DeserializeData(Meshes.back().Vertices, fromData, pointerPosition);
			DeserializeData(Meshes.back().Indices, fromData, pointerPosition);
			DeserializeData(Meshes.back().MaterialIndex, fromData, pointerPosition);
		}
	}
}