// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Assets/SourceAssetData.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	struct SSkeletalModelFileHeader
	{
		EAssetType AssetType = EAssetType::SkeletalMesh;
		std::string Name;
		U32 UID = 0;
		SSourceAssetData SourceData;
		U8 NumberOfMaterials = 0;
		U32 NumberOfMeshes = 0;
		std::vector<SSkeletalMesh> Meshes;
		std::vector<SSkeletalMeshBone> BindPoseBones;
		U32 NumberOfNodes = 0;
		std::vector<SSkeletalMeshNode> Nodes;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSkeletalModelFileHeader::GetSize() const
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

		size += GetDataSize(BindPoseBones);

		size += GetDataSize(NumberOfNodes);
		for (auto& node : Nodes)
		{
			size += GetDataSize(node.Name);
			size += GetDataSize(node.NodeTransform);
			size += GetDataSize(node.ChildIndices);
		}

		return size;
	}

	inline void SSkeletalModelFileHeader::Serialize(char* toData) const
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

		SerializeData(BindPoseBones, toData, pointerPosition);

		SerializeData(NumberOfNodes, toData, pointerPosition);
		for (auto& node : Nodes)
		{
			SerializeData(node.Name, toData, pointerPosition);
			SerializeData(node.NodeTransform, toData, pointerPosition);
			SerializeData(node.ChildIndices, toData, pointerPosition);
		}
	}

	inline void SSkeletalModelFileHeader::Deserialize(const char* fromData)
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

		DeserializeData(BindPoseBones, fromData, pointerPosition);

		DeserializeData(NumberOfNodes, fromData, pointerPosition);
		Nodes.reserve(NumberOfNodes);
		for (U16 i = 0; i < NumberOfNodes; i++)
		{
			Nodes.emplace_back();
			DeserializeData(Nodes.back().Name, fromData, pointerPosition);
			DeserializeData(Nodes.back().NodeTransform, fromData, pointerPosition);
			DeserializeData(Nodes.back().ChildIndices, fromData, pointerPosition);
		}
	}
}