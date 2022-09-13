// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Graphics/GraphicsStructs.h"
#include "Graphics/GraphicsEnums.h"

namespace Havtorn
{
#pragma region Serialize Template

	template<typename T>
	U32 SerializeSimple(const T& source, char* destination, U32 bufferPosition)
	{
		const U32 size = sizeof(T);
		memcpy(&destination[bufferPosition], &source, size);
		return size;
	}

	template<typename T>
	U32 SerializeVector(const std::vector<T>& source, char* destination, U32 bufferPosition)
	{
		const U32 size = sizeof(T) * static_cast<U32>(source.size());
		memcpy(&destination[bufferPosition], source.data(), size);
		return size;
	}

	inline U32 SerializeString(const std::string& source, char* destination, U32 bufferPosition)
	{
		const U32 size = sizeof(char) * static_cast<U32>(source.length());
		memcpy(&destination[bufferPosition], source.data(), size);
		return size;
	}

	template<typename T>
	U32 DeserializeSimple(T& destination, const char* source, U32 bufferPosition)
	{
		const U32 size = sizeof(T);
		memcpy(&destination, &source[bufferPosition], size);
		return size;
	}

	template<typename T>
	U32 DeserializeVector(std::vector<T>& destination, const char* source, U32 numberOfElements, U32 bufferPosition)
	{
		const U32 size = sizeof(T) * numberOfElements;
		const auto intermediateVector = new T[numberOfElements];
		memcpy(&intermediateVector[0], &source[bufferPosition], size);
		destination.assign(&intermediateVector[0], &intermediateVector[0] + numberOfElements);
		return size;
	}

	inline U32 DeserializeString(std::string& destination, const char* source, U32 numberOfElements, U32 bufferPosition)
	{
		const U32 size = sizeof(char) * numberOfElements;
		destination = std::string(&source[bufferPosition], size);
		return size;
	}

#pragma endregion

#pragma region Static Mesh

	struct SStaticMeshFileHeader
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

	inline U32 SStaticMeshFileHeader::GetSize() const
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

	inline void SStaticMeshFileHeader::Serialize(char* toData) const
	{
		U32 pointerPosition = 0;
		pointerPosition += SerializeSimple(AssetType, toData, pointerPosition);
		pointerPosition += SerializeSimple(NumberOfMaterials, toData, pointerPosition);
		pointerPosition += SerializeSimple(NumberOfMeshes, toData, pointerPosition);

		for (auto& mesh : Meshes)
		{
			pointerPosition += SerializeSimple(mesh.NameLength, toData, pointerPosition);
			pointerPosition += SerializeString(mesh.Name, toData, pointerPosition);
			pointerPosition += SerializeSimple(mesh.NumberOfVertices, toData, pointerPosition);
			pointerPosition += SerializeVector(mesh.Vertices, toData, pointerPosition);
			pointerPosition += SerializeSimple(mesh.NumberOfIndices, toData, pointerPosition);
			pointerPosition += SerializeVector(mesh.Indices, toData, pointerPosition);
		}
	}

	inline void SStaticMeshFileHeader::Deserialize(const char* fromData)
	{
		U32 pointerPosition = 0;
		pointerPosition += DeserializeSimple(AssetType, fromData, pointerPosition);
		pointerPosition += DeserializeSimple(NumberOfMaterials, fromData, pointerPosition);
		pointerPosition += DeserializeSimple(NumberOfMeshes, fromData, pointerPosition);

		Meshes.reserve(NumberOfMeshes);
		for (U16 i = 0; i < NumberOfMeshes; i++)
		{
			Meshes.emplace_back();
			pointerPosition += DeserializeSimple(Meshes.back().NameLength, fromData, pointerPosition);
			pointerPosition += DeserializeString(Meshes.back().Name, fromData, Meshes.back().NameLength, pointerPosition);
			pointerPosition += DeserializeSimple(Meshes.back().NumberOfVertices, fromData, pointerPosition);
			pointerPosition += DeserializeVector(Meshes.back().Vertices, fromData, Meshes.back().NumberOfVertices, pointerPosition);
			pointerPosition += DeserializeSimple(Meshes.back().NumberOfIndices, fromData, pointerPosition);
			pointerPosition += DeserializeVector(Meshes.back().Indices, fromData, Meshes.back().NumberOfIndices, pointerPosition);
		}
	}

#pragma endregion

#pragma region Skeletal Mesh

