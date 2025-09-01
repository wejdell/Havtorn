// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "AssetRegistry.h"
// TODO.NW: Unify these asset files under the same directory
#include "FileSystem/FileHeaderDeclarations.h"
#include "Core/RuntimeAssetDeclarations.h"
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

    bool CAssetRegistry::Init(CRenderManager* renderManager)
    {
        RenderManager = renderManager;

        // Add null asset
        LoadedAssets.emplace(0, SAsset());

        RefreshDatabase();

        return true;
    }

    SAsset* CAssetRegistry::RequestAsset(const SAssetReference& assetRef, const U64 requesterID)
    {
        // TODO.NW: Load on different thread and return null asset while loading?
        if (!LoadedAssets.contains(assetRef.UID) && !LoadAsset(assetRef))
            return &LoadedAssets[0];
        
        SAsset* loadedAsset = &LoadedAssets[assetRef.UID];
        loadedAsset->Requesters.insert(requesterID);
        RequestDependencies(assetRef.UID, requesterID);

        return loadedAsset;
    }

    void CAssetRegistry::UnrequestAsset(const SAssetReference& assetRef, const U64 requesterID)
    {
        // TODO.NW: Check if asset is loading on separate thread, or check if requests are still there when finished loading?
        if (!LoadedAssets.contains(assetRef.UID))
            return;

        SAsset* loadedAsset = &LoadedAssets[assetRef.UID];
        loadedAsset->Requesters.erase(requesterID);
        UnrequestDependencies(assetRef.UID, requesterID);

        if (loadedAsset->Requesters.empty())
            UnloadAsset(assetRef);
    }

    std::vector<SAsset*> CAssetRegistry::RequestAssets(const std::vector<SAssetReference>& assetRefs, const U64 requesterID)
    {
        std::vector<SAsset*> assets;

        for (const SAssetReference& ref : assetRefs)
            assets.emplace_back(RequestAsset(ref, requesterID));

        return assets;
    }

    void CAssetRegistry::UnrequestAssets(const std::vector<SAssetReference>& assetRefs, const U64 requesterID)
    {
        for (const SAssetReference& ref : assetRefs)
            UnrequestAsset(ref, requesterID);
    }

    SAsset* CAssetRegistry::RequestAsset(const U32 assetUID, const U64 requesterID)
    {
        if (LoadedAssets.contains(assetUID))
        {
            SAsset* loadedAsset = &LoadedAssets[assetUID];
            loadedAsset->Requesters.insert(requesterID);
            return loadedAsset;
        }

        if (AssetDatabase.contains(assetUID))
            return RequestAsset(SAssetReference(AssetDatabase[assetUID]), requesterID);

        HV_LOG_ERROR("CAssetRegistry::RequestAsset: Could not request asset with ID: %i, it is not loaded yet and has not been found in the database before. Please use the asset file path instead", assetUID);
        return nullptr;
    }

    void CAssetRegistry::UnrequestAsset(const U32 assetUID, const U64 requesterID)
    {
        // TODO.NW: Check if asset is loading on separate thread, or check if requests are still there when finished loading?
        if (!LoadedAssets.contains(assetUID))
            return;

        SAsset* loadedAsset = &LoadedAssets[assetUID];
        loadedAsset->Requesters.erase(requesterID);
        UnrequestDependencies(assetUID, requesterID);

        if (loadedAsset->Requesters.empty())
        {
            if (AssetDatabase.contains(assetUID))
                UnloadAsset(SAssetReference(AssetDatabase[assetUID]));
        }
    }

    std::string CAssetRegistry::GetAssetDatabaseEntry(const U32 uid)
    {
        if (!AssetDatabase.contains(uid))
        {
            HV_LOG_WARN("CAssetRegistry::GetAssetDatabaseEntry: UID %i was not found in the asset database, can't point to valid asset path!", uid);
            return std::string("INVALID_PATH");
        }
        
        return AssetDatabase[uid];
    }

    void CAssetRegistry::RequestDependencies(const U32 assetUID, const U64 requesterID)
    {
        SAsset* asset = &LoadedAssets[assetUID];
        if (std::holds_alternative<SGraphicsMaterialAsset>(asset->Data))
        {
            SGraphicsMaterialAsset assetData = std::get<SGraphicsMaterialAsset>(asset->Data);

            auto requestAssetDependency = [&](const SRuntimeGraphicsMaterialProperty& property)
                {
                    if (property.TextureChannelIndex <= -1.0f)
                        return;

                    RequestAsset(property.TextureUID, requesterID);
                };

            requestAssetDependency(assetData.Material.AlbedoA);
            requestAssetDependency(assetData.Material.AlbedoR);
            requestAssetDependency(assetData.Material.AlbedoG);
            requestAssetDependency(assetData.Material.AlbedoB);
            requestAssetDependency(assetData.Material.AlbedoA);
            requestAssetDependency(assetData.Material.NormalX);
            requestAssetDependency(assetData.Material.NormalY);
            requestAssetDependency(assetData.Material.NormalZ);
            requestAssetDependency(assetData.Material.AmbientOcclusion);
            requestAssetDependency(assetData.Material.Metalness);
            requestAssetDependency(assetData.Material.Roughness);
            requestAssetDependency(assetData.Material.Emissive);
        }
    }

    void CAssetRegistry::UnrequestDependencies(const U32 assetUID, const U64 requesterID)
    {
        SAsset* asset = &LoadedAssets[assetUID];
        if (std::holds_alternative<SGraphicsMaterialAsset>(asset->Data))
        {
            SGraphicsMaterialAsset assetData = std::get<SGraphicsMaterialAsset>(asset->Data);

            auto unrequestAssetDependency = [&](const SRuntimeGraphicsMaterialProperty& property)
                {
                    if (property.TextureChannelIndex <= -1.0f)
                        return;

                    UnrequestAsset(property.TextureUID, requesterID);
                };

            unrequestAssetDependency(assetData.Material.AlbedoA);
            unrequestAssetDependency(assetData.Material.AlbedoR);
            unrequestAssetDependency(assetData.Material.AlbedoG);
            unrequestAssetDependency(assetData.Material.AlbedoB);
            unrequestAssetDependency(assetData.Material.AlbedoA);
            unrequestAssetDependency(assetData.Material.NormalX);
            unrequestAssetDependency(assetData.Material.NormalY);
            unrequestAssetDependency(assetData.Material.NormalZ);
            unrequestAssetDependency(assetData.Material.AmbientOcclusion);
            unrequestAssetDependency(assetData.Material.Metalness);
            unrequestAssetDependency(assetData.Material.Roughness);
            unrequestAssetDependency(assetData.Material.Emissive);
        }
    }

    bool CAssetRegistry::LoadAsset(const SAssetReference& assetRef)
    {
        std::string filePath = assetRef.FilePath;
        if (!CFileSystem::DoesFileExist(filePath))
        {
            HV_LOG_WARN("CAssetRegistry::LoadAsset: Asset file pointed to by %s failed to load, does not exist!", assetRef.FilePath.c_str());
            return false;
        }

        const U64 fileSize = CFileSystem::GetFileSize(filePath);
        if (fileSize == 0)
        {
            HV_LOG_WARN("CAssetRegistry::LoadAsset: Asset file pointed to by %s failed to load, was empty!", assetRef.FilePath.c_str());
            return false;
        }

        char* data = new char[fileSize];

        GEngine::GetFileSystem()->Deserialize(filePath, data, STATIC_U32(fileSize));
        EAssetType type = EAssetType::None;
        U64 pointerPosition = 0;
        DeserializeData(type, data, pointerPosition);

        SAsset asset;
        asset.Type = type;
        asset.Reference = assetRef;

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

                meshAsset.BoundsCenter = meshAsset.BoundsMin + (meshAsset.BoundsMax - meshAsset.BoundsMin) * 0.5f;
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

                meshAsset.BoundsCenter = meshAsset.BoundsMin + (meshAsset.BoundsMax - meshAsset.BoundsMin) * 0.5f;
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
            return false;
        }
        delete[] data;

        // TODO.NW: Set asset source data and bind filewatchers?

        LoadedAssets.emplace(assetRef.UID, asset);
        return true;
    }

    bool CAssetRegistry::UnloadAsset(const SAssetReference& assetRef)
    {
        if (!LoadedAssets.contains(assetRef.UID))
            return false;

        // TODO.NW: If any filewatchers are watching the source of this asset, remove them now
        LoadedAssets.erase(assetRef.UID);
        return true;
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

            fileHeader.Name = UGeneralUtils::ExtractFileBaseNameFromPath(filePath);
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

        AssetDatabase.emplace(SAssetReference(hvaPath).UID, hvaPath);
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
            hvaPath = destinationPath + header.Name + ".hva";
            GEngine::GetFileSystem()->Serialize(hvaPath, &data[0], header.GetSize());
            delete[] data;
        }
        else if (std::holds_alternative<SMaterialAssetFileHeader>(fileHeader))
        {
            SMaterialAssetFileHeader header = std::get<SMaterialAssetFileHeader>(fileHeader);
            const auto data = new char[header.GetSize()];
            header.Serialize(data);
            hvaPath = destinationPath + header.Name + ".hva";
            GEngine::GetFileSystem()->Serialize(hvaPath, &data[0], header.GetSize());
            delete[] data;
        }

        if (hvaPath == "INVALID_PATH")
            HV_LOG_WARN("CAssetRegistry::SaveAsset: The chosen file header had no serialization implemented. Could not create asset at %s!", destinationPath.c_str());

        return hvaPath;
    }

    void CAssetRegistry::RefreshDatabase()
    {
        AssetDatabase.clear();

        std::vector<std::string> topLevelDirectories = { "Resources/", "Assets/"};
        for (const std::string& directory : topLevelDirectories)
        {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory))
            {
                std::string path = UGeneralUtils::ConvertToPlatformAgnosticPath(entry.path().string());
                if (!entry.is_directory() && UGeneralUtils::ExtractFileExtensionFromPath(path) == "hva")
                {
                    AssetDatabase.emplace(SAssetReference(path).UID, path);
                }
            }
        }
    }

    std::set<U64> CAssetRegistry::GetReferencers(const SAssetReference& assetRef)
    {
        if (!LoadedAssets.contains(assetRef.UID))
            return std::set<U64>();

        SAsset* loadedAsset = &LoadedAssets[assetRef.UID];
        return loadedAsset->Requesters;
    }

    std::string CAssetRegistry::GetDebugString(const bool shouldExpand) const
    {
        std::string debugString = "Asset Registry | Loaded Assets: " + std::to_string(LoadedAssets.size()) + " | Database Entries: " + std::to_string(AssetDatabase.size()) + " |";
        
        if (shouldExpand)
        {
            for (auto const& [uid, asset] : LoadedAssets)
            {
                debugString.append("\n");
                debugString.append(std::to_string(asset.Requesters.size()));
                debugString.append("\t");
                if (!asset.Requesters.empty())
                {
                    debugString.append(std::to_string(*asset.Requesters.begin()));
                    debugString.append("\t");
                }
                debugString.append(asset.Reference.FilePath);
            }
        }
        
        return debugString;
    }
}
