// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EditorResourceManager.h"
#include "Graphics/GraphicsUtilities.h"
#include "Graphics/RenderManager.h"
#include "ModelImporter.h"

#include <ECS/Systems/AnimatorGraphSystem.h>

namespace Havtorn
{
	CRenderTexture CEditorResourceManager::GetEditorTexture(EEditorTexture texture) const
	{
		U64 index = static_cast<I64>(texture);
		
		if (index >= Textures.size())
			return Textures[STATIC_U64(EEditorTexture::None)];

		return Textures[index];
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
			//return std::move(RenderManager->GetTextureAssetTexture(filePath));
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

		return hvaPath;
	}

	std::string CEditorResourceManager::ConvertToHVA(const std::string& filePath, const std::string& destinationPath, const SAssetImportOptions& importOptions) const
	{
		std::string hvaPath;
		switch (importOptions.AssetType)
		{
		case EAssetType::StaticMesh: // fallthrough
		case EAssetType::SkeletalMesh: // fallthrough
		case EAssetType::Animation:
		{
			hvaPath = CreateAsset(destinationPath, UModelImporter::ImportFBX(filePath, importOptions));
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

			hvaPath = CreateAsset(destinationPath, fileHeader);
		}
		break;
		case EAssetType::Material:
			// TODO.NW: Maybe make it clear here that we're expecting CreateAsset calls for this instead
			break;
		break;
		case EAssetType::AudioOneShot:
			break;
		case EAssetType::AudioCollection:
			break;
		case EAssetType::VisualFX:
			break;
		}

		return hvaPath;
	}

	void CEditorResourceManager::CreateMaterial(const std::string& destinationPath, const SMaterialAssetFileHeader& fileHeader) const
	{
		const auto data = new char[fileHeader.GetSize()];
		fileHeader.Serialize(data);
		GEngine::GetFileSystem()->Serialize(destinationPath, &data[0], fileHeader.GetSize());
		delete[] data;
	}

	void CEditorResourceManager::CreateMaterial(const std::string& destinationPath, const std::array<std::string, 3>& texturePaths) const
	{
		SMaterialAssetFileHeader asset;

		std::string materialName = UGeneralUtils::ExtractFileBaseNameFromPath(destinationPath);

		asset.MaterialName = "M_" + materialName;
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

	std::string CEditorResourceManager::GetFileName(EEditorTexture texture)
	{
		std::string extension = ".dds";

		switch (texture)
		{
		case EEditorTexture::FolderIcon:
			return ResourceAssetPath + "FolderIcon" + extension;

		case EEditorTexture::FileIcon:
			return ResourceAssetPath + "FileIcon" + extension;

		case EEditorTexture::PlayIcon:
			return ResourceAssetPath + "PlayIcon" + extension;

		case EEditorTexture::PauseIcon:
			return ResourceAssetPath + "PauseIcon" + extension;

		case EEditorTexture::StopIcon:
			return ResourceAssetPath + "StopIcon" + extension;

		case EEditorTexture::SceneIcon:
			return ResourceAssetPath + "SceneIcon" + extension;

		case EEditorTexture::SequencerIcon:
			return ResourceAssetPath + "SequencerIcon" + extension;

		case EEditorTexture::EnvironmentLightIcon:
			return ResourceAssetPath + "EnvironmentLightIcon" + extension;

		case EEditorTexture::DirectionalLightIcon:
			return ResourceAssetPath + "DirectionalLightIcon" + extension;

		case EEditorTexture::PointLightIcon:
			return ResourceAssetPath + "PointLightIcon" + extension;

		case EEditorTexture::SpotlightIcon:
			return ResourceAssetPath + "SpotlightIcon" + extension;

		case EEditorTexture::DecalIcon:
			return ResourceAssetPath + "DecalIcon" + extension;

		case EEditorTexture::ScriptIcon:
			return ResourceAssetPath + "ScriptIcon" + extension;

		case EEditorTexture::ColliderIcon:
			return ResourceAssetPath + "ColliderIcon" + extension;

		case EEditorTexture::NodeBackground:
			return ResourceAssetPath + "NodeBackground" + extension;

		case EEditorTexture::MinimizeWindow:
			return ResourceAssetPath + "MinimizeWindow" + extension;

		case EEditorTexture::MaximizeWindow:
			return ResourceAssetPath + "MaximizeWindow" + extension;

		case EEditorTexture::CloseWindow:
			return ResourceAssetPath + "CloseWindow" + extension;
		
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

	bool CEditorResourceManager::Init(CRenderManager* renderManager, const CGraphicsFramework* /*framework*/)
	{
		RenderManager = renderManager;
		//ID3D11Device* device = framework->GetDevice();
		I64 textureCount = static_cast<I64>(EEditorTexture::Count);
		
		Textures.resize(textureCount);

		//CreateMaterial("Assets/Materials/M_Checkboard_128x128.hva");

		//SMaterialAssetFileHeader previewMaterial;
		//previewMaterial.MaterialName = "M_MeshPreview";

		///*
		//* 	F32 ConstantValue = -1.0f;
		//*	std::string TexturePath;
		//*	I16 TextureChannelIndex = -1;
		//*/

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

		//CreateMaterial("Resources/" + previewMaterial.MaterialName + ".hva", previewMaterial);

		for (I64 index = 0; index < textureCount; index++)
		{
			//UGraphicsUtils::CreateShaderResourceViewFromSource(device, GetFileName(static_cast<EEditorTexture>(index)), &Textures[index]);
			Textures[index] = RenderManager->CreateRenderTextureFromSource(GetFileName(static_cast<EEditorTexture>(index)));
			GEngine::GetTextureBank()->AddTexture(Textures[index].GetShaderResourceView());
		}

		// Adding None texture at the end
		Textures.emplace_back(CRenderTexture());

		return true;
	}
}
