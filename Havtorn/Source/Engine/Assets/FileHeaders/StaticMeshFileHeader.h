// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/AssetFileHeaderBase.h"
#include "Assets/SourceAssetData.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	struct SStaticMeshFileHeader
	{
		SAssetFileHeaderBase HeaderBase = { .AssetType = EAssetType::StaticMesh, .Version = 1 };
		SSourceAssetData SourceData;
		std::string Name = "";
		U8 NumberOfMaterials = 0;
		std::vector<SStaticMesh> Meshes;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SStaticMeshFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(HeaderBase);
		size += SourceData.GetSize();
		size += GetDataSize(Name);
		size += GetDataSize(NumberOfMaterials);

		size += GetDataSize(U32());
		for (auto& mesh : Meshes)
		{
			size += GetDataSize(mesh.Name);
			size += GetDataSize(mesh.Vertices);
			size += GetDataSize(mesh.Indices);
			size += GetDataSize(mesh.MaterialIndex);
		}
		return size;
	}

	inline void SStaticMeshFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(HeaderBase, toData, pointerPosition);
		SourceData.Serialize(toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(NumberOfMaterials, toData, pointerPosition);

		SerializeData(STATIC_U32(Meshes.size()), toData, pointerPosition);
		for (auto& mesh : Meshes)
		{
			SerializeData(mesh.Name, toData, pointerPosition);
			SerializeData(mesh.Vertices, toData, pointerPosition);
			SerializeData(mesh.Indices, toData, pointerPosition);
			SerializeData(mesh.MaterialIndex, toData, pointerPosition);
		}
	}

	inline void SStaticMeshFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(HeaderBase, fromData, pointerPosition);
		SourceData.Deserialize(fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(NumberOfMaterials, fromData, pointerPosition);

		U32 numberOfMeshes = 0;
		DeserializeData(numberOfMeshes, fromData, pointerPosition);
		Meshes.reserve(numberOfMeshes);
		for (U16 i = 0; i < numberOfMeshes; i++)
		{
			Meshes.emplace_back();
			DeserializeData(Meshes.back().Name, fromData, pointerPosition);
			DeserializeData(Meshes.back().Vertices, fromData, pointerPosition);
			DeserializeData(Meshes.back().Indices, fromData, pointerPosition);
			DeserializeData(Meshes.back().MaterialIndex, fromData, pointerPosition);
		}
	}
}
