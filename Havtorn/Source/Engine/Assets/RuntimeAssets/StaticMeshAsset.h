// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "Assets/FileHeaders/StaticMeshFileHeader.h"

namespace Havtorn 
{
	struct SStaticMeshAsset
	{
		SStaticMeshAsset() = default;

		explicit SStaticMeshAsset(const SStaticMeshFileHeader& assetFileData)
			: AssetType(assetFileData.HeaderBase.AssetType)
			, Name(assetFileData.Name)
			, NumberOfMaterials(assetFileData.NumberOfMaterials)
		{
			for (auto& mesh : assetFileData.Meshes)
			{
				DrawCallData.emplace_back();
				DrawCallData.back().IndexCount = STATIC_U32(mesh.Indices.size());
				DrawCallData.back().MaterialIndex = STATIC_U16(mesh.MaterialIndex);
			}
		}

		EAssetType AssetType = EAssetType::StaticMesh;
		std::string Name = "";
		U8 NumberOfMaterials = 0;
		std::vector<SDrawCallData> DrawCallData = {};
		std::vector<SMaterialVertex> MaterialVertexAssociations = {};
		SVector BoundsMin = SVector(FLT_MAX);
		SVector BoundsMax = SVector(-FLT_MAX);
		SVector BoundsCenter = SVector(0.0f);
	};
}
