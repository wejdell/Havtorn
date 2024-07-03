// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "AssetRegistry.h"
#include "FileSystem/FileHeaderDeclarations.h"

namespace Havtorn
{
    void CAssetRegistry::Register(const std::string& assetPath, SAssetReferenceCounter&& counter)
    {
        if (!Registry.contains(assetPath))
            Registry.emplace(assetPath, std::vector<SAssetReferenceCounter>());

        std::vector<SAssetReferenceCounter>& counters = Registry[assetPath];
        
        if (std::find(counters.begin(), counters.end(), counter) == counters.end())
            counters.emplace_back(std::move(counter));
    }

    void CAssetRegistry::Register(const std::vector<std::string>& assetPaths, const SAssetReferenceCounter& counter)
    {
        for (U8 i = 0; i < static_cast<U8>(assetPaths.size()); i++)
        {
            const std::string& assetPath = assetPaths[i];
            SAssetReferenceCounter newCounter = counter;
            newCounter.ComponentSubIndex = i;
            Register(assetPath, std::move(newCounter));
        }
    }

    void CAssetRegistry::Unregister(const std::string& assetPath, const SAssetReferenceCounter& counter)
    {
        if (!Registry.contains(assetPath))
            return;

        std::vector<SAssetReferenceCounter>& counters = Registry[assetPath];
        auto it = std::find(counters.begin(), counters.end(), counter);

        if (it != counters.end())
            counters.erase(it);

        if (counters.empty())
            Registry.erase(assetPath);
    }

    const std::string& CAssetRegistry::GetAssetPath(const SAssetReferenceCounter& counter)
    {
        for (const auto& assetRef : Registry)
        {
            const std::vector<SAssetReferenceCounter>& counters = assetRef.second;
            if (std::find(counters.begin(), counters.end(), counter) != counters.end())
                return assetRef.first;
        }

        return InvalidPath;
    }

    std::vector<std::string> CAssetRegistry::GetAssetPaths(const SAssetReferenceCounter& counter)
    {
        std::vector<std::string> assetPaths;
        
        struct SUniqueRegistryEntry
        {
            std::string AssetPath;
            SAssetReferenceCounter Counter;
        };

        std::vector<SUniqueRegistryEntry> tempEntries;

        for (const auto& assetRef : Registry)
        {
            const std::vector<SAssetReferenceCounter>& counters = assetRef.second;
            for (const SAssetReferenceCounter& innerCounter : counters)
            {
                if (innerCounter.HasSameComponent(counter))
                    //assetPaths.emplace_back(assetRef.first);
                    tempEntries.emplace_back(SUniqueRegistryEntry(assetRef.first, assetRef.second[0]));
            }
        }

        std::sort(tempEntries.begin(), tempEntries.end(), [] (const SUniqueRegistryEntry& a, const SUniqueRegistryEntry& b) { return a.Counter.ComponentSubIndex < b.Counter.ComponentSubIndex; });

        for (const SUniqueRegistryEntry& entry : tempEntries)
        {
            assetPaths.emplace_back(entry.AssetPath);
        }

        return assetPaths;
    }

    U32 CAssetRegistry::GetSize(I64 sceneIndex) const
    {
        U32 size = 0;
        size += sizeof(U32);

        for (const auto& assetRef : Registry)
        {
            const std::vector<SAssetReferenceCounter>& counters = assetRef.second;
            for (const SAssetReferenceCounter& counter : counters)
            {
                if (counter.SceneIndex == sceneIndex)
                {
                    size += sizeof(U32);
                    size += sizeof(char) * static_cast<U32>(assetRef.first.length());

                    size += sizeof(SAssetReferenceCounter) * static_cast<U32>(counters.size());
                }
            }
        }

        return size;
    }

    void CAssetRegistry::Serialize(I64 sceneIndex, char* toData, U64& pointerPosition) const
    {        
        // NR: Pre-pass to know how many entries are expected from the file
        U32 entriesForScene = 0;
        for (const auto& assetRef : Registry)
        {
            const std::vector<SAssetReferenceCounter>& counters = assetRef.second;
            for (const SAssetReferenceCounter& counter : counters)
            {
                if (counter.SceneIndex == sceneIndex)
                    entriesForScene++;
            }
        }

        SerializeData(entriesForScene, toData, pointerPosition);

        for (const auto& assetRef : Registry)
        {
            const std::vector<SAssetReferenceCounter>& counters = assetRef.second;
            for (const SAssetReferenceCounter& counter : counters)
            {
                if (counter.SceneIndex == sceneIndex)
                {
                    SerializeData(static_cast<U32>(assetRef.first.length()), toData, pointerPosition);
                    SerializeData(assetRef.first, toData, pointerPosition);

                    SerializeData(counter, toData, pointerPosition);
                }
            }
        }
    }

    void CAssetRegistry::Deserialize(I64 sceneIndex, const char* fromData, U64& pointerPosition)
    {
        U32 entriesForScene = 0;
        DeserializeData(entriesForScene, fromData, pointerPosition);

        for (U32 i = 0; i < entriesForScene; i++)
        {
            U32 assetRefNameLength = 0;
            DeserializeData(assetRefNameLength, fromData, pointerPosition);
            std::string assetRefName = "";
            DeserializeData(assetRefName, fromData, assetRefNameLength, pointerPosition);

            SAssetReferenceCounter newCounter = {};
            DeserializeData(newCounter, fromData, pointerPosition);

            newCounter.SceneIndex = static_cast<U8>(sceneIndex);
            Register(std::move(assetRefName), std::move(newCounter));
        }
    }

    bool SAssetReferenceCounter::HasSameComponent(const SAssetReferenceCounter& other) const
    {
        return (ComponentIndex == other.ComponentIndex) && (ComponentType == other.ComponentType) && (SceneIndex == other.SceneIndex);
    }

    bool SAssetReferenceCounter::operator==(const SAssetReferenceCounter& other) const
    {
        return (ComponentIndex == other.ComponentIndex) && (ComponentSubIndex == other.ComponentSubIndex) && (ComponentType == other.ComponentType) && (SceneIndex == other.SceneIndex);
    }
}
