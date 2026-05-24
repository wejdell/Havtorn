// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/SourceAssetData.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	struct SSkeletalMeshFileHeader
	{
		EAssetType AssetType = EAssetType::SkeletalMesh;
		U16 Version = 1;
		SSourceAssetData SourceData;
		std::string Name = "";
		U8 NumberOfMaterials = 0;
		std::vector<SSkeletalMesh> Meshes;
		std::vector<SSkeletalMeshBone> BindPoseBones;
		std::vector<SSkeletalMeshNode> Nodes;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSkeletalMeshFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Version);
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

		size += GetDataSize(BindPoseBones);

		size += GetDataSize(U32());
		for (auto& node : Nodes)
		{
			size += GetDataSize(node.Name);
			size += GetDataSize(node.NodeTransform);
			size += GetDataSize(node.ChildIndices);
		}

		return size;
	}

	inline void SSkeletalMeshFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Version, toData, pointerPosition);
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

		SerializeData(BindPoseBones, toData, pointerPosition);

		SerializeData(STATIC_U32(Nodes.size()), toData, pointerPosition);
		for (auto& node : Nodes)
		{
			SerializeData(node.Name, toData, pointerPosition);
			SerializeData(node.NodeTransform, toData, pointerPosition);
			SerializeData(node.ChildIndices, toData, pointerPosition);
		}
	}

	inline void SSkeletalMeshFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Version, fromData, pointerPosition);
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

		DeserializeData(BindPoseBones, fromData, pointerPosition);

		U32 numberOfNodes = 0;
		DeserializeData(numberOfNodes, fromData, pointerPosition);
		Nodes.reserve(numberOfNodes);
		for (U16 i = 0; i < numberOfNodes; i++)
		{
			Nodes.emplace_back();
			DeserializeData(Nodes.back().Name, fromData, pointerPosition);
			DeserializeData(Nodes.back().NodeTransform, fromData, pointerPosition);
			DeserializeData(Nodes.back().ChildIndices, fromData, pointerPosition);
		}
	}
}
