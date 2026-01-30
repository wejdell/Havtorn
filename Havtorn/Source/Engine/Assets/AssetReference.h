// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include <GeneralUtilities.h>

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
		static std::vector<std::string> GetPaths(const std::vector<SAssetReference*>& references);
		static std::vector<SAssetReference*> ConvertToPointers(SAssetReference& reference);
		static std::vector<SAssetReference*> ConvertToPointers(std::vector<SAssetReference>& references);
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

	inline std::vector<std::string> SAssetReference::GetPaths(const std::vector<SAssetReference*>& references)
	{
		// TODO.NW: Still want an algo library for operations like this
		std::vector<std::string> paths;
		for (const SAssetReference* ref : references)
		{
			if (ref != nullptr)
				paths.push_back(ref->FilePath);
		}

		return paths;
	}

	inline std::vector<SAssetReference*> SAssetReference::ConvertToPointers(SAssetReference& reference)
	{
		return { &reference };
	}

	inline std::vector<SAssetReference*> SAssetReference::ConvertToPointers(std::vector<SAssetReference>& references)
	{
		std::vector<SAssetReference*> pointers;
		for (SAssetReference& ref : references)
			pointers.push_back(&ref);

		return pointers;
	}

	inline std::vector<SAssetReference> SAssetReference::MakeVectorFromPaths(const std::vector<std::string>& paths)
	{
		std::vector<SAssetReference> references;

		for (const std::string& path : paths)
			references.emplace_back(path);

		return references;
	}
}