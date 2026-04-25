// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"

#include "GameplayTag.h"
#include "GameplayTagManager.h"

namespace Havtorn
{
	SGameplayTag SGameplayTag::None = SGameplayTag();
	
	U32 SGameplayTag::GetSize() const
	{
		U32 size = GetDataSize(Name);
		return size;
	}

	void SGameplayTag::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(Name, toData, pointerPosition);
	}

	void SGameplayTag::Deserialize(const char* fromData, U64& pointerPosition)
	{
		std::string tagString;
		DeserializeData(tagString, fromData, pointerPosition);

		*this = GGameplayTagManager::RequestTag(tagString);
	}

	SGameplayTagContainer::SGameplayTagContainer(const std::vector<SGameplayTag>& tags)
		: Tags(tags)
	{
		UpdateParentContainer();
	}

	SGameplayTagContainer::SGameplayTagContainer(const SGameplayTag& tag)
		: SGameplayTagContainer(std::vector<SGameplayTag>{tag})
	{
	}

	void SGameplayTagContainer::AddTag(const SGameplayTag& tag)
	{
		if (auto it = std::ranges::find(Tags, tag.Hash, &SGameplayTag::Hash); it != Tags.end())
			return;

		Tags.emplace_back(tag);
		UpdateParentContainer();
	}

	void SGameplayTagContainer::RemoveTag(const SGameplayTag& tag)
	{
		if (auto it = std::ranges::find(Tags, tag.Hash, &SGameplayTag::Hash); it == Tags.end())
			return;

		std::erase(Tags, tag);
		UpdateParentContainer();
	}

	void SGameplayTagContainer::ClearTags()
	{
		Tags.clear();
		ParentTags.clear();
	}

	U32 SGameplayTagContainer::GetSize() const
	{
		U32 size = 0;
		size += GetDataSize(U32());
		for (const SGameplayTag& tag : Tags)
			size += GetDataSize(tag.Name);

		return size;
	}

	void SGameplayTagContainer::Serialize(char* toData, U64& pointerPosition) const
	{
		SerializeData(STATIC_U32(Tags.size()), toData, pointerPosition);
		for (const SGameplayTag& tag : Tags)
			SerializeData(tag.Name, toData, pointerPosition);

		// NW: Parent tags are derived from Manager when requesting container
	}

	void SGameplayTagContainer::Deserialize(const char* fromData, U64& pointerPosition)
	{
		U32 numTags = 0;
		DeserializeData(numTags, fromData, pointerPosition);

		std::vector<std::string> tagStrings;
		tagStrings.resize(numTags);
		for (std::string& tag : tagStrings)
		{
			DeserializeData(tag, fromData, pointerPosition);
		}

		*this = GGameplayTagManager::RequestTagContainer(tagStrings);
	}

	void SGameplayTagContainer::UpdateParentContainer()
	{
		ParentTags.clear();
		for (const SGameplayTag& tag : Tags)
		{
			std::vector<SGameplayTag> parentTags = GGameplayTagManager::Instance->GetParentTags(tag);
			for (const SGameplayTag& parentTag : parentTags)
			{
				if (auto it = std::ranges::find(ParentTags, parentTag.Hash, &SGameplayTag::Hash); it == ParentTags.end())
					ParentTags.push_back(parentTag);
			}
		}
	}
}
