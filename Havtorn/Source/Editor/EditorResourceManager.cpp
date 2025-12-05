// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "EditorResourceManager.h"
#include "EditorManager.h"

#include <Graphics/GraphicsUtilities.h>
#include <Graphics/GeometryPrimitives.h>
#include <Graphics/RenderManager.h>
#include <Assets/AssetRegistry.h>
#include <MathTypes/MathUtilities.h>

#include <ECS/Systems/AnimatorGraphSystem.h>

namespace Havtorn
{
	intptr_t CEditorResourceManager::GetStaticEditorTextureResource(const EEditorTexture texture) const
	{
		U64 index = static_cast<I64>(texture);

		if (index >= Textures.size())
			return Textures[STATIC_U64(EEditorTexture::None)].GetResource();

		return Textures[index].GetResource();
	}

	void CEditorResourceManager::RequestThumbnailRender(SEditorAssetRepresentation* assetRep, const std::string& filePath) const
	{
		U32 assetID = SAssetReference(assetRep->Name).UID;
		// TODO.NW: Set up ownership using smart pointers, use weak ptr to capture 'this' here. 
		auto replaceTexture = [this, filePath](CRenderTexture finishedCopy)
			{
				Manager->GetAssetRepFromDirEntry(std::filesystem::directory_entry(filePath))->TextureRef = std::move(finishedCopy);
			};
		RenderManager->RequestRenderView(assetID, replaceTexture);

		switch (assetRep->AssetType)
		{
		case EAssetType::StaticMesh:
		{
			SStaticMeshAsset* meshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SStaticMeshAsset>(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);
			F32 aspectRatio = 1.0f;
			F32 marginPercentage = 1.5f;
			SVector2<F32> fov = { aspectRatio * 70.0f, 70.0f };

			STransform cameraTransform;
			cameraTransform.Orbit(SVector4(), SMatrix::CreateRotationFromEuler(30.0f, 30.0f, 0.0f));
			cameraTransform.Translate(SVector(meshAsset->BoundsCenter.X, meshAsset->BoundsCenter.Y, -UMathUtilities::GetFocusDistanceForBounds(meshAsset->BoundsCenter, SVector::GetAbsMaxKeepValue(meshAsset->BoundsMax, meshAsset->BoundsMin), fov, marginPercentage)));
			SMatrix camView = cameraTransform.GetMatrix();
			SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(fov.Y), aspectRatio, 0.001f, 100.0f);
				
			SRenderCommand command;
			command.Type = ERenderCommandType::StaticMeshAssetThumbnail;
			command.DrawCallData = meshAsset->DrawCallData;
			command.Matrices.emplace_back(camView);
			command.Matrices.emplace_back(camProjection);
			RenderManager->PushRenderCommand(command, assetID);

			GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);
			
		}
			break;
		case EAssetType::SkeletalMesh:
		{
			SSkeletalMeshAsset* meshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalMeshAsset>(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);
			F32 aspectRatio = 1.0f;
			F32 marginPercentage = 1.5f;
			SVector2<F32> fov = { aspectRatio * 70.0f, 70.0f };

			STransform cameraTransform;
			cameraTransform.Orbit(SVector4(), SMatrix::CreateRotationFromEuler(30.0f, 30.0f, 0.0f));
			cameraTransform.Translate(SVector(meshAsset->BoundsCenter.X, meshAsset->BoundsCenter.Y, -UMathUtilities::GetFocusDistanceForBounds(meshAsset->BoundsCenter, SVector::GetAbsMaxKeepValue(meshAsset->BoundsMax, meshAsset->BoundsMin), fov, marginPercentage)));
			SMatrix camView = cameraTransform.GetMatrix();
			SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(fov.Y), aspectRatio, 0.001f, 100.0f);

			SRenderCommand command;
			command.Type = ERenderCommandType::SkeletalMeshAssetThumbnail;
			command.DrawCallData = meshAsset->DrawCallData;
			command.BoneMatrices = std::vector<SMatrix>();
			command.Matrices.emplace_back(camView);
			command.Matrices.emplace_back(camProjection);
			RenderManager->PushRenderCommand(command, assetID);

			GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);
		}
			break;
		case EAssetType::Material:
		{
			{
				const F32 radius = 2.0f;
				SVector location = SVector(0.0f, 0.0f, radius);
				SMatrix camView = SMatrix::LookAtLH(location, SVector(), SVector::Up).FastInverse();
				SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), 1.0f, 0.01f, 10.0f);

				SRenderCommand command = SRenderCommand(ERenderCommandType::CameraDataStorage);
				command.Matrices.emplace_back(camView);
				command.Matrices.emplace_back(camProjection);
				RenderManager->PushRenderCommand(command, assetID);
			}

			{
				STransformComponent component;
				SMatrix objectMatrix = SMatrix::CreateRotationFromEuler(SVector(-25.0f, 30.0f, 0.0f)).FastInverse();
				component.Transform.SetMatrix(objectMatrix);
				component.Owner = SEntity(assetID);

				SDrawCallData drawCallData;
				drawCallData.IndexCount = STATIC_U32(GeometryPrimitives::Icosphere.Indices.size());
				drawCallData.VertexBufferIndex = STATIC_U8(EVertexBufferPrimitives::Icosphere);
				drawCallData.IndexBufferIndex = STATIC_U8(EIndexBufferPrimitives::Icosphere);
				drawCallData.VertexStrideIndex = 0;
				drawCallData.VertexOffsetIndex = 0;
				drawCallData.MaterialIndex = 0;

				SGraphicsMaterialAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<SGraphicsMaterialAsset>(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);

				SRenderCommand command;
				command.Type = ERenderCommandType::GBufferDataInstanced;
				command.U32s.push_back(assetID);
				command.DrawCallData.emplace_back(drawCallData);
				command.Materials.push_back(asset->Material);
				command.MaterialRenderTextures.push_back(asset->Material.GetRenderTextures(assetID));
				RenderManager->AddStaticMeshToInstancedRenderList(assetID, &component, assetID);
				RenderManager->PushRenderCommand(command, assetID);

				GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);
			}

			STextureCubeAsset* skybox = GEngine::GetAssetRegistry()->RequestAssetData<STextureCubeAsset>(SAssetReference("Resources/DefaultSkybox.hva"), assetID);
			CStaticRenderTexture skyboxTexture = CStaticRenderTexture();

			if (!skybox)
				HV_LOG_ERROR("No Default Skylight found for rendering material asset thumbnails");
			else
				skyboxTexture = skybox->RenderTexture;

			{
				const SVector4 directionalLightDirection = { 1.0f, 0.0f, 1.0f, 0.0f };
				const SVector4 directionalLightColor = SVector4(1.0f);

				SShadowmapViewData shadowmapView;
				shadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(8.0f, 8.0f, -8.0f, 8.0f);

				SRenderCommand command;
				command.Type = ERenderCommandType::DeferredLightingDirectional;
				command.Vectors.push_back(directionalLightDirection);
				command.Colors.push_back(directionalLightColor);
				command.ShadowmapViews.push_back(shadowmapView);
				command.RenderTextures.push_back(skyboxTexture);
				RenderManager->PushRenderCommand(command, assetID);
			}

			{
				SRenderCommand command;
				command.RenderTextures.push_back(skyboxTexture);
				command.Type = ERenderCommandType::Skybox;
				RenderManager->PushRenderCommand(command, assetID);
			}
			GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference("Resources/DefaultSkybox.hva"), assetID);

			{
				SRenderCommand command;
				command.Type = ERenderCommandType::PreLightingPass;
				RenderManager->PushRenderCommand(command, assetID);
			}

			{
				SRenderCommand command;
				command.Type = ERenderCommandType::PostBaseLightingPass;
				RenderManager->PushRenderCommand(command, assetID);
			}

			{
				SRenderCommand command;
				command.Type = ERenderCommandType::Bloom;
				RenderManager->PushRenderCommand(command, assetID);
			}

			{
				SRenderCommand command;
				command.Type = ERenderCommandType::Tonemapping;
				RenderManager->PushRenderCommand(command, assetID);
			}

			{
				SRenderCommand command;
				command.Type = ERenderCommandType::AntiAliasing;
				RenderManager->PushRenderCommand(command, assetID);
			}

			{
				SRenderCommand command;
				command.Type = ERenderCommandType::GammaCorrection;
				RenderManager->PushRenderCommand(command, assetID);
			}
		}
			break;
		case EAssetType::Animation:
		{
			SSkeletalAnimationAsset* animationAsset = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalAnimationAsset>(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);
			SSkeletalMeshAsset* meshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalMeshAsset>(SAssetReference(animationAsset->RigPath), CAssetRegistry::EditorManagerRequestID);
			F32 aspectRatio = 1.0f;
			F32 marginPercentage = 1.5f;
			SVector2<F32> fov = { aspectRatio * 70.0f, 70.0f };

			STransform cameraTransform;
			cameraTransform.Orbit(SVector4(), SMatrix::CreateRotationFromEuler(30.0f, 30.0f, 0.0f));
			cameraTransform.Translate(SVector(meshAsset->BoundsCenter.X, meshAsset->BoundsCenter.Y, -UMathUtilities::GetFocusDistanceForBounds(meshAsset->BoundsCenter, SVector::GetAbsMaxKeepValue(meshAsset->BoundsMax, meshAsset->BoundsMin), fov, marginPercentage)));
			SMatrix camView = cameraTransform.GetMatrix();
			SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(fov.Y), aspectRatio, 0.001f, 100.0f);

			SRenderCommand command;
			command.Type = ERenderCommandType::SkeletalMeshAssetThumbnail;
			command.DrawCallData = meshAsset->DrawCallData;
			
			GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(animationAsset->RigPath), CAssetRegistry::EditorManagerRequestID);
			GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);

			command.BoneMatrices = GEngine::GetWorld()->GetSystem<CAnimatorGraphSystem>()->ReadAssetAnimationPose(filePath, 0.0f);
			command.Matrices.emplace_back(camView);
			command.Matrices.emplace_back(camProjection);
			RenderManager->PushRenderCommand(command, assetID);
		}
			break;
		case EAssetType::Texture:
		{
			STextureAsset* textureAsset = GEngine::GetAssetRegistry()->RequestAssetData<STextureAsset>(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);

			SRenderCommand command;
			command.Type = ERenderCommandType::TextureDraw;
			command.RenderTextures.push_back(textureAsset->RenderTexture);
			
			GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);

			RenderManager->PushRenderCommand(command, assetID);
		}
			break;
		case EAssetType::TextureCube:
		{
			STextureCubeAsset* textureAsset = GEngine::GetAssetRegistry()->RequestAssetData<STextureCubeAsset>(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);

			{
				SVector location = SVector(0.0f, 0.0f, 1.0f);
				SMatrix camView = SMatrix::LookAtLH(location, SVector(), SVector::Up).FastInverse();
				SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(70.0f), 1.0f, 0.01f, 10.0f);

				SRenderCommand command = SRenderCommand(ERenderCommandType::CameraDataStorage);
				command.Matrices.emplace_back(camView);
				command.Matrices.emplace_back(camProjection);
				RenderManager->PushRenderCommand(command, assetID);
			}

			{
				SRenderCommand command;
				command.Type = ERenderCommandType::TextureCubeDraw;
				command.RenderTextures.push_back(textureAsset->RenderTexture);

				GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);

				RenderManager->PushRenderCommand(command, assetID);
			}
		}
			break;
		default:
			break;
		}
	}

	void CEditorResourceManager::AnimateAssetTexture(SEditorAssetRepresentation* assetRep, const std::string& filePath, const F32 animationTime) const
	{
		U32 assetID = SAssetReference(assetRep->Name).UID;

		switch (assetRep->AssetType)
		{
		case EAssetType::Animation:
		{
			SSkeletalAnimationAsset* animationAsset = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalAnimationAsset>(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);
			SSkeletalMeshAsset* meshAsset = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalMeshAsset>(SAssetReference(animationAsset->RigPath), CAssetRegistry::EditorManagerRequestID);
			F32 aspectRatio = 1.0f;
			F32 marginPercentage = 1.5f;
			SVector2<F32> fov = { aspectRatio * 70.0f, 70.0f };

			STransform cameraTransform;
			cameraTransform.Orbit(SVector4(), SMatrix::CreateRotationFromEuler(30.0f, 30.0f, 0.0f));
			cameraTransform.Translate(SVector(meshAsset->BoundsCenter.X, meshAsset->BoundsCenter.Y, -UMathUtilities::GetFocusDistanceForBounds(meshAsset->BoundsCenter, SVector::GetAbsMaxKeepValue(meshAsset->BoundsMax, meshAsset->BoundsMin), fov, marginPercentage)));
			SMatrix camView = cameraTransform.GetMatrix();
			SMatrix camProjection = SMatrix::PerspectiveFovLH(UMath::DegToRad(fov.Y), aspectRatio, 0.001f, 100.0f);

			SRenderCommand command;
			command.Type = ERenderCommandType::SkeletalMeshAssetThumbnail;
			command.DrawCallData = meshAsset->DrawCallData;

			GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(animationAsset->RigPath), CAssetRegistry::EditorManagerRequestID);
			GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(filePath), CAssetRegistry::EditorManagerRequestID);

			command.BoneMatrices = GEngine::GetWorld()->GetSystem<CAnimatorGraphSystem>()->ReadAssetAnimationPose(filePath, animationTime);
			command.Matrices.emplace_back(camView);
			command.Matrices.emplace_back(camProjection);
			RenderManager->PushRenderCommand(command, assetID);

			//std::vector<SMatrix> boneTransforms = GEngine::GetWorld()->GetSystem<CAnimatorGraphSystem>()->ReadAssetAnimationPose(filePath, animationTime);
			//RenderManager->RenderSkeletalAnimationAssetTexture(assetTexture, filePath, boneTransforms);
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
		case EAssetType::TextureCube:
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

		case EEditorTexture::CameraIcon:
			return ResourceAssetPath + prefix + "CameraIcon" + extension;
		
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

	bool CEditorResourceManager::Init(CEditorManager* editorManager, CRenderManager* renderManager)
	{
		Manager = editorManager;
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
			if (!UFileSystem::Exists(assetPath))
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
