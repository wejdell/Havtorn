// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Graphics/GraphicsStructs.h"
#include "Graphics/GraphicsEnums.h"
#include "Scene/Scene.h"
#include "Scene/AssetRegistry.h"

namespace Havtorn
{
	template<typename T>
	void SerializeSimple(const T& source, char* destination, U64& pointerPosition)
	{
		const U32 size = sizeof(T);
		memcpy(&destination[pointerPosition], &source, size);
		pointerPosition += size;
	}

	template<typename T>
	void SerializeVector(const std::vector<T>& source, char* destination, U64& pointerPosition)
	{
		const U32 size = sizeof(T) * static_cast<U32>(source.size());
		memcpy(&destination[pointerPosition], source.data(), size);
		pointerPosition += size;
	}

	inline void SerializeString(const std::string& source, char* destination, U64& pointerPosition)
	{
		const U32 size = sizeof(char) * static_cast<U32>(source.length());
		memcpy(&destination[pointerPosition], source.data(), size);
		pointerPosition += size;
	}

	inline void SerializeString(const std::string& source, char* destination, U32 numberOfElements, U64& pointerPosition)
	{
		const U32 size = sizeof(char) * numberOfElements;
		memcpy(&destination[pointerPosition], source.data(), size);
		pointerPosition += size;
	}

	template<typename T>
	void DeserializeSimple(T& destination, const char* source, U64& pointerPosition)
	{
		const U32 size = sizeof(T);
		memcpy(&destination, &source[pointerPosition], size);
		pointerPosition += size;
	}

	template<typename T>
	void DeserializeVector(std::vector<T>& destination, const char* source, U32 numberOfElements, U64& pointerPosition)
	{
		const U32 size = sizeof(T) * numberOfElements;
		const auto intermediateVector = new T[numberOfElements];
		memcpy(&intermediateVector[0], &source[pointerPosition], size);
		destination.assign(&intermediateVector[0], &intermediateVector[0] + numberOfElements);
		delete[] intermediateVector;
		pointerPosition += size;
	}

	inline void DeserializeString(std::string& destination, const char* source, U32 numberOfElements, U64& pointerPosition)
	{
		const U32 size = sizeof(char) * numberOfElements;
		destination = std::string(&source[pointerPosition], size);
		pointerPosition += size;
	}

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
		U32 SceneNameLength = 0;
		std::string SceneName = "";
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
		size += sizeof(char) * SceneNameLength;

		size += sizeof(U32);

		size += Scene->GetSize();

		return size;
	}

	inline void SSceneFileHeader::Serialize(char* toData, U64& pointerPosition, CAssetRegistry* assetRegistry, I64 sceneIndex) const
	{
		SerializeSimple(AssetType, toData, pointerPosition); //4
		SerializeSimple(SceneNameLength, toData, pointerPosition); //8
		SerializeString(SceneName, toData, pointerPosition);	//17
		
		SerializeSimple(sceneIndex, toData, pointerPosition); //25
		assetRegistry->Serialize(sceneIndex, toData, pointerPosition); //233

		Scene->Serialize(toData, pointerPosition);
	}

	inline void SSceneFileHeader::Deserialize(const char* fromData, U64& pointerPosition, CScene* outScene, CAssetRegistry* assetRegistry)
	{
		DeserializeSimple(AssetType, fromData, pointerPosition); //4
		DeserializeSimple(SceneNameLength, fromData, pointerPosition); //8
		DeserializeString(SceneName, fromData, SceneNameLength, pointerPosition); //17
		
		I64 sceneIndex = 0;
		DeserializeSimple(sceneIndex, fromData, pointerPosition); //25
		assetRegistry->Deserialize(sceneIndex, fromData, pointerPosition); //233

		outScene->Deserialize(fromData, pointerPosition, assetRegistry);
	}
}
