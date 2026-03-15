// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "hvpch.h"
#include "GameplayTag.h"

#include <map>

namespace Havtorn
{
    struct STagNode
    {
        std::vector<Ref<STagNode>> Children;
        Ref<STagNode> Parent;
        SGameplayTag Tag = SGameplayTag::None;
    };

	class GGameplayTagManager
	{
        friend SGameplayTagContainer;
	public:

        CORE_API static bool Init(const std::string& initFilePath);

        CORE_API static SGameplayTag RequestTag(const std::string& newTag);
        CORE_API static SGameplayTagContainer RequestTagContainer(const std::vector<std::string>& tags);
        CORE_API static bool AnyTagsMatch(const SGameplayTagContainer& primary, const SGameplayTagContainer& secondary, const bool includeParentTags = true);
        CORE_API static bool AllTagsMatch(const SGameplayTagContainer& primary, const SGameplayTagContainer& secondary, const bool includeParentTags = true);
        CORE_API static bool NoTagsMatch(const SGameplayTagContainer& primary, const SGameplayTagContainer& secondary, const bool includeParentTags = true);

        CORE_API static const Ref<STagNode>& GetRootNode();
        CORE_API static bool ContainsTag(const SGameplayTag& tag, const SGameplayTagContainer& container, const bool includeParentTags = true);

	private:
		void RegisterTag(const std::string& newTag);
        std::vector<std::string> SplitTag(const std::string& tag) const;
        U32 Hash(const std::string& tag) const;
        std::vector<SGameplayTag> GetParentTags(const SGameplayTag& tag) const;
        
        static GGameplayTagManager* Instance;
        std::map<U32, Ref<STagNode>> HashToNode;
        Ref<STagNode> RootNode = nullptr;
	};
}
