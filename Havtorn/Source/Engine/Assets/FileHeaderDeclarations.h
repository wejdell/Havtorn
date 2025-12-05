// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include "Graphics/GraphicsStructs.h"
#include "Graphics/GraphicsEnums.h"
#include "Scene/Scene.h"
#include "Assets/SequencerAsset.h"
#include "HexRune/HexRune.h"

#include <variant>

namespace Havtorn
{
	struct SAssetReference
	{
		U32 UID = 0;
		std::string FilePath = "NullAsset";

		SAssetReference() = default;
		explicit SAssetReference(const std::string& filePath)
		{
			FilePath = UGeneralUtils::ConvertToPlatformAgnosticPath(filePath);
			U32 prime = 0x1000193;
			UID = 0x811c9dc5;

			for (U64 i = 0; i < FilePath.size(); ++i)
			{
				U8 value = FilePath[i];
				UID = UID ^ value;
				UID *= prime;
			}
		}

		bool operator==(const SAssetReference& other) const { return UID == other.UID && FilePath == other.FilePath; }
		
		const bool IsValid() const { return UID != 0 && FilePath != ""; }

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);

		static std::vector<U32> GetIDs(const std::vector<SAssetReference>& references);
		static std::vector<std::string> GetPaths(const std::vector<SAssetReference>& references);
		static std::vector<SAssetReference> MakeVectorFromPaths(const std::vector<std::string>& paths);
	};

	inline U32 SAssetReference::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(UID);
		size += GetDataSize(FilePath);
		return size;
	}

	inline void SAssetReference::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(UID, toData, pointerPosition);
		SerializeData(FilePath, toData, pointerPosition);
	}

	inline void SAssetReference::Deserialize(const char* fromData, U64& pointerPosition)
	{
		DeserializeData(UID, fromData, pointerPosition);
		DeserializeData(FilePath, fromData, pointerPosition);
	}

	inline std::vector<U32> SAssetReference::GetIDs(const std::vector<SAssetReference>& references)
	{
		// TODO.NW: Still want an algo library for operations like this
		std::vector<U32> ids;
		for (const SAssetReference& ref : references)
			ids.push_back(ref.UID);

		return ids;
	}

	inline std::vector<std::string> SAssetReference::GetPaths(const std::vector<SAssetReference>& references)
	{
		// TODO.NW: Still want an algo library for operations like this
		std::vector<std::string> paths;
		for (const SAssetReference& ref : references)
			paths.push_back(ref.FilePath);

		return paths;
	}

	inline std::vector<SAssetReference> SAssetReference::MakeVectorFromPaths(const std::vector<std::string>& paths)
	{
		std::vector<SAssetReference> references;
		
		for (const std::string& path : paths)
			references.emplace_back(path);

		return references;
	}

	struct SSourceAssetData
	{
		EAssetType AssetType = EAssetType::None;
		CHavtornStaticString<128> SourcePath;
		CHavtornStaticString<128> AssetDependencyPath;
		F32 ImportScale = 1.0f;

		const bool IsValid() const { return SourcePath.Length() != 0; }
	};

	struct SStaticModelFileHeader
	{
		EAssetType AssetType = EAssetType::StaticMesh;
		std::string Name;
		// TODO.NW: Remove all UIDs, should not be needed anymore
		U32 UID = 0;
		SSourceAssetData SourceData;
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
		size += GetDataSize(Name);
		size += GetDataSize(UID);
		size += GetDataSize(SourceData);
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
		SerializeData(Name, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);
		SerializeData(SourceData, toData, pointerPosition);
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
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(UID, fromData, pointerPosition);
		DeserializeData(SourceData, fromData, pointerPosition);
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
		U32 UID = 0;
		SSourceAssetData SourceData;
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
		size += GetDataSize(Name);
		size += GetDataSize(UID);
		size += GetDataSize(SourceData);
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
		SerializeData(Name, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);
		SerializeData(SourceData, toData, pointerPosition);
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
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(UID, fromData, pointerPosition);
		DeserializeData(SourceData, fromData, pointerPosition);
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
		U32 UID = 0;
		SSourceAssetData SourceData;
		U32 DurationInTicks = 0;
		U32 TickRate = 0;
		U32 NumberOfBones = 0;
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
		size += GetDataSize(UID);
		size += GetDataSize(SourceData);
		size += GetDataSize(DurationInTicks);
		size += GetDataSize(TickRate);
		size += GetDataSize(NumberOfBones);

		for (auto& track : BoneAnimationTracks)
			size += track.GetSize();

		return size;
	}

	inline void SSkeletalAnimationFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);
		SerializeData(SourceData, toData, pointerPosition);
		SerializeData(DurationInTicks, toData, pointerPosition);
		SerializeData(TickRate, toData, pointerPosition);
		SerializeData(NumberOfBones, toData, pointerPosition);

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
		DeserializeData(UID, fromData, pointerPosition);
		DeserializeData(SourceData, fromData, pointerPosition);
		DeserializeData(DurationInTicks, fromData, pointerPosition);
		DeserializeData(TickRate, fromData, pointerPosition);
		DeserializeData(NumberOfBones, fromData, pointerPosition);

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
		std::string Name = "";
		// TODO.NW: remove this, should not be needed anymore now that the asset registry handles import
		U32 UID = 0;
		SSourceAssetData SourceData;
		ETextureFormat OriginalFormat = ETextureFormat::DDS;
		// TODO.NW: remove this, should not be needed anymore now that the asset registry handles import
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
		size += GetDataSize(Name);
		size += GetDataSize(UID);
		size += GetDataSize(SourceData);
		size += GetDataSize(OriginalFormat);
		size += GetDataSize(Suffix);
		size += GetDataSize(Data);

		return size;
	}

	inline void STextureFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);
		SerializeData(SourceData, toData, pointerPosition);
		SerializeData(OriginalFormat, toData, pointerPosition);
		SerializeData(Suffix, toData, pointerPosition);
		SerializeData(Data, toData, pointerPosition);
	}

	inline void STextureFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(UID, fromData, pointerPosition);
		DeserializeData(SourceData, fromData, pointerPosition);
		DeserializeData(OriginalFormat, fromData, pointerPosition);
		DeserializeData(Suffix, fromData, pointerPosition);
		DeserializeData(Data, fromData, pointerPosition);
	}

	struct STextureCubeFileHeader
	{
		EAssetType AssetType = EAssetType::TextureCube;
		std::string Name = "";
		SSourceAssetData SourceData;
		ETextureFormat OriginalFormat = ETextureFormat::DDS;
		std::string Data = "";

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 STextureCubeFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += GetDataSize(SourceData);
		size += GetDataSize(OriginalFormat);
		size += GetDataSize(Data);

		return size;
	}

	inline void STextureCubeFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(SourceData, toData, pointerPosition);
		SerializeData(OriginalFormat, toData, pointerPosition);
		SerializeData(Data, toData, pointerPosition);
	}

	inline void STextureCubeFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(SourceData, fromData, pointerPosition);
		DeserializeData(OriginalFormat, fromData, pointerPosition);
		DeserializeData(Data, fromData, pointerPosition);
	}

	// TODO.NW: Rename to SMaterialFileHeader?
	struct SMaterialAssetFileHeader
	{
		EAssetType AssetType = EAssetType::Material;
		std::string Name = "";
		U32 UID = 0;
		SOfflineGraphicsMaterial Material;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData);
	};

	inline U32 SMaterialAssetFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += GetDataSize(UID);
		size += Material.GetSize();

		return size;
	}

	inline void SMaterialAssetFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);

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
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(UID, fromData, pointerPosition);

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
		U32 UID = 0;
		CScene* Scene = nullptr;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition, CScene* outScene);
	};

	inline U32 SSceneFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(UID);
		size += Scene->GetSize();

		return size;
	}

	inline void SSceneFileHeader::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);
		Scene->Serialize(toData, pointerPosition);
	}

	inline void SSceneFileHeader::Deserialize(const char* fromData, U64& pointerPosition, CScene* outScene)
	{
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(UID, fromData, pointerPosition);
		outScene->Deserialize(fromData, pointerPosition);
	}

	struct SSpriteAnimationClipFileHeader
	{
		EAssetType AssetType = EAssetType::SpriteAnimation;
		std::string Name;
		U32 UID = 0;
		SSourceAssetData SourceData;
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
		size += GetDataSize(Name);
		size += GetDataSize(UID);
		size += GetDataSize(UVRects);
		size += GetDataSize(Durations);
		size += GetDataSize(IsLooping);
		return size;
	}

	inline void SSpriteAnimationClipFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		SerializeData(UID, toData, pointerPosition);
		SerializeData(UVRects, toData, pointerPosition);
		SerializeData(Durations, toData, pointerPosition);
		SerializeData(IsLooping, toData, pointerPosition);
	}

	inline void SSpriteAnimationClipFileHeader::Deserialize(const char* fromData)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		DeserializeData(UID, fromData, pointerPosition);
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
		std::string Name;

		HexRune::SScript* Script = nullptr;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData) const;
		void Deserialize(const char* fromData, HexRune::SScript* outScript);
	};

	inline U32 SScriptFileHeader::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(AssetType);
		size += GetDataSize(Name);
		size += Script->GetSize();
		return size;
	}

	inline void SScriptFileHeader::Serialize(char* toData) const
	{
		U64 pointerPosition = 0;
		SerializeData(AssetType, toData, pointerPosition);
		SerializeData(Name, toData, pointerPosition);
		Script->Serialize(toData, pointerPosition);
	}

	inline void SScriptFileHeader::Deserialize(const char* fromData, HexRune::SScript* outScript)
	{
		U64 pointerPosition = 0;
		DeserializeData(AssetType, fromData, pointerPosition);
		DeserializeData(Name, fromData, pointerPosition);
		outScript->Deserialize(fromData, pointerPosition);
	}

	// TODO.NW: Make an alias for a reasonable name instead of monostate
	typedef std::variant<std::monostate, SStaticModelFileHeader, SSkeletalModelFileHeader, SSkeletalAnimationFileHeader, STextureFileHeader, STextureCubeFileHeader, SMaterialAssetFileHeader, SSceneFileHeader, SScriptFileHeader> SAssetFileHeader;
}
