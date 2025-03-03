// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EditorResourceManager.h"
#include "Graphics/GraphicsUtilities.h"
#include "Graphics/RenderManager.h"
#include "ModelImporter.h"

namespace Havtorn
{
	ID3D11ShaderResourceView* CEditorResourceManager::GetEditorTexture(EEditorTexture texture) const
	{
		U64 index = static_cast<I64>(texture);
		
		if (index >= Textures.size())
			return nullptr;

		return Textures[index];
	}

	void* CEditorResourceManager::RenderAssetTexure(EAssetType assetType, const std::string& filePath)
	{
		switch (assetType)
		{
		case EAssetType::StaticMesh:
			return std::move(RenderManager->RenderStaticMeshAssetTexture(filePath));
		case EAssetType::SkeletalMesh:
			return std::move(RenderManager->RenderSkeletalMeshAssetTexture(filePath));
		case EAssetType::Texture:
			return std::move(RenderManager->GetTextureAssetTexture(filePath));
		case EAssetType::Material:
			return std::move(RenderManager->RenderMaterialAssetTexture(filePath));
		case EAssetType::Animation:
			break;
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
		case EAssetType::None:
		default:
			break;
		}

		return nullptr;
	}

	void CEditorResourceManager::CreateAsset(const std::string& destinationPath, EAssetType assetType) const
	{
		switch (assetType)
		{
		case EAssetType::StaticMesh:
			break;
		case EAssetType::SkeletalMesh:
			break;
		case EAssetType::Texture:
			break;
		case EAssetType::Material:
		{
			SMaterialAssetFileHeader asset;

			asset.MaterialName = "M_Bed";
			
			std::vector<std::string> materialTextures = { "Assets/Textures/T_Bed_c.hva", "Assets/Textures/T_Bed_m.hva", "Assets/Textures/T_Bed_n.hva" };
			asset.Material.Properties[0] = { -1.0f, materialTextures[0], 0 };
			asset.Material.Properties[1] = { -1.0f, materialTextures[0], 1 };
			asset.Material.Properties[2] = { -1.0f, materialTextures[0], 2 };
			asset.Material.Properties[3] = { -1.0f, materialTextures[0], 3 };
			asset.Material.Properties[4] = { -1.0f, materialTextures[2], 3 };
			asset.Material.Properties[5] = { -1.0f, materialTextures[2], 1 };
			asset.Material.Properties[6] = { -1.0f, "", -1};
			asset.Material.Properties[7] = { -1.0f, materialTextures[2], 2 };
			asset.Material.Properties[8] = { -1.0f, materialTextures[1], 0 };
			asset.Material.Properties[9] = { -1.0f, materialTextures[1], 1 };
			asset.Material.Properties[10] = { -1.0f, materialTextures[1], 2 };
			asset.Material.RecreateZ = true;

			const auto data = new char[asset.GetSize()];

			asset.Serialize(data);
			GEngine::GetFileSystem()->Serialize(destinationPath, &data[0], asset.GetSize());
			delete[] data;
		}
		break;
		case EAssetType::Animation:
			break;
		case EAssetType::SpriteAnimation:
			break;
		case EAssetType::AudioOneShot:
			break;
		case EAssetType::AudioCollection:
			break;
		case EAssetType::VisualFX:
			break;
		default:
			break;
		}
	}

	std::string CEditorResourceManager::ConvertToHVA(const std::string& filePath, const std::string& destination, EAssetType assetType) const
	{
		std::string hvaPath;
		switch (assetType)
		{
		case EAssetType::StaticMesh: // fallthrough
		case EAssetType::SkeletalMesh: // fallthrough
		case EAssetType::Animation:
		{
			// TODO.NR: Take destination into account
			hvaPath = UModelImporter::ImportFBX(filePath, assetType);
		}
		break;
		case EAssetType::Texture:
		{
			std::string textureFileData;
			GEngine::GetFileSystem()->Deserialize(filePath, textureFileData);

			ETextureFormat format = {};
			if (const std::string extension = filePath.substr(filePath.size() - 4); extension == ".dds")
				format = ETextureFormat::DDS;
			else if (extension == ".tga")
				format = ETextureFormat::TGA;

			STextureFileHeader asset;
			asset.AssetType = EAssetType::Texture;

			asset.MaterialName = destination + filePath.substr(filePath.find_last_of('\\'), filePath.find_first_of('.') - filePath.find_last_of('\\'));// destination.substr(0, destination.find_last_of("."));
			asset.OriginalFormat = format;
			asset.Suffix = filePath[filePath.find_last_of(".") - 1];
			asset.Data = std::move(textureFileData);

			const auto data = new char[asset.GetSize()];

			asset.Serialize(data);
			GEngine::GetFileSystem()->Serialize(asset.MaterialName + ".hva", &data[0], asset.GetSize());
			delete[] data;

			hvaPath = asset.MaterialName + ".hva";
		}
		break;
		case EAssetType::Material:
			// NW: Call CreateMaterial?
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

	void CEditorResourceManager::CreateMaterial(const std::string& destinationPath)
	{
		SMaterialAssetFileHeader asset;

		size_t startIndex = destinationPath.find_first_of("_") + 1;
		std::string materialName = destinationPath.substr(startIndex, destinationPath.find_last_of(".") - startIndex);

		asset.MaterialName = "M_" + materialName;

		std::vector<std::string> materialTextures = { "Assets/Textures/T_" + materialName + "_c.hva", "Assets/Textures/T_" + materialName + "_m.hva", "Assets/Textures/T_" + materialName + "_n.hva" };
		asset.Material.Properties[0] = { -1.0f, materialTextures[0], 0 };
		asset.Material.Properties[1] = { -1.0f, materialTextures[0], 1 };
		asset.Material.Properties[2] = { -1.0f, materialTextures[0], 2 };
		asset.Material.Properties[3] = { -1.0f, materialTextures[0], 3 };
		asset.Material.Properties[4] = { -1.0f, materialTextures[2], 3 };
		asset.Material.Properties[5] = { -1.0f, materialTextures[2], 1 };
		asset.Material.Properties[6] = { -1.0f, "", -1 };
		asset.Material.Properties[7] = { -1.0f, materialTextures[2], 2 };
		asset.Material.Properties[8] = { -1.0f, materialTextures[1], 0 };
		asset.Material.Properties[9] = { -1.0f, materialTextures[1], 1 };
		asset.Material.Properties[10] = { -1.0f, materialTextures[1], 2 };
		asset.Material.RecreateZ = true;

		const auto data = new char[asset.GetSize()];

		asset.Serialize(data);
		GEngine::GetFileSystem()->Serialize(destinationPath, &data[0], asset.GetSize());
		delete[] data;
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
		
		case EEditorTexture::Count:
		default:
			return std::string();
		}
	}

	bool CEditorResourceManager::Init(CRenderManager* renderManager, const CGraphicsFramework* framework)
	{
		RenderManager = renderManager;
		ID3D11Device* device = framework->GetDevice();
		I64 textureCount = static_cast<I64>(EEditorTexture::Count);
		
		Textures.assign(textureCount, nullptr);

		CreateMaterial("Assets/Materials/M_Checkboard_128x128.hva");

		for (I64 index = 0; index < textureCount; index++)
		{
			UGraphicsUtils::CreateShaderResourceViewFromResource(device, GetFileName(static_cast<EEditorTexture>(index)), &Textures[index]);
			GEngine::GetTextureBank()->AddTexture(Textures[index]);
		}

		return true;
	}
}
