// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "AssetRegistry.h"
#include "FileSystem/FileHeaderDeclarations.h"
#include "ECS/GUIDManager.h"

namespace Havtorn
{
    U64 CAssetRegistry::Register(const std::string& assetPath)
    {
        if (!Registry.contains(assetPath))
            Registry.emplace(assetPath, SReferenceCounter(UGUIDManager::Generate()));

        SReferenceCounter& counter = Registry[assetPath];
        counter.NumberOfReferences++;
        
        return counter.GUID;
    }

    std::vector<U64> CAssetRegistry::Register(const std::vector<std::string>& assetPaths)
    {
        std::vector<U64> guids;
        for (const std::string& assetPath : assetPaths)
            guids.push_back(Register(assetPath));

        return guids;
    }

    void CAssetRegistry::Unregister(const std::string& assetPath)
    {
        if (!Registry.contains(assetPath))
            return;

        SReferenceCounter& counter = Registry[assetPath];
        if (counter.NumberOfReferences > 0)
            counter.NumberOfReferences--;

        if (counter.NumberOfReferences == 0)
            Registry.erase(assetPath);
    }

    const std::string& CAssetRegistry::GetAssetPath(const U64& guid)
    {
        for (const auto& [assetPath, referenceCounter] : Registry)
        {
            if (referenceCounter.GUID == guid)
                return assetPath;
        }

        return InvalidPath;
    }

    std::vector<std::string> CAssetRegistry::GetAssetPaths(const std::vector<U64>& guids)
    {
        std::vector<std::string> assetPaths;
        for (U64 guid : guids)
            assetPaths.push_back(GetAssetPath(guid));

        return assetPaths;
    }

    U32 CAssetRegistry::GetSize() const
    {
        U32 size = 0;
        size += GetDataSize(STATIC_U32(Registry.size()));

        for (const auto& [assetPath, referenceCounter] : Registry)
        {
            size += GetDataSize(assetPath);
            size += GetDataSize(referenceCounter);
        }

        return size;
    }

    void CAssetRegistry::Serialize(char* toData, U64& pointerPosition) const
    {   
        SerializeData(STATIC_U32(Registry.size()), toData, pointerPosition);

        for (const auto& [assetPath, referenceCounter] : Registry)
        {
            SerializeData(assetPath, toData, pointerPosition);
            SerializeData(referenceCounter, toData, pointerPosition);
        }
    }

    void CAssetRegistry::Deserialize(const char* fromData, U64& pointerPosition)
    {
        Registry.clear();

        U32 numberOfEntries = 0;
        DeserializeData(numberOfEntries, fromData, pointerPosition);

        for (U32 i = 0; i < numberOfEntries; i++)
        {
            std::string assetPath = "";
            DeserializeData(assetPath, fromData, pointerPosition);
            SReferenceCounter referenceCounter{0};
            DeserializeData(referenceCounter, fromData, pointerPosition);

            Registry.emplace(assetPath, referenceCounter);
        }
    }
}