	struct SSkeletalMeshFileHeader
	{
		EAssetType AssetType = EAssetType::SkeletalMesh;
		U32 NameLength = 0;
		std::string Name;
		U8 NumberOfMaterials = 0;
		U32 NumberOfMeshes = 0;
		std::vector<SSkeletalMesh> Meshes;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SSkeletalMeshFileHeader::GetSize() const
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
			size += sizeof(SSkeletalMeshVertex) * mesh.NumberOfVertices;
			size += sizeof(U32);
			size += sizeof(U32) * mesh.NumberOfIndices;
		}
		return size;
	}

	inline void SSkeletalMeshFileHeader::Serialize(char* toData) const
	{
		U32 pointerPosition = 0;
		pointerPosition += SerializeSimple(AssetType, toData, pointerPosition);
		pointerPosition += SerializeSimple(NumberOfMaterials, toData, pointerPosition);
		pointerPosition += SerializeSimple(NumberOfMeshes, toData, pointerPosition);

		for (auto& mesh : Meshes)
		{
			pointerPosition += SerializeSimple(mesh.NameLength, toData, pointerPosition);
			pointerPosition += SerializeString(mesh.Name, toData, pointerPosition);
			pointerPosition += SerializeSimple(mesh.NumberOfVertices, toData, pointerPosition);
			pointerPosition += SerializeVector(mesh.Vertices, toData, pointerPosition);
			pointerPosition += SerializeSimple(mesh.NumberOfIndices, toData, pointerPosition);
			pointerPosition += SerializeVector(mesh.Indices, toData, pointerPosition);
		}
	}

	inline void SSkeletalMeshFileHeader::Deserialize(const char* fromData)
	{
		U32 pointerPosition = 0;
		pointerPosition += DeserializeSimple(AssetType, fromData, pointerPosition);
		pointerPosition += DeserializeSimple(NumberOfMaterials, fromData, pointerPosition);
		pointerPosition += DeserializeSimple(NumberOfMeshes, fromData, pointerPosition);

		Meshes.reserve(NumberOfMeshes);
		for (U16 i = 0; i < NumberOfMeshes; i++)
		{
			Meshes.emplace_back();
			pointerPosition += DeserializeSimple(Meshes.back().NameLength, fromData, pointerPosition);
			pointerPosition += DeserializeString(Meshes.back().Name, fromData, Meshes.back().NameLength, pointerPosition);
			pointerPosition += DeserializeSimple(Meshes.back().NumberOfVertices, fromData, pointerPosition);
			pointerPosition += DeserializeVector(Meshes.back().Vertices, fromData, Meshes.back().NumberOfVertices, pointerPosition);
			pointerPosition += DeserializeSimple(Meshes.back().NumberOfIndices, fromData, pointerPosition);
			pointerPosition += DeserializeVector(Meshes.back().Indices, fromData, Meshes.back().NumberOfIndices, pointerPosition);
		}
	}

#pragma endregion

#pragma region Texture File

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
		U32 pointerPosition = 0;
		pointerPosition += SerializeSimple(AssetType, toData, pointerPosition);
		pointerPosition += SerializeSimple(MaterialNameLength, toData, pointerPosition);
		pointerPosition += SerializeString(MaterialName, toData, pointerPosition);
		pointerPosition += SerializeSimple(OriginalFormat, toData, pointerPosition);
		pointerPosition += SerializeSimple(MaterialConfiguration, toData, pointerPosition);
		pointerPosition += SerializeSimple(Suffix, toData, pointerPosition);
		pointerPosition += SerializeSimple(DataSize, toData, pointerPosition);
		SerializeString(Data, toData, pointerPosition);
	}

	inline void STextureFileHeader::Deserialize(const char* fromData)
	{
		U32 pointerPosition = 0;
		pointerPosition += DeserializeSimple(AssetType, fromData, pointerPosition);
		pointerPosition += DeserializeSimple(MaterialNameLength, fromData, pointerPosition);
		pointerPosition += DeserializeString(MaterialName, fromData, MaterialNameLength, pointerPosition);
		pointerPosition += DeserializeSimple(OriginalFormat, fromData, pointerPosition);
		pointerPosition += DeserializeSimple(MaterialConfiguration, fromData, pointerPosition);
		pointerPosition += DeserializeSimple(Suffix, fromData, pointerPosition);
		pointerPosition += DeserializeSimple(DataSize, fromData, pointerPosition);
		DeserializeString(Data, fromData, DataSize, pointerPosition);
	}

#pragma endregion
}
