// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include "Graphics/GraphicsStructs.h"
#include "Graphics/GraphicsEnums.h"
#include "Scene/Scene.h"
#include "Scene/AssetRegistry.h"
#include "Assets/SequencerAsset.h"

namespace Havtorn
{
	struct SStaticModelFileHeader
	{
		EAssetType AssetType = EAssetType::StaticMesh;
		U32 NameLength = 0;
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
		U32 size = sizeof(EAssetType);
		size += sizeof(U32);
		size += sizeof(char) * NameLength;
		size += sizeof(U8);
		size += sizeof(U32);

		for (auto& mesh : Meshes)
		{
			size += sizeof(U32);
			size += sizeof(char) * mesh.NameLength;
			size += sizeof(U32);
			size += sizeof(SStaticMeshVertex) * mesh.NumberOfVertices;
			size += sizeof(U32);
			size += sizeof(U32) * mesh.NumberOfIndices;
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
			SerializeData(mesh.NameLength, toData, pointerPosition);
			SerializeData(mesh.Name, toData, pointerPosition);
			SerializeData(mesh.NumberOfVertices, toData, pointerPosition);
			SerializeData(mesh.Vertices, toData, pointerPosition);
			SerializeData(mesh.NumberOfIndices, toData, pointerPosition);
			SerializeData(mesh.Indices, toData, pointerPosition);
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
			DeserializeData(Meshes.back().NameLength, fromData, pointerPosition);
			DeserializeData(Meshes.back().Name, fromData, Meshes.back().NameLength, pointerPosition);
			DeserializeData(Meshes.back().NumberOfVertices, fromData, pointerPosition);
			DeserializeData(Meshes.back().Vertices, fromData, Meshes.back().NumberOfVertices, pointerPosition);
			DeserializeData(Meshes.back().NumberOfIndices, fromData, pointerPosition);
			DeserializeData(Meshes.back().Indices, fromData, Meshes.back().NumberOfIndices, pointerPosition);
		}
	}

	struct STextureFileHeader
	{
		EAssetType AssetType = EAssetType::Texture;
		U32 MaterialNameLength = 0;
		std::string MaterialName = "";
		ETextureFormat OriginalFormat = ETextureFormat::DDS;
		EMaterialConfiguration MaterialConfiguration = EMaterialConfiguration::AlbedoMaterialNormal_Packed;
		char Suffix = 0;
		U32 DataSize = 0;
		std::string Data = "";

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 STextureFileHeader::GetSize() const
	{
		U32 size = sizeof(EAssetType);
		size += sizeof(U32);
		size += sizeof(char) * MaterialNameLength;
		size += sizeof(ETextureFormat);
		size += sizeof(EMaterialConfiguration);
		size += sizeof(char);
		size += sizeof(U32);
		size += sizeof(char) * DataSize;

		return size;
	}

	inline void STextureFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(MaterialNameLength, toData, pointerPosition);
		SerializeData(MaterialName, toData, pointerPosition);
		SerializeData(OriginalFormat, toData, pointerPosition);
		SerializeData(MaterialConfiguration, toData, pointerPosition);
		SerializeData(Suffix, toData, pointerPosition);
		SerializeData(DataSize, toData, pointerPosition);
		SerializeData(Data, toData, pointerPosition);
	}

