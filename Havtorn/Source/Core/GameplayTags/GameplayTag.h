// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	struct SGameplayTag
	{
		friend class GGameplayTagManager;

		CORE_API static SGameplayTag None;

		std::string Name = "";
		U32 Hash = 0;
		U8 Depth = 0;

		bool operator==(const SGameplayTag& other) const { return Hash == other.Hash; }

		CORE_API const bool IsValid() const { return Hash != 0; }

		CORE_API [[nodiscard]] U32 GetSize() const;
		CORE_API void Serialize(char* toData, U64& pointerPosition) const;
		CORE_API void Deserialize(const char* fromData, U64& pointerPosition);
	};
	
	struct SGameplayTagContainer
	{
		SGameplayTagContainer() = default;
		CORE_API SGameplayTagContainer(const std::vector<SGameplayTag>& tags);
		CORE_API SGameplayTagContainer(const SGameplayTag& tag);

		CORE_API void AddTag(const SGameplayTag& tag);
		CORE_API void RemoveTag(const SGameplayTag& tag);
		CORE_API void ClearTags();

		CORE_API [[nodiscard]] U32 GetSize() const;
		CORE_API void Serialize(char* toData, U64& pointerPosition) const;
		CORE_API void Deserialize(const char* fromData, U64& pointerPosition);

		void UpdateParentContainer();

		std::vector<SGameplayTag> Tags;
		std::vector<SGameplayTag> ParentTags;
	};
}
