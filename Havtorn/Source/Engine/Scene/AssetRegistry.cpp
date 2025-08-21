// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "AssetRegistry.h"
#include "FileSystem/FileHeaderDeclarations.h"
#include "ECS/GUIDManager.h"

#include "Graphics/RenderManager.h"
#include "Graphics/TextureBank.h"

#include "ModelImporter.h"

namespace Havtorn
{
    CAssetRegistry::CAssetRegistry()
    {
    }

    CAssetRegistry::~CAssetRegistry()
    {
    }

    bool CAssetRegistry::Init(CGraphicsFramework* framework, CRenderManager* renderManager)
    {
        RenderManager = renderManager;

        // Add null asset
        SAsset& asset = Registry[SAssetReference()] = SAsset();
        asset.Type = EAssetType::None;
    }


    /// Loading: from serialization using Accessor, should serialize that with UID and Path info so we can extract the path
    ///          UID for fast access, filepath for initial loading, source asset path for file watching? file watching should be done by asset registry
    /// Saving:  
    /// 
    /// 
    /// 

    SAsset* CAssetRegistry::RequestAsset(const SAssetReference& reference, const U64 requesterID)
    {
        if (Registry.contains(reference))
        {
            SAsset* loadedAsset = &Registry[reference];
            loadedAsset->References.insert(requesterID);
            return loadedAsset;
        }
    
        // TODO.NW: Load on different thread and return null asset while loading?
        return LoadAsset(reference, requesterID);
    }

    void CAssetRegistry::UnrequestAsset(const SAssetReference& reference, const U64 requesterID)
    {
        // TODO.NW: Check if asset is loading on separate thread, or check if requests are still there when finished loading?
        if (!Registry.contains(reference))
            return;

        SAsset* loadedAsset = &Registry[reference];
        loadedAsset->References.erase(requesterID);

        if (loadedAsset->References.empty())
            UnloadAsset(reference);
    }

    SAsset* CAssetRegistry::LoadAsset(const SAssetReference& reference, const U64 requesterID)
    {
        std::string filePath = reference.FilePath;
        const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
        char* data = new char[fileSize];

        GEngine::GetFileSystem()->Deserialize(filePath, data, STATIC_U32(fileSize));
        EAssetType type = EAssetType::None;
        U64 pointerPosition = 0;
        DeserializeData(type, data, pointerPosition);

        SAsset asset;
        asset.Type = type;
        asset.UID = reference.UID;
        asset.AssetPath = filePath;
        asset.References.insert(requesterID);

        switch (type)
        {
        case EAssetType::StaticMesh:
        {
            SStaticModelFileHeader assetFile;
            assetFile.Deserialize(data);
            SStaticMeshAsset meshAsset(assetFile);

            for (U16 i = 0; i < assetFile.NumberOfMeshes; i++)
            {
                const SStaticMesh& mesh = assetFile.Meshes[i];
                SDrawCallData& drawCallData = meshAsset.DrawCallData[i];

                // TODO.NW: Check for existing buffers
                drawCallData.VertexBufferIndex = RenderManager->RenderStateManager.AddVertexBuffer(mesh.Vertices);
                drawCallData.IndexBufferIndex = RenderManager->RenderStateManager.AddIndexBuffer(mesh.Indices);
                drawCallData.VertexStrideIndex = 0;
                drawCallData.VertexOffsetIndex = 0;

                for (const SStaticMeshVertex& vertex : mesh.Vertices)
                {
                    meshAsset.BoundsMin.X = UMath::Min(vertex.x, meshAsset.BoundsMin.X);
                    meshAsset.BoundsMin.Y = UMath::Min(vertex.y, meshAsset.BoundsMin.Y);
                    meshAsset.BoundsMin.Z = UMath::Min(vertex.z, meshAsset.BoundsMin.Z);

                    meshAsset.BoundsMax.X = UMath::Max(vertex.x, meshAsset.BoundsMax.X);
                    meshAsset.BoundsMax.Y = UMath::Max(vertex.y, meshAsset.BoundsMax.Y);
                    meshAsset.BoundsMax.Z = UMath::Max(vertex.z, meshAsset.BoundsMax.Z);
                }
            }

            asset.Data = meshAsset;
        }
        break;
        case EAssetType::SkeletalMesh:
        {
            SSkeletalModelFileHeader assetFile;
            assetFile.Deserialize(data);
            SSkeletalMeshAsset meshAsset(assetFile);

            for (U16 i = 0; i < assetFile.NumberOfMeshes; i++)
            {
                const SSkeletalMesh& mesh = assetFile.Meshes[i];
                SDrawCallData& drawCallData = meshAsset.DrawCallData[i];

                // TODO.NW: Check for existing buffers
                drawCallData.VertexBufferIndex = RenderManager->RenderStateManager.AddVertexBuffer(mesh.Vertices);
                drawCallData.IndexBufferIndex = RenderManager->RenderStateManager.AddIndexBuffer(mesh.Indices);
                drawCallData.VertexStrideIndex = 2;
                drawCallData.VertexOffsetIndex = 0;

                for (const SSkeletalMeshVertex& vertex : mesh.Vertices)
                {
                    meshAsset.BoundsMin.X = UMath::Min(vertex.x, meshAsset.BoundsMin.X);
                    meshAsset.BoundsMin.Y = UMath::Min(vertex.y, meshAsset.BoundsMin.Y);
                    meshAsset.BoundsMin.Z = UMath::Min(vertex.z, meshAsset.BoundsMin.Z);

                    meshAsset.BoundsMax.X = UMath::Max(vertex.x, meshAsset.BoundsMax.X);
                    meshAsset.BoundsMax.Y = UMath::Max(vertex.y, meshAsset.BoundsMax.Y);
                    meshAsset.BoundsMax.Z = UMath::Max(vertex.z, meshAsset.BoundsMax.Z);
                }
            }

            asset.Data = meshAsset;
        }
        break;
        case EAssetType::Texture:
        {
            STextureFileHeader assetFile;
            assetFile.Deserialize(data);
            STextureAsset textureAsset(assetFile);
            textureAsset.RenderTexture = RenderManager->RenderTextureFactory.CreateStaticTexture(filePath);
            asset.Data = textureAsset;
        }
        break;
        case EAssetType::Material:
        {
            SMaterialAssetFileHeader assetFile;
            assetFile.Deserialize(data);
            asset.Data = SGraphicsMaterialAsset(assetFile);
        }
        break;
        case EAssetType::Animation:
        {
            SSkeletalAnimationFileHeader assetFile;
            assetFile.Deserialize(data);
            asset.Data = SSkeletalAnimationAsset(assetFile);
        }
        break;
        case EAssetType::SpriteAnimation:
        case EAssetType::AudioOneShot:
        case EAssetType::AudioCollection:
        case EAssetType::VisualFX:
        case EAssetType::Scene:
        case EAssetType::Sequencer:
        case EAssetType::Script:
            // TODO.NW: Use magic enum to write out enum type
            HV_LOG_WARN("CAssetRegistry: Asset Resolving for asset type SEE_TODO is not yet implemented.");
            delete[] data;
            return &Registry[SAssetReference()];
        }
        delete[] data;

        // TODO.NW: Set asset source data and bind filewatchers?

        Registry.emplace(reference, asset);
        return &Registry[reference];
    }