	inline void STextureFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(MaterialNameLength, fromData, pointerPosition);
		DeserializeData(MaterialName, fromData, MaterialNameLength, pointerPosition);
		DeserializeData(OriginalFormat, fromData, pointerPosition);
		DeserializeData(MaterialConfiguration, fromData, pointerPosition);
		DeserializeData(Suffix, fromData, pointerPosition);
		DeserializeData(DataSize, fromData, pointerPosition);
		DeserializeData(Data, fromData, DataSize, pointerPosition);
	}

	struct SMaterialAssetFileHeader
	{
		EAssetType AssetType = EAssetType::Material;
		U32 MaterialNameLength = 0;
		std::string MaterialName = "";
		SOfflineGraphicsMaterial Material;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SMaterialAssetFileHeader::GetSize() const
	{
		U32 size = sizeof(EAssetType);
		size += sizeof(U32);
		size += sizeof(char) * MaterialNameLength;
		size += Material.GetSize();

		return size;
	}

	inline void SMaterialAssetFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(MaterialNameLength, toData, pointerPosition);
		SerializeData(MaterialName, toData, pointerPosition);

		for (auto& materialProperty : Material.Properties)
		{
			SerializeData(materialProperty.ConstantValue, toData, pointerPosition);
			SerializeData(materialProperty.TexturePathLength, toData, pointerPosition);
			SerializeData(materialProperty.TexturePath, toData, pointerPosition);
			SerializeData(materialProperty.TextureChannelIndex, toData, pointerPosition);
		}

		SerializeData(Material.RecreateZ, toData, pointerPosition);
	}

	inline void SMaterialAssetFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(MaterialNameLength, fromData, pointerPosition);
		DeserializeData(MaterialName, fromData, MaterialNameLength, pointerPosition);

		for (auto& materialProperty : Material.Properties)
		{
			DeserializeData(materialProperty.ConstantValue, fromData, pointerPosition);
			DeserializeData(materialProperty.TexturePathLength, fromData, pointerPosition);
			DeserializeData(materialProperty.TexturePath, fromData, materialProperty.TexturePathLength, pointerPosition);
			DeserializeData(materialProperty.TextureChannelIndex, fromData, pointerPosition);
		}

		DeserializeData(Material.RecreateZ, fromData, pointerPosition);
	}

	struct SSceneFileHeader
	{
		EAssetType AssetType = EAssetType::Scene;
		U32 NumberOfEntities = 0;
		CScene* Scene = nullptr;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition, CAssetRegistry* assetRegistry, I64 sceneIndex) const;
		void Deserialize(const char* fromData, U64& pointerPosition, CScene* outScene, CAssetRegistry* assetRegistry);
	};

	inline U32 SSceneFileHeader::GetSize() const
	{
		U32 size = sizeof(EAssetType);
		size += sizeof(U32);
		size += Scene->GetSize();

		return size;
	}

	inline void SSceneFileHeader::Serialize(char* toData, U64& pointerPosition, CAssetRegistry* assetRegistry, I64 sceneIndex) const
	{
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(sceneIndex, toData, pointerPosition);
		assetRegistry->Serialize(sceneIndex, toData, pointerPosition);

		Scene->Serialize(toData, pointerPosition);
	}

	inline void SSceneFileHeader::Deserialize(const char* fromData, U64& pointerPosition, CScene* outScene, CAssetRegistry* assetRegistry)
	{
		DeserializeData(AssetType, fromData, pointerPosition);
		
		I64 sceneIndex = 0;
		DeserializeData(sceneIndex, fromData, pointerPosition);
		assetRegistry->Deserialize(sceneIndex, fromData, pointerPosition);

		outScene->Deserialize(fromData, pointerPosition, assetRegistry);
	}

	struct SSpriteAnimationClipFileHeader
	{
		EAssetType AssetType = EAssetType::SpriteAnimation;
		U32 NumberOfUVRects;
		std::vector<SVector4> UVRects;
		U32 NumberOfDurations;
		std::vector<F32> Durations;
		bool IsLooping = false;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSpriteAnimationClipFileHeader::GetSize() const
	{
		U32 size = sizeof(EAssetType);
		size += sizeof(U32);
		size += sizeof(SVector4) * NumberOfUVRects;
		size += sizeof(U32);
		size += sizeof(F32) * NumberOfDurations;
		size += sizeof(bool);
		return size;
	}

	inline void SSpriteAnimationClipFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(NumberOfUVRects, toData, pointerPosition);
		SerializeData(UVRects, toData, pointerPosition);
		SerializeData(NumberOfDurations, toData, pointerPosition);
		SerializeData(Durations, toData, pointerPosition);
		SerializeData(IsLooping, toData, pointerPosition);
	}

	inline void SSpriteAnimationClipFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(NumberOfUVRects, fromData, pointerPosition);
		DeserializeData(UVRects, fromData, NumberOfUVRects, pointerPosition);
		DeserializeData(NumberOfDurations, fromData, pointerPosition);
		DeserializeData(Durations, fromData, NumberOfDurations, pointerPosition);
		DeserializeData(IsLooping, fromData, pointerPosition);
	}

	struct SSequencerFileHeader
	{
		EAssetType AssetType = EAssetType::Sequencer;
		U32 SequencerNameLength = 0;
		std::string SequencerName = "";
		U32 NumberOfEntityReferences = 0;
		std::vector<SSequencerEntityReference> EntityReferences;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSequencerFileHeader::GetSize() const
	{
		U32 size = sizeof(EAssetType);
		size += sizeof(U32);
		size += sizeof(char) * SequencerNameLength;
		size += sizeof(U32);

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
		SerializeData(SequencerNameLength, toData, pointerPosition);
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
		DeserializeData(SequencerNameLength, fromData, pointerPosition);
		DeserializeData(SequencerName, fromData, SequencerNameLength, pointerPosition);
		DeserializeData(NumberOfEntityReferences, fromData, pointerPosition);

		for (U64 index = 0; index < NumberOfEntityReferences; index++)
		{
			EntityReferences.emplace_back();
			EntityReferences.back().Deserialize(fromData, pointerPosition);
		}
	}
}
