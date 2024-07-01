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
		SerializeSimple(AssetType, toData, pointerPosition);
		SerializeSimple(NumberOfMaterials, toData, pointerPosition);
		SerializeSimple(NumberOfMeshes, toData, pointerPosition);

		for (auto& mesh : Meshes)
		{
			SerializeSimple(mesh.NameLength, toData, pointerPosition);
			SerializeString(mesh.Name, toData, pointerPosition);
			SerializeSimple(mesh.NumberOfVertices, toData, pointerPosition);
			SerializeVector(mesh.Vertices, toData, pointerPosition);
			SerializeSimple(mesh.NumberOfIndices, toData, pointerPosition);
			SerializeVector(mesh.Indices, toData, pointerPosition);
		}
	}

	inline void SStaticModelFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeSimple(AssetType, fromData, pointerPosition);
		DeserializeSimple(NumberOfMaterials, fromData, pointerPosition);
		DeserializeSimple(NumberOfMeshes, fromData, pointerPosition);

		Meshes.reserve(NumberOfMeshes);
		for (U16 i = 0; i < NumberOfMeshes; i++)
		{
			Meshes.emplace_back();
			DeserializeSimple(Meshes.back().NameLength, fromData, pointerPosition);
			DeserializeString(Meshes.back().Name, fromData, Meshes.back().NameLength, pointerPosition);
			DeserializeSimple(Meshes.back().NumberOfVertices, fromData, pointerPosition);
			DeserializeVector(Meshes.back().Vertices, fromData, Meshes.back().NumberOfVertices, pointerPosition);
			DeserializeSimple(Meshes.back().NumberOfIndices, fromData, pointerPosition);
			DeserializeVector(Meshes.back().Indices, fromData, Meshes.back().NumberOfIndices, pointerPosition);
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
		SerializeSimple(AssetType, toData, pointerPosition);
		SerializeSimple(MaterialNameLength, toData, pointerPosition);
		SerializeString(MaterialName, toData, pointerPosition);
		SerializeSimple(OriginalFormat, toData, pointerPosition);
		SerializeSimple(MaterialConfiguration, toData, pointerPosition);
		SerializeSimple(Suffix, toData, pointerPosition);
		SerializeSimple(DataSize, toData, pointerPosition);
		SerializeString(Data, toData, pointerPosition);
	}

	inline void STextureFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeSimple(AssetType, fromData, pointerPosition);
		DeserializeSimple(MaterialNameLength, fromData, pointerPosition);
		DeserializeString(MaterialName, fromData, MaterialNameLength, pointerPosition);
		DeserializeSimple(OriginalFormat, fromData, pointerPosition);
		DeserializeSimple(MaterialConfiguration, fromData, pointerPosition);
		DeserializeSimple(Suffix, fromData, pointerPosition);
		DeserializeSimple(DataSize, fromData, pointerPosition);
		DeserializeString(Data, fromData, DataSize, pointerPosition);
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
		SerializeSimple(AssetType, toData, pointerPosition);
		SerializeSimple(MaterialNameLength, toData, pointerPosition);
		SerializeString(MaterialName, toData, pointerPosition);

		for (auto& materialProperty : Material.Properties)
		{
			SerializeSimple(materialProperty.ConstantValue, toData, pointerPosition);
			SerializeSimple(materialProperty.TexturePathLength, toData, pointerPosition);
			SerializeString(materialProperty.TexturePath, toData, pointerPosition);
			SerializeSimple(materialProperty.TextureChannelIndex, toData, pointerPosition);
		}

		SerializeSimple(Material.RecreateZ, toData, pointerPosition);
	}

	inline void SMaterialAssetFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeSimple(AssetType, fromData, pointerPosition);
		DeserializeSimple(MaterialNameLength, fromData, pointerPosition);
		DeserializeString(MaterialName, fromData, MaterialNameLength, pointerPosition);

		for (auto& materialProperty : Material.Properties)
		{
			DeserializeSimple(materialProperty.ConstantValue, fromData, pointerPosition);
			DeserializeSimple(materialProperty.TexturePathLength, fromData, pointerPosition);
			DeserializeString(materialProperty.TexturePath, fromData, materialProperty.TexturePathLength, pointerPosition);
			DeserializeSimple(materialProperty.TextureChannelIndex, fromData, pointerPosition);
		}

		DeserializeSimple(Material.RecreateZ, fromData, pointerPosition);
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
		SerializeSimple(AssetType, toData, pointerPosition);
		SerializeSimple(sceneIndex, toData, pointerPosition);
		assetRegistry->Serialize(sceneIndex, toData, pointerPosition);

		Scene->Serialize(toData, pointerPosition);
	}

	inline void SSceneFileHeader::Deserialize(const char* fromData, U64& pointerPosition, CScene* outScene, CAssetRegistry* assetRegistry)
	{
		DeserializeSimple(AssetType, fromData, pointerPosition);
		
		I64 sceneIndex = 0;
		DeserializeSimple(sceneIndex, fromData, pointerPosition);
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
		SerializeSimple(AssetType, toData, pointerPosition);
		SerializeSimple(NumberOfUVRects, toData, pointerPosition);
		SerializeVector(UVRects, toData, pointerPosition);
		SerializeSimple(NumberOfDurations, toData, pointerPosition);
		SerializeVector(Durations, toData, pointerPosition);
		SerializeSimple(IsLooping, toData, pointerPosition);
	}

	inline void SSpriteAnimationClipFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeSimple(AssetType, fromData, pointerPosition);
		DeserializeSimple(NumberOfUVRects, fromData, pointerPosition);
		DeserializeVector(UVRects, fromData, NumberOfUVRects, pointerPosition);
		DeserializeSimple(NumberOfDurations, fromData, pointerPosition);
		DeserializeVector(Durations, fromData, NumberOfDurations, pointerPosition);
		DeserializeSimple(IsLooping, fromData, pointerPosition);
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
		SerializeSimple(AssetType, toData, pointerPosition);
		SerializeSimple(SequencerNameLength, toData, pointerPosition);
		SerializeString(SequencerName, toData, pointerPosition);
		SerializeSimple(NumberOfEntityReferences, toData, pointerPosition);
		
		for (const SSequencerEntityReference& reference : EntityReferences)
		{
			reference.Serialize(toData, pointerPosition);
		}
	}

	inline void SSequencerFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeSimple(AssetType, fromData, pointerPosition);
		DeserializeSimple(SequencerNameLength, fromData, pointerPosition);
		DeserializeString(SequencerName, fromData, SequencerNameLength, pointerPosition);
		DeserializeSimple(NumberOfEntityReferences, fromData, pointerPosition);

		for (U64 index = 0; index < NumberOfEntityReferences; index++)
		{
			EntityReferences.emplace_back();
			EntityReferences.back().Deserialize(fromData, pointerPosition);
		}
	}
}
