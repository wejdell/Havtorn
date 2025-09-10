// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Assets/FileHeaderDeclarations.h"
#include "Graphics/GraphicsStructs.h"
#include "Graphics/GraphicsUtilities.h"
#include "Graphics/GraphicsMaterial.h"
#include "Graphics/RenderingPrimitives/RenderTexture.h"
#include "ECS/Components/SpriteAnimatorGraphNode.h"
#include "HexRune/HexRune.h"

#include <set>

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
				DrawCallData.back().MaterialIndex = STATIC_U16(mesh.MaterialIndex);
			}
		}

		EAssetType AssetType = EAssetType::StaticMesh;
		std::string Name = "";
		U8 NumberOfMaterials = 0;
		std::vector<SDrawCallData> DrawCallData = {};
		SVector BoundsMin = SVector(FLT_MAX);
		SVector BoundsMax = SVector(-FLT_MAX);
		SVector BoundsCenter = SVector(0.0f);
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

	struct SSkeletalAnimationAsset
	{
		SSkeletalAnimationAsset() = default;

		explicit SSkeletalAnimationAsset(const SSkeletalAnimationFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Name(assetFileData.Name)
			, RigPath(assetFileData.SourceData.AssetDependencyPath.AsString())
			, DurationInTicks(assetFileData.DurationInTicks)
			, TickRate(assetFileData.TickRate)
			, NumberOfTracks(assetFileData.NumberOfBones)
			, ImportScale(assetFileData.SourceData.ImportScale)
			, BoneAnimationTracks(assetFileData.BoneAnimationTracks)
		{
		}

		EAssetType AssetType = EAssetType::Animation;
		std::string Name = "";
		std::string RigPath = "";
		U32 DurationInTicks = 0;
		U32 TickRate = 0;
		U32 NumberOfTracks = 0;
		F32 ImportScale = 1.0f;
		std::vector<SBoneAnimationTrack> BoneAnimationTracks;
	};

	struct STextureAsset
	{
		STextureAsset() = default;

		explicit STextureAsset(const STextureFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Name(assetFileData.Name)
		{
			// NW: RenderTexture is assigned in CAssetRegistry::LoadAsset
		}

		EAssetType AssetType = EAssetType::Texture;
		std::string Name = "";
		CStaticRenderTexture RenderTexture;
	};

	struct SGraphicsMaterialAsset
	{
		SGraphicsMaterialAsset() = default;

		explicit SGraphicsMaterialAsset(const SMaterialAssetFileHeader& assetFileData)
			: AssetType(assetFileData.AssetType)
			, Material(assetFileData.Material, assetFileData.Name)
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

	typedef std::variant<std::monostate, SStaticMeshAsset, SSkeletalMeshAsset, SSkeletalAnimationAsset, STextureAsset, SGraphicsMaterialAsset, SSpriteAninmationClipAsset, HexRune::SScript> SAssetData;

	struct SAsset
	{
		EAssetType Type = EAssetType::None;
		SAssetReference Reference;
		SSourceAssetData SourceData;

		std::set<U64> Requesters = {};
		SAssetData Data = std::monostate();

		const bool IsValid() const { return Reference.IsValid() && Type != EAssetType::None; }
	};
}