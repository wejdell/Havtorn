// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EditorResourceManager.h"
#include "EditorManager.h"

#include <Graphics/GraphicsUtilities.h>
#include <Graphics/RenderManager.h>
#include <Assets/AssetRegistry.h>

#include <ECS/Systems/AnimatorGraphSystem.h>

namespace Havtorn
{
	CRenderTexture CEditorResourceManager::GetEditorTexture(EEditorTexture texture) const
	{
		U64 index = static_cast<I64>(texture);
		
		if (index >= Textures.size())
			return CRenderTexture(Textures[STATIC_U64(EEditorTexture::None)]);

		return CRenderTexture(Textures[index]);
	}

	CRenderTexture CEditorResourceManager::RenderAssetTexure(EAssetType assetType, const std::string& filePath) const
	{
		switch (assetType)
		{
		case EAssetType::StaticMesh:
			return std::move(RenderManager->RenderStaticMeshAssetTexture(filePath));
		case EAssetType::SkeletalMesh:
			return std::move(RenderManager->RenderSkeletalMeshAssetTexture(filePath));
		case EAssetType::Texture:
			return std::move(RenderManager->CreateRenderTextureFromAsset(filePath));
		case EAssetType::Material:
			return std::move(RenderManager->RenderMaterialAssetTexture(filePath));
		case EAssetType::Animation:
			return std::move(RenderManager->RenderSkeletalAnimationAssetTexture(filePath));
		case EAssetType::AudioOneShot:
			break;
		case EAssetType::AudioCollection:
			break;
		case EAssetType::VisualFX:
			break;
		case EAssetType::Scene:
			break;
		case EAssetType::Sequencer:
			return std::move(GetEditorTexture(EEditorTexture::SequencerIcon));
		case EAssetType::Script:
			return std::move(GetEditorTexture(EEditorTexture::ScriptIcon));
		case EAssetType::None:
		default:
			break;
		}

		return CRenderTexture();
	}

	void CEditorResourceManager::AnimateAssetTexture(CRenderTexture& assetTexture, const EAssetType assetType, const std::string& fileName, const F32 animationTime) const
	{
		switch (assetType)
		{
		case EAssetType::Animation:
		{
			std::vector<SMatrix> boneTransforms = GEngine::GetWorld()->GetSystem<CAnimatorGraphSystem>()->ReadAssetAnimationPose(fileName, animationTime);
			RenderManager->RenderSkeletalAnimationAssetTexture(assetTexture, fileName, boneTransforms);
		}
		case EAssetType::AudioOneShot:
			break;
		case EAssetType::VisualFX:
			break;
		case EAssetType::SpriteAnimation:
			break;
		case EAssetType::AudioCollection:
		case EAssetType::StaticMesh:
		case EAssetType::SkeletalMesh:
		case EAssetType::Texture:
		case EAssetType::Material:
		case EAssetType::Scene:
		case EAssetType::Sequencer:
		case EAssetType::None:
		default:
			break;
		}
	}

	std::string CEditorResourceManager::CreateAsset(const std::string& destinationPath, const SAssetFileHeader& fileHeader) const
	{
		return GEngine::GetAssetRegistry()->SaveAsset(destinationPath, fileHeader);
	}

	std::string CEditorResourceManager::ConvertToHVA(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions) const
	{
		SSourceAssetData sourceData;
		sourceData.AssetType = importOptions.AssetType;
		sourceData.SourcePath = filePath;
		sourceData.AssetDependencyPath = importOptions.AssetRep != nullptr ? importOptions.AssetRep->DirectoryEntry.path().string() : "N/A";
		sourceData.ImportScale = importOptions.Scale;

		return GEngine::GetAssetRegistry()->ImportAsset(filePath, destinationPath, sourceData);
	}

	void CEditorResourceManager::CreateMaterial(const std::string& destinationPath, const SMaterialAssetFileHeader& fileHeader) const
	{
		const auto data = new char[fileHeader.GetSize()];
		fileHeader.Serialize(data);
		UFileSystem::Serialize(destinationPath, &data[0], fileHeader.GetSize());
		delete[] data;
	}

	void CEditorResourceManager::CreateMaterial(const std::string& destinationPath, const std::array<std::string, 3>& texturePaths) const
	{
		SMaterialAssetFileHeader asset;

		std::string materialName = UGeneralUtils::ExtractFileBaseNameFromPath(destinationPath);

		asset.Name = "M_" + materialName;
		asset.Material.Properties[0] = { -1.0f, texturePaths[0], 0 };
		asset.Material.Properties[1] = { -1.0f, texturePaths[0], 1 };
		asset.Material.Properties[2] = { -1.0f, texturePaths[0], 2 };
		asset.Material.Properties[3] = { -1.0f, texturePaths[0], 3 };
		asset.Material.Properties[4] = { -1.0f, texturePaths[2], 3 };
		asset.Material.Properties[5] = { -1.0f, texturePaths[2], 1 };
		asset.Material.Properties[6] = { -1.0f, "", -1 };
		asset.Material.Properties[7] = { -1.0f, texturePaths[2], 2 };
		asset.Material.Properties[8] = { -1.0f, texturePaths[1], 0 };
		asset.Material.Properties[9] = { -1.0f, texturePaths[1], 1 };
		asset.Material.Properties[10] = { -1.0f, texturePaths[1], 2 };
		asset.Material.RecreateZ = true;

		CreateMaterial(destinationPath, asset);
	}

