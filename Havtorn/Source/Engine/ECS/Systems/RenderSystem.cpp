// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "RenderSystem.h"
#include "Engine.h"
#include "Scene/World.h"
#include "Scene/Scene.h"
#include "ECS/ECSInclude.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderCommand.h"
#include "Graphics/RenderGraph.h"
#include "Graphics/RenderResourceRegistry.h"
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
				PushCommandsForScene(scene.get(), cameraEntity.GUID, cameraEntity, !isInPlayingPlayState && cameraEntity == World->GetMainCamera(), true);
			}

			PushUniqueCommands(cameraEntity.GUID);
		}
	}

	void CRenderSystem::PushCommandsForScene(CScene* scene, const U64& renderViewID, const SEntity& cameraEntity, const bool runEditorDataPasses, const bool doCulling) const
	{
		const std::vector<SDirectionalLightComponent*>& directionalLightComponents = scene->GetComponents<SDirectionalLightComponent>();
		const std::vector<SPointLightComponent*>& pointLightComponents = scene->GetComponents<SPointLightComponent>();
		const std::vector<SSpotLightComponent*>& spotLightComponents = scene->GetComponents<SSpotLightComponent>();

		const SEntity& editorRenderExemptEntity = GEngine::GetWorld()->GetEditorRenderExemptEntity();

		CRenderGraph& renderGraph = RenderManager->GetRenderGraph();

		SFrustum cameraFrustum = SFrustum(SMatrix::Identity, SMatrix::Identity);
		if (doCulling)
		{
			// TODO.NW: Consider sending in CameraData instead of entity?
			const STransformComponent* cameraTransform = scene->GetComponent<STransformComponent>(cameraEntity);
			if (!SComponent::IsValid(cameraTransform))
				return;

			const SCameraComponent* cameraComponent = scene->GetComponent<SCameraComponent>(cameraEntity);
			if (!SComponent::IsValid(cameraComponent))
				return;

			cameraFrustum = SFrustum(cameraTransform->Transform.GetMatrix(), cameraComponent->ProjectionMatrix);
		}

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

			const SMatrix& transformMatrix = transformComp->Transform.GetMatrix();
			// NW: This seems to give the right results, notice the Hadamard multiplication between bounds vector and scale
			const F32 radius = ((asset->BoundsMax - asset->BoundsMin) * transformMatrix.GetScale()).Size();
			const SSphere meshBoundingSphere = SSphere((SVector4(asset->BoundsCenter, 1.0f) * transformMatrix).ToVector3(), radius);
			if (doCulling && IsCulled(scene, meshBoundingSphere, cameraFrustum))
				continue;
			std::vector<SGraphicsMaterialAsset*> materialAssets = GEngine::GetAssetRegistry()->RequestAssetData<SGraphicsMaterialAsset>(materialComp->AssetReferences, materialComp->Owner.GUID);

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

	void CRenderSystem::AddPasses(SRenderView* renderView, const bool runEditorDataPasses)
	{
		// TODO.NW: Put passes for thumbnail rendering in Editor project?

		SVector2<F32> shadowAtlasResolution = RenderManager->GetShadowAtlasResolution();

		struct SShadowmapPrePassData : public SRenderPassParams
		{
			SRenderResourceHandle ViewDataBufferHandle;
			SRenderResourceHandle TransformBufferHandle;

			SRenderResourceHandle EmptyRenderTargetHandle;
			SRenderResourceHandle ShadowmapRenderTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SShadowmapPrePassData>("ShadowmapPrePass",
			[&](CRenderResourceRegistry& registry)
			{
				SShadowmapPrePassData passData;

				passData.TransformBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("Shadowmap Frame Buffer"));

				// TODO.NW: Figure out if it's reasonable to bind any other existing render target here, do they even need to be the same dimensions?
				passData.EmptyRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Empty Render Target", ERenderResourceFormat::R8_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y))));
				passData.ShadowmapRenderTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y))));

				return passData;
			},
			[](const SShadowmapPrePassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SMainTargetRenderData targetData;
				targetData.MainRenderTarget = registry.GetResource(data.EmptyRenderTargetHandle);
				targetData.DepthTexture = registry.GetResource(data.ShadowmapRenderTargetHandle);
				renderManager->LightRenderer.SetupShadowPrepassRender(targetData);

				{
					SStaticMeshShadowRenderData rendererData;

					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);

					for (auto& [lightCollectionIndex, shadowCasterDataCollection] : renderView.DirectionalLightShadowCasterData)
					{
						const SDirectionalLightDrawData& lightSourceData = renderView.DirectionalLightDrawData[lightCollectionIndex];

						rendererData.ShadowmapViewportIndex = lightSourceData.ShadowmapViewportIndex;
						const SShadowmapBufferData& bufferData = lightSourceData.ShadowmapBufferData;
						SFrameBufferData frameBufferData;
						frameBufferData.ToCameraFromWorld = bufferData.ToShadowmapView;
						frameBufferData.ToWorldFromCamera = bufferData.ToShadowmapView.FastInverse();
						frameBufferData.ToProjectionFromCamera = bufferData.ToShadowmapProjection;
						frameBufferData.ToCameraFromProjection = bufferData.ToShadowmapProjection.Inverse();
						frameBufferData.CameraPosition = bufferData.ShadowmapPosition;
						rendererData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

						for (const SStaticShadowCasterData& shadowCasterData : shadowCasterDataCollection)
						{
							rendererData.TransformBuffer->DataBuffer->BindBuffer(shadowCasterData.Transforms);
							rendererData.InstanceCount = STATIC_U32(shadowCasterData.Transforms.size());

							for (const SPrimitiveData& primitiveData : shadowCasterData.PrimitiveData)
							{
								rendererData.VertexBuffer = registry.GetResource(primitiveData.VertexBufferHandle);
								rendererData.IndexBuffer = registry.GetResource(primitiveData.IndexBufferHandle);
								rendererData.IndexCount = primitiveData.IndexCount;

								renderManager->LightRenderer.RenderStaticMeshDepthPrePass(rendererData);
							}
						}
					}

					for (auto& [lightCollectionIndex, shadowCasterDataCollection] : renderView.PointLightShadowCasterData)
					{
						const SPointLightDrawData& lightSourceData = renderView.PointLightDrawData[lightCollectionIndex];

						rendererData.ShadowmapViewportIndex = lightSourceData.ShadowmapViewportIndex;
						for (const SShadowmapBufferData& bufferData : lightSourceData.ShadowmapBufferData)
						{
							SFrameBufferData frameBufferData;
							frameBufferData.ToCameraFromWorld = bufferData.ToShadowmapView;
							frameBufferData.ToWorldFromCamera = bufferData.ToShadowmapView.FastInverse();
							frameBufferData.ToProjectionFromCamera = bufferData.ToShadowmapProjection;
							frameBufferData.ToCameraFromProjection = bufferData.ToShadowmapProjection.Inverse();
							frameBufferData.CameraPosition = bufferData.ShadowmapPosition;
							rendererData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

							for (const SStaticShadowCasterData& shadowCasterData : shadowCasterDataCollection)
							{
								rendererData.TransformBuffer->DataBuffer->BindBuffer(shadowCasterData.Transforms);
								rendererData.InstanceCount = STATIC_U32(shadowCasterData.Transforms.size());

								for (const SPrimitiveData& primitiveData : shadowCasterData.PrimitiveData)
								{
									rendererData.VertexBuffer = registry.GetResource(primitiveData.VertexBufferHandle);
									rendererData.IndexBuffer = registry.GetResource(primitiveData.IndexBufferHandle);
									rendererData.IndexCount = primitiveData.IndexCount;

									renderManager->LightRenderer.RenderStaticMeshDepthPrePass(rendererData);
								}
							}
						}
					}

					for (auto& [lightCollectionIndex, shadowCasterDataCollection] : renderView.SpotlightShadowCasterData)
					{
						const SSpotlightDrawData& lightSourceData = renderView.SpotlightDrawData[lightCollectionIndex];

						rendererData.ShadowmapViewportIndex = lightSourceData.ShadowmapViewportIndex;
						const SShadowmapBufferData& bufferData = lightSourceData.ShadowmapBufferData;
						SFrameBufferData frameBufferData;
						frameBufferData.ToCameraFromWorld = bufferData.ToShadowmapView;
						frameBufferData.ToWorldFromCamera = bufferData.ToShadowmapView.FastInverse();
						frameBufferData.ToProjectionFromCamera = bufferData.ToShadowmapProjection;
						frameBufferData.ToCameraFromProjection = bufferData.ToShadowmapProjection.Inverse();
						frameBufferData.CameraPosition = bufferData.ShadowmapPosition;
						rendererData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

						for (const SStaticShadowCasterData& shadowCasterData : shadowCasterDataCollection)
						{
							rendererData.TransformBuffer->DataBuffer->BindBuffer(shadowCasterData.Transforms);
							rendererData.InstanceCount = STATIC_U32(shadowCasterData.Transforms.size());

							for (const SPrimitiveData& primitiveData : shadowCasterData.PrimitiveData)
							{
								rendererData.VertexBuffer = registry.GetResource(primitiveData.VertexBufferHandle);
								rendererData.IndexBuffer = registry.GetResource(primitiveData.IndexBufferHandle);
								rendererData.IndexCount = primitiveData.IndexCount;

								renderManager->LightRenderer.RenderStaticMeshDepthPrePass(rendererData);
							}
						}
					}
				}

				{
					// TODO.NW: Skeletal mesh shadows
				}
			}
		);

		struct SGBufferBasePassData : public SRenderPassParams
		{
			SRenderResourceHandle ViewDataBufferHandle;
			SRenderResourceHandle TransformBufferHandle;
			SRenderResourceHandle MaterialBufferHandle;
			SRenderResourceHandle BoneBufferHandle;
			SRenderResourceHandle UVRectBufferHandle;
			SRenderResourceHandle ColorBufferHandle;
			SRenderResourceHandle EntityBufferHandle;

			SRenderResourceHandle GBufferAlbedoTargetHandle;
			SRenderResourceHandle GBufferWNormalTargetHandle;
			SRenderResourceHandle GBufferVNormalTargetHandle;
			SRenderResourceHandle GBufferMaterialTargetHandle;
			SRenderResourceHandle GBufferWorldPositionTargetHandle;
			SRenderResourceHandle GBufferEditorDataTargetHandle;
			SRenderResourceHandle IntermediateDepthTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SGBufferBasePassData>("GBufferBasePass",
			[&](CRenderResourceRegistry& registry)
			{
				SGBufferBasePassData passData;

				passData.AddDependency<SShadowmapPrePassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.TransformBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.UVRectBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SVector4>("Instanced UV Rect Buffer", InstancedDrawInstanceLimit));
				passData.ColorBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SVector4>("Instanced Color Buffer", InstancedDrawInstanceLimit));
				passData.EntityBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SEntity>("Instanced Entity Buffer", InstancedDrawInstanceLimit));
				passData.MaterialBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SMaterialBufferData>("Material Buffer"));
				passData.BoneBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SBoneBufferData>("Bone Buffer"));

				// NOTE.NW: It's interesting to note here that whenever we're rendering geometry, it's assumed we'd like to do it to a full resolution target. 
				// That may not be the case, and we might want to make use of the render view parameter in this case or have some option to decide for each pass 
				// how it should be handled.

				passData.GBufferAlbedoTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt));
				passData.GBufferWNormalTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt));
				passData.GBufferVNormalTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt));
				passData.GBufferMaterialTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt));

				if (runEditorDataPasses)
				{
					passData.GBufferWorldPositionTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer World Position", ERenderResourceFormat::R32G32B32A32_Float));
					passData.GBufferEditorDataTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Editor Data", ERenderResourceFormat::R32G32_UnsignedInt));
				}

				passData.IntermediateDepthTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless));

				return passData;
			},
			[=](const SGBufferBasePassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData constantRenderData;
				constantRenderData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				constantRenderData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SGeometryTargetRenderData targetRenderData;
				targetRenderData.AlbedoRenderTarget = registry.GetResource(data.GBufferAlbedoTargetHandle);
				targetRenderData.WorldNormalRenderTarget = registry.GetResource(data.GBufferWNormalTargetHandle);
				targetRenderData.VertexNormalRenderTarget = registry.GetResource(data.GBufferVNormalTargetHandle);
				targetRenderData.MaterialRenderTarget = registry.GetResource(data.GBufferMaterialTargetHandle);
				targetRenderData.DepthTexture = registry.GetResource(data.IntermediateDepthTargetHandle);

				SEditorRenderData editorData;
				if (runEditorDataPasses)
				{
					editorData.WorldPositionRenderTarget = registry.GetResource(data.GBufferWorldPositionTargetHandle);
					editorData.EditorDataRenderTarget = registry.GetResource(data.GBufferEditorDataTargetHandle);
					editorData.EntityBuffer = registry.GetResource(data.EntityBufferHandle);

					renderManager->GeometryRenderer.SetupRender(constantRenderData, targetRenderData, editorData);
				}
				else
				{
					renderManager->GeometryRenderer.SetupRender(constantRenderData, targetRenderData);
				}

				{
					SStaticMeshRenderData rendererData;
					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.MaterialBuffer = registry.GetResource(data.MaterialBufferHandle);

					for (auto& [meshID, meshData] : renderView.StaticMeshDrawData) // Static Mesh Level
					{
						rendererData.TransformBuffer->DataBuffer->BindBuffer(meshData.Transforms);

						if (runEditorDataPasses)
							editorData.EntityBuffer->DataBuffer->BindBuffer(meshData.Entities);

						for (const SMeshDrawData& drawData : meshData.DrawData) // Sub Mesh Level
						{
							for (const SRenderResourceHandle& handle : drawData.MaterialTextureHandles) // Material Texture Level
								rendererData.MaterialTextures.emplace_back(registry.GetResource(handle));

							rendererData.MaterialBuffer->DataBuffer->BindBuffer(drawData.MaterialBufferData);

							rendererData.VertexBuffer = registry.GetResource(drawData.PrimitiveData.VertexBufferHandle);
							rendererData.IndexBuffer = registry.GetResource(drawData.PrimitiveData.IndexBufferHandle);

							rendererData.IndexCount = drawData.PrimitiveData.IndexCount;
							rendererData.InstanceCount = STATIC_U32(meshData.Transforms.size());

							if (runEditorDataPasses)
								renderManager->GeometryRenderer.RenderStaticMesh(rendererData, editorData);
							else
								renderManager->GeometryRenderer.RenderStaticMesh(rendererData);
						}
					}
				}

				{
					SSkeletalMeshRenderData rendererData;
					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.MaterialBuffer = registry.GetResource(data.MaterialBufferHandle);
					rendererData.BoneBuffer = registry.GetResource(data.BoneBufferHandle);

					for (auto& [meshID, meshData] : renderView.SkeletalMeshDrawData)
					{
						rendererData.TransformBuffer->DataBuffer->BindBuffer(meshData.Transforms);
						rendererData.BoneBuffer->DataBuffer->BindBuffer(meshData.Bones);

						if (runEditorDataPasses)
							editorData.EntityBuffer->DataBuffer->BindBuffer(meshData.Entities);

						for (const SMeshDrawData& drawData : meshData.DrawData)
						{
							for (const SRenderResourceHandle& handle : drawData.MaterialTextureHandles)
								rendererData.MaterialTextures.emplace_back(registry.GetResource(handle));

							rendererData.MaterialBuffer->DataBuffer->BindBuffer(drawData.MaterialBufferData);

							rendererData.VertexBuffer = registry.GetResource(drawData.PrimitiveData.VertexBufferHandle);
							rendererData.IndexBuffer = registry.GetResource(drawData.PrimitiveData.IndexBufferHandle);

							rendererData.IndexCount = drawData.PrimitiveData.IndexCount;
							rendererData.InstanceCount = STATIC_U32(meshData.Transforms.size());

							if (runEditorDataPasses)
								renderManager->GeometryRenderer.RenderSkeletalMesh(rendererData, editorData);
							else
								renderManager->GeometryRenderer.RenderSkeletalMesh(rendererData);
						}
					}
				}

				{
					SSpriteRenderData rendererData;
					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.UVRectBuffer = registry.GetResource(data.UVRectBufferHandle);
					rendererData.ColorBuffer = registry.GetResource(data.ColorBufferHandle);

					for (auto& [spriteID, drawData] : renderView.WorldSpaceSpriteDrawData)
					{
						rendererData.TransformBuffer->DataBuffer->BindBuffer(drawData.Transforms);
						rendererData.UVRectBuffer->DataBuffer->BindBuffer(drawData.UVRects);
						rendererData.ColorBuffer->DataBuffer->BindBuffer(drawData.Colors);

						if (runEditorDataPasses)
							editorData.EntityBuffer->DataBuffer->BindBuffer(drawData.Entities);

						rendererData.SpriteTexture = registry.GetResource(drawData.SpriteTextureHandle);

						if (runEditorDataPasses)
							renderManager->SpriteRenderer.RenderEditorWorldSpaceSprite(rendererData, editorData);
						else
							renderManager->SpriteRenderer.RenderWorldSpaceSprite(rendererData);
					}
				}
			}
		);

		struct SGeometryDepthCopyPassData : public SRenderPassParams
		{
			SRenderResourceHandle DepthTextureHandle;
			SRenderResourceHandle DepthRenderTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SGeometryDepthCopyPassData>("GeometryDepthCopy",
			[](CRenderResourceRegistry& registry)
			{
				SGeometryDepthCopyPassData passData;

				passData.AddDependency<SGBufferBasePassData>();

				passData.DepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless));
				passData.DepthRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Depth Copy Target", ERenderResourceFormat::R32_Float));

				return passData;
			},
			[](const SGeometryDepthCopyPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFullscreenRenderData rendererData;
				rendererData.TextureResource1 = registry.GetResource(data.DepthTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.DepthRenderTargetHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenCopyDepth;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);
			}
		);

		struct SGeometryDecalPassData : public SRenderPassParams
		{
			SRenderResourceHandle ViewDataBufferHandle;
			SRenderResourceHandle TransformBufferHandle;
			SRenderResourceHandle MaterialBufferHandle;
			SRenderResourceHandle DepthCopyTextureHandle;

			SRenderResourceHandle GBufferAlbedoTargetHandle;
			SRenderResourceHandle GBufferWNormalTargetHandle;
			SRenderResourceHandle GBufferVNormalTargetHandle;
			SRenderResourceHandle GBufferMaterialTargetHandle;
			SRenderResourceHandle IntermediateDepthTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SGeometryDecalPassData>("GBufferDecalPass",
			[](CRenderResourceRegistry& registry)
			{
				SGeometryDecalPassData passData;

				passData.AddDependency<SGBufferBasePassData>();
				passData.AddDependency<SGeometryDepthCopyPassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.DepthCopyTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Depth Copy", ERenderResourceFormat::R32_Float));
				passData.TransformBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.MaterialBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SMaterialBufferData>("Material Buffer"));

				passData.GBufferAlbedoTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt));
				passData.GBufferWNormalTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt));
				passData.GBufferVNormalTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt));
				passData.GBufferMaterialTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt));
				passData.IntermediateDepthTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless));

				return passData;
			},
			[](const SGeometryDecalPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData constantRenderData;
				constantRenderData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				constantRenderData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SGeometryTargetRenderData targetRenderData;
				targetRenderData.AlbedoRenderTarget = registry.GetResource(data.GBufferAlbedoTargetHandle);
				targetRenderData.WorldNormalRenderTarget = registry.GetResource(data.GBufferWNormalTargetHandle);
				targetRenderData.VertexNormalRenderTarget = registry.GetResource(data.GBufferVNormalTargetHandle);
				targetRenderData.MaterialRenderTarget = registry.GetResource(data.GBufferMaterialTargetHandle);
				targetRenderData.DepthTexture = registry.GetResource(data.IntermediateDepthTargetHandle);

				renderManager->GeometryRenderer.SetupRender(constantRenderData, targetRenderData);

				SDecalRenderData rendererData;
				rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
				rendererData.MaterialBuffer = registry.GetResource(data.MaterialBufferHandle);
				rendererData.DepthCopyTexture = registry.GetResource(data.DepthCopyTextureHandle);

				for (auto& [materialID, drawData] : renderView.DecalDrawData)
				{
					rendererData.TransformBuffer->DataBuffer->BindBuffer(drawData.Transforms);
					rendererData.MaterialBuffer->DataBuffer->BindBuffer(drawData.MaterialBufferData);

					for (const SRenderResourceHandle& handle : drawData.MaterialTextureHandles)
						rendererData.MaterialTextures.emplace_back(registry.GetResource(handle));

					rendererData.InstanceCount = STATIC_U32(drawData.Transforms.size());

					renderManager->GeometryRenderer.RenderDecal(rendererData);
				}
			}
		);

		struct SSSAOPassData : public SRenderPassParams
		{
			SRenderResourceHandle SSAOBufferTextureHandle;
			SRenderResourceHandle DepthTextureHandle;
			SRenderResourceHandle GBufferWNormalTextureHandle;
			SRenderResourceHandle SSAOBlurTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SSSAOPassData>("SSAO",
			[](CRenderResourceRegistry& registry)
			{
				SSSAOPassData passData;

				passData.AddDependency<SGeometryDecalPassData>();

				passData.SSAOBufferTextureHandle = passData.AddOutput(registry.DeclareRenderTexture("SSAO Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution));
				passData.DepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless));
				passData.GBufferWNormalTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt));
				passData.SSAOBlurTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("SSAO Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution));

				return passData;
			},
			[](const SSSAOPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFullscreenRenderData rendererData;
				rendererData.TextureResource1 = registry.GetResource(data.DepthTextureHandle);
				rendererData.TextureResource2 = registry.GetResource(data.GBufferWNormalTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.SSAOBufferTextureHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenSSAO;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);

				rendererData.TextureResource1 = registry.GetResource(data.SSAOBufferTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.SSAOBlurTargetHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenSSAOBlur;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);
			}
		);

		struct SLightPassData : public SRenderPassParams
		{
			SRenderResourceHandle ViewDataBufferHandle;
			SRenderResourceHandle GBufferAlbedoTextureHandle;
			SRenderResourceHandle GBufferWNormalTextureHandle;
			SRenderResourceHandle GBufferVNormalTextureHandle;
			SRenderResourceHandle GBufferMaterialTextureHandle;
			SRenderResourceHandle IntermediateDepthTextureHandle;
			SRenderResourceHandle ShadowAtlasDepthTextureHandle;
			SRenderResourceHandle SSAOBlurTextureHandle;

			SRenderResourceHandle DirectionalLightBufferHandle;
			SRenderResourceHandle PointLightBufferHandle;
			SRenderResourceHandle SpotlightBufferHandle;
			SRenderResourceHandle ShadowmapBufferHandle;
			SRenderResourceHandle EmissiveBufferHandle;

			SRenderResourceHandle MainRenderTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SLightPassData>("LightingPass",
			[&](CRenderResourceRegistry& registry)
			{
				SLightPassData passData;

				passData.AddDependency<SSSAOPassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.GBufferAlbedoTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)); // slot 1
				passData.GBufferWNormalTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)); // slot 2
				passData.GBufferVNormalTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)); // slot 3
				passData.GBufferMaterialTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)); // slot 4
				passData.IntermediateDepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)); // slot 21
				passData.ShadowAtlasDepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y)))); // slot 22
				passData.SSAOBlurTextureHandle = passData.AddInput(registry.DeclareRenderTexture("SSAO Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution)); // slot 23

				passData.DirectionalLightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Directional Light Buffer"));
				passData.PointLightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Point Light Buffer"));
				passData.SpotlightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Spotlight Buffer"));
				passData.ShadowmapBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SShadowmapBufferData>("Shadowmap Buffer", 6));
				passData.EmissiveBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SEmissiveBufferData>("Emissive Buffer"));

				passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Main Render Target", ERenderResourceFormat::R16G16B16A16_Float));

				return passData;
			},
			[](const SLightPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData viewData;
				viewData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				viewData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SLightConstantRenderData constantData;
				constantData.GBufferAlbedoTexture = registry.GetResource(data.GBufferAlbedoTextureHandle);
				constantData.GBufferWNormalTexture = registry.GetResource(data.GBufferWNormalTextureHandle);
				constantData.GBufferVNormalTexture = registry.GetResource(data.GBufferVNormalTextureHandle);
				constantData.GBufferMaterialTexture = registry.GetResource(data.GBufferMaterialTextureHandle);
				constantData.IntermediateDepthTexture = registry.GetResource(data.IntermediateDepthTextureHandle);
				constantData.ShadowAtlasDepthTexture = registry.GetResource(data.ShadowAtlasDepthTextureHandle);
				constantData.SSAOBlurTexture = registry.GetResource(data.SSAOBlurTextureHandle);

				SMainTargetRenderData targetData;
				targetData.MainRenderTarget = registry.GetResource(data.MainRenderTargetHandle);

				renderManager->LightRenderer.SetupLightRender(viewData, constantData, targetData);

				{
					SDirectionalLightRenderData rendererData;

					for (SDirectionalLightDrawData& drawData : renderView.DirectionalLightDrawData)
					{
						rendererData.LightBuffer = registry.GetResource(data.DirectionalLightBufferHandle);
						rendererData.LightBuffer->DataBuffer->BindBuffer(drawData.LightBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData);

						rendererData.EmissiveBuffer = registry.GetResource(data.EmissiveBufferHandle);
						rendererData.EmissiveBuffer->DataBuffer->BindBuffer(SEmissiveBufferData{ .EmissiveStrength = 2.1f }); // TODO.NW: Extract this from fullscreenrenderer post processing buffer data

						rendererData.CubemapTexture = registry.GetResource(drawData.CubemapTextureHandle);

						renderManager->LightRenderer.RenderDirectionalLight(rendererData);
					}
				}

				{
					SPointLightRenderData rendererData;

					for (SPointLightDrawData& drawData : renderView.PointLightDrawData)
					{
						rendererData.LightBuffer = registry.GetResource(data.PointLightBufferHandle);
						rendererData.LightBuffer->DataBuffer->BindBuffer(drawData.LightBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData.data());

						renderManager->LightRenderer.RenderPointLight(rendererData);
					}
				}

				{
					SSpotlightRenderData rendererData;

					for (SSpotlightDrawData& drawData : renderView.SpotlightDrawData)
					{
						rendererData.PointLightBuffer = registry.GetResource(data.PointLightBufferHandle);
						rendererData.PointLightBuffer->DataBuffer->BindBuffer(drawData.PointLightBufferData);

						rendererData.SpotlightBuffer = registry.GetResource(data.SpotlightBufferHandle);
						rendererData.SpotlightBuffer->DataBuffer->BindBuffer(drawData.SpotlightBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData);

						renderManager->LightRenderer.RenderSpotlight(rendererData);
					}
				}

				if (renderView.SkyboxCubemapTextureHandle.IsValid())
				{
					targetData.DepthTexture = constantData.IntermediateDepthTexture;
					renderManager->LightRenderer.SetupSkyboxRender(viewData, targetData);

					SSkyboxRenderData rendererData;
					rendererData.CubemapTexture = registry.GetResource(renderView.SkyboxCubemapTextureHandle);
					renderManager->LightRenderer.RenderSkybox(rendererData);
				}

				// TODO.NW: See if there's any issue with binding depth for skybox and rendering to 
				// main target instead of keeping a LitScene copy
			}
		);

		struct SVolumetricLightPassData : public SRenderPassParams
		{
			SRenderResourceHandle ViewDataBufferHandle;
			SRenderResourceHandle IntermediateDepthTextureHandle;
			SRenderResourceHandle ShadowAtlasDepthTextureHandle;
			SRenderResourceHandle DirectionalLightBufferHandle;
			SRenderResourceHandle PointLightBufferHandle;
			SRenderResourceHandle SpotlightBufferHandle;
			SRenderResourceHandle VolumetricLightBufferHandle;
			SRenderResourceHandle ShadowmapBufferHandle;
			SRenderResourceHandle AccumulationRenderTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SVolumetricLightPassData>("VolumetricAccumulationPass",
			[&](CRenderResourceRegistry& registry)
			{
				SVolumetricLightPassData passData;

				passData.AddDependency<SLightPassData>();
				passData.AddDependency<SShadowmapPrePassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.IntermediateDepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)); // slot 21
				passData.ShadowAtlasDepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y)))); // slot 22	
				passData.DirectionalLightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Directional Light Buffer"));
				passData.PointLightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Point Light Buffer"));
				passData.SpotlightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDirectionalLightBufferData>("Spotlight Buffer"));
				passData.VolumetricLightBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SVolumetricLightBufferData>("Volumetric Light Buffer"));
				passData.ShadowmapBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SShadowmapBufferData>("Shadowmap Buffer", 6));

				passData.AccumulationRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Volumetric Accumulation", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution));

				return passData;
			},
			[](const SVolumetricLightPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData viewData;
				viewData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				viewData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SVolumetricConstantRenderData constantData;
				constantData.IntermediateDepthTexture = registry.GetResource(data.IntermediateDepthTextureHandle);
				constantData.ShadowAtlasDepthTexture = registry.GetResource(data.ShadowAtlasDepthTextureHandle);

				SMainTargetRenderData targetData;
				targetData.MainRenderTarget = registry.GetResource(data.AccumulationRenderTargetHandle);

				renderManager->LightRenderer.SetupVolumetricRender(viewData, constantData, targetData);

				// TODO.NW: Find neat way of skipping these calls if the volumetrics aren't enabled
				{
					SVolumetricDirectionalLightRenderData rendererData;

					for (SDirectionalLightDrawData& drawData : renderView.DirectionalLightDrawData)
					{
						if (!drawData.RenderVolumetrics)
							continue;

						rendererData.LightBuffer = registry.GetResource(data.DirectionalLightBufferHandle);
						rendererData.LightBuffer->DataBuffer->BindBuffer(drawData.LightBufferData);

						rendererData.VolumetricLightBuffer = registry.GetResource(data.VolumetricLightBufferHandle);
						rendererData.VolumetricLightBuffer->DataBuffer->BindBuffer(drawData.VolumetricBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData);

						renderManager->LightRenderer.RenderVolumetricDirectionalLight(rendererData);
					}
				}

				{
					SVolumetricPointLightRenderData rendererData;

					for (SPointLightDrawData& drawData : renderView.PointLightDrawData)
					{
						if (!drawData.RenderVolumetrics)
							continue;

						rendererData.LightBuffer = registry.GetResource(data.PointLightBufferHandle);
						rendererData.LightBuffer->DataBuffer->BindBuffer(drawData.LightBufferData);

						rendererData.VolumetricLightBuffer = registry.GetResource(data.VolumetricLightBufferHandle);
						rendererData.VolumetricLightBuffer->DataBuffer->BindBuffer(drawData.VolumetricBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData);

						renderManager->LightRenderer.RenderVolumetricPointLight(rendererData);
					}
				}

				{
					SVolumetricSpotlightRenderData rendererData;

					for (SSpotlightDrawData& drawData : renderView.SpotlightDrawData)
					{
						if (!drawData.RenderVolumetrics)
							continue;

						rendererData.PointLightBuffer = registry.GetResource(data.PointLightBufferHandle);
						rendererData.PointLightBuffer->DataBuffer->BindBuffer(drawData.PointLightBufferData);

						rendererData.SpotlightBuffer = registry.GetResource(data.SpotlightBufferHandle);
						rendererData.SpotlightBuffer->DataBuffer->BindBuffer(drawData.SpotlightBufferData);

						rendererData.VolumetricLightBuffer = registry.GetResource(data.VolumetricLightBufferHandle);
						rendererData.VolumetricLightBuffer->DataBuffer->BindBuffer(drawData.VolumetricBufferData);

						rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
						rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(drawData.ShadowmapBufferData);

						renderManager->LightRenderer.RenderVolumetricSpotlight(rendererData);
					}
				}
			}
		);

		struct SVolumetricBlurPassData : public SRenderPassParams
		{
			SRenderResourceHandle DepthTextureHandle;
			SRenderResourceHandle DownsamplingDepthTextureHandle;
			SRenderResourceHandle PrimaryBlurTextureHandle;
			SRenderResourceHandle SecondaryBlurTextureHandle;
			SRenderResourceHandle RenderTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SVolumetricBlurPassData>("VolumetricBlurPass",
			[](CRenderResourceRegistry& registry)
			{
				SVolumetricBlurPassData passData;

				passData.AddDependency<SVolumetricLightPassData>();
				
				passData.DepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless));
				passData.DownsamplingDepthTextureHandle = passData.AddInput(registry.DeclareRenderDepth("Downsampling Depth", ERenderResourceFormat::R32_Float, ERenderTextureSizeHint::HalfResolution));
				passData.PrimaryBlurTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Volumetric Accumulation", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution));
				passData.SecondaryBlurTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Volumetric Blur", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution));

				passData.RenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Main Render Target", ERenderResourceFormat::R16G16B16A16_Float));

				return passData;
			},
			[](const SVolumetricBlurPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SBilateralBlurRenderData rendererData;
				rendererData.RenderTarget = registry.GetResource(data.RenderTargetHandle);
				rendererData.DepthTexture = registry.GetResource(data.DepthTextureHandle);
				rendererData.DownsamplingDepthTexture = registry.GetResource(data.DownsamplingDepthTextureHandle);
				rendererData.PrimaryBlurTexture = registry.GetResource(data.PrimaryBlurTextureHandle);
				rendererData.SecondaryBlurTexture = registry.GetResource(data.SecondaryBlurTextureHandle);
				renderManager->FullscreenRenderer.RenderBilateralBlur(rendererData);
			}
		);

		struct SBloomPassData : public SRenderPassParams
		{
			SRenderResourceHandle BloomBufferTextureHandle;
			SRenderResourceHandle TargetCopyTextureHandle;
			SRenderResourceHandle HalfResolutionTextureHandle;
			SRenderResourceHandle PrimaryBlurTextureHandle;
			SRenderResourceHandle SecondaryBlurTextureHandle;
			SRenderResourceHandle MainRenderTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SBloomPassData>("Bloom",
			[](CRenderResourceRegistry& registry)
			{
				SBloomPassData passData;

				passData.AddDependency<SLightPassData>();
				passData.AddDependency<SVolumetricBlurPassData>();

				passData.BloomBufferTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Bloom Buffer", ERenderResourceFormat::R16G16B16A16_Float));
				passData.TargetCopyTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Render Target Copy", ERenderResourceFormat::R16G16B16A16_Float));
				passData.HalfResolutionTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Half Resolution Target", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution));
				passData.PrimaryBlurTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Primary Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::QuarterResolution));
				passData.SecondaryBlurTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Secondary Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::QuarterResolution));
				
				passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Main Render Target", ERenderResourceFormat::R16G16B16A16_Float));

				return passData;
			},
			[](const SBloomPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFullscreenRenderData fullscreenRenderData;
				fullscreenRenderData.PixelShader = EPixelShaders::FullscreenCopy;
				fullscreenRenderData.BlendState = EBlendStates::Disable;
				fullscreenRenderData.RenderTarget = registry.GetResource(data.TargetCopyTextureHandle);
				fullscreenRenderData.TextureResource1 = registry.GetResource(data.MainRenderTargetHandle);
				renderManager->FullscreenRenderer.Render(fullscreenRenderData);

				fullscreenRenderData.PixelShader = EPixelShaders::FullscreenLuminance;
				fullscreenRenderData.BlendState = EBlendStates::Disable;
				fullscreenRenderData.RenderTarget = registry.GetResource(data.BloomBufferTextureHandle);
				fullscreenRenderData.TextureResource1 = registry.GetResource(data.MainRenderTargetHandle);
				renderManager->FullscreenRenderer.Render(fullscreenRenderData);
				
				SGaussianBlurRenderData blurRenderData;
				blurRenderData.RenderTarget = registry.GetResource(data.BloomBufferTextureHandle);
				blurRenderData.HalfResolutionTexture = registry.GetResource(data.HalfResolutionTextureHandle);
				blurRenderData.PrimaryBlurTexture = registry.GetResource(data.PrimaryBlurTextureHandle);
				blurRenderData.SecondaryBlurTexture = registry.GetResource(data.SecondaryBlurTextureHandle);
				renderManager->FullscreenRenderer.RenderGaussianBlur(blurRenderData);

				// TODO.NW: Could potentially be a thing to additively blend on the bloom buffer directly 
				// to the render target, using only one resource (and additive blend state)
				// Alternatively, use a UAV?
				fullscreenRenderData.PixelShader = EPixelShaders::FullscreenBloom;
				fullscreenRenderData.BlendState = EBlendStates::Disable;
				fullscreenRenderData.RenderTarget = registry.GetResource(data.MainRenderTargetHandle);
				fullscreenRenderData.TextureResource1 = registry.GetResource(data.TargetCopyTextureHandle);
				fullscreenRenderData.TextureResource2 = registry.GetResource(data.BloomBufferTextureHandle);
				renderManager->FullscreenRenderer.Render(fullscreenRenderData);
			}
		);

		struct STonemappingPassData : public SRenderPassParams
		{
			SRenderResourceHandle RenderTargetTextureHandle;
			SRenderResourceHandle TonemappingTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<STonemappingPassData>("Tonemapping",
			[](CRenderResourceRegistry& registry)
			{
				STonemappingPassData passData;

				passData.AddDependency<SBloomPassData>();

				passData.RenderTargetTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Main Render Target", ERenderResourceFormat::R16G16B16A16_Float));
				passData.TonemappingTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Tonemapped Target", ERenderResourceFormat::R16G16B16A16_Float));
				
				return passData;
			},
			[](const STonemappingPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFullscreenRenderData rendererData;
				rendererData.TextureResource1 = registry.GetResource(data.RenderTargetTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.TonemappingTargetHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenTonemap;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);
			}
		);

		struct SEditorWidgetPassData : public SRenderPassParams
		{
			SRenderResourceHandle ViewDataBufferHandle;
			SRenderResourceHandle TransformBufferHandle;
			SRenderResourceHandle UVRectBufferHandle;
			SRenderResourceHandle ColorBufferHandle;
			SRenderResourceHandle EntityBufferHandle;

			SRenderResourceHandle RenderTargetHandle;
			SRenderResourceHandle GBufferWorldPositionTargetHandle;
			SRenderResourceHandle GBufferEditorDataTargetHandle;
			SRenderResourceHandle DepthTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SEditorWidgetPassData>("EditorWidgetPass",
			[](CRenderResourceRegistry& registry)
			{
				SEditorWidgetPassData passData;

				passData.AddDependency<STonemappingPassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.TransformBufferHandle = registry.DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit);
				passData.UVRectBufferHandle = registry.DeclareVertexBuffer<SVector4>("Instanced UV Rect Buffer", InstancedDrawInstanceLimit);
				passData.ColorBufferHandle = registry.DeclareVertexBuffer<SVector4>("Instanced Color Buffer", InstancedDrawInstanceLimit);
				passData.EntityBufferHandle = registry.DeclareVertexBuffer<SEntity>("Instanced Entity Buffer", InstancedDrawInstanceLimit);

				passData.RenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float));
				passData.GBufferWorldPositionTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer World Position", ERenderResourceFormat::R32G32B32A32_Float));
				passData.GBufferEditorDataTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("GBuffer Editor Data", ERenderResourceFormat::R32G32_UnsignedInt));

				// NW: This was a separate depth called EditorWidgetDepth before, does it need to be clear for some reason?
				passData.DepthTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless));

				return passData;
			},
			[](const SEditorWidgetPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData constantRenderData;
				constantRenderData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				constantRenderData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SMainTargetRenderData targetData;
				targetData.MainRenderTarget = registry.GetResource(data.RenderTargetHandle);
				targetData.DepthTexture = registry.GetResource(data.DepthTargetHandle);

				SEditorRenderData editorData;
				editorData.WorldPositionRenderTarget = registry.GetResource(data.GBufferWorldPositionTargetHandle);
				editorData.EditorDataRenderTarget = registry.GetResource(data.GBufferEditorDataTargetHandle);
				editorData.EntityBuffer = registry.GetResource(data.EntityBufferHandle);

				renderManager->SpriteRenderer.SetupWidgetRender(constantRenderData, targetData, editorData);

				{
					SSpriteRenderData rendererData;
					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.UVRectBuffer = registry.GetResource(data.UVRectBufferHandle);
					rendererData.ColorBuffer = registry.GetResource(data.ColorBufferHandle);

					for (auto& [spriteID, drawData] : renderView.WidgetSpriteDrawData)
					{
						rendererData.TransformBuffer->DataBuffer->BindBuffer(drawData.Transforms);
						rendererData.UVRectBuffer->DataBuffer->BindBuffer(drawData.UVRects);
						rendererData.ColorBuffer->DataBuffer->BindBuffer(drawData.Colors);

						rendererData.SpriteTexture = registry.GetResource(drawData.SpriteTextureHandle);

						editorData.EntityBuffer->DataBuffer->BindBuffer(drawData.Entities);

						renderManager->SpriteRenderer.RenderEditorWorldSpaceWidget(rendererData, editorData);
					}
				}
			}
		);

		struct SUIPassData : public SRenderPassParams
		{
			SRenderResourceHandle TransformBufferHandle;
			SRenderResourceHandle UVRectBufferHandle;
			SRenderResourceHandle ColorBufferHandle;

			SRenderResourceHandle MainRenderTargetHandle;
			SRenderResourceHandle DepthTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SUIPassData>("UIPass",
			[](CRenderResourceRegistry& registry)
			{
				SUIPassData passData;

				passData.AddDependency<STonemappingPassData>();

				passData.TransformBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.UVRectBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SVector4>("Instanced UV Rect Buffer", InstancedDrawInstanceLimit));
				passData.ColorBufferHandle = passData.AddInput(registry.DeclareVertexBuffer<SVector4>("Instanced Color Buffer", InstancedDrawInstanceLimit));

				passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float));

				// NW: Should this pass have depth?
				passData.DepthTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless));

				return passData;
			},
			[](const SUIPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SMainTargetRenderData targetData;
				targetData.MainRenderTarget;
				targetData.DepthTexture;
				renderManager->SpriteRenderer.SetupScreenSpaceRender(targetData);

				{
					SSpriteRenderData rendererData;
					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.UVRectBuffer = registry.GetResource(data.UVRectBufferHandle);
					rendererData.ColorBuffer = registry.GetResource(data.ColorBufferHandle);

					for (auto& [spriteID, drawData] : renderView.UISpriteDrawData)
					{
						rendererData.TransformBuffer->DataBuffer->BindBuffer(drawData.Transforms);
						rendererData.UVRectBuffer->DataBuffer->BindBuffer(drawData.UVRects);
						rendererData.ColorBuffer->DataBuffer->BindBuffer(drawData.Colors);

						rendererData.SpriteTexture = registry.GetResource(drawData.SpriteTextureHandle);

						renderManager->SpriteRenderer.RenderScreenSpaceSprite(rendererData);
					}
				}
			}
		);

		struct SDebugDrawPass : public SRenderPassParams
		{
			SRenderResourceHandle ViewDataBufferHandle;
			SRenderResourceHandle DebugObjectBufferHandle;
			SRenderResourceHandle MainRenderTargetHandle;
			SRenderResourceHandle DepthTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SDebugDrawPass>("DebugLines",
			[](CRenderResourceRegistry& registry)
			{
				SDebugDrawPass passData;

				passData.AddDependency<STonemappingPassData>();

				passData.ViewDataBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SFrameBufferData>("View Data Buffer"));
				passData.DebugObjectBufferHandle = passData.AddInput(registry.DeclareConstantBuffer<SDebugShapeObjectBufferData>("Debug Shape Buffer"));

				passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float));
				passData.DepthTargetHandle = passData.AddOutput(registry.DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless));

				return passData;
			},
			[](const SDebugDrawPass& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SDebugRenderData rendererData;
				rendererData.ObjectBuffer = registry.GetResource(data.DebugObjectBufferHandle);
				SDebugShapeObjectBufferData bufferData;

				SFrameBufferData frameBufferData;
				frameBufferData.ToWorldFromCamera = renderView.ViewMatrix;
				frameBufferData.ToCameraFromWorld = renderView.ViewMatrix.FastInverse();
				frameBufferData.ToProjectionFromCamera = renderView.ProjectionMatrix;
				frameBufferData.ToCameraFromProjection = renderView.ProjectionMatrix.Inverse();
				frameBufferData.CameraPosition = renderView.ViewMatrix.GetTranslation4();
				SGeometryConstantRenderData viewData;
				viewData.ViewDataBuffer = registry.GetResource(data.ViewDataBufferHandle);
				viewData.ViewDataBuffer->DataBuffer->BindBuffer(frameBufferData);

				SMainTargetRenderData targetData;
				targetData.MainRenderTarget = registry.GetResource(data.MainRenderTargetHandle);
				targetData.DepthTexture = registry.GetResource(data.DepthTargetHandle);

				auto fillRendererData = [&](const SDebugObjectDrawData& drawData)
					{
						bufferData.ToWorldFromObject = drawData.ToWorldFromObject;
						bufferData.Color = drawData.Color;
						bufferData.HalfThickness = drawData.HalfThickness;

						rendererData.ObjectBuffer->DataBuffer->BindBuffer(bufferData);
						rendererData.VertexBuffer = registry.GetResource(drawData.PrimitiveData.VertexBufferHandle);
						rendererData.IndexBuffer = registry.GetResource(drawData.PrimitiveData.IndexBufferHandle);
						rendererData.IndexCount = drawData.PrimitiveData.IndexCount;

						return rendererData;
					};

				renderManager->DebugRenderer.SetupDepthRender(viewData, targetData);
				{
					for (const SDebugObjectDrawData& drawData : renderView.DebugObjectsDepth3D)
						renderManager->DebugRenderer.RenderDebugShape3D(fillRendererData(drawData));

					for (const SDebugObjectDrawData& drawData : renderView.DebugObjectsDepth2D)
						renderManager->DebugRenderer.RenderDebugShape2D(fillRendererData(drawData));
				}

				renderManager->DebugRenderer.SetupNoDepthRender(viewData, targetData);
				{
					for (const SDebugObjectDrawData& drawData : renderView.DebugObjectsNoDepth3D)
						renderManager->DebugRenderer.RenderDebugShape3D(fillRendererData(drawData));
					
					for (const SDebugObjectDrawData& drawData : renderView.DebugObjectsNoDepth2D)
						renderManager->DebugRenderer.RenderDebugShape2D(fillRendererData(drawData));
				}
			}
		);

		struct SAntialiasingPassData : public SRenderPassParams
		{
			SRenderResourceHandle TonemappedTextureHandle;
			SRenderResourceHandle AntialiasingTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SAntialiasingPassData>("Antialiasing",
			[](CRenderResourceRegistry& registry)
			{
				SAntialiasingPassData passData;

				passData.AddDependency<STonemappingPassData>();
				passData.AddDependency<SEditorWidgetPassData>();
				passData.AddDependency<SUIPassData>();
				passData.AddDependency<SDebugDrawPass>();

				passData.TonemappedTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Tonemapped Target", ERenderResourceFormat::R16G16B16A16_Float));
				passData.AntialiasingTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Antialiased Target", ERenderResourceFormat::R16G16B16A16_Float));

				return passData;
			},
			[](const SAntialiasingPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFullscreenRenderData rendererData;
				rendererData.TextureResource1 = registry.GetResource(data.TonemappedTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.AntialiasingTargetHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenFXAA;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);
			}
		);

		struct SGammaCorrectionPassData : public SRenderPassParams
		{
			SRenderResourceHandle AntialiasedTextureHandle;
			SRenderResourceHandle MainRenderTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SGammaCorrectionPassData>("GammaCorrection",
			[](CRenderResourceRegistry& registry)
			{
				SGammaCorrectionPassData passData;

				passData.AddDependency<SAntialiasingPassData>();
				
				passData.AntialiasedTextureHandle = passData.AddInput(registry.DeclareRenderTexture("Antialiased Target", ERenderResourceFormat::R16G16B16A16_Float));
				passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Main Render Target", ERenderResourceFormat::R16G16B16A16_Float));

				return passData;
			},
			[](const SGammaCorrectionPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				SFullscreenRenderData rendererData;
				rendererData.TextureResource1 = registry.GetResource(data.AntialiasedTextureHandle);
				rendererData.RenderTarget = registry.GetResource(data.MainRenderTargetHandle);
				rendererData.PixelShader = EPixelShaders::FullscreenGammaCorrection;
				rendererData.BlendState = EBlendStates::Disable;
				renderManager->FullscreenRenderer.Render(rendererData);
			}
		);

		struct SFinalizePassData : public SRenderPassParams
		{
			SRenderResourceHandle PrimaryOptionalTextureHandle;
			SRenderResourceHandle SecondaryOptionalTextureHandle;
			SRenderResourceHandle MainRenderTargetHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SFinalizePassData>("Finalize",
			[&](CRenderResourceRegistry& registry)
			{
				SFinalizePassData passData;
				passData.SetIsRootPass();

				// TODO.NW: Figure out what we can do debug wise. By setting dependencies we should
				// be able to automatically disable passes that aren't used. What can we do to override
				// Rasterizer states globally (for geometry) for wireframe debugging?
				// if (!RenderManager->ParseDebugBuffer)
				passData.AddDependency<SGammaCorrectionPassData>();
				//else
				//{
				//	passData.AddDependency<SGeometryBasePassData>();
				//	passData.PrimaryOptionalTextureHandle = passData.AddInput(registry.DeclareRenderTexture("GBuffer Albedo Texture", ERenderResourceFormat::R16G16B16A16_Float));
				//	passData.MainRenderTargetHandle = passData.AddOutput(registry.DeclareRenderTexture("Main Render Target", ERenderResourceFormat::R16G16B16A16_Float));
				//}

				return passData;
			},
			[](const SFinalizePassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager, SRenderView& renderView)
			{
				//SFullscreenRenderData rendererData;
				//rendererData.TextureResource1 = registry.GetResource(data.PrimaryOptionalTextureHandle);
				////rendererData.TextureResource2 = registry.GetResource(data.SecondaryOptionalTextureHandle);
				//rendererData.RenderTarget = registry.GetResource(data.MainRenderTargetHandle);
				//rendererData.PixelShader = EPixelShaders::FullscreenCopy;
				//rendererData.BlendState = EBlendStates::Disable;
				//renderManager->FullscreenRenderer.Render(rendererData);
			}
		);
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
