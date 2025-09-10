// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Assets/FileHeaderDeclarations.h"

struct aiScene;

namespace Havtorn
{
	enum class EModelProperty
	{
		HasPositions				= BIT(0),
		HasNormals					= BIT(1),
		HasTangentsAndBitangents	= BIT(2),
		HasUVs						= BIT(3),
		HasBones					= BIT(4)
	};

	class UModelImporter
	{
	public:
		static ENGINE_API SAssetFileHeader ImportFBX(const std::string& filePath, const SSourceAssetData& sourceAssetData);

	private:
		static SStaticModelFileHeader ImportStaticMesh(const aiScene* assimpScene, const SSourceAssetData& sourceAssetData);
		static SSkeletalModelFileHeader ImportSkeletalMesh(const aiScene* assimpScene, const SSourceAssetData& sourceAssetData);
		static SSkeletalAnimationFileHeader ImportAnimation(const aiScene* assimpScene, const SSourceAssetData& sourceAssetData);
	};
}