	std::string CEditorResourceManager::GetFileName(EEditorTexture texture, const std::string& extension, const std::string& prefix)
	{
		switch (texture)
		{
		case EEditorTexture::FolderIcon:
			return ResourceAssetPath + prefix + "FolderIcon" + extension;

		case EEditorTexture::FileIcon:
			return ResourceAssetPath + prefix + "FileIcon" + extension;

		case EEditorTexture::PlayIcon:
			return ResourceAssetPath + prefix + "PlayIcon" + extension;

		case EEditorTexture::PauseIcon:
			return ResourceAssetPath + prefix + "PauseIcon" + extension;

		case EEditorTexture::StopIcon:
			return ResourceAssetPath + prefix + "StopIcon" + extension;

		case EEditorTexture::SceneIcon:
			return ResourceAssetPath + prefix + "SceneIcon" + extension;

		case EEditorTexture::SequencerIcon:
			return ResourceAssetPath + prefix + "SequencerIcon" + extension;

		case EEditorTexture::EnvironmentLightIcon:
			return ResourceAssetPath + prefix + "EnvironmentLightIcon" + extension;

		case EEditorTexture::DirectionalLightIcon:
			return ResourceAssetPath + prefix + "DirectionalLightIcon" + extension;

		case EEditorTexture::PointLightIcon:
			return ResourceAssetPath + prefix + "PointLightIcon" + extension;

		case EEditorTexture::SpotlightIcon:
			return ResourceAssetPath + prefix + "SpotlightIcon" + extension;

		case EEditorTexture::DecalIcon:
			return ResourceAssetPath + prefix + "DecalIcon" + extension;

		case EEditorTexture::ScriptIcon:
			return ResourceAssetPath + prefix + "ScriptIcon" + extension;

		case EEditorTexture::ColliderIcon:
			return ResourceAssetPath + prefix + "ColliderIcon" + extension;

		case EEditorTexture::NodeBackground:
			return ResourceAssetPath + prefix + "NodeBackground" + extension;

		case EEditorTexture::MinimizeWindow:
			return ResourceAssetPath + prefix + "MinimizeWindow" + extension;

		case EEditorTexture::MaximizeWindow:
			return ResourceAssetPath + prefix + "MaximizeWindow" + extension;

		case EEditorTexture::CloseWindow:
			return ResourceAssetPath + prefix + "CloseWindow" + extension;
		
		case EEditorTexture::Count:
		default:
			return std::string();
		}
	}

//#define ALBEDO_R            0
//#define ALBEDO_G            1
//#define ALBEDO_B            2
//#define ALBEDO_A            3
//#define NORMAL_X            4
//#define NORMAL_Y            5
//#define NORMAL_Z            6
//#define AMBIENT_OCCLUSION   7
//#define METALNESS           8
//#define ROUGHNESS           9
//#define EMISSIVE            10

	bool CEditorResourceManager::Init(CRenderManager* renderManager)
	{
		RenderManager = renderManager;
		U64 textureCount = static_cast<U64>(EEditorTexture::Count);
		Textures.resize(textureCount);

		//SMaterialAssetFileHeader previewMaterial;
		//previewMaterial.Name = "M_MeshPreview";
		//previewMaterial.Material.Properties[ALBEDO_R] = { 0.8f, "", -1 };
		//previewMaterial.Material.Properties[ALBEDO_G] = { 0.8f, "", -1 };
		//previewMaterial.Material.Properties[ALBEDO_B] = { 0.8f, "", -1 };
		//previewMaterial.Material.Properties[ALBEDO_A] = { 1.0f, "", -1 };
		//previewMaterial.Material.Properties[NORMAL_X] = { 0.0f, "", -1 };
		//previewMaterial.Material.Properties[NORMAL_Y] = { 0.0f, "", -1 };
		//previewMaterial.Material.Properties[NORMAL_Z] = { 1.0f, "", -1 };
		//previewMaterial.Material.Properties[AMBIENT_OCCLUSION] = { 0.0f, "", -1 };
		//previewMaterial.Material.Properties[METALNESS] = { 0.0f, "", -1 };
		//previewMaterial.Material.Properties[ROUGHNESS] = { 1.0f, "", -1 };
		//previewMaterial.Material.Properties[EMISSIVE] = { 0.0f, "", -1 };
		//previewMaterial.Material.RecreateZ = true;

		//CreateMaterial("Resources/" + previewMaterial.Name + ".hva", previewMaterial);

		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		for (U64 index = 0; index < textureCount; index++)
		{
			const EEditorTexture texture = static_cast<EEditorTexture>(index);
			const std::string assetSubDirectory = "Assets/";
			const std::string assetPath = GetFileName(texture, ".hva", assetSubDirectory);
			if (!UFileSystem::DoesFileExist(assetPath))
			{
				const std::string sourcePath = GetFileName(texture, ".dds");
				SSourceAssetData sourceData;
				sourceData.AssetType = EAssetType::Texture;
				sourceData.SourcePath = sourcePath;
				assetRegistry->ImportAsset(sourcePath, ResourceAssetPath + assetSubDirectory, sourceData);
			}

			STextureAsset* assetData = assetRegistry->RequestAssetData<STextureAsset>(SAssetReference(assetPath), CAssetRegistry::EditorManagerRequestID);
			Textures[index] = assetData->RenderTexture;
			assetRegistry->UnrequestAsset(SAssetReference(assetPath), CAssetRegistry::EditorManagerRequestID);
		}

		// Adding None texture at the end
		Textures.emplace_back(CStaticRenderTexture());

		return true;
	}
}
