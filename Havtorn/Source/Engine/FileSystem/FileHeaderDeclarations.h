// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include "Graphics/GraphicsStructs.h"
#include "Graphics/GraphicsEnums.h"
#include "Scene/Scene.h"
#include "Scene/AssetRegistry.h"
#include "Assets/SequencerAsset.h"
#include "HexRune/HexRune.h"

namespace Havtorn
{
	struct SStaticModelFileHeader
	{
		EAssetType AssetType = EAssetType::StaticMesh;
		std::string Name;
		U8 NumberOfMaterials = 0;
		U32 NumberOfMeshes = 0;
		std::vector<SStaticMesh> Meshes;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SStaticModelFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(NumberOfMaterials);
		size += GetDataSize(NumberOfMeshes);

		for (auto& mesh : Meshes)
		{
			size += GetDataSize(mesh.Name);
			size += GetDataSize(mesh.Vertices);
			size += GetDataSize(mesh.Indices);
			size += GetDataSize(mesh.MaterialIndex);
		}
		return size;
	}

	inline void SStaticModelFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(NumberOfMaterials, toData, pointerPosition);
		SerializeData(NumberOfMeshes, toData, pointerPosition);

		for (auto& mesh : Meshes)
		{
			SerializeData(mesh.Name, toData, pointerPosition);
			SerializeData(mesh.Vertices, toData, pointerPosition);
			SerializeData(mesh.Indices, toData, pointerPosition);
			SerializeData(mesh.MaterialIndex, toData, pointerPosition);
		}
	}

	inline void SStaticModelFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
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
	}

	struct SSkeletalModelFileHeader
	{
		EAssetType AssetType = EAssetType::SkeletalMesh;
		std::string Name;
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

	struct SSkeletalAnimationFileHeader
	{
		EAssetType AssetType = EAssetType::Animation;
		std::string Name;
		std::string SkeletonName;
		U32 DurationInTicks = 0;
		U32 TickRate = 0;
		U32 NumberOfBones = 0;
		F32 ImportScale = 1.0f;
		std::vector<SBoneAnimationTrack> BoneAnimationTracks;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSkeletalAnimationFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += GetDataSize(SkeletonName);
		size += GetDataSize(DurationInTicks);
		size += GetDataSize(TickRate);
		size += GetDataSize(NumberOfBones);
		size += GetDataSize(ImportScale);

		for (auto& track : BoneAnimationTracks)
			size += track.GetSize();

		return size;
	}

	inline void SSkeletalAnimationFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(SkeletonName, toData, pointerPosition);
		SerializeData(DurationInTicks, toData, pointerPosition);
		SerializeData(TickRate, toData, pointerPosition);
		SerializeData(NumberOfBones, toData, pointerPosition);
		SerializeData(ImportScale, toData, pointerPosition);

		for (auto& track : BoneAnimationTracks)
		{
			SerializeData(track.TranslationKeys, toData, pointerPosition);
			SerializeData(track.RotationKeys, toData, pointerPosition);
			SerializeData(track.ScaleKeys, toData, pointerPosition);
			SerializeData(track.TrackName, toData, pointerPosition);
		}
	}

	inline void SSkeletalAnimationFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(SkeletonName, fromData, pointerPosition);
		DeserializeData(DurationInTicks, fromData, pointerPosition);
		DeserializeData(TickRate, fromData, pointerPosition);
		DeserializeData(NumberOfBones, fromData, pointerPosition);
		DeserializeData(ImportScale, fromData, pointerPosition);

		for (U16 i = 0; i < NumberOfBones; i++)
		{
			BoneAnimationTracks.emplace_back();
			DeserializeData(BoneAnimationTracks.back().TranslationKeys, fromData, pointerPosition);
			DeserializeData(BoneAnimationTracks.back().RotationKeys, fromData, pointerPosition);
			DeserializeData(BoneAnimationTracks.back().ScaleKeys, fromData, pointerPosition);
			DeserializeData(BoneAnimationTracks.back().TrackName, fromData, pointerPosition);
		}
	}

	struct STextureFileHeader
	{
		EAssetType AssetType = EAssetType::Texture;
		std::string MaterialName = "";
		ETextureFormat OriginalFormat = ETextureFormat::DDS;
		EMaterialConfiguration MaterialConfiguration = EMaterialConfiguration::AlbedoMaterialNormal_Packed;
		char Suffix = 0;
		std::string Data = "";

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 STextureFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(MaterialName);
		size += GetDataSize(OriginalFormat);
		size += GetDataSize(MaterialConfiguration);
		size += GetDataSize(Suffix);
		size += GetDataSize(Data);

		return size;
	}

	inline void STextureFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(MaterialName, toData, pointerPosition);
		SerializeData(OriginalFormat, toData, pointerPosition);
		SerializeData(MaterialConfiguration, toData, pointerPosition);
		SerializeData(Suffix, toData, pointerPosition);
		SerializeData(Data, toData, pointerPosition);
	}

	inline void STextureFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(MaterialName, fromData, pointerPosition);
		DeserializeData(OriginalFormat, fromData, pointerPosition);
		DeserializeData(MaterialConfiguration, fromData, pointerPosition);
		DeserializeData(Suffix, fromData, pointerPosition);
		DeserializeData(Data, fromData, pointerPosition);
	}

	struct SMaterialAssetFileHeader
	{
		EAssetType AssetType = EAssetType::Material;
		std::string MaterialName = "";
		SOfflineGraphicsMaterial Material;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SMaterialAssetFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(MaterialName);
		size += Material.GetSize();

		return size;
	}

	inline void SMaterialAssetFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(MaterialName, toData, pointerPosition);

		for (auto& materialProperty : Material.Properties)
		{
			SerializeData(materialProperty.ConstantValue, toData, pointerPosition);
			SerializeData(materialProperty.TexturePath, toData, pointerPosition);
			SerializeData(materialProperty.TextureChannelIndex, toData, pointerPosition);
		}

		SerializeData(Material.RecreateZ, toData, pointerPosition);
	}

	inline void SMaterialAssetFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(MaterialName, fromData, pointerPosition);

		for (auto& materialProperty : Material.Properties)
		{
			DeserializeData(materialProperty.ConstantValue, fromData, pointerPosition);
			DeserializeData(materialProperty.TexturePath, fromData, pointerPosition);
			DeserializeData(materialProperty.TextureChannelIndex, fromData, pointerPosition);
		}

		DeserializeData(Material.RecreateZ, fromData, pointerPosition);
	}

	struct SSceneFileHeader
	{
		EAssetType AssetType = EAssetType::Scene;
		CScene* Scene = nullptr;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition, CAssetRegistry* assetRegistry) const;
		void Deserialize(const char* fromData, U64& pointerPosition, CScene* outScene, CAssetRegistry* assetRegistry);
	};

	inline U32 SSceneFileHeader::GetSize() const
	{
		U32 size = 0;
		// NR: Asset Registry size and asset type size is calculated in the call site of this function.
		size += Scene->GetSize();

		return size;
	}

	inline void SSceneFileHeader::Serialize(char* toData, U64& pointerPosition, CAssetRegistry* assetRegistry) const
	{
		SerializeData(AssetType, toData, pointerPosition);
		assetRegistry->Serialize(toData, pointerPosition);
		Scene->Serialize(toData, pointerPosition);
	}

	inline void SSceneFileHeader::Deserialize(const char* fromData, U64& pointerPosition, CScene* outScene, CAssetRegistry* assetRegistry)
	{
		DeserializeData(AssetType, fromData, pointerPosition);
		assetRegistry->Deserialize(fromData, pointerPosition);
		outScene->Deserialize(fromData, pointerPosition, assetRegistry);
	}

	struct SSpriteAnimationClipFileHeader
	{
		EAssetType AssetType = EAssetType::SpriteAnimation;
		std::vector<SVector4> UVRects;
		std::vector<F32> Durations;
		bool IsLooping = false;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSpriteAnimationClipFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(UVRects);
		size += GetDataSize(Durations);
		size += GetDataSize(IsLooping);
		return size;
	}

	inline void SSpriteAnimationClipFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(UVRects, toData, pointerPosition);
		SerializeData(Durations, toData, pointerPosition);
		SerializeData(IsLooping, toData, pointerPosition);
	}

	inline void SSpriteAnimationClipFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(UVRects, fromData, pointerPosition);
		DeserializeData(Durations, fromData, pointerPosition);
		DeserializeData(IsLooping, fromData, pointerPosition);
	}

	struct SSequencerFileHeader
	{
		EAssetType AssetType = EAssetType::Sequencer;
		std::string SequencerName = "";
		U32 NumberOfEntityReferences = 0;
		std::vector<SSequencerEntityReference> EntityReferences;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSequencerFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(SequencerName);
		size += GetDataSize(NumberOfEntityReferences);

		for (const SSequencerEntityReference& reference : EntityReferences)
		{
			size += reference.GetSize();
		}

		return size;
	}

	inline void SSequencerFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(SequencerName, toData, pointerPosition);
		SerializeData(NumberOfEntityReferences, toData, pointerPosition);
		
		for (const SSequencerEntityReference& reference : EntityReferences)
		{
			reference.Serialize(toData, pointerPosition);
		}
	}

	inline void SSequencerFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(SequencerName, fromData, pointerPosition);
		DeserializeData(NumberOfEntityReferences, fromData, pointerPosition);

		for (U64 index = 0; index < NumberOfEntityReferences; index++)
		{
			EntityReferences.emplace_back();
			EntityReferences.back().Deserialize(fromData, pointerPosition);
		}
	}

	struct SScriptFileHeader
	{
		EAssetType AssetType = EAssetType::Script;
		HexRune::SScript* Script = nullptr;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData, HexRune::SScript* outScript);
	};

	inline U32 SScriptFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += Script->GetSize();

		return size;
	}

	inline void SScriptFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		Script->Serialize(toData, pointerPosition);
	}

	inline void SScriptFileHeader::Deserialize(const char* fromData, HexRune::SScript* outScript)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		outScript->Deserialize(fromData, pointerPosition);
	}
}
