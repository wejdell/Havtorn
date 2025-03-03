// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

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

	// TODO.NR: Use these
	struct SImportSettings
	{
		F32 Scale = 0.0f;
		std::string RefSkeleton = "";
	};

	class UModelImporter
	{
	public:
		static ENGINE_API std::string ImportFBX(const std::string& filePath, const EAssetType assetType);

	private:
		static std::string ImportStaticMesh(const std::string& filePath, const aiScene* assimpScene);
		static std::string ImportSkeletalMesh(const std::string& filePath, const aiScene* assimpScene);
		static std::string ImportAnimation(const std::string& filePath, const aiScene* assimpScene);
	};
}