    void CAssetRegistry::UnloadAsset(const SAssetReference& reference)
    {
        // TODO.NW: If any filewatchers are watching the source of this asset, remove them now
        Registry.erase(reference);
    }

    std::string CAssetRegistry::ImportAsset(const std::string& filePath, const std::string& destinationPath, const SSourceAssetData& sourceData)
    {
        std::string hvaPath = "INVALID_PATH";
        switch (sourceData.AssetType)
        {
        case EAssetType::StaticMesh: // fallthrough
        case EAssetType::SkeletalMesh: // fallthrough
        case EAssetType::Animation:
        {
            hvaPath = SaveAsset(destinationPath, UModelImporter::ImportFBX(filePath, sourceData));
        }
        break;
        case EAssetType::Texture:
        {
            std::string textureFileData;
            GEngine::GetFileSystem()->Deserialize(filePath, textureFileData);

            ETextureFormat format = {};
            if (const std::string extension = UGeneralUtils::ExtractFileExtensionFromPath(filePath); extension == "dds")
                format = ETextureFormat::DDS;
            else if (extension == "tga")
                format = ETextureFormat::TGA;

            STextureFileHeader fileHeader;
            fileHeader.AssetType = EAssetType::Texture;

            fileHeader.MaterialName = UGeneralUtils::ExtractFileBaseNameFromPath(filePath);
            fileHeader.OriginalFormat = format;
            fileHeader.Suffix = filePath[filePath.find_last_of(".") - 1];
            fileHeader.Data = std::move(textureFileData);

            // TODO.NW: Make sure file header gets source data set, in ModelImport as well

            hvaPath = SaveAsset(destinationPath, fileHeader);
        }
        break;
        case EAssetType::AudioOneShot:
            break;
        case EAssetType::AudioCollection:
            break;
        }

        if (hvaPath == "INVALID_PATH")
            HV_LOG_WARN("CAssetRegistry::ImportAsset: The chosen source data refers to an asset type doesn't have import logic implemented. Could not create asset at %s for %s!", destinationPath.c_str(), filePath.c_str());

        return hvaPath;
    }

