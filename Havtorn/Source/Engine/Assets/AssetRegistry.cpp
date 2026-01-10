// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "AssetRegistry.h"
#include "Assets/FileHeaderDeclarations.h"
#include "FileSystem/FileWatcher.h"
#include "Assets/RuntimeAssetDeclarations.h"
#include "ECS/GUIDManager.h"

#include "Graphics/RenderManager.h"
#include "Graphics/TextureBank.h"

#include "ModelImporter.h"

#include <magic_enum.h>

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
        SAsset nullAsset = SAsset();
        AddAsset(0, nullAsset);

        RefreshDatabase();

        return true;
    }

    SAsset* CAssetRegistry::RequestAsset(const SAssetReference& assetRef, const U64 requesterID)
    {
        // TODO.NW: Load on different thread and return null asset while loading?
        if (!LoadedAssets.contains(assetRef.UID) && !LoadAsset(assetRef))
            return GetAsset(0);
        
        SAsset* loadedAsset = GetAsset(assetRef.UID);
        loadedAsset->Requesters.insert(requesterID);
        RequestDependencies(assetRef.UID, requesterID);

        return loadedAsset;
    }

    void CAssetRegistry::UnrequestAsset(const SAssetReference& assetRef, const U64 requesterID)
    {
        // TODO.NW: Check if asset is loading on separate thread, or check if requests are still there when finished loading?
        if (!LoadedAssets.contains(assetRef.UID))
            return;

        SAsset* loadedAsset = GetAsset(assetRef.UID);
        loadedAsset->Requesters.erase(requesterID);
        UnrequestDependencies(assetRef.UID, requesterID);

        if (loadedAsset->Requesters.empty())
            UnloadAsset(assetRef);
    }

    SAsset* CAssetRegistry::RequestAsset(const U32 assetUID, const U64 requesterID)
    {
        if (LoadedAssets.contains(assetUID))
        {
            SAsset* loadedAsset = GetAsset(assetUID);
            loadedAsset->Requesters.insert(requesterID);
            RequestDependencies(assetUID, requesterID);
            
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

        SAsset* loadedAsset = GetAsset(assetUID);
        loadedAsset->Requesters.erase(requesterID);
        UnrequestDependencies(assetUID, requesterID);

        if (loadedAsset->Requesters.empty())
        {
            if (AssetDatabase.contains(assetUID))
                UnloadAsset(SAssetReference(AssetDatabase[assetUID]));
        }
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
        SAsset* asset = GetAsset(assetUID);
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
        SAsset* asset = GetAsset(assetUID);
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
        if (!UFileSystem::Exists(filePath))
        {
            CJsonDocument config = UFileSystem::OpenJson(UFileSystem::EngineConfig);
            std::string redirection = config.GetValueFromArray("Asset Redirectors", filePath, "");
            
            while (!UFileSystem::Exists(redirection) && redirection != "")
            {
                redirection = config.GetValueFromArray("Asset Redirectors", redirection, "");
            } 

            if (redirection == "")
            {
                HV_LOG_WARN("CAssetRegistry::LoadAsset: Asset file pointed to by %s failed to load, does not exist!", assetRef.FilePath.c_str());
                return false;
            }

            filePath = redirection;
        }

        const U64 fileSize = UFileSystem::GetFileSize(filePath);
        if (fileSize == 0)
        {
            HV_LOG_WARN("CAssetRegistry::LoadAsset: Asset file pointed to by %s failed to load, was empty!", assetRef.FilePath.c_str());
            return false;
        }

        char* data = new char[fileSize];

        UFileSystem::Deserialize(filePath, data, STATIC_U32(fileSize));
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
            asset.SourceData = assetFile.SourceData;
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
            asset.SourceData = assetFile.SourceData;
        }
        break;
        case EAssetType::Texture:
        {
            STextureFileHeader assetFile;
            assetFile.Deserialize(data);
            STextureAsset textureAsset(assetFile);
            textureAsset.RenderTexture = RenderManager->RenderTextureFactory.CreateStaticTexture(filePath, assetFile.AssetType);
            asset.Data = textureAsset;
            asset.SourceData = assetFile.SourceData;
        }
        break;
        case EAssetType::TextureCube:
        {
            STextureCubeFileHeader assetFile;
            assetFile.Deserialize(data);
            STextureCubeAsset textureAsset(assetFile);
            textureAsset.RenderTexture = RenderManager->RenderTextureFactory.CreateStaticTexture(filePath, assetFile.AssetType);
            asset.Data = textureAsset;
            asset.SourceData = assetFile.SourceData;
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
            asset.SourceData = assetFile.SourceData;
        }
        break;
        case EAssetType::Script:
            HV_LOG_ERROR("Use RequestGameAsset Instead");
        break;
        case EAssetType::SpriteAnimation:
        case EAssetType::AudioOneShot:
        case EAssetType::AudioCollection:
        case EAssetType::VisualFX:
        break;
        case EAssetType::Scene:
            HV_LOG_ERROR("Use RequestGameAsset Instead");
        break;
        case EAssetType::Sequencer:
            HV_LOG_WARN("CAssetRegistry: Asset Resolving for asset type %s is not yet implemented.", magic_enum::enum_name<EAssetType>(type).data());
            delete[] data;
            return false;
        }
        delete[] data;

        // TODO.NW: Bind filewatchers?

        AddAsset(assetRef.UID, asset);
        return true;
    }

    bool CAssetRegistry::UnloadAsset(const SAssetReference& assetRef)
    {
        if (!LoadedAssets.contains(assetRef.UID))
            return false;

        // TODO.NW: If any filewatchers are watching the source of this asset, remove them now
        RemoveAsset(assetRef.UID);
        return true;
    }

    SAsset* CAssetRegistry::GetAsset(const U32 assetUID)
    {
        std::shared_lock lock(RegistryMutex);
        return &LoadedAssets[assetUID];
    }

    void CAssetRegistry::AddAsset(const U32 assetUID, SAsset& asset)
    {
        // NW: This combination of locks (shared in get asset and unique in remove asset)
        // seems stable for now, but if we continue crashing due to asset requesting we
        // might need to take a proper thread safety pass. Should also get to async loading.

        // NW: It's important that we move the asset here, otherwise pointers in the assets (e.g. to data bindings in data binding nodes in scripts)
        // may be left dangling.
        if (LoadedAssets.contains(assetUID))
        {
            LoadedAssets[assetUID] = std::move(asset);
            return;
        }

        LoadedAssets.emplace(assetUID, std::move(asset));
    }

    void CAssetRegistry::RemoveAsset(const U32 assetUID)
    {
        std::unique_lock lock(RegistryMutex);
        LoadedAssets.erase(assetUID);
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
            UFileSystem::Deserialize(filePath, textureFileData);

            ETextureFormat format = {};
            if (const std::string extension = UGeneralUtils::ExtractFileExtensionFromPath(filePath); extension == "dds")
                format = ETextureFormat::DDS;
            else if (extension == "tga")
                format = ETextureFormat::TGA;

            STextureFileHeader fileHeader;
            fileHeader.AssetType = EAssetType::Texture;

            fileHeader.Name = UGeneralUtils::ExtractFileBaseNameFromPath(filePath);
            fileHeader.OriginalFormat = format;
            fileHeader.SourceData = sourceData;
            fileHeader.Suffix = filePath[filePath.find_last_of(".") - 1];
            fileHeader.Data = std::move(textureFileData);

            // TODO.NW: Make sure file header gets source data set, in ModelImport as well

            hvaPath = SaveAsset(destinationPath, fileHeader);
        }
        break;
        case EAssetType::TextureCube:
        {
            std::string textureFileData;
            UFileSystem::Deserialize(filePath, textureFileData);

            ETextureFormat format = {};
            if (const std::string extension = UGeneralUtils::ExtractFileExtensionFromPath(filePath); extension == "dds")
                format = ETextureFormat::DDS;
            else if (extension == "tga")
                format = ETextureFormat::TGA;

            STextureCubeFileHeader fileHeader;
            fileHeader.AssetType = EAssetType::TextureCube;

            fileHeader.Name = UGeneralUtils::ExtractFileBaseNameFromPath(filePath);
            fileHeader.OriginalFormat = format;
            fileHeader.SourceData = sourceData;
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

        SAssetReference ref(hvaPath);
        if (!AssetDatabase.contains(ref.UID))
            AssetDatabase.emplace(ref.UID, hvaPath);
        
        return hvaPath;
    }

    std::string CAssetRegistry::SaveAsset(const std::string& destinationPath, const SAssetFileHeader& fileHeader)
    {
        // TODO.NW: See if we can make char stream we can then convert to data buffer,
        // so as to not repeat the logic for every case

        std::vector<std::string> paths = UFileSystem::SplitPath(destinationPath);
        for (std::string& path : paths)
        {
            if (!UFileSystem::Exists(path))
                UFileSystem::AddDirectory(path);
        }

        std::string hvaPath = "INVALID_PATH";
        if (std::holds_alternative<SStaticModelFileHeader>(fileHeader))
        {
            SStaticModelFileHeader header = std::get<SStaticModelFileHeader>(fileHeader);
            U32 size = header.GetSize();
            const auto data = new char[size];
            header.Serialize(data);
            hvaPath = destinationPath + header.Name + ".hva";
            UFileSystem::Serialize(hvaPath, &data[0], size);
            delete[] data;
        }
        else if (std::holds_alternative<SSkeletalModelFileHeader>(fileHeader))
        {
            SSkeletalModelFileHeader header = std::get<SSkeletalModelFileHeader>(fileHeader);
            U32 size = header.GetSize();
            const auto data = new char[size];
            header.Serialize(data);
            hvaPath = destinationPath + header.Name + ".hva";
            UFileSystem::Serialize(hvaPath, &data[0], size);
            delete[] data;
        }
        else if (std::holds_alternative<SSkeletalAnimationFileHeader>(fileHeader))
        {
            SSkeletalAnimationFileHeader header = std::get<SSkeletalAnimationFileHeader>(fileHeader);
            U32 size = header.GetSize();
            const auto data = new char[size];
            header.Serialize(data);
            hvaPath = destinationPath + header.Name + ".hva";
            UFileSystem::Serialize(hvaPath, &data[0], size);
            delete[] data;
        }
        else if (std::holds_alternative<STextureFileHeader>(fileHeader))
        {
            STextureFileHeader header = std::get<STextureFileHeader>(fileHeader);
            U32 size = header.GetSize();
            const auto data = new char[size];
            header.Serialize(data);
            hvaPath = destinationPath + header.Name + ".hva";
            UFileSystem::Serialize(hvaPath, &data[0], size);
            delete[] data;
        }
        else if (std::holds_alternative<STextureCubeFileHeader>(fileHeader))
        {
            STextureCubeFileHeader header = std::get<STextureCubeFileHeader>(fileHeader);
            U32 size = header.GetSize();
            const auto data = new char[size];
            header.Serialize(data);
            hvaPath = destinationPath + header.Name + ".hva";
            UFileSystem::Serialize(hvaPath, &data[0], size);
            delete[] data;
        }
        else if (std::holds_alternative<SMaterialAssetFileHeader>(fileHeader))
        {
            SMaterialAssetFileHeader header = std::get<SMaterialAssetFileHeader>(fileHeader);
            U32 size = header.GetSize();
            const auto data = new char[size];
            header.Serialize(data);
            hvaPath = destinationPath + header.Name + ".hva";
            UFileSystem::Serialize(hvaPath, &data[0], size);
            delete[] data;
        }
        else if (std::holds_alternative<SScriptFileHeader>(fileHeader))
        {
            SScriptFileHeader header = std::get<SScriptFileHeader>(fileHeader);
            U32 size = header.GetSize();
            const auto data = new char[size];
            header.Serialize(data);
            hvaPath = destinationPath + header.Name + ".hva";
            UFileSystem::Serialize(hvaPath, &data[0], size);
            delete[] data;
        }
        else if (std::holds_alternative<SSceneFileHeader>(fileHeader))
        {
            SSceneFileHeader header = std::get<SSceneFileHeader>(fileHeader);
            U32 size = header.GetSize();
            const auto data = new char[size];
            U64 pointerPosition = 0;
            header.Serialize(data, pointerPosition);
            hvaPath = destinationPath + header.Scene->GetSceneName() + ".hva";
            UFileSystem::Serialize(hvaPath, &data[0], size);
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

    void CAssetRegistry::FixUpAssetRedirectors()
    {
        // NW: Go through all assets on disk and if they have AssetDependencies, redirect them
        for (const auto& entry : std::filesystem::recursive_directory_iterator("Assets/"))
        {
            if (entry.is_directory())
                continue;

            // TODO.NW: Make function of this, following the trail of redirection
            SAssetReference assetRef = SAssetReference(entry.path().string());
            if (!UFileSystem::Exists(assetRef.FilePath))
            {
                CJsonDocument config = UFileSystem::OpenJson(UFileSystem::EngineConfig);
                std::string redirection = config.GetValueFromArray("Asset Redirectors", assetRef.FilePath, "");

                while (!UFileSystem::Exists(redirection) && redirection != "")
                {
                    redirection = config.GetValueFromArray("Asset Redirectors", redirection, "");
                }

                if (redirection == "")
                {
                    HV_LOG_WARN("CAssetRegistry::FixUpAssetRedirectors: Asset file pointed to by %s failed to load, does not exist!", assetRef.FilePath.c_str());
                    continue;
                }

                assetRef = SAssetReference(redirection);
            }

            SAsset* asset = RequestAsset(assetRef, AssetRegistryRequestID);
            if (!asset->IsValid())
                continue;

            std::string dependencyPath = UGeneralUtils::ConvertToPlatformAgnosticPath(asset->SourceData.AssetDependencyPath.AsString());
            if (dependencyPath == "N/A" || dependencyPath.length() == 0)
            {
                UnrequestAsset(assetRef, AssetRegistryRequestID);
                continue;
            }

            if (!UFileSystem::Exists(dependencyPath))
            {
                CJsonDocument config = UFileSystem::OpenJson(UFileSystem::EngineConfig);
                std::string redirection = config.GetValueFromArray("Asset Redirectors", dependencyPath, "");

                while (!UFileSystem::Exists(redirection) && redirection != "")
                {
                    redirection = config.GetValueFromArray("Asset Redirectors", redirection, "");
                }

                if (redirection == "")
                    continue;

                dependencyPath = redirection;
            }

            HV_LOG_INFO("CAssetRegistry::FixUpAssetRedirectors: Asset file %s changed Asset Dependency Path from %s to %s.", assetRef.FilePath.c_str(), asset->SourceData.AssetDependencyPath.AsString().c_str(), dependencyPath.c_str());
            asset->SourceData.AssetDependencyPath = dependencyPath;
            ImportAsset(asset->SourceData.SourcePath.AsString(), UGeneralUtils::ExtractParentDirectoryFromPath(asset->Reference.FilePath), asset->SourceData);

            UnrequestAsset(assetRef, AssetRegistryRequestID);
        }

        CJsonDocument config = UFileSystem::OpenJson(UFileSystem::EngineConfig);
        config.ClearArray("Asset Redirectors");
    }

    std::set<U64> CAssetRegistry::GetReferencers(const SAssetReference& assetRef)
    {
        std::shared_lock lock(RegistryMutex);

        if (!LoadedAssets.contains(assetRef.UID))
            return std::set<U64>();

        SAsset* loadedAsset = GetAsset(assetRef.UID);
        return loadedAsset->Requesters;
    }

    void CAssetRegistry::StartSourceFileWatch(const SAssetReference& assetRef)
    {
        SAsset* asset = RequestAsset(assetRef, AssetRegistryRequestID);
        if (!asset->IsValid())
        {
            UnrequestAsset(assetRef, AssetRegistryRequestID);
            return;
        }

        std::string sourcePath = UGeneralUtils::ConvertToPlatformAgnosticPath(asset->SourceData.SourcePath.AsString());
        if (sourcePath == "N/A" || sourcePath.length() == 0)
        {
            UnrequestAsset(assetRef, AssetRegistryRequestID);
            return;
        }

        if (!UFileSystem::Exists(sourcePath))
        {
            UnrequestAsset(assetRef, AssetRegistryRequestID);
            return;
        }
        
        GEngine::GetFileWatcher()->WatchFileChange(sourcePath, std::bind(&CAssetRegistry::OnSourceFileChanged, this, std::placeholders::_1));
        WatchedAssets.emplace(sourcePath, asset);

        // NW: Only unrequest the asset when stopping file watch, to maintain a live connection in WatchedAssets
    }

    void CAssetRegistry::StopSourceFileWatch(const SAssetReference& assetRef)
    {
        SAsset* asset = RequestAsset(assetRef, AssetRegistryRequestID);
        if (!asset->IsValid())
        {
            UnrequestAsset(assetRef, AssetRegistryRequestID);
            return;
        }

        std::string sourcePath = UGeneralUtils::ConvertToPlatformAgnosticPath(asset->SourceData.SourcePath.AsString());
        if (sourcePath == "N/A" || sourcePath.length() == 0)
        {
            UnrequestAsset(assetRef, AssetRegistryRequestID);
            return;
        }

        if (!UFileSystem::Exists(sourcePath))
        {
            UnrequestAsset(assetRef, AssetRegistryRequestID);
            return;
        }

        GEngine::GetFileWatcher()->StopWatchFileChange(sourcePath, std::bind(&CAssetRegistry::OnSourceFileChanged, this, std::placeholders::_1));
        WatchedAssets.erase(sourcePath);
    }

    void CAssetRegistry::OnSourceFileChanged(const std::string& sourceFilePath)
    {
        if (!WatchedAssets.contains(sourceFilePath))
        {
            HV_LOG_WARN("CAssetRegistry::OnSourceFileChanged: The list of watched assets does not include %s, cannot reimport the asset that is using this source.", sourceFilePath.c_str());
            return;
        }

        SAsset* asset = WatchedAssets[sourceFilePath];
        ImportAsset(asset->SourceData.SourcePath.AsString(), UGeneralUtils::ExtractParentDirectoryFromPath(asset->Reference.FilePath), asset->SourceData);

        if (LoadAsset(asset->Reference))
            HV_LOG_INFO("CAssetRegistry::OnSourceFileChanged: Asset file %s was reimported after source change in %s.", asset->Reference.FilePath.c_str(), sourceFilePath.c_str());
        else
            HV_LOG_WARN("CAssetRegistry::OnSourceFileChanged: Asset file %s was reimported after source change in %s, but could not be reloaded! Please reload manually.", asset->Reference.FilePath.c_str(), sourceFilePath.c_str());
        
        OnAssetReloaded.Broadcast(asset->Reference.FilePath);
    }

    std::string CAssetRegistry::GetDebugString(const bool shouldExpand)
    {
        std::shared_lock lock(RegistryMutex);

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
