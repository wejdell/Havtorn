// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "RenderSystem.h"
#include "Engine.h"
#include "Scene/World.h"
#include "Scene/Scene.h"
#include "ECS/ECSInclude.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderCommand.h"
#include "ECS/ComponentAlgo.h"
#include "Input/Input.h"
#include "Assets/AssetRegistry.h"

#include <MathTypes/Sphere.h>
#include <MathTypes/Frustum.h>

namespace Havtorn
{
	CRenderSystem::CRenderSystem(CRenderManager* renderManager, CWorld* world)
		: ISystem()
		, RenderManager(renderManager)
		, World(world)
	{
	}

	void CRenderSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		const bool isInPlayingPlayState = World->GetWorldPlayState() == EWorldPlayState::Playing;
		
		// Render View Pre-Pass
		// TODO.NW: Unify?
		std::vector<U64> renderViewEntities = {};
		std::vector<SCameraData> activeCameras = {};
		for (Ptr<CScene>& scene : scenes)
		{
			std::vector<SCameraComponent*> cameraComponents = scene->GetComponents<SCameraComponent>();
			
			for (SCameraComponent* cameraComponent : cameraComponents)
			{
				if (cameraComponent->IsActive)
				{
					renderViewEntities.push_back(cameraComponent->Owner.GUID);
					RenderManager->RequestRenderView(cameraComponent->Owner.GUID);
					
					SCameraData data;
					data.TransformComponent = scene->GetComponent<STransformComponent>(cameraComponent->Owner);
					data.CameraComponent = cameraComponent;
					activeCameras.push_back(data);
				}
				else
				{
					RenderManager->UnrequestRenderView(cameraComponent->Owner.GUID);
				}
			}
		}

