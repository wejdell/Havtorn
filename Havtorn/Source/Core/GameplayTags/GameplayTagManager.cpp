// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"

#include "GameplayTagManager.h"
#include "FileSystem.h"

#include <GeneralUtilities.h>

namespace Havtorn
{
	GGameplayTagManager* GGameplayTagManager::Instance = nullptr;

	bool GGameplayTagManager::Init(const std::string& initFilePath)
	{
		GGameplayTagManager::Instance = new GGameplayTagManager();

		Instance->RootNode = std::make_shared<STagNode>();
		Instance->RootNode->Tag = SGameplayTag{ .Name = "Root", .Hash = 0, .Depth = 0 };

		CJsonDocument initDocument = UFileSystem::OpenJson(initFilePath);
		std::vector<const char*> tagsFromFile = initDocument.GetArray<const char*>("Gameplay Tags");

		for (auto string : tagsFromFile)
			Instance->RegisterTag(string);

		return true;
	}

	SGameplayTag GGameplayTagManager::RequestTag(const std::string& newTag)
	{
		const U32 hash = UGeneralUtils::HashString(newTag);

		// NW: We could choose to only accept tags that are preloaded (registered) from the 
		// game config file. That way serialized out-of-date tags don't make it back into the manager. Probably.
		if (!Instance->HashToNode.contains(hash))
			Instance->RegisterTag(newTag);

		return Instance->HashToNode.at(hash)->Tag;
	}

	SGameplayTagContainer GGameplayTagManager::RequestTagContainer(const std::vector<std::string>& tags)
	{
		std::vector<SGameplayTag> gameplayTags;
		for (const std::string& tagString : tags)
			gameplayTags.push_back(RequestTag(tagString));

		return SGameplayTagContainer(gameplayTags);
	}

	bool GGameplayTagManager::AnyTagsMatch(const SGameplayTagContainer& primary, const SGameplayTagContainer& secondary, const bool includeParentTags)
	{
		for (const SGameplayTag& primaryTag : primary.Tags)
		{
			if (Instance->ContainsTag(primaryTag, secondary, includeParentTags))
				return true;
		}

		return false;
	}

	bool GGameplayTagManager::AllTagsMatch(const SGameplayTagContainer& primary, const SGameplayTagContainer& secondary, const bool includeParentTags)
	{
		for (const SGameplayTag& primaryTag : primary.Tags)
		{
			if (!Instance->ContainsTag(primaryTag, secondary, includeParentTags))
				return false;
		}

		return true;
	}

	bool GGameplayTagManager::NoTagsMatch(const SGameplayTagContainer& primary, const SGameplayTagContainer& secondary, const bool includeParentTags)
	{
		for (const SGameplayTag& primaryTag : primary.Tags)
		{
			if (Instance->ContainsTag(primaryTag, secondary, includeParentTags))
				return false;
		}

		return true;
	}

	const Ref<STagNode>& GGameplayTagManager::GetRootNode()
	{
		return Instance->RootNode;
	}

	void GGameplayTagManager::RegisterTag(const std::string& newTag)
	{
		if (newTag == "")
			return;

		if (HashToNode.contains(UGeneralUtils::HashString(newTag)))
			return;

		std::vector<std::string> splitTag = SplitTag(newTag);
		Ref<STagNode> currentNode = RootNode;

		for (U8 i = 0; i < STATIC_U8(splitTag.size()); i++)
		{
			const std::string& subTag = splitTag[i];
			if (auto it = std::ranges::find(currentNode->Children, subTag, [](const Ref<STagNode>& node) { return node->Tag.Name; }); it != currentNode->Children.end())
			{
				currentNode = *it;
				continue;
			}

			const U32 hash = UGeneralUtils::HashString(subTag);

			currentNode->Children.push_back(std::make_shared<STagNode>());
			auto& newNode = currentNode->Children.back();
			newNode->Parent = currentNode;
			newNode->Tag = SGameplayTag{ .Name = subTag, .Hash = hash, .Depth = STATIC_U8(i + 1) };

			HashToNode.emplace(hash, newNode);
			currentNode = newNode;
		}
	}

	std::vector<std::string> GGameplayTagManager::SplitTag(const std::string& tag) const
	{
		std::vector<std::string> splitTag;
		U64 index = 0;
		
		while (index != STATIC_U64(-1))
		{
			index = tag.find_first_of('.', index + 1);
			splitTag.emplace_back(tag.substr(0, index));
		}

		return splitTag;
	}

	std::vector<SGameplayTag> GGameplayTagManager::GetParentTags(const SGameplayTag& tag) const
	{
		std::vector<SGameplayTag> parentTags;
		if (!HashToNode.contains(tag.Hash))
			return parentTags;

		STagNode* currentNode = HashToNode.at(tag.Hash)->Parent.get();
		while (currentNode != nullptr && currentNode->Tag.Depth != 0)
		{
			parentTags.push_back(currentNode->Tag);
			currentNode = currentNode->Parent.get();
		}

		return parentTags;
	}

	bool GGameplayTagManager::ContainsTag(const SGameplayTag& tag, const SGameplayTagContainer& container, const bool includeParentTags)
	{
		if (!tag.IsValid() || !Instance->HashToNode.contains(tag.Hash) || tag == Instance->RootNode->Tag)
			return false;
		
		for (const SGameplayTag& containerTag : container.Tags)
		{
			// Explicit match
			if (tag.Hash == containerTag.Hash)
				return true;

			if (!includeParentTags)
				continue;

			if (tag.Depth == containerTag.Depth)
				continue;

			if (tag.Depth < containerTag.Depth)
			{
				if (auto parentIterator = std::ranges::find(container.ParentTags, tag.Hash, &SGameplayTag::Hash); parentIterator != container.ParentTags.end())
					return true;
			}

			if (tag.Depth > containerTag.Depth)
			{
				STagNode* currentNode = Instance->HashToNode.at(tag.Hash).get();
				while (currentNode != nullptr && currentNode->Tag.Depth != 0)
				{
					if (auto explicitIterator = std::ranges::find(container.Tags, currentNode->Tag.Hash, &SGameplayTag::Hash); explicitIterator != container.Tags.end())
						return true;
		
					currentNode = currentNode->Parent.get();				
				}
			}
		}

		return false;
	}
}
