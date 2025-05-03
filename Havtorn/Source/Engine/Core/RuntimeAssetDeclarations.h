// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "FileSystem/FileHeaderDeclarations.h"
#include "Graphics/GraphicsStructs.h"
#include "Graphics/GraphicsUtilities.h"
#include "Graphics/GraphicsMaterial.h"

namespace Havtorn
{
	struct SStaticMeshAsset
	{
		SStaticMeshAsset() = default;

		explicit SStaticMeshAsset(const SStaticModelFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Name(assetFileData.Name)
			, NumberOfMaterials(assetFileData.NumberOfMaterials)
		{
			for (auto& mesh : assetFileData.Meshes)
			{
				DrawCallData.emplace_back();
				DrawCallData.back().IndexCount = STATIC_U32(mesh.Indices.size());
			}
		}

		EAssetType AssetType = EAssetType::StaticMesh;
		std::string Name = "";
		U8 NumberOfMaterials = 0;
		std::vector<SDrawCallData> DrawCallData = {};
		SVector BoundsMin = SVector(FLT_MAX);
		SVector BoundsMax = SVector(-FLT_MAX);
	};

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
	};

	struct SSkeletalAnimationAsset
	{
		SSkeletalAnimationAsset() = default;

		explicit SSkeletalAnimationAsset(const SSkeletalAnimationFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Name(assetFileData.Name)
			, SkeletonName(assetFileData.SkeletonName)
			, DurationInTicks(assetFileData.DurationInTicks)
			, TickRate(assetFileData.TickRate)
			, NumberOfTracks(assetFileData.NumberOfBones)
			, ImportScale(assetFileData.ImportScale)
			, BoneAnimationTracks(assetFileData.BoneAnimationTracks)
		{
			//PreprocessAnimation(assetFileData);
		}

		EAssetType AssetType = EAssetType::Animation;
		std::string Name = "";
		std::string SkeletonName = "";
		U32 DurationInTicks = 0;
		U32 TickRate = 0;
		U32 NumberOfTracks = 0;
		F32 ImportScale = 1.0f;
		//std::vector<SBoneAnimDataTransform> EncodedBoneAnimTransforms;
		std::vector<SBoneAnimationTrack> BoneAnimationTracks;

		//SBoneAnimDataTransform EncodeTransform(const SVecBoneAnimationKey& translationKey, const SQuatBoneAnimationKey& rotationKey, const SVecBoneAnimationKey& scaleKey)
		//SBoneAnimDataTransform EncodeTransform(const SBoneAnimationKey& animationKey)
		//SBoneAnimDataTransform EncodeTransform(const SSkeletalMeshBone& bone)
		//{
		//	SBoneAnimDataTransform dataFrame;
		//	SMatrix boneMatrix = bone.Transform;
		//	//SMatrix::Recompose(translationKey.Value, rotationKey.Value.ToEuler(), scaleKey.Value, boneMatrix);
		//	SVector translation = boneMatrix.GetTranslation();

		//	dataFrame.Row1TX = boneMatrix.GetRow(0);
		//	dataFrame.Row1TX.W = translation.X;
		//	dataFrame.Row2TY = boneMatrix.GetRow(1);
		//	dataFrame.Row2TY.W = translation.Y;
		//	dataFrame.Row3TZ = boneMatrix.GetRow(2);
		//	dataFrame.Row3TZ.W = translation.Z;

		//	return dataFrame;
		//}

		void PreprocessAnimation(const SSkeletalAnimationFileHeader& /*assetFileData*/)
		{
			//F32 accumulatedTime = 0.0f;
			//for (U32 tick = 0; tick < DurationInTicks; tick++)
			//{
			//	accumulatedTime += 1.0f / STATIC_F32(TickRate);
			//	//for (const SBoneAnimationTrack& track : assetFileData.BoneAnimationTracks)
			//	//{
			//		//SVecBoneAnimationKey translationKey;
			//		//for (const SVecBoneAnimationKey& key : track.TranslationKeys)
			//		//	translationKey = (key.Time <= accumulatedTime) ? key : translationKey;

			//		//SQuatBoneAnimationKey rotationKey;
			//		//for (const SQuatBoneAnimationKey& key : track.RotationKeys)
			//		//	rotationKey = (key.Time <= accumulatedTime) ? key : rotationKey;

			//		//SVecBoneAnimationKey scaleKey;
			//		//for (const SVecBoneAnimationKey& key : track.ScaleKeys)
			//		//	scaleKey = (key.Time <= accumulatedTime) ? key : scaleKey;

			//	//	EncodedBoneAnimTransforms.emplace_back(EncodeTransform(translationKey, rotationKey, scaleKey));
			//		//SBoneAnimationKey animationKey;
			//		//for (const SBoneAnimationKey& key : track.AnimationKeys)
			//		//	animationKey = (key.Time <= accumulatedTime) ? key : animationKey;

			//		//EncodedBoneAnimTransforms.emplace_back(EncodeTransform(animationKey));
			//	//}
			//}
			//for (const SSkeletalMeshBone& bone : assetFileData.SequentialPosedBones)
			//{
			//	EncodedBoneAnimTransforms.emplace_back(EncodeTransform(bone));
			//}
		}
	};

	struct STextureAsset
	{
		STextureAsset() = default;

		explicit STextureAsset(const STextureFileHeader& assetFileData, ID3D11Device* graphicsDevice)
			: AssetType(assetFileData.AssetType)
			, MaterialName(assetFileData.MaterialName)
			, MaterialConfiguration(assetFileData.MaterialConfiguration)
			, Suffix(assetFileData.Suffix)
		{
			ShaderResourceView = std::move(UGraphicsUtils::TryGetShaderResourceView(graphicsDevice, assetFileData.Data.data(), assetFileData.Data.size(), assetFileData.OriginalFormat));
		}

		EAssetType AssetType = EAssetType::Texture;
		std::string MaterialName = "";
		EMaterialConfiguration MaterialConfiguration = EMaterialConfiguration::AlbedoMaterialNormal_Packed;
		char Suffix = 0;
		ID3D11ShaderResourceView* ShaderResourceView = nullptr;
	};

	struct SGraphicsMaterialAsset
	{
		SGraphicsMaterialAsset() = default;

		explicit SGraphicsMaterialAsset(const SMaterialAssetFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Material(assetFileData.Material, assetFileData.MaterialName)
		{
		}

		EAssetType AssetType = EAssetType::Material;
		SEngineGraphicsMaterial Material;
	};

	struct SSpriteAninmationClipAsset
	{
		SSpriteAninmationClipAsset() = default;

		explicit SSpriteAninmationClipAsset(const SSpriteAnimationClipFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, SpriteAnimationClip(SSpriteAnimationClip(assetFileData.UVRects, assetFileData.Durations, assetFileData.IsLooping))
		{
		}

		EAssetType AssetType = EAssetType::SpriteAnimation;
		SSpriteAnimationClip SpriteAnimationClip;
	};
}