    std::string CAssetRegistry::SaveAsset(const std::string& destinationPath, const SAssetFileHeader& fileHeader)
    {
        // TODO.NW: See if we can make char stream we can then convert to data buffer,
        // so as to not repeat the logic for every case

        std::string hvaPath = "INVALID_PATH";
        if (std::holds_alternative<SStaticModelFileHeader>(fileHeader))
        {
            SStaticModelFileHeader header = std::get<SStaticModelFileHeader>(fileHeader);
            const auto data = new char[header.GetSize()];
            header.Serialize(data);
            hvaPath = destinationPath + header.Name + ".hva";
            GEngine::GetFileSystem()->Serialize(hvaPath, &data[0], header.GetSize());
            delete[] data;
        }
        else if (std::holds_alternative<SSkeletalModelFileHeader>(fileHeader))
        {
            SSkeletalModelFileHeader header = std::get<SSkeletalModelFileHeader>(fileHeader);
            const auto data = new char[header.GetSize()];
            header.Serialize(data);
            hvaPath = destinationPath + header.Name + ".hva";
            GEngine::GetFileSystem()->Serialize(hvaPath, &data[0], header.GetSize());
            delete[] data;
        }
        else if (std::holds_alternative<SSkeletalAnimationFileHeader>(fileHeader))
        {
            SSkeletalAnimationFileHeader header = std::get<SSkeletalAnimationFileHeader>(fileHeader);
            const auto data = new char[header.GetSize()];
            header.Serialize(data);
            hvaPath = destinationPath + header.Name + ".hva";
            GEngine::GetFileSystem()->Serialize(hvaPath, &data[0], header.GetSize());
            delete[] data;
        }
        else if (std::holds_alternative<STextureFileHeader>(fileHeader))
        {
            STextureFileHeader header = std::get<STextureFileHeader>(fileHeader);
            const auto data = new char[header.GetSize()];
            header.Serialize(data);
            hvaPath = destinationPath + header.MaterialName + ".hva";
            GEngine::GetFileSystem()->Serialize(hvaPath, &data[0], header.GetSize());
            delete[] data;
        }
        else if (std::holds_alternative<SMaterialAssetFileHeader>(fileHeader))
        {
            SMaterialAssetFileHeader header = std::get<SMaterialAssetFileHeader>(fileHeader);
            const auto data = new char[header.GetSize()];
            header.Serialize(data);
            hvaPath = destinationPath + header.MaterialName + ".hva";
            GEngine::GetFileSystem()->Serialize(hvaPath, &data[0], header.GetSize());
            delete[] data;
        }

        if (hvaPath == "INVALID_PATH")
            HV_LOG_WARN("CAssetRegistry::SaveAsset: The chosen file header had no serialization implemented. Could not create asset at %s!", destinationPath.c_str());

        return hvaPath;
    }

    //U64 CAssetRegistry::Register(const std::string& assetPath)
    //{
    //    if (!Registry.contains(assetPath))
    //        Registry.emplace(assetPath, SReferenceCounter(UGUIDManager::Generate()));

    //    SReferenceCounter& counter = Registry[assetPath];
    //    counter.NumberOfReferences++;
    //    
    //    return counter.GUID;
    //}

    //std::vector<U64> CAssetRegistry::Register(const std::vector<std::string>& assetPaths)
    //{
    //    std::vector<U64> guids;
    //    for (const std::string& assetPath : assetPaths)
    //        guids.push_back(Register(assetPath));

    //    return guids;
    //}

    //void CAssetRegistry::Unregister(const std::string& assetPath)
    //{
    //    if (!Registry.contains(assetPath))
    //        return;

    //    SReferenceCounter& counter = Registry[assetPath];
    //    if (counter.NumberOfReferences > 0)
    //        counter.NumberOfReferences--;

    //    if (counter.NumberOfReferences == 0)
    //        Registry.erase(assetPath);
    //}

    //const std::string& CAssetRegistry::GetAssetPath(const U64& guid)
    //{
    //    for (const auto& [assetPath, referenceCounter] : Registry)
    //    {
    //        if (referenceCounter.GUID == guid)
    //            return assetPath;
    //    }

    //    return InvalidPath;
    //}

    //std::vector<std::string> CAssetRegistry::GetAssetPaths(const std::vector<U64>& guids)
    //{
    //    std::vector<std::string> assetPaths;
    //    for (U64 guid : guids)
    //        assetPaths.push_back(GetAssetPath(guid));

    //    return assetPaths;
    //}

    //bool CAssetRegistry::IsAssetRegistered(const std::string& assetPath)
    //{
    //    return Registry.contains(assetPath);
    //}

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
