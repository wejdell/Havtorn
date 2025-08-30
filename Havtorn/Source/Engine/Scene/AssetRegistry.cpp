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

        RefreshRegisteredAssets();

        return true;
    }

    SAsset* CAssetRegistry::RequestAsset(const SAssetReference& assetRef, const U64 requesterID)
    {
        if (LoadedAssets.contains(assetRef.UID))
        {
            SAsset* loadedAsset = &LoadedAssets[assetRef.UID];
            loadedAsset->Requesters.insert(requesterID);
            return loadedAsset;
        }

        // TODO.NW: Load on different thread and return null asset while loading?
        return LoadAsset(assetRef, requesterID);
    }

    void CAssetRegistry::UnrequestAsset(const SAssetReference& assetRef, const U64 requesterID)
    {
        // TODO.NW: Check if asset is loading on separate thread, or check if requests are still there when finished loading?
        if (!LoadedAssets.contains(assetRef.UID))
            return;

        SAsset* loadedAsset = &LoadedAssets[assetRef.UID];
        loadedAsset->Requesters.erase(requesterID);

        if (loadedAsset->Requesters.empty())
            UnloadAsset(assetRef);
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
        {
            SAsset* asset = RequestAsset(SAssetReference(AssetDatabase[assetUID]), requesterID);
            asset->Requesters.insert(requesterID);
            return asset;
        }

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
    }

    SAsset* CAssetRegistry::LoadAsset(const SAssetReference& assetRef, const U64 requesterID)
    {
        std::string filePath = assetRef.FilePath;
        if (!CFileSystem::DoesFileExist(filePath))
        {
            HV_LOG_WARN("CAssetRegistry::LoadAsset: Asset file pointed to by %s failed to load, does not exist!", assetRef.FilePath.c_str());
            return &LoadedAssets[0];
        }

        const U64 fileSize = CFileSystem::GetFileSize(filePath);
        if (fileSize == 0)
        {
            HV_LOG_WARN("CAssetRegistry::LoadAsset: Asset file pointed to by %s failed to load, was empty!", assetRef.FilePath.c_str());
            return &LoadedAssets[0];
        }

        char* data = new char[fileSize];

        GEngine::GetFileSystem()->Deserialize(filePath, data, STATIC_U32(fileSize));
        EAssetType type = EAssetType::None;
        U64 pointerPosition = 0;
        DeserializeData(type, data, pointerPosition);

        SAsset asset;
        asset.Type = type;
        asset.Reference = assetRef;
        asset.Requesters.insert(requesterID);

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
            return &LoadedAssets[0];
        }
        delete[] data;

        // TODO.NW: Set asset source data and bind filewatchers?

        LoadedAssets.emplace(assetRef.UID, asset);
        return &LoadedAssets[assetRef.UID];
    }

    void CAssetRegistry::UnloadAsset(const SAssetReference& assetRef)
    {
        // TODO.NW: If any filewatchers are watching the source of this asset, remove them now
        LoadedAssets.erase(assetRef.UID);
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

    void CAssetRegistry::RefreshRegisteredAssets()
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
}
