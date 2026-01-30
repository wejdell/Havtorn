// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include "Assets/FileHeaders/SkeletalModelFileHeader.h"

namespace Havtorn 
{
	struct SSkeletalMeshAsset
	{
		SSkeletalMeshAsset() = default;

		explicit SSkeletalMeshAsset(const SSkeletalModelFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Name(assetFileData.Name)
			, NumberOfMaterials(assetFileData.NumberOfMaterials)
			, BindPoseBones(assetFileData.BindPoseBones)
			, Nodes(assetFileData.Nodes)
		{
			for (auto& mesh : assetFileData.Meshes)
			{
				DrawCallData.emplace_back();
				DrawCallData.back().IndexCount = STATIC_U32(mesh.Indices.size());
				DrawCallData.back().MaterialIndex = STATIC_U16(mesh.MaterialIndex);
			}
		}

		EAssetType AssetType = EAssetType::SkeletalMesh;
		std::string Name = "";
		U8 NumberOfMaterials = 0;
		std::vector<SDrawCallData> DrawCallData = {};
		std::vector<SSkeletalMeshBone> BindPoseBones = {};
		std::vector<SSkeletalMeshNode> Nodes = {};
		SVector BoundsMin = SVector(FLT_MAX);
		SVector BoundsMax = SVector(-FLT_MAX);
		SVector BoundsCenter = SVector(0.0f);
	};
}