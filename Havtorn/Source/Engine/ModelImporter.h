// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

struct aiScene;

namespace Havtorn
{
	struct SAssetImportOptions;

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
		static ENGINE_API std::string ImportFBX(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions);

	private:
		static std::string ImportStaticMesh(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions, const aiScene* assimpScene);
		static std::string ImportSkeletalMesh(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions, const aiScene* assimpScene);
		static std::string ImportAnimation(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions, const aiScene* assimpScene);
	};
}
