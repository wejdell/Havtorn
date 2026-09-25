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

	void CRenderSystem::AddStaticMeshShadowmapPass(const std::vector<SDrawCallData>& staticMeshData, const std::vector<SMatrix>& transforms, const std::vector<SShadowmapViewData>& shadowmapData)
	{
		struct SStaticMeshShadowmapPassData : public SRenderPassParams
		{
			SRenderResourceHandle TransformBufferHandle;
			SRenderResourceHandle FrameBufferHandle;
			
			struct SSubPassData
			{
				SRenderResourceHandle VertexBufferHandle;
				SRenderResourceHandle IndexBufferHandle;
				U32 IndexCount = 0;
			};
			std::vector<SSubPassData> SubPasses;
		};
		RenderManager->GetRenderGraph().AddPass<SStaticMeshShadowmapPassData>("StaticMeshShadowmapPass",
			[&](CRenderResourceRegistry& registry)
			{
				SStaticMeshShadowmapPassData passData;

				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(8192, 8192))));

				passData.TransformBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.FrameBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SFrameBufferData>("Shadowmap Frame Buffer"));

				CRenderStateManager& stateManager = RenderManager->GetRenderStateManager();

				for (const SDrawCallData& drawCallData : staticMeshData)
				{
					SStaticMeshShadowmapPassData::SSubPassData& subPassData = passData.SubPasses.emplace_back();
					subPassData.VertexBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SStaticMeshVertex>("Static Mesh Vertex Buffer", &stateManager.VertexBuffers[drawCallData.VertexBufferIndex]));
					subPassData.IndexBufferHandle = registry.DeclareResource(DeclareIndexBuffer("Mesh Index Buffer", &stateManager.IndexBuffers[drawCallData.IndexBufferIndex]));
					subPassData.IndexCount = drawCallData.IndexCount;
				}				

				return passData;
			},
			[shadowmapData, transforms](const SStaticMeshShadowmapPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
			{
				SStaticMeshShadowRenderData rendererData;

				rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
				rendererData.TransformBuffer->DataBuffer->BindBuffer(transforms);
				rendererData.InstanceCount = STATIC_U32(transforms.size());

				rendererData.FrameBuffer = registry.GetResource(data.FrameBufferHandle);

				for (const SShadowmapViewData& viewData : shadowmapData)
				{
					rendererData.ShadowmapViewportIndex = viewData.ShadowmapViewportIndex;

					SFrameBufferData frameBufferData;
					frameBufferData.ToCameraFromWorld = viewData.ShadowViewMatrix;
					frameBufferData.ToWorldFromCamera = viewData.ShadowViewMatrix.FastInverse();
					frameBufferData.ToProjectionFromCamera = viewData.ShadowProjectionMatrix;
					frameBufferData.ToCameraFromProjection = viewData.ShadowProjectionMatrix.Inverse();
					frameBufferData.CameraPosition = viewData.ShadowPosition;

					rendererData.FrameBuffer->DataBuffer->BindBuffer(frameBufferData);

					for (const SStaticMeshShadowmapPassData::SSubPassData& subPassData : data.SubPasses)
					{
						rendererData.VertexBuffer = registry.GetResource(subPassData.VertexBufferHandle);
						rendererData.IndexBuffer = registry.GetResource(subPassData.IndexBufferHandle);
						rendererData.IndexCount = subPassData.IndexCount;

						renderManager->LightRenderer.RenderStaticMeshDepthPrePass(rendererData);
					}
				}
			}
		);
	}

	void CRenderSystem::AddStaticMeshPass(const std::vector<SDrawCallData>& staticMeshData, const std::vector<SGraphicsMaterialAsset*>& materials, const std::vector<SMatrix>& transforms, const std::vector<SEntity>& entities, const bool runEditorDataPasses)
	{
		if (runEditorDataPasses)
		{
			struct SGeometryEditorPassData : public SRenderPassParams
			{
				// Memory managed by RenderGraph
				SRenderResourceHandle TransformBufferHandle;
				SRenderResourceHandle MaterialBufferHandle;
				SRenderResourceHandle EntityBufferHandle;

				// External memory from Asset Registry
				// Static, taken from Asset Registry and used as external source for render resources
				std::vector<SRenderResourceHandle> MaterialTextureHandles;
				std::map<U32, F32> MaterialUIDToRuntimeIndex;

				struct SSubPassData
				{
					SRenderResourceHandle VertexBufferHandle;
					SRenderResourceHandle IndexBufferHandle;
					U32 IndexCount = 0;
					U16 MaterialIndex = 0;
				};
				std::vector<SSubPassData> SubPassData;
			};
			RenderManager->GetRenderGraph().AddPass<SGeometryEditorPassData>("BasePassInstancedEditor",
				[&](CRenderResourceRegistry& registry)
				{
					SGeometryEditorPassData passData;

					// TODO.NW: It's a bit weird that the render function has no control over where the inputs are bound (to what slot). Can only do this where there's a strong 
					// assumption of how it should work (as a sync point). 
					passData.Inputs.emplace_back(registry.DeclareResource(DeclareConstantBuffer<SFrameBufferData>("Frame Buffer")));

					// TODO.NW: These inputs and outputs must be able to be aggregately bound by the render graph somehow
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Position", ERenderResourceFormat::R32G32B32A32_Float)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Editor Data", ERenderResourceFormat::R32G32_UnsignedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));

					passData.TransformBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
					passData.EntityBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SEntity>("Instanced Entity Buffer", InstancedDrawInstanceLimit));
					
					passData.MaterialBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SMaterialBufferData>("Material Buffer"));
					
					// Import resources from asset registry

					for (SGraphicsMaterialAsset* materialAsset : materials)
					{
						// TODO.NW: Figure out how we can deal with this requesting? Materials are shared for each invocation here
						constexpr U64 tempRequester = 10000;
						std::map<U32, CStaticRenderTexture*> mappedTextures = materialAsset->Material.GetRenderTexturePointers(tempRequester);

						for (auto& [uid, texture] : mappedTextures)
						{
							passData.MaterialUIDToRuntimeIndex.emplace(uid, STATIC_F32(passData.MaterialTextureHandles.size()));
							passData.MaterialTextureHandles.emplace_back(registry.DeclareResource(DeclareRenderTexture(std::to_string(uid).c_str(), texture)));
						}
					}

					CRenderStateManager& stateManager = RenderManager->GetRenderStateManager();

					for (const SDrawCallData& drawCallData : staticMeshData)
					{
						SGeometryEditorPassData::SSubPassData& subPassData = passData.SubPassData.emplace_back();
						subPassData.VertexBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SStaticMeshVertex>("Static Mesh Vertex Buffer", &stateManager.VertexBuffers[drawCallData.VertexBufferIndex]));
						subPassData.IndexBufferHandle = registry.DeclareResource(DeclareIndexBuffer("Mesh Index Buffer", &stateManager.IndexBuffers[drawCallData.IndexBufferIndex]));
						subPassData.IndexCount = drawCallData.IndexCount;
						subPassData.MaterialIndex = drawCallData.MaterialIndex;
					}

					return passData;
				},
				[materials, transforms, entities](const SGeometryEditorPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
				{
					SStaticMeshRenderData rendererData;
					SEditorGeometryRenderData editorData;

					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.MaterialBuffer = registry.GetResource(data.MaterialBufferHandle);
					editorData.EntityBuffer = registry.GetResource(data.EntityBufferHandle);

					for (const SRenderResourceHandle& handle : data.MaterialTextureHandles)
						rendererData.MaterialTextures.emplace_back(registry.GetResource(handle));

					rendererData.TransformBuffer->DataBuffer->BindBuffer(transforms);
					rendererData.InstanceCount = STATIC_U32(transforms.size());
					editorData.EntityBuffer->DataBuffer->BindBuffer(entities);

					for (const SGeometryEditorPassData::SSubPassData& subPassData : data.SubPassData)
					{
						SMaterialBufferData materialBufferData = SMaterialBufferData(materials[subPassData.MaterialIndex]->Material);
						for (SRuntimeGraphicsMaterialProperty& property : materialBufferData.Properties)
						{
							if (property.TextureChannelIndex > -1.0f)
								property.TextureIndex = data.MaterialUIDToRuntimeIndex.at(property.TextureUID);
						}
						rendererData.MaterialBuffer->DataBuffer->BindBuffer(materialBufferData);

						rendererData.VertexBuffer = registry.GetResource(subPassData.VertexBufferHandle);
						rendererData.IndexBuffer = registry.GetResource(subPassData.IndexBufferHandle);
						rendererData.IndexCount = subPassData.IndexCount;

						renderManager->GeometryRenderer.RenderEditorStaticMesh(rendererData, editorData);
					}
				}
			);
		}
		else
		{
			struct SGeometryPassData : public SRenderPassParams
			{
				// Memory managed by RenderGraph
				SRenderResourceHandle TransformBufferHandle;
				SRenderResourceHandle MaterialBufferHandle;

				// External memory from Asset Registry
				// Static, taken from Asset Registry and used as external source for render resources
				std::vector<SRenderResourceHandle> MaterialTextureHandles;
				std::map<U32, F32> MaterialUIDToRuntimeIndex;

				struct SSubPassData
				{
					SRenderResourceHandle VertexBufferHandle;
					SRenderResourceHandle IndexBufferHandle;
					U32 IndexCount = 0;
					U16 MaterialIndex = 0;
				};
				std::vector<SSubPassData> SubPassData;
			};
			RenderManager->GetRenderGraph().AddPass<SGeometryPassData>("BasePassInstanced",
				[&](CRenderResourceRegistry& registry)
				{
					SGeometryPassData passData;

					// TODO.NW: It's a bit weird that the render function has no control over where the inputs are bound (to what slot). Can only do this where there's a strong 
					// assumption of how it should work (as a sync point). 
					passData.Inputs.emplace_back(registry.DeclareResource(DeclareConstantBuffer<SFrameBufferData>("Frame Buffer")));

					// TODO.NW: These inputs and outputs must be able to be aggregately bound by the render graph somehow
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));

					passData.TransformBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));

					passData.TransformBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
					passData.MaterialBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SMaterialBufferData>("Material Buffer"));

					// Import resources from asset registry

					for (SGraphicsMaterialAsset* materialAsset : materials)
					{
						// TODO.NW: Figure out how we can deal with this requesting? Materials are shared for each invocation here
						constexpr U64 tempRequester = 10000;
						std::map<U32, CStaticRenderTexture*> mappedTextures = materialAsset->Material.GetRenderTexturePointers(tempRequester);

						for (auto& [uid, texture] : mappedTextures)
						{
							passData.MaterialUIDToRuntimeIndex.emplace(uid, STATIC_F32(passData.MaterialTextureHandles.size()));
							passData.MaterialTextureHandles.emplace_back(registry.DeclareResource(DeclareRenderTexture(std::to_string(uid).c_str(), texture)));
						}
					}

					CRenderStateManager& stateManager = RenderManager->GetRenderStateManager();

					for (const SDrawCallData& drawCallData : staticMeshData)
					{
						SGeometryPassData::SSubPassData& subPassData = passData.SubPassData.emplace_back();
						subPassData.VertexBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SStaticMeshVertex>("Static Mesh Vertex Buffer", &stateManager.VertexBuffers[drawCallData.VertexBufferIndex]));
						subPassData.IndexBufferHandle = registry.DeclareResource(DeclareIndexBuffer("Mesh Index Buffer", &stateManager.IndexBuffers[drawCallData.IndexBufferIndex]));
						subPassData.IndexCount = drawCallData.IndexCount;
						subPassData.MaterialIndex = drawCallData.MaterialIndex;
					}

					return passData;
				},
				[materials, transforms](const SGeometryPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
				{
					SStaticMeshRenderData rendererData;

					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.MaterialBuffer = registry.GetResource(data.MaterialBufferHandle);

					for (const SRenderResourceHandle& handle : data.MaterialTextureHandles)
						rendererData.MaterialTextures.emplace_back(registry.GetResource(handle));

					rendererData.TransformBuffer->DataBuffer->BindBuffer(transforms);
					rendererData.InstanceCount = STATIC_U32(transforms.size());

					for (const SGeometryPassData::SSubPassData& subPassData : data.SubPassData)
					{
						SMaterialBufferData materialBufferData = SMaterialBufferData(materials[subPassData.MaterialIndex]->Material);
						for (SRuntimeGraphicsMaterialProperty& property : materialBufferData.Properties)
						{
							if (property.TextureChannelIndex > -1.0f)
								property.TextureIndex = data.MaterialUIDToRuntimeIndex.at(property.TextureUID);
						}
						rendererData.MaterialBuffer->DataBuffer->BindBuffer(materialBufferData);

						rendererData.VertexBuffer = registry.GetResource(subPassData.VertexBufferHandle);
						rendererData.IndexBuffer = registry.GetResource(subPassData.IndexBufferHandle);
						rendererData.IndexCount = subPassData.IndexCount;

						renderManager->GeometryRenderer.RenderStaticMesh(rendererData);
					}
				}
			);
		}
	}

	void CRenderSystem::AddSkeletalMeshPass(const std::vector<SDrawCallData>& skeletalMeshData, const std::vector<SGraphicsMaterialAsset*>& materials, const std::vector<SMatrix>& transforms, const std::vector<SMatrix>& posedBones, const std::vector<SEntity>& entities, const bool runEditorDataPasses)
	{
		if (runEditorDataPasses)
		{
			struct SSkeletalGeometryEditorPassData : public SRenderPassParams
			{
				// Memory managed by RenderGraph
				SRenderResourceHandle TransformBufferHandle;
				SRenderResourceHandle BoneBufferHandle;
				SRenderResourceHandle EntityBufferHandle;
				SRenderResourceHandle MaterialBufferHandle;

				// External memory from Asset Registry
				// Static, taken from Asset Registry and used as external source for render resources
				std::vector<SRenderResourceHandle> MaterialTextureHandles;
				std::map<U32, F32> MaterialUIDToRuntimeIndex;

				struct SSubPassData
				{
					SRenderResourceHandle VertexBufferHandle;
					SRenderResourceHandle IndexBufferHandle;
					U32 IndexCount = 0;
					U16 MaterialIndex = 0;
				};
				std::vector<SSubPassData> SubPassData;
			};
			RenderManager->GetRenderGraph().AddPass<SSkeletalGeometryEditorPassData>("SkelBasePassInstancedEditor",
				[&](CRenderResourceRegistry& registry)
				{
					SSkeletalGeometryEditorPassData passData;

					// TODO.NW: It's a bit weird that the render function has no control over where the inputs are bound (to what slot). Can only do this where there's a strong 
					// assumption of how it should work (as a sync point). 
					passData.Inputs.emplace_back(registry.DeclareResource(DeclareConstantBuffer<SFrameBufferData>("Frame Buffer")));

					// TODO.NW: These inputs and outputs must be able to be aggregately bound by the render graph somehow
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Position", ERenderResourceFormat::R32G32B32A32_Float)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Editor Data", ERenderResourceFormat::R32G32_UnsignedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));

					passData.TransformBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
					passData.EntityBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SEntity>("Instanced Entity Buffer", InstancedDrawInstanceLimit));

					passData.BoneBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SBoneBufferData>("Bone Buffer"));
					passData.MaterialBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SMaterialBufferData>("Material Buffer"));

					// Import resources from asset registry

					for (SGraphicsMaterialAsset* materialAsset : materials)
					{
						// TODO.NW: Figure out how we can deal with this requesting? Materials are shared for each invocation here
						constexpr U64 tempRequester = 10000;
						std::map<U32, CStaticRenderTexture*> mappedTextures = materialAsset->Material.GetRenderTexturePointers(tempRequester);

						for (auto& [uid, texture] : mappedTextures)
						{
							passData.MaterialUIDToRuntimeIndex.emplace(uid, STATIC_F32(passData.MaterialTextureHandles.size()));
							passData.MaterialTextureHandles.emplace_back(registry.DeclareResource(DeclareRenderTexture(std::to_string(uid).c_str(), texture)));
						}
					}

					CRenderStateManager& stateManager = RenderManager->GetRenderStateManager();

					for (const SDrawCallData& drawCallData : skeletalMeshData)
					{
						SSkeletalGeometryEditorPassData::SSubPassData& subPassData = passData.SubPassData.emplace_back();
						subPassData.VertexBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SSkeletalMeshVertex>("Skeletal Mesh Vertex Buffer", &stateManager.VertexBuffers[drawCallData.VertexBufferIndex]));						
						subPassData.IndexBufferHandle = registry.DeclareResource(DeclareIndexBuffer("Mesh Index Buffer", &stateManager.IndexBuffers[drawCallData.IndexBufferIndex]));
						subPassData.IndexCount = drawCallData.IndexCount;
						subPassData.MaterialIndex = drawCallData.MaterialIndex;
					}

					return passData;
				},
				[materials, transforms, posedBones, entities](const SSkeletalGeometryEditorPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
				{
					SSkeletalMeshRenderData rendererData;
					SEditorGeometryRenderData editorData;

					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.BoneBuffer = registry.GetResource(data.BoneBufferHandle);
					rendererData.MaterialBuffer = registry.GetResource(data.MaterialBufferHandle);
					editorData.EntityBuffer = registry.GetResource(data.EntityBufferHandle);

					for (const SRenderResourceHandle& handle : data.MaterialTextureHandles)
						rendererData.MaterialTextures.emplace_back(registry.GetResource(handle));

					rendererData.TransformBuffer->DataBuffer->BindBuffer(transforms);
					rendererData.BoneBuffer->DataBuffer->BindBuffer(posedBones);
					rendererData.InstanceCount = STATIC_U32(transforms.size());
					editorData.EntityBuffer->DataBuffer->BindBuffer(entities);

					for (const SSkeletalGeometryEditorPassData::SSubPassData& subPassData : data.SubPassData)
					{
						SMaterialBufferData materialBufferData = SMaterialBufferData(materials[subPassData.MaterialIndex]->Material);
						for (SRuntimeGraphicsMaterialProperty& property : materialBufferData.Properties)
						{
							if (property.TextureChannelIndex > -1.0f)
								property.TextureIndex = data.MaterialUIDToRuntimeIndex.at(property.TextureUID);
						}
						rendererData.MaterialBuffer->DataBuffer->BindBuffer(materialBufferData);

						rendererData.VertexBuffer = registry.GetResource(subPassData.VertexBufferHandle);
						rendererData.IndexBuffer = registry.GetResource(subPassData.IndexBufferHandle);
						rendererData.IndexCount = subPassData.IndexCount;

						renderManager->GeometryRenderer.RenderEditorSkeletalMesh(rendererData, editorData);
					}
				}
			);
		}
		else
		{
			struct SSkeletalGeometryPassData : public SRenderPassParams
			{
				// Memory managed by RenderGraph
				SRenderResourceHandle TransformBufferHandle;
				SRenderResourceHandle MaterialBufferHandle;
				SRenderResourceHandle BoneBufferHandle;

				// External memory from Asset Registry
				// Static, taken from Asset Registry and used as external source for render resources
				std::vector<SRenderResourceHandle> MaterialTextureHandles;
				std::map<U32, F32> MaterialUIDToRuntimeIndex;

				struct SSubPassData
				{
					SRenderResourceHandle VertexBufferHandle;
					SRenderResourceHandle IndexBufferHandle;
					U32 IndexCount = 0;
					U16 MaterialIndex = 0;
				};
				std::vector<SSubPassData> SubPassData;
			};
			RenderManager->GetRenderGraph().AddPass<SSkeletalGeometryPassData>("SkelBasePassInstanced",
				[&](CRenderResourceRegistry& registry)
				{
					SSkeletalGeometryPassData passData;
					
					// TODO.NW: It's a bit weird that the render function has no control over where the inputs are bound (to what slot). Can only do this where there's a strong 
					// assumption of how it should work (as a sync point). 
					passData.Inputs.emplace_back(registry.DeclareResource(DeclareConstantBuffer<SFrameBufferData>("Frame Buffer")));

					// TODO.NW: These inputs and outputs must be able to be aggregately bound by the render graph somehow
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));

					passData.TransformBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));

					passData.MaterialBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SMaterialBufferData>("Material Buffer"));
					passData.BoneBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SBoneBufferData>("Bone Buffer"));

					// Import resources from asset registry

					for (SGraphicsMaterialAsset* materialAsset : materials)
					{
						// TODO.NW: Figure out how we can deal with this requesting? Materials are shared for each invocation here
						constexpr U64 tempRequester = 10000;
						std::map<U32, CStaticRenderTexture*> mappedTextures = materialAsset->Material.GetRenderTexturePointers(tempRequester);

						for (auto& [uid, texture] : mappedTextures)
						{
							passData.MaterialUIDToRuntimeIndex.emplace(uid, STATIC_F32(passData.MaterialTextureHandles.size()));
							passData.MaterialTextureHandles.emplace_back(registry.DeclareResource(DeclareRenderTexture(std::to_string(uid).c_str(), texture)));
						}
					}

					CRenderStateManager& stateManager = RenderManager->GetRenderStateManager();

					for (const SDrawCallData& drawCallData : skeletalMeshData)
					{
						SSkeletalGeometryPassData::SSubPassData& subPassData = passData.SubPassData.emplace_back();
						subPassData.VertexBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SSkeletalMeshVertex>("Skeletal Mesh Vertex Buffer", &stateManager.VertexBuffers[drawCallData.VertexBufferIndex]));
						subPassData.IndexBufferHandle = registry.DeclareResource(DeclareIndexBuffer("Mesh Index Buffer", &stateManager.IndexBuffers[drawCallData.IndexBufferIndex]));
						subPassData.IndexCount = drawCallData.IndexCount;
						subPassData.MaterialIndex = drawCallData.MaterialIndex;
					}

					return passData;
				},
				[materials, transforms, posedBones](const SSkeletalGeometryPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
				{
					SSkeletalMeshRenderData rendererData;

					rendererData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					rendererData.MaterialBuffer = registry.GetResource(data.MaterialBufferHandle);
					rendererData.BoneBuffer = registry.GetResource(data.BoneBufferHandle);

					for (const SRenderResourceHandle& handle : data.MaterialTextureHandles)
						rendererData.MaterialTextures.emplace_back(registry.GetResource(handle));

					rendererData.TransformBuffer->DataBuffer->BindBuffer(transforms);
					rendererData.BoneBuffer->DataBuffer->BindBuffer(posedBones),
					rendererData.InstanceCount = STATIC_U32(transforms.size());

					for (const SSkeletalGeometryPassData::SSubPassData& subPassData : data.SubPassData)
					{
						SMaterialBufferData materialBufferData = SMaterialBufferData(materials[subPassData.MaterialIndex]->Material);
						for (SRuntimeGraphicsMaterialProperty& property : materialBufferData.Properties)
						{
							if (property.TextureChannelIndex > -1.0f)
								property.TextureIndex = data.MaterialUIDToRuntimeIndex.at(property.TextureUID);
						}
						rendererData.MaterialBuffer->DataBuffer->BindBuffer(materialBufferData);

						rendererData.VertexBuffer = registry.GetResource(subPassData.VertexBufferHandle);
						rendererData.IndexBuffer = registry.GetResource(subPassData.IndexBufferHandle);
						rendererData.IndexCount = subPassData.IndexCount;

						renderManager->GeometryRenderer.RenderSkeletalMesh(rendererData);
					}
				}
			);
		}
	}

	void CRenderSystem::AddBillboardPass(STextureAsset* textureAsset, const std::vector<SMatrix>& transforms, const std::vector<SVector4>& uvRects, const std::vector<SVector4>& colors, const std::vector<SEntity>& entities, const bool runEditorDataPasses)
	{
		if (runEditorDataPasses)
		{
			struct SEditorBillboardPassData : public SRenderPassParams
			{
				SRenderResourceHandle TransformBufferHandle;
				SRenderResourceHandle UVRectBufferHandle;
				SRenderResourceHandle ColorBufferHandle;
				SRenderResourceHandle EntityBufferHandle;
				SRenderResourceHandle SpriteTextureHandle;
			};
			RenderManager->GetRenderGraph().AddPass<SEditorBillboardPassData>("EditorBillboardPass",
				[&](CRenderResourceRegistry& registry)
				{
					SEditorBillboardPassData passData;

					// TODO.NW: It's a bit weird that the render function has no control over where the inputs are bound (to what slot). Can only do this where there's a strong 
					// assumption of how it should work (as a sync point). 
					passData.Inputs.emplace_back(registry.DeclareResource(DeclareConstantBuffer<SFrameBufferData>("Frame Buffer")));

					// TODO.NW: These inputs and outputs must be able to be aggregately bound by the render graph somehow
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Position", ERenderResourceFormat::R32G32B32A32_Float)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Editor Data", ERenderResourceFormat::R32G32_UnsignedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));

					passData.TransformBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
					passData.UVRectBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SVector4>("Instanced UV Rect Buffer", InstancedDrawInstanceLimit));
					passData.ColorBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SVector4>("Instanced Color Buffer", InstancedDrawInstanceLimit));
					passData.EntityBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SEntity>("Instanced Entity Buffer", InstancedDrawInstanceLimit));
					passData.SpriteTextureHandle = registry.DeclareResource(DeclareRenderTexture("Billboard Texture", &textureAsset->RenderTexture));
					return passData;
				},
				[=](const SEditorBillboardPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
				{
					// TODO.NW: Bind outputs, needs to include gbuffer targets
					SSpriteRenderData renderData;
					renderData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					renderData.TransformBuffer->DataBuffer->BindBuffer(transforms);

					renderData.UVRectBuffer = registry.GetResource(data.UVRectBufferHandle);
					renderData.UVRectBuffer->DataBuffer->BindBuffer(uvRects);

					renderData.ColorBuffer = registry.GetResource(data.ColorBufferHandle);
					renderData.ColorBuffer->DataBuffer->BindBuffer(colors);

					renderData.SpriteTexture = registry.GetResource(data.SpriteTextureHandle);

					SEditorSpriteRenderData editorRenderData;
					editorRenderData.EntityBuffer = registry.GetResource(data.EntityBufferHandle);
					editorRenderData.EntityBuffer->DataBuffer->BindBuffer(entities);

					renderManager->SpriteRenderer.RenderEditorWorldSpaceSprite(renderData, editorRenderData);
				}
			);
		}
		else
		{
			struct SBillboardPassData : public SRenderPassParams
			{
				SRenderResourceHandle TransformBufferHandle;
				SRenderResourceHandle UVRectBufferHandle;
				SRenderResourceHandle ColorBufferHandle;
				SRenderResourceHandle SpriteTextureHandle;
			};
			RenderManager->GetRenderGraph().AddPass<SBillboardPassData>("BillboardPass",
				[&](CRenderResourceRegistry& registry)
				{
					SBillboardPassData passData;

					// TODO.NW: It's a bit weird that the render function has no control over where the inputs are bound (to what slot). Can only do this where there's a strong 
					// assumption of how it should work (as a sync point). 
					passData.Inputs.emplace_back(registry.DeclareResource(DeclareConstantBuffer<SFrameBufferData>("Frame Buffer")));

					// TODO.NW: These inputs and outputs must be able to be aggregately bound by the render graph somehow
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
					passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));

					passData.TransformBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
					passData.UVRectBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SVector4>("Instanced UV Rect Buffer", InstancedDrawInstanceLimit));
					passData.ColorBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SVector4>("Instanced Color Buffer", InstancedDrawInstanceLimit));
					passData.SpriteTextureHandle = registry.DeclareResource(DeclareRenderTexture("Billboard Texture", &textureAsset->RenderTexture));
					return passData;
				},
				[=](const SBillboardPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
				{
					// TODO.NW: Bind outputs, needs to include gbuffer targets
					SSpriteRenderData renderData;
					renderData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
					renderData.TransformBuffer->DataBuffer->BindBuffer(transforms);

					renderData.UVRectBuffer = registry.GetResource(data.UVRectBufferHandle);
					renderData.UVRectBuffer->DataBuffer->BindBuffer(uvRects);

					renderData.ColorBuffer = registry.GetResource(data.ColorBufferHandle);
					renderData.ColorBuffer->DataBuffer->BindBuffer(colors);

					renderData.SpriteTexture = registry.GetResource(data.SpriteTextureHandle);

					renderManager->SpriteRenderer.RenderWorldSpaceSprite(renderData);
				}
			);
		}
	}
	
	void CRenderSystem::AddDecalPass(const SMatrix& transformMatrix, const std::vector<STextureAsset*>& textureAssets)
	{
		struct SDecalGeometryPassData : public SRenderPassParams
		{
			SRenderResourceHandle DecalBufferHandle;

			// Owned by Asset Registry
			SRenderResourceHandle AlbedoTextureHandle;
			SRenderResourceHandle MaterialTextureHandle;
			SRenderResourceHandle NormalTextureHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SDecalGeometryPassData>("DecalPass",
			[&](CRenderResourceRegistry& registry)
			{
				SDecalGeometryPassData passData;
				// TODO.NW: It's a bit weird that the render function has no control over where the inputs are bound (to what slot). Can only do this where there's a strong 
				// assumption of how it should work (as a sync point). 
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareConstantBuffer<SFrameBufferData>("Frame Buffer")));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Depth Copy", ERenderResourceFormat::R32_Float)));

				// TODO.NW: These inputs and outputs must be able to be aggregately bound by the render graph somehow
				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));

				passData.DecalBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SDecalBufferData>("Decal Buffer"));

				if (textureAssets[0] != nullptr)
					passData.AlbedoTextureHandle = registry.DeclareResource(DeclareRenderTexture("Decal Albedo", &textureAssets[0]->RenderTexture));

				if (textureAssets[1] != nullptr)
					passData.MaterialTextureHandle = registry.DeclareResource(DeclareRenderTexture("Decal Material", &textureAssets[1]->RenderTexture));

				if (textureAssets[2] != nullptr)
					passData.NormalTextureHandle = registry.DeclareResource(DeclareRenderTexture("Decal Normal", &textureAssets[2]->RenderTexture));
				
				return passData;
			},
			[transformMatrix](const SDecalGeometryPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
			{
				SDecalRenderData rendererData;

				const SDecalBufferData bufferData = { .ToWorld = transformMatrix, .ToObjectSpace = transformMatrix.Inverse() };
				
				rendererData.DecalBuffer = registry.GetResource(data.DecalBufferHandle);
				rendererData.DecalBuffer->DataBuffer->BindBuffer(bufferData);

				rendererData.OptionalAlbedoTexture = registry.GetResource(data.AlbedoTextureHandle);
				rendererData.OptionalMaterialTexture = registry.GetResource(data.MaterialTextureHandle);
				rendererData.OptionalNormalTexture = registry.GetResource(data.NormalTextureHandle);

				renderManager->GeometryRenderer.RenderDecal(rendererData);
			}
		);
	}
	
	void CRenderSystem::AddDirectionalLightPass(SDirectionalLightComponent* component, SVolumetricLightComponent* volumetricLightComp, CStaticRenderTexture* cubemapTexture, const SShadowmapViewData& shadowmapViewData)
	{
		const SVector4& direction = component->Direction;
		const SVector4& color = component->Color;
		const SVector2<F32>& shadowAtlasResolution = RenderManager->GetShadowAtlasResolution();

		struct SDirectionalLightPassData : public SRenderPassParams
		{
			SRenderResourceHandle LightBufferHandle;
			SRenderResourceHandle ShadowmapBufferHandle;
			SRenderResourceHandle EmissiveBufferHandle;
			SRenderResourceHandle CubemapTextureHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SDirectionalLightPassData>("DirectionalLightPass",
			[&](CRenderResourceRegistry& registry)
			{
				SDirectionalLightPassData passData;

				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y)))));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("SSAO Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution)));

				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Lit Scene", ERenderResourceFormat::R16G16B16A16_Float)));

				passData.LightBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SDirectionalLightBufferData>("Directional Light Buffer"));
				passData.ShadowmapBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SShadowmapBufferData>("Shadowmap Buffer", 6));
				passData.EmissiveBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SEmissiveBufferData>("Emissive Buffer"));

				passData.CubemapTextureHandle = registry.DeclareResource(DeclareRenderTexture("Directional Light Cubemap", cubemapTexture));
				
				return passData;
			},
			[direction, color, shadowAtlasResolution, shadowmapViewData](const SDirectionalLightPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
			{
				SDirectionalLightRenderData rendererData;

				SDirectionalLightBufferData lightBufferData;
				lightBufferData.ToDirectionalLight = -direction;
				lightBufferData.DirectionalLightColor = color;
				rendererData.LightBuffer = registry.GetResource(data.LightBufferHandle);
				rendererData.LightBuffer->DataBuffer->BindBuffer(lightBufferData);

				const CRenderViewport& viewport = renderManager->GetRenderStateManager().Viewports[shadowmapViewData.ShadowmapViewportIndex];

				SShadowmapBufferData shadowmapBufferData;
				shadowmapBufferData.ToShadowmapView = shadowmapViewData.ShadowViewMatrix;
				shadowmapBufferData.ToShadowmapProjection = shadowmapViewData.ShadowProjectionMatrix;
				shadowmapBufferData.ShadowmapPosition = shadowmapViewData.ShadowPosition;
				shadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
				shadowmapBufferData.ShadowAtlasResolution = shadowAtlasResolution;
				shadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / shadowAtlasResolution.X, viewport.TopLeftY / shadowAtlasResolution.Y };
				shadowmapBufferData.ShadowTestTolerance = 0.001f;
				rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
				rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(shadowmapBufferData);

				rendererData.EmissiveBuffer = registry.GetResource(data.EmissiveBufferHandle);
				rendererData.EmissiveBuffer->DataBuffer->BindBuffer(SEmissiveBufferData{ .EmissiveStrength = 2.1f }); // TODO.NW: Extract this from fullscreenrenderer post processing buffer data

				rendererData.CubemapTexture = registry.GetResource(data.CubemapTextureHandle);

				renderManager->LightRenderer.RenderDirectionalLight(rendererData);
			}
		);

		if (!SComponent::IsValid(volumetricLightComp) || !volumetricLightComp->IsActive)
			return;

		const F32 numberOfSamples = volumetricLightComp->NumberOfSamples;
		const F32 lightPower = volumetricLightComp->LightPower;
		const F32 scatteringProbability = volumetricLightComp->ScatteringProbability;
		const F32 henyeyGreensteinGValue = volumetricLightComp->HenyeyGreensteinGValue;

		struct SVolumetricLightPassData : public SRenderPassParams
		{
			SRenderResourceHandle LightBufferHandle;
			SRenderResourceHandle VolumetricLightBufferHandle;
			SRenderResourceHandle ShadowmapBufferHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SVolumetricLightPassData>("VolumetricDirectionalPass",
			[&](CRenderResourceRegistry& registry)
			{
				SVolumetricLightPassData passData;

				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y)))));
				
				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Volumetric Accumulation", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution)));

				passData.LightBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SDirectionalLightBufferData>("Directional Light Buffer"));
				passData.VolumetricLightBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SVolumetricLightBufferData>("Volumetric Light Buffer"));
				passData.ShadowmapBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SShadowmapBufferData>("Shadowmap Buffer", 6));

				return passData;
			},
			[=](const SVolumetricLightPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
			{
				SVolumetricDirectionalLightRenderData rendererData;

				SDirectionalLightBufferData lightBufferData;
				lightBufferData.ToDirectionalLight = -direction;
				lightBufferData.DirectionalLightColor = color;
				rendererData.LightBuffer = registry.GetResource(data.LightBufferHandle);
				rendererData.LightBuffer->DataBuffer->BindBuffer(lightBufferData);

				SVolumetricLightBufferData volumetricBufferData;
				volumetricBufferData.NumberOfSamplesReciprocal = 1.0f / numberOfSamples;
				volumetricBufferData.LightPower = lightPower;
				volumetricBufferData.ScatteringProbability = scatteringProbability;
				volumetricBufferData.HenyeyGreensteinGValue = henyeyGreensteinGValue;
				rendererData.VolumetricLightBuffer = registry.GetResource(data.VolumetricLightBufferHandle);
				rendererData.VolumetricLightBuffer->DataBuffer->BindBuffer(volumetricBufferData);

				const CRenderViewport& viewport = renderManager->GetRenderStateManager().Viewports[shadowmapViewData.ShadowmapViewportIndex];

				SShadowmapBufferData shadowmapBufferData;
				shadowmapBufferData.ToShadowmapView = shadowmapViewData.ShadowViewMatrix;
				shadowmapBufferData.ToShadowmapProjection = shadowmapViewData.ShadowProjectionMatrix;
				shadowmapBufferData.ShadowmapPosition = shadowmapViewData.ShadowPosition;
				shadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
				shadowmapBufferData.ShadowAtlasResolution = shadowAtlasResolution;
				shadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / shadowAtlasResolution.X, viewport.TopLeftY / shadowAtlasResolution.Y };
				shadowmapBufferData.ShadowTestTolerance = 0.001f;
				rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
				rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(shadowmapBufferData);

				renderManager->LightRenderer.RenderVolumetricDirectionalLight(rendererData);
			}
		);
	}

	void CRenderSystem::AddPointLightPass(SPointLightComponent* component, SVolumetricLightComponent* volumetricLightComp, const SMatrix& transform, const std::vector<SShadowmapViewData>& shadowmapViewData)
	{
		const SVector4& colorAndIntensity = component->ColorAndIntensity;
		const F32 range = component->Range;
		const SVector2<F32>& shadowAtlasResolution = RenderManager->GetShadowAtlasResolution();

		struct SPointLightPassData : SRenderPassParams
		{
			SRenderResourceHandle LightBufferHandle;
			SRenderResourceHandle ShadowmapBufferHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SPointLightPassData>("PointLightPass",
			[&](CRenderResourceRegistry& registry)
			{
				SPointLightPassData passData;

				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y)))));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("SSAO Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution)));

				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Lit Scene", ERenderResourceFormat::R16G16B16A16_Float)));

				passData.LightBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SPointLightBufferData>("Point Light Buffer"));
				passData.ShadowmapBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SShadowmapBufferData>("Shadowmap Buffer", 6));

				return passData;
			},
			[colorAndIntensity, range, shadowAtlasResolution, transform, shadowmapViewData](const SPointLightPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
			{
				SPointLightRenderData rendererData;

				SPointLightBufferData lightBufferData;
				lightBufferData.ToWorldFromObject = transform;
				lightBufferData.ColorAndIntensity = colorAndIntensity;
				lightBufferData.PositionAndRange = SVector4(transform.GetTranslation(), range);
				rendererData.LightBuffer = registry.GetResource(data.LightBufferHandle);
				rendererData.LightBuffer->DataBuffer->BindBuffer(lightBufferData);

				SShadowmapBufferData shadowmapBufferData[6];
				for (U8 shadowmapViewIndex = 0; shadowmapViewIndex < 6; shadowmapViewIndex++)
				{
					shadowmapBufferData[shadowmapViewIndex].ToShadowmapView = shadowmapViewData[shadowmapViewIndex].ShadowViewMatrix;
					shadowmapBufferData[shadowmapViewIndex].ToShadowmapProjection = shadowmapViewData[shadowmapViewIndex].ShadowProjectionMatrix;
					shadowmapBufferData[shadowmapViewIndex].ShadowmapPosition = shadowmapViewData[shadowmapViewIndex].ShadowPosition;

					const auto& viewport = renderManager->GetRenderStateManager().Viewports[shadowmapViewData[shadowmapViewIndex].ShadowmapViewportIndex];
					shadowmapBufferData[shadowmapViewIndex].ShadowmapResolution = { viewport.Width, viewport.Height };
					shadowmapBufferData[shadowmapViewIndex].ShadowAtlasResolution = shadowAtlasResolution;
					shadowmapBufferData[shadowmapViewIndex].ShadowmapStartingUV = { viewport.TopLeftX / shadowAtlasResolution.X, viewport.TopLeftY / shadowAtlasResolution.Y };
					shadowmapBufferData[shadowmapViewIndex].ShadowTestTolerance = 0.00001f;	// TODO.NW: make a constant somewhere
				}

				rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
				rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(shadowmapBufferData);

				renderManager->LightRenderer.RenderPointLight(rendererData);
			}
		);

		if (!SComponent::IsValid(volumetricLightComp) || !volumetricLightComp->IsActive)
			return;

		const F32 numberOfSamples = volumetricLightComp->NumberOfSamples;
		const F32 lightPower = volumetricLightComp->LightPower;
		const F32 scatteringProbability = volumetricLightComp->ScatteringProbability;
		const F32 henyeyGreensteinGValue = volumetricLightComp->HenyeyGreensteinGValue;

		struct SVolumetricLightPassData : public SRenderPassParams
		{
			SRenderResourceHandle LightBufferHandle;
			SRenderResourceHandle VolumetricLightBufferHandle;
			SRenderResourceHandle ShadowmapBufferHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SVolumetricLightPassData>("VolumetricPointPass",
			[&](CRenderResourceRegistry& registry)
			{
				SVolumetricLightPassData passData;

				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y)))));

				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Volumetric Accumulation", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution)));

				passData.LightBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SPointLightBufferData>("Point Light Buffer"));
				passData.VolumetricLightBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SVolumetricLightBufferData>("Volumetric Light Buffer"));
				passData.ShadowmapBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SShadowmapBufferData>("Shadowmap Buffer", 6));

				return passData;
			},
			[=](const SVolumetricLightPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
			{
				SVolumetricPointLightRenderData rendererData;

				SPointLightBufferData lightBufferData;
				lightBufferData.ToWorldFromObject = transform;
				lightBufferData.ColorAndIntensity = colorAndIntensity;
				lightBufferData.PositionAndRange = SVector4(transform.GetTranslation(), range);
				rendererData.LightBuffer = registry.GetResource(data.LightBufferHandle);
				rendererData.LightBuffer->DataBuffer->BindBuffer(lightBufferData);

				SVolumetricLightBufferData volumetricBufferData;
				volumetricBufferData.NumberOfSamplesReciprocal = 1.0f / numberOfSamples;
				volumetricBufferData.LightPower = lightPower;
				volumetricBufferData.ScatteringProbability = scatteringProbability;
				volumetricBufferData.HenyeyGreensteinGValue = henyeyGreensteinGValue;
				rendererData.VolumetricLightBuffer = registry.GetResource(data.VolumetricLightBufferHandle);
				rendererData.VolumetricLightBuffer->DataBuffer->BindBuffer(volumetricBufferData);

				SShadowmapBufferData shadowmapBufferData[6];
				for (U8 shadowmapViewIndex = 0; shadowmapViewIndex < 6; shadowmapViewIndex++)
				{
					shadowmapBufferData[shadowmapViewIndex].ToShadowmapView = shadowmapViewData[shadowmapViewIndex].ShadowViewMatrix;
					shadowmapBufferData[shadowmapViewIndex].ToShadowmapProjection = shadowmapViewData[shadowmapViewIndex].ShadowProjectionMatrix;
					shadowmapBufferData[shadowmapViewIndex].ShadowmapPosition = shadowmapViewData[shadowmapViewIndex].ShadowPosition;

					const auto& viewport = renderManager->GetRenderStateManager().Viewports[shadowmapViewData[shadowmapViewIndex].ShadowmapViewportIndex];
					shadowmapBufferData[shadowmapViewIndex].ShadowmapResolution = { viewport.Width, viewport.Height };
					shadowmapBufferData[shadowmapViewIndex].ShadowAtlasResolution = shadowAtlasResolution;
					shadowmapBufferData[shadowmapViewIndex].ShadowmapStartingUV = { viewport.TopLeftX / shadowAtlasResolution.X, viewport.TopLeftY / shadowAtlasResolution.Y };
					shadowmapBufferData[shadowmapViewIndex].ShadowTestTolerance = 0.00001f;	// TODO.NW: make a constant somewhere
				}

				rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
				rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(shadowmapBufferData);

				renderManager->LightRenderer.RenderVolumetricPointLight(rendererData);
			}
		);
	}

	void CRenderSystem::AddSpotlightPass(SSpotLightComponent* component, SVolumetricLightComponent* volumetricLightComp, const SMatrix& transform, const SShadowmapViewData& shadowmapViewData)
	{
		const SVector4& colorAndIntensity = component->ColorAndIntensity;
		const F32 range = component->Range;
		const F32 outerAngle = component->OuterAngle;
		const F32 innerAngle = component->InnerAngle;
		const SVector4& direction = component->Direction;
		const SVector4& directionNormal1 = component->DirectionNormal1;
		const SVector4& directionNormal2 = component->DirectionNormal2;
		const SVector2<F32>& shadowAtlasResolution = RenderManager->GetShadowAtlasResolution();

		struct SSpotLightPassData : public SRenderPassParams
		{
			SRenderResourceHandle LightBufferHandle;
			SRenderResourceHandle PointLightBufferHandle;
			SRenderResourceHandle ShadowmapBufferHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SSpotLightPassData>("SpotLightPass",
			[&](CRenderResourceRegistry& registry)
			{
				SSpotLightPassData passData;

				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Albedo", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Vertex Normal", ERenderResourceFormat::R16G16B16A16_SignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Material", ERenderResourceFormat::R8G8B8A8_UnsignedNormalizedInt)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y)))));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("SSAO Blur Texture", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution)));

				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Lit Scene", ERenderResourceFormat::R16G16B16A16_Float)));

				passData.LightBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SSpotLightBufferData>("Spotlight Buffer"));
				passData.PointLightBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SPointLightBufferData>("Point Light Buffer"));
				passData.ShadowmapBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SShadowmapBufferData>("Shadowmap Buffer", 6));

				return passData;
			},
			[=](const SSpotLightPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
			{
				SSpotlightRenderData rendererData;

				SPointLightBufferData pointLightBufferData;
				pointLightBufferData.ToWorldFromObject = transform;
				pointLightBufferData.ColorAndIntensity = colorAndIntensity;
				pointLightBufferData.PositionAndRange = SVector4(transform.GetTranslation(), range);
				rendererData.PointLightBuffer = registry.GetResource(data.PointLightBufferHandle);
				rendererData.PointLightBuffer->DataBuffer->BindBuffer(pointLightBufferData);

				SSpotLightBufferData spotlightBufferData;
				spotlightBufferData.ColorAndIntensity = pointLightBufferData.ColorAndIntensity;
				spotlightBufferData.PositionAndRange = pointLightBufferData.PositionAndRange;
				spotlightBufferData.Direction = direction;
				spotlightBufferData.DirectionNormal1 = directionNormal1;
				spotlightBufferData.DirectionNormal2 = directionNormal2;
				spotlightBufferData.OuterAngle = outerAngle;
				spotlightBufferData.InnerAngle = innerAngle;
				rendererData.SpotlightBuffer = registry.GetResource(data.LightBufferHandle);
				rendererData.SpotlightBuffer->DataBuffer->BindBuffer(spotlightBufferData);

				SShadowmapBufferData shadowmapBufferData;				
				shadowmapBufferData.ToShadowmapView = shadowmapViewData.ShadowViewMatrix;
				shadowmapBufferData.ToShadowmapProjection = shadowmapViewData.ShadowProjectionMatrix;
				shadowmapBufferData.ShadowmapPosition = shadowmapViewData.ShadowPosition;

				const auto& viewport = renderManager->GetRenderStateManager().Viewports[shadowmapViewData.ShadowmapViewportIndex];
				shadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
				shadowmapBufferData.ShadowAtlasResolution = shadowAtlasResolution;
				shadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / shadowAtlasResolution.X, viewport.TopLeftY / shadowAtlasResolution.Y };
				shadowmapBufferData.ShadowTestTolerance = 0.00001f;	// TODO.NW: make a constant somewhere
				
				rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
				rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(shadowmapBufferData);

				renderManager->LightRenderer.RenderSpotlight(rendererData);
			}
		);

		if (!SComponent::IsValid(volumetricLightComp) || !volumetricLightComp->IsActive)
			return;

		const F32 numberOfSamples = volumetricLightComp->NumberOfSamples;
		const F32 lightPower = volumetricLightComp->LightPower;
		const F32 scatteringProbability = volumetricLightComp->ScatteringProbability;
		const F32 henyeyGreensteinGValue = volumetricLightComp->HenyeyGreensteinGValue;

		struct SVolumetricLightPassData : public SRenderPassParams
		{
			SRenderResourceHandle SpotlightBufferHandle;
			SRenderResourceHandle PointLightBufferHandle;
			SRenderResourceHandle VolumetricLightBufferHandle;
			SRenderResourceHandle ShadowmapBufferHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SVolumetricLightPassData>("VolumetricSpotPass",
			[&](CRenderResourceRegistry& registry)
			{
				SVolumetricLightPassData passData;

				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Shadow Atlas Depth", ERenderResourceFormat::R32_Typeless, SVector2<U16>(STATIC_U16(shadowAtlasResolution.X), STATIC_U16(shadowAtlasResolution.Y)))));

				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Volumetric Accumulation", ERenderResourceFormat::R16G16B16A16_Float, ERenderTextureSizeHint::HalfResolution)));

				passData.SpotlightBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SSpotLightBufferData>("Spotlight Buffer"));
				passData.PointLightBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SPointLightBufferData>("Point Light Buffer"));
				passData.VolumetricLightBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SVolumetricLightBufferData>("Volumetric Light Buffer"));
				passData.ShadowmapBufferHandle = registry.DeclareResource(DeclareConstantBuffer<SShadowmapBufferData>("Shadowmap Buffer", 6));

				return passData;
			},
			[=](const SVolumetricLightPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
			{
				SVolumetricSpotlightRenderData rendererData;

				SPointLightBufferData pointLightBufferData;
				pointLightBufferData.ToWorldFromObject = transform;
				pointLightBufferData.ColorAndIntensity = colorAndIntensity;
				pointLightBufferData.PositionAndRange = SVector4(transform.GetTranslation(), range);
				rendererData.PointLightBuffer = registry.GetResource(data.PointLightBufferHandle);
				rendererData.PointLightBuffer->DataBuffer->BindBuffer(pointLightBufferData);

				SSpotLightBufferData spotlightBufferData;
				spotlightBufferData.ColorAndIntensity = pointLightBufferData.ColorAndIntensity;
				spotlightBufferData.PositionAndRange = pointLightBufferData.PositionAndRange;
				spotlightBufferData.Direction = direction;
				spotlightBufferData.DirectionNormal1 = directionNormal1;
				spotlightBufferData.DirectionNormal2 = directionNormal2;
				spotlightBufferData.OuterAngle = outerAngle;
				spotlightBufferData.InnerAngle = innerAngle;
				rendererData.SpotlightBuffer = registry.GetResource(data.SpotlightBufferHandle);
				rendererData.SpotlightBuffer->DataBuffer->BindBuffer(spotlightBufferData);

				SVolumetricLightBufferData volumetricBufferData;
				volumetricBufferData.NumberOfSamplesReciprocal = 1.0f / numberOfSamples;
				volumetricBufferData.LightPower = lightPower;
				volumetricBufferData.ScatteringProbability = scatteringProbability;
				volumetricBufferData.HenyeyGreensteinGValue = henyeyGreensteinGValue;
				rendererData.VolumetricLightBuffer = registry.GetResource(data.VolumetricLightBufferHandle);
				rendererData.VolumetricLightBuffer->DataBuffer->BindBuffer(volumetricBufferData);

				const CRenderViewport& viewport = renderManager->GetRenderStateManager().Viewports[shadowmapViewData.ShadowmapViewportIndex];

				SShadowmapBufferData shadowmapBufferData;
				shadowmapBufferData.ToShadowmapView = shadowmapViewData.ShadowViewMatrix;
				shadowmapBufferData.ToShadowmapProjection = shadowmapViewData.ShadowProjectionMatrix;
				shadowmapBufferData.ShadowmapPosition = shadowmapViewData.ShadowPosition;
				shadowmapBufferData.ShadowmapResolution = { viewport.Width, viewport.Height };
				shadowmapBufferData.ShadowAtlasResolution = shadowAtlasResolution;
				shadowmapBufferData.ShadowmapStartingUV = { viewport.TopLeftX / shadowAtlasResolution.X, viewport.TopLeftY / shadowAtlasResolution.Y };
				shadowmapBufferData.ShadowTestTolerance = 0.001f;
				rendererData.ShadowmapBuffer = registry.GetResource(data.ShadowmapBufferHandle);
				rendererData.ShadowmapBuffer->DataBuffer->BindBuffer(shadowmapBufferData);

				renderManager->LightRenderer.RenderVolumetricSpotlight(rendererData);
			}
		);
	}

	void CRenderSystem::AddSkyboxPass(STextureCubeAsset* cubemapTextureAsset)
	{
		struct SSkyboxPassData : public SRenderPassParams
		{
			SRenderResourceHandle CubemapTextureHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SSkyboxPassData>("SkyboxPass",
			[&](CRenderResourceRegistry& registry)
			{
				SSkyboxPassData passData;

				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Lit Scene", ERenderResourceFormat::R16G16B16A16_Float)));

				passData.CubemapTextureHandle = registry.DeclareResource(DeclareRenderTexture("Skybox Cubemap Texture", &cubemapTextureAsset->RenderTexture));
				return passData;
			},
			[](const SSkyboxPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
			{
				SSkyboxRenderData renderData;
				renderData.CubemapTexture = registry.GetResource(data.CubemapTextureHandle);
				renderManager->GeometryRenderer.RenderSkybox(renderData);
			}
		);
	}

	void CRenderSystem::AddEditorWidgetPass(STextureAsset* textureAsset, const std::vector<SMatrix>& transforms, const std::vector<SVector4>& uvRects, const std::vector<SVector4>& colors, const std::vector<SEntity>& entities)
	{
		struct SEditorWidgetPassData : public SRenderPassParams
		{
			SRenderResourceHandle TransformBufferHandle;
			SRenderResourceHandle UVRectBufferHandle;
			SRenderResourceHandle ColorBufferHandle;
			SRenderResourceHandle EntityBufferHandle;
			SRenderResourceHandle SpriteTextureHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SEditorWidgetPassData>("EditorWidgetPass",
			[&](CRenderResourceRegistry& registry)
			{
				SEditorWidgetPassData passData;

				// NW: Bound as input here for dependency
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float)));

				passData.Dependencies.push_back(typeid().hash_code());

				// TODO.NW: Need some control over what slots these outputs are bound to?
				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float)));
				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer World Position", ERenderResourceFormat::R32G32B32A32_Float)));
				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("GBuffer Editor Data", ERenderResourceFormat::R32G32_UnsignedInt)));
				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));

				passData.TransformBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.UVRectBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SVector4>("Instanced UV Rect Buffer", InstancedDrawInstanceLimit));
				passData.ColorBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SVector4>("Instanced Color Buffer", InstancedDrawInstanceLimit));
				passData.EntityBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SEntity>("Instanced Entity Buffer", InstancedDrawInstanceLimit));
				passData.SpriteTextureHandle = registry.DeclareResource(DeclareRenderTexture("Widget Texture", &textureAsset->RenderTexture));
				return passData;
			},
			[=](const SEditorWidgetPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
			{
				// TODO.NW: Bind outputs, needs to include gbuffer targets
				SSpriteRenderData renderData;
				renderData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
				renderData.TransformBuffer->DataBuffer->BindBuffer(transforms);

				renderData.UVRectBuffer = registry.GetResource(data.UVRectBufferHandle);
				renderData.UVRectBuffer->DataBuffer->BindBuffer(uvRects);

				renderData.ColorBuffer = registry.GetResource(data.ColorBufferHandle);
				renderData.ColorBuffer->DataBuffer->BindBuffer(colors);

				renderData.SpriteTexture = registry.GetResource(data.SpriteTextureHandle);

				SEditorSpriteRenderData editorRenderData;
				editorRenderData.EntityBuffer = registry.GetResource(data.EntityBufferHandle);
				editorRenderData.EntityBuffer->DataBuffer->BindBuffer(entities);

				renderManager->SpriteRenderer.RenderEditorWorldSpaceWidget(renderData, editorRenderData);
			}
		);
	}

	void CRenderSystem::AddUIPass(STextureAsset* textureAsset, const std::vector<SMatrix>& transforms, const std::vector<SVector4>& uvRects, const std::vector<SVector4>& colors)
	{
		struct SUIPassData : public SRenderPassParams
		{
			SRenderResourceHandle TransformBufferHandle;
			SRenderResourceHandle UVRectBufferHandle;
			SRenderResourceHandle ColorBufferHandle;
			SRenderResourceHandle SpriteTextureHandle;
		};
		RenderManager->GetRenderGraph().AddPass<SUIPassData>("UIPass",
			[&](CRenderResourceRegistry& registry)
			{
				SUIPassData passData;

				// NW: Bound as input here for dependency
				passData.Inputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float)));

				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderTexture("Tonemapped Texture", ERenderResourceFormat::R16G16B16A16_Float)));
				passData.Outputs.emplace_back(registry.DeclareResource(DeclareRenderDepth("Intermediate Depth", ERenderResourceFormat::R24G8_Typeless)));

				passData.TransformBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SMatrix>("Instanced Transform Buffer", InstancedDrawInstanceLimit));
				passData.UVRectBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SVector4>("Instanced UV Rect Buffer", InstancedDrawInstanceLimit));
				passData.ColorBufferHandle = registry.DeclareResource(DeclareVertexBuffer<SVector4>("Instanced Color Buffer", InstancedDrawInstanceLimit));
				passData.SpriteTextureHandle = registry.DeclareResource(DeclareRenderTexture("UI Texture", &textureAsset->RenderTexture));
				return passData;
			},
			[=](const SUIPassData& data, CRenderResourceRegistry& registry, CRenderManager* renderManager)
			{
				// TODO.NW: Make sure this is applied at the right time, after tonemapping and stuff

				SSpriteRenderData renderData;
				renderData.TransformBuffer = registry.GetResource(data.TransformBufferHandle);
				renderData.TransformBuffer->DataBuffer->BindBuffer(transforms);

				renderData.UVRectBuffer = registry.GetResource(data.UVRectBufferHandle);
				renderData.UVRectBuffer->DataBuffer->BindBuffer(uvRects);

				renderData.ColorBuffer = registry.GetResource(data.ColorBufferHandle);
				renderData.ColorBuffer->DataBuffer->BindBuffer(colors);

				renderData.SpriteTexture = registry.GetResource(data.SpriteTextureHandle);

				renderManager->SpriteRenderer.RenderScreenSpaceSprite(renderData);
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