		// TODO.NW: Would be cool to explore a render graph solution for this, now that it is more clear what need to happen for every rendered frame
		for (const SCameraData& cameraData : activeCameras)
		{
			SEntity& cameraEntity = cameraData.CameraComponent->Owner;				

			{
				SRenderCommand command;
				command.Type = ERenderCommandType::CameraDataStorage;
				command.Matrices.push_back(cameraData.TransformComponent->Transform.GetMatrix());
				command.Matrices.push_back(cameraData.CameraComponent->ProjectionMatrix);
				RenderManager->PushRenderCommand(command, cameraEntity.GUID);
			}

			for (Ptr<CScene>& scene : scenes)
			{
				// TODO.NW: Skip editor data if Game view mode is active as well
				PushCommandsForScene(scene.get(), cameraEntity.GUID, cameraEntity, !isInPlayingPlayState && cameraEntity == World->GetMainCamera());
			}

			PushUniqueCommands(cameraEntity.GUID);
		}
	}

	void CRenderSystem::PushCommandsForScene(CScene* scene, const U64& renderViewID, const SEntity& cameraEntity, const bool runEditorDataPasses) const
	{
		const std::vector<SDirectionalLightComponent*>& directionalLightComponents = scene->GetComponents<SDirectionalLightComponent>();
		const std::vector<SPointLightComponent*>& pointLightComponents = scene->GetComponents<SPointLightComponent>();
		const std::vector<SSpotLightComponent*>& spotLightComponents = scene->GetComponents<SSpotLightComponent>();

		const SEntity& editorRenderExemptEntity = GEngine::GetWorld()->GetEditorRenderExemptEntity();

		// TODO.NW: Consider sending in CameraData instead of entity?
		const STransformComponent* cameraTransform = scene->GetComponent<STransformComponent>(cameraEntity);
		if (!SComponent::IsValid(cameraTransform))
			return;

		const SCameraComponent* cameraComponent = scene->GetComponent<SCameraComponent>(cameraEntity);
		if (!SComponent::IsValid(cameraComponent))
			return;

		const SFrustum cameraFrustum = SFrustum(cameraTransform->Transform.GetMatrix(), cameraComponent->ProjectionMatrix);

		for (const SStaticMeshComponent* staticMeshComponent : scene->GetComponents<SStaticMeshComponent>())
		{
			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(staticMeshComponent);
			SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(staticMeshComponent);

			if (!SComponent::IsValid(staticMeshComponent) || !SComponent::IsValid(transformComp) || !SComponent::IsValid(materialComp))
				continue;

			const bool deferCommand = staticMeshComponent->Owner == editorRenderExemptEntity;
			U32 meshID = staticMeshComponent->AssetReference.UID + UComponentAlgo::CalculateAggregateMaterialID(materialComp);
			if (deferCommand)
				meshID += 1000;

			SStaticMeshAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<SStaticMeshAsset>(staticMeshComponent->AssetReference, staticMeshComponent->Owner.GUID);
			if (asset == nullptr)
				continue;			

			// TODO.NW: This is still not a super good culling system, would probably want to resolve for every light if a particular instance uses it for shadow casting?
			// Not very fond of the idea of saving an instance list for each light source. Should probably look more closely at cascaded shadow maps
			// https://learnopengl.com/Guest-Articles/2021/CSM
			// https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf

			const SSphere meshBoundingSphere = SSphere((SVector4(asset->BoundsCenter, 1.0f) * transformComp->Transform.GetMatrix()).ToVector3(), asset->BoundsMin.Distance(asset->BoundsMax) * 0.5f);
			if (IsCulled(scene, meshBoundingSphere, cameraFrustum))
				continue;

			// NW: Note that all registered instances of any mesh ID will be used for shadowcasting if that mesh ID has been registered for any light
			if (!RenderManager->IsMeshShadowCastingForType(meshID, ERenderCommandType::ShadowAtlasPrePassDirectional, renderViewID))
			{
				for (const SDirectionalLightComponent* directionalLightComp : directionalLightComponents)
				{
					if (!SComponent::IsValid(directionalLightComp) || !directionalLightComp->IsActive)
						continue;

					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassDirectional;
						command.ShadowmapViews.push_back(directionalLightComp->ShadowmapView);
						command.U32s.push_back(meshID);
						command.DrawCallData = asset->DrawCallData;
						RenderManager->PushRenderCommand(command, renderViewID);
					}
				}
				RenderManager->AddMeshShadowCastingForType(meshID, ERenderCommandType::ShadowAtlasPrePassDirectional, renderViewID);
			}

			if (!RenderManager->IsMeshShadowCastingForType(meshID, ERenderCommandType::ShadowAtlasPrePassPoint, renderViewID))
			{
				for (const SPointLightComponent* pointLightComp : pointLightComponents)
				{
					if (!SComponent::IsValid(pointLightComp) || !pointLightComp->IsActive)
						continue;

					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassPoint;
						command.Matrices.push_back(transformComp->Transform.GetMatrix());
						command.U32s.push_back(meshID);
						command.DrawCallData = asset->DrawCallData;
						command.SetShadowMapViews(pointLightComp->ShadowmapViews);
						RenderManager->PushRenderCommand(command, renderViewID);
					}
				}
				RenderManager->AddMeshShadowCastingForType(meshID, ERenderCommandType::ShadowAtlasPrePassPoint, renderViewID);
			}

			if (!RenderManager->IsMeshShadowCastingForType(meshID, ERenderCommandType::ShadowAtlasPrePassSpot, renderViewID))
			{
				for (const SSpotLightComponent* spotLightComp : spotLightComponents)
				{
					if (!SComponent::IsValid(spotLightComp) || !spotLightComp->IsActive)
						continue;

					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassSpot;
						command.Matrices.push_back(transformComp->Transform.GetMatrix());
						command.U32s.push_back(meshID);
						command.DrawCallData = asset->DrawCallData;
						command.ShadowmapViews.push_back(spotLightComp->ShadowmapView);
						RenderManager->PushRenderCommand(command, renderViewID);
					}
				}
				RenderManager->AddMeshShadowCastingForType(meshID, ERenderCommandType::ShadowAtlasPrePassSpot, renderViewID);
			}

			if (!RenderManager->IsStaticMeshInInstancedRenderList(meshID, renderViewID)) // if static, if instanced
			{
				if (materialComp->AssetReferences.size() != asset->NumberOfMaterials)
					materialComp->AssetReferences.resize(asset->NumberOfMaterials, SAssetReference("Resources/M_MeshPreview.hva"));

				std::vector<SGraphicsMaterialAsset*> materialAssets = GEngine::GetAssetRegistry()->RequestAssetData<SGraphicsMaterialAsset>(materialComp->AssetReferences, materialComp->Owner.GUID);

				if (!runEditorDataPasses)
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferDataInstanced;
					command.U32s.push_back(meshID);
					command.DrawCallData = asset->DrawCallData;

					for (SGraphicsMaterialAsset* materialAsset : materialAssets)
					{
						command.Materials.push_back(materialAsset->Material);
						command.MaterialRenderTextures.push_back(std::move(materialAsset->Material.GetRenderTextures(materialComp->Owner.GUID)));
					}

					if (deferCommand)
						command.InternalPriority++;
					
					RenderManager->PushRenderCommand(command, renderViewID);
				}
				else
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferDataInstancedEditor;
					command.U32s.push_back(meshID);
					command.DrawCallData = asset->DrawCallData;

					for (SGraphicsMaterialAsset* materialAsset : materialAssets)
					{
						command.Materials.push_back(materialAsset->Material);
						command.MaterialRenderTextures.push_back(std::move(materialAsset->Material.GetRenderTextures(materialComp->Owner.GUID)));
					}

					if (deferCommand)
						command.InternalPriority++;

					RenderManager->PushRenderCommand(command, renderViewID);
				}
			}

			// TODO.NW: Might want to have two separate instance lists for shadowcasters vs visible instances?
			// Should we add a boolean property for enabling shadowcasting so we can directly skip culling checks otherwise?
			RenderManager->AddStaticMeshToInstancedRenderList(meshID, transformComp, renderViewID);
		}

		for (const SSkeletalMeshComponent* skeletalMeshComponent : scene->GetComponents<SSkeletalMeshComponent>())
		{
			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(skeletalMeshComponent);
			SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(skeletalMeshComponent);

			if (!SComponent::IsValid(skeletalMeshComponent) || !SComponent::IsValid(transformComp) || !SComponent::IsValid(materialComp))
				continue;

			const bool deferCommand = skeletalMeshComponent->Owner == editorRenderExemptEntity;
			U32 meshID = skeletalMeshComponent->AssetReference.UID + UComponentAlgo::CalculateAggregateMaterialID(materialComp);
			if (deferCommand)
				meshID += 1000;

			if (!RenderManager->IsSkeletalMeshInInstancedRenderList(meshID, renderViewID))
			{
				// TODO.NR: Make shadow pass for skeletal meshes possible
				//for (const SDirectionalLightComponent* directionalLightComp : directionalLightComponents)
				//{
				//	if (SComponent::IsValid(directionalLightComp))
				//	{
				//		SRenderCommand command;
				//		command.Type = ERenderCommandType::ShadowAtlasPrePassDirectional;
				//		command.ShadowmapViews.push_back(directionalLightComp->ShadowmapView);
				//		command.Matrices.push_back(transformComp->Transform.GetMatrix());
				//		command.U64s.push_back(skeletalMeshComponent->AssetUID);
				//		command.DrawCallData = skeletalMeshComponent->DrawCallData;
				//		RenderManager->PushRenderCommand(command, renderViewID);
				//	}
				//}

				//for (const SPointLightComponent* pointLightComp : pointLightComponents)
				//{
				//	if (SComponent::IsValid(pointLightComp))
				//	{
				//		SRenderCommand command;
				//		command.Type = ERenderCommandType::ShadowAtlasPrePassPoint;
				//		command.Matrices.push_back(transformComp->Transform.GetMatrix());
				//		command.U64s.push_back(skeletalMeshComponent->AssetUID);
				//		command.DrawCallData = skeletalMeshComponent->DrawCallData;
				//		command.SetShadowMapViews(pointLightComp->ShadowmapViews);
				//		RenderManager->PushRenderCommand(command, renderViewID);
				//	}
				//}

				//for (const SSpotLightComponent* spotLightComp : spotLightComponents)
				//{
				//	if (SComponent::IsValid(spotLightComp))
				//	{
				//		SRenderCommand command;
				//		command.Type = ERenderCommandType::ShadowAtlasPrePassSpot;
				//		command.Matrices.push_back(transformComp->Transform.GetMatrix());
				//		command.U64s.push_back(skeletalMeshComponent->AssetUID);
				//		command.DrawCallData = skeletalMeshComponent->DrawCallData;
				//		command.ShadowmapViews.push_back(spotLightComp->ShadowmapView);
				//		RenderManager->PushRenderCommand(command, renderViewID);
				//	}
				//}

				if (!SComponent::IsValid(scene->GetComponent<SSkeletalAnimationComponent>(transformComp)))
					continue;

				SSkeletalMeshAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<SSkeletalMeshAsset>(skeletalMeshComponent->AssetReference, skeletalMeshComponent->Owner.GUID);
				if (asset == nullptr)
					continue;

				if (materialComp->AssetReferences.size() != asset->NumberOfMaterials)
					materialComp->AssetReferences.resize(asset->NumberOfMaterials, SAssetReference("Resources/M_MeshPreview.hva"));

				std::vector<SGraphicsMaterialAsset*> materialAssets = GEngine::GetAssetRegistry()->RequestAssetData<SGraphicsMaterialAsset>(materialComp->AssetReferences, materialComp->Owner.GUID);

				if (!runEditorDataPasses)
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferSkeletalInstanced;
					command.Matrices.push_back(transformComp->Transform.GetMatrix());
					command.U32s.push_back(meshID);
					command.DrawCallData = asset->DrawCallData;

					for (SGraphicsMaterialAsset* materialAsset : materialAssets)
					{
						command.Materials.push_back(materialAsset->Material);
						command.MaterialRenderTextures.push_back(std::move(materialAsset->Material.GetRenderTextures(materialComp->Owner.GUID)));
					}

					if (deferCommand)
						command.InternalPriority++;

					RenderManager->PushRenderCommand(command, renderViewID);
				}
				else
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferSkeletalInstancedEditor;
					command.Matrices.push_back(transformComp->Transform.GetMatrix());
					command.U32s.push_back(meshID);
					command.DrawCallData = asset->DrawCallData;

					for (SGraphicsMaterialAsset* materialAsset : materialAssets)
					{
						command.Materials.push_back(materialAsset->Material);
						command.MaterialRenderTextures.push_back(std::move(materialAsset->Material.GetRenderTextures(materialComp->Owner.GUID)));
					}

					if (deferCommand)
						command.InternalPriority++;

					RenderManager->PushRenderCommand(command, renderViewID);
				}
			}

			RenderManager->AddSkeletalMeshToInstancedRenderList(meshID, transformComp, scene->GetComponent<SSkeletalAnimationComponent>(transformComp), renderViewID);
		}

		for (const SDecalComponent* decalComponent : scene->GetComponents<SDecalComponent>())
		{
			if (!SComponent::IsValid(decalComponent))
				continue;

			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(decalComponent);
			std::vector<STextureAsset*> assets = GEngine::GetAssetRegistry()->RequestAssetData<STextureAsset>(decalComponent->AssetReferences, transformComp->Owner.GUID);

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredDecal;
			command.Matrices.push_back(transformComp->Transform.GetMatrix());

			if (assets[0] != nullptr)
			{
				command.Flags.push_back(decalComponent->ShouldRenderAlbedo);
				command.RenderTextures.push_back(assets[0]->RenderTexture);
			}
			else
			{
				command.Flags.push_back(false);
				command.RenderTextures.push_back(CStaticRenderTexture());
			}

			if (assets[1] != nullptr)
			{
				command.Flags.push_back(decalComponent->ShouldRenderMaterial);
				command.RenderTextures.push_back(assets[1]->RenderTexture);
			}
			else
			{
				command.Flags.push_back(false);
				command.RenderTextures.push_back(CStaticRenderTexture());
			}

			if (assets[2] != nullptr)
			{
				command.Flags.push_back(decalComponent->ShouldRenderNormal);
				command.RenderTextures.push_back(assets[2]->RenderTexture);
			}
			else
			{
				command.Flags.push_back(false);
				command.RenderTextures.push_back(CStaticRenderTexture());
			}

			RenderManager->PushRenderCommand(command, renderViewID);
		}

		for (const SDirectionalLightComponent* directionalLightComp : directionalLightComponents)
		{
			if (!SComponent::IsValid(directionalLightComp))
				continue;

			const SEntity& closestEnvironmentLightEntity = UComponentAlgo::GetClosestEntity3D(directionalLightComp->Owner, scene->GetComponents<SEnvironmentLightComponent>(), scene);
			const SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(closestEnvironmentLightEntity);
			if (!SComponent::IsValid(environmentLightComp))
				continue;

			STextureCubeAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<STextureCubeAsset>(environmentLightComp->AssetReference, environmentLightComp->Owner.GUID);
			if (asset == nullptr)
				continue;

			SRenderCommand command;
			if (directionalLightComp->IsActive)
			{
				command.Type = ERenderCommandType::DeferredLightingDirectional;
				command.Vectors.push_back(directionalLightComp->Direction);
				command.Colors.push_back(directionalLightComp->Color);
				command.ShadowmapViews.push_back(directionalLightComp->ShadowmapView);
				command.RenderTextures.push_back(asset->RenderTexture);
				RenderManager->PushRenderCommand(command, renderViewID);
			}

			if (const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(directionalLightComp))
			{
				if (directionalLightComp->IsActive && volumetricLightComp->IsActive)
				{
					command.Type = ERenderCommandType::VolumetricLightingDirectional;
					command.SetVolumetricDataFromComponent(*volumetricLightComp);
					RenderManager->PushRenderCommand(command, renderViewID);
				}
			}
		}

		for (const SPointLightComponent* pointLightComp : pointLightComponents)
		{
			if (!SComponent::IsValid(pointLightComp))
				continue;

			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(pointLightComp);

			SRenderCommand command;
			if (pointLightComp->IsActive)
			{
				command.Type = ERenderCommandType::DeferredLightingPoint;
				command.Matrices.push_back(transformComp->Transform.GetMatrix());
				command.Colors.push_back(SColor(pointLightComp->ColorAndIntensity.X, pointLightComp->ColorAndIntensity.Y, pointLightComp->ColorAndIntensity.Z, 1.0f));
				command.F32s.push_back(pointLightComp->ColorAndIntensity.W);
				command.F32s.push_back(pointLightComp->Range);
				command.SetShadowMapViews(pointLightComp->ShadowmapViews);
				RenderManager->PushRenderCommand(command, renderViewID);
			}

			if (const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(pointLightComp))
			{
				if (pointLightComp->IsActive && volumetricLightComp->IsActive)
				{
					command.Type = ERenderCommandType::VolumetricLightingPoint;
					command.SetVolumetricDataFromComponent(*volumetricLightComp);
					RenderManager->PushRenderCommand(command, renderViewID);
				}
			}
		}

		for (const SSpotLightComponent* spotLightComp : spotLightComponents)
		{
			if (!SComponent::IsValid(spotLightComp))
				continue;

			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(spotLightComp);

			SRenderCommand command;
			if (spotLightComp->IsActive)
			{
				command.Type = ERenderCommandType::DeferredLightingSpot;
				command.Matrices.push_back(transformComp->Transform.GetMatrix());
				command.Colors.push_back(SColor(spotLightComp->ColorAndIntensity.X, spotLightComp->ColorAndIntensity.Y, spotLightComp->ColorAndIntensity.Z, 1.0f));
				command.F32s.push_back(spotLightComp->ColorAndIntensity.W);
				command.F32s.push_back(spotLightComp->Range);
				command.F32s.push_back(spotLightComp->OuterAngle);
				command.F32s.push_back(spotLightComp->InnerAngle);
				command.Vectors.push_back(spotLightComp->Direction);
				command.Vectors.push_back(spotLightComp->DirectionNormal1);
				command.Vectors.push_back(spotLightComp->DirectionNormal2);
				command.ShadowmapViews.push_back(spotLightComp->ShadowmapView);
				RenderManager->PushRenderCommand(command, renderViewID);
			}

			if (const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(spotLightComp))
			{
				if (spotLightComp->IsActive && volumetricLightComp->IsActive)
				{
					command.Type = ERenderCommandType::VolumetricLightingSpot;
					command.SetVolumetricDataFromComponent(*volumetricLightComp);
					RenderManager->PushRenderCommand(command, renderViewID);
				}
			}
		}

		// TODO.NW: Do we need to find just one closest environmentlight from all of the scenes?
		{
			const SEntity& closestEnvironmentLightEntity = UComponentAlgo::GetClosestEntity3D(cameraEntity, scene->GetComponents<SEnvironmentLightComponent>(), scene);
			if (closestEnvironmentLightEntity.IsValid())
			{
				const SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(closestEnvironmentLightEntity);
				if (SComponent::IsValid(environmentLightComp))
				{
					STextureCubeAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<STextureCubeAsset>(environmentLightComp->AssetReference, environmentLightComp->Owner.GUID);
					if (asset != nullptr)
					{
						SRenderCommand command;
						command.RenderTextures.push_back(asset->RenderTexture);
						command.Type = ERenderCommandType::Skybox;
						RenderManager->PushRenderCommand(command, renderViewID);
					}
				}
			}
		}

		for (const SSpriteComponent* spriteComp : scene->GetComponents<SSpriteComponent>())
		{
			if (!SComponent::IsValid(spriteComp))
				continue;

			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(spriteComp);
			const STransform2DComponent* transform2DComp = scene->GetComponent<STransform2DComponent>(spriteComp);
			STextureAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<STextureAsset>(spriteComp->AssetReference, spriteComp->Owner.GUID);
			if (asset == nullptr)
				continue;

			if (SComponent::IsValid(transformComp))
			{
				if (!RenderManager->IsSpriteInWorldSpaceInstancedRenderList(spriteComp->AssetReference.UID, renderViewID))
				{
					// NW: Don't push a command every time
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferSpriteInstanced;
					command.U32s.push_back(spriteComp->AssetReference.UID);
					command.RenderTextures.push_back(asset->RenderTexture);
					RenderManager->PushRenderCommand(command, renderViewID);
				}

				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(spriteComp->AssetReference.UID, transformComp, spriteComp, renderViewID);
			}
			else if (SComponent::IsValid(transform2DComp))
			{
				if (!RenderManager->IsSpriteInScreenSpaceInstancedRenderList(spriteComp->AssetReference.UID, renderViewID))
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::ScreenSpaceSprite;
					command.U32s.push_back(spriteComp->AssetReference.UID);
					command.RenderTextures.push_back(asset->RenderTexture);
					RenderManager->PushRenderCommand(command, renderViewID);
				}

				RenderManager->AddSpriteToScreenSpaceInstancedRenderList(spriteComp->AssetReference.UID, transform2DComp, spriteComp, renderViewID);
			}
		}

		for (const SUICanvasComponent* uiCanvasComp : scene->GetComponents<SUICanvasComponent>())
		{
			if (!SComponent::IsValid(uiCanvasComp) || !uiCanvasComp->IsActive)
				continue;

			const STransform2DComponent* transform2DComp = scene->GetComponent<STransform2DComponent>(uiCanvasComp);
			if (SComponent::IsValid(transform2DComp))
			{
				for (const SUIElement& element : uiCanvasComp->Elements)
				{
					if (element.StateAssetReferences.size() != STATIC_U64(EUIElementState::Count))
						continue;

					const SAssetReference& assetReference = element.StateAssetReferences[STATIC_U8(element.State)];
					if (!assetReference.IsValid())
						continue;

					STextureAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<STextureAsset>(assetReference, uiCanvasComp->Owner.GUID);
					if (asset == nullptr)
						continue;

					if (!RenderManager->IsSpriteInScreenSpaceInstancedRenderList(assetReference.UID, renderViewID))
					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ScreenSpaceUISprite;
						command.U32s.push_back(assetReference.UID);
						command.RenderTextures.push_back(asset->RenderTexture);
						RenderManager->PushRenderCommand(command, renderViewID);
					}

					RenderManager->AddSpriteToScreenSpaceInstancedRenderList(assetReference.UID, transform2DComp, element, renderViewID);
				}
			}
		}
	}

	void CRenderSystem::PushUniqueCommands(const U64& renderViewID) const
	{
		// NW: Unique commands that are added once per active camera - automatically sorted into heap

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::DecalDepthCopy;
			RenderManager->PushRenderCommand(command, renderViewID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::PreLightingPass;
			RenderManager->PushRenderCommand(command, renderViewID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::PostBaseLightingPass;
			RenderManager->PushRenderCommand(command, renderViewID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::VolumetricBufferBlurPass;
			RenderManager->PushRenderCommand(command, renderViewID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::Bloom;
			RenderManager->PushRenderCommand(command, renderViewID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::Tonemapping;
			RenderManager->PushRenderCommand(command, renderViewID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::AntiAliasing;
			RenderManager->PushRenderCommand(command, renderViewID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::GammaCorrection;
			RenderManager->PushRenderCommand(command, renderViewID);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::RendererDebug;
			RenderManager->PushRenderCommand(command, renderViewID);
		}
	}

	bool CRenderSystem::IsCulled(CScene* scene, const SSphere& boundingSphere, const SFrustum& cameraFrustum) const
	{
		if (cameraFrustum.Intersects(boundingSphere))
			return false;

		// TODO.NW: Lights themselves should be culled in a pass before this, based on camera location and their own bounding geometry,
		// then the culled set should be used in this function

		for (const SDirectionalLightComponent* directionalLightComp : scene->GetComponents<SDirectionalLightComponent>())
		{
			if (!SComponent::IsValid(directionalLightComp) || !directionalLightComp->IsActive)
				continue;

			const SFrustum lightFrustum = SFrustum(directionalLightComp->ShadowmapView.ShadowViewMatrix, directionalLightComp->ShadowmapView.ShadowProjectionMatrix);
			if (lightFrustum.Intersects(boundingSphere))
				return false;
		}

		for (const SPointLightComponent* pointLightComp : scene->GetComponents<SPointLightComponent>())
		{
			if (!SComponent::IsValid(pointLightComp) || !pointLightComp->IsActive)
				continue;

			const STransformComponent* pointLightTransform = scene->GetComponent<STransformComponent>(pointLightComp);
			if (!SComponent::IsValid(pointLightTransform))
				continue;

			if (SSphere(pointLightTransform->Transform.GetMatrix().GetTranslation(), pointLightComp->Range).Intersects(boundingSphere))
				return false;
		}

		for (const SSpotLightComponent* spotLightComp : scene->GetComponents<SSpotLightComponent>())
		{
			if (!SComponent::IsValid(spotLightComp) || !spotLightComp->IsActive)
				continue;

			const SFrustum lightFrustum = SFrustum(spotLightComp->ShadowmapView.ShadowViewMatrix, spotLightComp->ShadowmapView.ShadowProjectionMatrix);
			if (lightFrustum.Intersects(boundingSphere))
				return false;
		}

		return true;
	}
}
