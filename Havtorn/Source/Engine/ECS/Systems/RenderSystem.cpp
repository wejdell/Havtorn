// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "RenderSystem.h"
#include "Scene/World.h"
#include "Scene/Scene.h"
#include "ECS/ECSInclude.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderCommand.h"
#include "ECS/ComponentAlgo.h"
#include "Input/Input.h"
#include "Assets/AssetRegistry.h"

namespace Havtorn
{
	// TODO.NW: Read from config? Would rather not involve editor resource manager here. 
	// Want to move this anyway to some sort of editor rendering system
	static const SAssetReference CameraWidgetReference = SAssetReference("Resources/Assets/CameraIcon.hva");
	static const SAssetReference ColliderWidgetReference = SAssetReference("Resources/Assets/ColliderIcon.hva");
	static const SAssetReference DecalWidgetReference = SAssetReference("Resources/Assets/DecalIcon.hva");
	static const SAssetReference DirectionalLightWidgetReference = SAssetReference("Resources/Assets/DirectionalLightIcon.hva");
	static const SAssetReference EnvironmentLightWidgetReference = SAssetReference("Resources/Assets/EnvironmentLightIcon.hva");
	static const SAssetReference PointLightWidgetReference = SAssetReference("Resources/Assets/PointLightIcon.hva");
	static const SAssetReference SpotlightWidgetReference = SAssetReference("Resources/Assets/SpotlightIcon.hva");

	CRenderSystem::CRenderSystem(CRenderManager* renderManager, CWorld* world)
		: ISystem()
		, RenderManager(renderManager)
		, World(world)
	{
	}

	void CRenderSystem::Update(CScene* scene)
	{
		const std::vector<SDirectionalLightComponent*>& directionalLightComponents = scene->GetComponents<SDirectionalLightComponent>();
		const std::vector<SPointLightComponent*>& pointLightComponents = scene->GetComponents<SPointLightComponent>();
		const std::vector<SSpotLightComponent*>& spotLightComponents = scene->GetComponents<SSpotLightComponent>();

		RenderManager->ClearSystemStaticMeshInstanceData();
		RenderManager->ClearSystemSkeletalMeshInstanceData();
		RenderManager->ClearSystemWorldSpaceSpriteInstanceData();
		RenderManager->ClearSystemScreenSpaceSpriteInstanceData();

		if (!scene->OnEntityPreDestroy.IsBoundTo(Handle))
		{
			Handle = scene->OnEntityPreDestroy.AddMember(this, &CRenderSystem::OnEntityPreDestroy);
		}

		bool sceneHasActiveCamera = false;
		const bool isInPlayingPlayState = World->GetWorldPlayState() == EWorldPlayState::Playing;

		// TODO.NR: Could probably merge all of these loops into one
		// NR: Not worth doing right now

		for (const SCameraComponent* cameraComponent : scene->GetComponents<SCameraComponent>())
		{
			if (!SComponent::IsValid(cameraComponent))
				continue;

			const STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(cameraComponent);

			if (cameraComponent->Owner == scene->MainCameraEntity)
			{
				sceneHasActiveCamera = true;

				SRenderCommand command;
				command.Type = ERenderCommandType::CameraDataStorage;
				command.Matrices.push_back(transformComponent->Transform.GetMatrix());
				command.Matrices.push_back(cameraComponent->ProjectionMatrix);
				RenderManager->PushRenderCommand(command);
			}
			else if (!isInPlayingPlayState)
			{
				GEngine::GetAssetRegistry()->RequestAsset(CameraWidgetReference.UID, transformComponent->Owner.GUID);
				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(CameraWidgetReference.UID, transformComponent, scene->GetComponent<STransformComponent>(scene->MainCameraEntity));

				SRenderCommand command;
				command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
				command.U32s.push_back(CameraWidgetReference.UID);
				RenderManager->PushRenderCommand(command);
			}
		}

		if (!sceneHasActiveCamera)
			return;

		for (const SStaticMeshComponent* staticMeshComponent : scene->GetComponents<SStaticMeshComponent>())
		{
			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(staticMeshComponent);
			SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(staticMeshComponent);

			if (!SComponent::IsValid(staticMeshComponent) || !SComponent::IsValid(transformComp) || !SComponent::IsValid(materialComp))
				continue;
		
			if (!RenderManager->IsStaticMeshInInstancedRenderList(staticMeshComponent->AssetReference.UID)) // if static, if instanced
			{		
				SStaticMeshAsset* asset = GEngine::GetAssetRegistry()->RequestAssetData<SStaticMeshAsset>(staticMeshComponent->AssetReference, staticMeshComponent->Owner.GUID);
				if (asset == nullptr)
					continue;

				for (const SDirectionalLightComponent* directionalLightComp : directionalLightComponents)
				{
					if (SComponent::IsValid(directionalLightComp) && directionalLightComp->IsActive)
					{
						// TODO.NW: Make these instanced calls? Also really need frustum culling
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassDirectional;
						command.ShadowmapViews.push_back(directionalLightComp->ShadowmapView);
						command.Matrices.push_back(transformComp->Transform.GetMatrix());
						command.U32s.push_back(staticMeshComponent->AssetReference.UID);
						command.DrawCallData = asset->DrawCallData;
						RenderManager->PushRenderCommand(command);
					}
				}

				for (const SPointLightComponent* pointLightComp : pointLightComponents)
				{
					if (SComponent::IsValid(pointLightComp) && pointLightComp->IsActive)
					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassPoint;
						command.Matrices.push_back(transformComp->Transform.GetMatrix());
						command.U32s.push_back(staticMeshComponent->AssetReference.UID);
						command.DrawCallData = asset->DrawCallData;
						command.SetShadowMapViews(pointLightComp->ShadowmapViews);
						RenderManager->PushRenderCommand(command);
					}
				}

				for (const SSpotLightComponent* spotLightComp : spotLightComponents)
				{
					if (SComponent::IsValid(spotLightComp) && spotLightComp->IsActive)
					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassSpot;
						command.Matrices.push_back(transformComp->Transform.GetMatrix());
						command.U32s.push_back(staticMeshComponent->AssetReference.UID);
						command.DrawCallData = asset->DrawCallData;
						command.ShadowmapViews.push_back(spotLightComp->ShadowmapView);
						RenderManager->PushRenderCommand(command);
					}
				}

				if (materialComp->AssetReferences.size() != asset->NumberOfMaterials)
					materialComp->AssetReferences.resize(asset->NumberOfMaterials, SAssetReference("Resources/M_MeshPreview.hva"));

				std::vector<SGraphicsMaterialAsset*> materialAssets = GEngine::GetAssetRegistry()->RequestAssetData<SGraphicsMaterialAsset>(materialComp->AssetReferences, materialComp->Owner.GUID);

				if (isInPlayingPlayState)
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferDataInstanced;
					command.Matrices.push_back(transformComp->Transform.GetMatrix());
					command.U32s.push_back(staticMeshComponent->AssetReference.UID);
					command.DrawCallData = asset->DrawCallData;

					for (SGraphicsMaterialAsset* materialAsset : materialAssets)
						command.Materials.push_back(materialAsset->Material);	
					
					RenderManager->PushRenderCommand(command);
				}
				else 
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferDataInstancedEditor;
					command.Matrices.push_back(transformComp->Transform.GetMatrix());
					command.U32s.push_back(staticMeshComponent->AssetReference.UID);
					command.DrawCallData = asset->DrawCallData;

					for (SGraphicsMaterialAsset* materialAsset : materialAssets)
						command.Materials.push_back(materialAsset->Material);
					
					RenderManager->PushRenderCommand(command);
				}
			}

			RenderManager->AddStaticMeshToInstancedRenderList(staticMeshComponent->AssetReference.UID, transformComp);
		}

		for (const SSkeletalMeshComponent* skeletalMeshComponent : scene->GetComponents<SSkeletalMeshComponent>())
		{
			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(skeletalMeshComponent);
			SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(skeletalMeshComponent);

			if (!SComponent::IsValid(skeletalMeshComponent) || !SComponent::IsValid(transformComp) || !SComponent::IsValid(materialComp))
				continue;

			if (!RenderManager->IsSkeletalMeshInInstancedRenderList(skeletalMeshComponent->AssetReference.UID))
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
				//		RenderManager->PushRenderCommand(command);
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
				//		RenderManager->PushRenderCommand(command);
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
				//		RenderManager->PushRenderCommand(command);
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

				if (isInPlayingPlayState)
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferSkeletalInstanced;
					command.Matrices.push_back(transformComp->Transform.GetMatrix());
					command.U32s.push_back(skeletalMeshComponent->AssetReference.UID);
					command.DrawCallData = asset->DrawCallData;

					for (SGraphicsMaterialAsset* materialAsset : materialAssets)
						command.Materials.push_back(materialAsset->Material);

					RenderManager->PushRenderCommand(command);
				}
				else
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferSkeletalInstancedEditor;
					command.Matrices.push_back(transformComp->Transform.GetMatrix());
					command.U32s.push_back(skeletalMeshComponent->AssetReference.UID);
					command.DrawCallData = asset->DrawCallData;
					
					for (SGraphicsMaterialAsset* materialAsset : materialAssets)
						command.Materials.push_back(materialAsset->Material);
					
					RenderManager->PushRenderCommand(command);
				}
			}

			RenderManager->AddSkeletalMeshToInstancedRenderList(skeletalMeshComponent->AssetReference.UID, transformComp, scene->GetComponent<SSkeletalAnimationComponent>(transformComp));		
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::DecalDepthCopy;
			RenderManager->PushRenderCommand(command);
		}
		
		for (const SDecalComponent* decalComponent : scene->GetComponents<SDecalComponent>())
		{
			if (!SComponent::IsValid(decalComponent))
				continue;

			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(decalComponent);
			GEngine::GetAssetRegistry()->RequestAssets(decalComponent->AssetReferences, transformComp->Owner.GUID);

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredDecal;
			command.Matrices.push_back(transformComp->Transform.GetMatrix());
			command.Flags.push_back(decalComponent->ShouldRenderAlbedo);
			command.Flags.push_back(decalComponent->ShouldRenderMaterial);
			command.Flags.push_back(decalComponent->ShouldRenderNormal);
			command.U32s = SAssetReference::GetIDs(decalComponent->AssetReferences);
			RenderManager->PushRenderCommand(command);

			// TODO.NW: This really should be living in the editor project, can we move it there to a new system?
			if (!isInPlayingPlayState)
			{
				GEngine::GetAssetRegistry()->RequestAsset(DecalWidgetReference.UID, transformComp->Owner.GUID);
				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(DecalWidgetReference.UID, transformComp, scene->GetComponent<STransformComponent>(scene->MainCameraEntity));

				command = {};
				command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
				command.U32s.push_back(DecalWidgetReference.UID);
				RenderManager->PushRenderCommand(command);
			}
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::PreLightingPass;
			RenderManager->PushRenderCommand(command);
		}

		for (const SEnvironmentLightComponent* environmentLightComp : scene->GetComponents<SEnvironmentLightComponent>())
		{
			if (!isInPlayingPlayState)
			{
				const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(environmentLightComp);
				GEngine::GetAssetRegistry()->RequestAsset(EnvironmentLightWidgetReference.UID, transformComp->Owner.GUID);
				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(EnvironmentLightWidgetReference.UID, transformComp, scene->GetComponent<STransformComponent>(scene->MainCameraEntity));

				SRenderCommand command;
				command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
				command.U32s.push_back(EnvironmentLightWidgetReference.UID);
				RenderManager->PushRenderCommand(command);
			}
		}

		for (const SDirectionalLightComponent* directionalLightComp : directionalLightComponents)
		{
			if (!SComponent::IsValid(directionalLightComp))
				continue;

			const SEntity& closestEnvironmentLightEntity = UComponentAlgo::GetClosestEntity3D(directionalLightComp->Owner, scene->GetComponents<SEnvironmentLightComponent>(), scene);
			const SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(closestEnvironmentLightEntity);
			if (!SComponent::IsValid(environmentLightComp))
				continue;

			GEngine::GetAssetRegistry()->RequestAsset(environmentLightComp->AssetReference, environmentLightComp->Owner.GUID);

			SRenderCommand command;
			if (directionalLightComp->IsActive)
			{
				command.Type = ERenderCommandType::DeferredLightingDirectional;
				command.U32s.push_back(environmentLightComp->AssetReference.UID);
				command.Vectors.push_back(directionalLightComp->Direction);
				command.Colors.push_back(directionalLightComp->Color);
				command.ShadowmapViews.push_back(directionalLightComp->ShadowmapView);
				RenderManager->PushRenderCommand(command);
			}

			if (const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(directionalLightComp))
			{
				if (volumetricLightComp->IsActive)
				{
					command.Type = ERenderCommandType::VolumetricLightingDirectional;
					command.SetVolumetricDataFromComponent(*volumetricLightComp);
					RenderManager->PushRenderCommand(command);
				}
			}

			if (!isInPlayingPlayState)
			{
				const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(directionalLightComp);
				GEngine::GetAssetRegistry()->RequestAsset(DirectionalLightWidgetReference.UID, transformComp->Owner.GUID);
				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(DirectionalLightWidgetReference.UID, transformComp, scene->GetComponent<STransformComponent>(scene->MainCameraEntity));

				command = {};
				command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
				command.U32s.push_back(DirectionalLightWidgetReference.UID);
				RenderManager->PushRenderCommand(command);
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
				RenderManager->PushRenderCommand(command);
			}

			if (const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(pointLightComp))
			{
				if (volumetricLightComp->IsActive)
				{
					command.Type = ERenderCommandType::VolumetricLightingPoint;
					command.SetVolumetricDataFromComponent(*volumetricLightComp);
					RenderManager->PushRenderCommand(command);
				}
			}

			if (!isInPlayingPlayState)
			{
				GEngine::GetAssetRegistry()->RequestAsset(PointLightWidgetReference.UID, transformComp->Owner.GUID);
				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(PointLightWidgetReference.UID, transformComp, scene->GetComponent<STransformComponent>(scene->MainCameraEntity));

				command = {};
				command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
				command.U32s.push_back(PointLightWidgetReference.UID);
				RenderManager->PushRenderCommand(command);
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
				RenderManager->PushRenderCommand(command);
			}

			if (const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(spotLightComp))
			{
				if (volumetricLightComp->IsActive)
				{
					command.Type = ERenderCommandType::VolumetricLightingSpot;
					command.SetVolumetricDataFromComponent(*volumetricLightComp);
					RenderManager->PushRenderCommand(command);
				}
			}

			if (!isInPlayingPlayState)
			{
				GEngine::GetAssetRegistry()->RequestAsset(SpotlightWidgetReference.UID, transformComp->Owner.GUID);
				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(SpotlightWidgetReference.UID, transformComp, scene->GetComponent<STransformComponent>(scene->MainCameraEntity));

				command = {};
				command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
				command.U32s.push_back(SpotlightWidgetReference.UID);
				RenderManager->PushRenderCommand(command);
			}
		}

		{
			const SEntity& closestEnvironmentLightEntity = UComponentAlgo::GetClosestEntity3D(scene->MainCameraEntity, scene->GetComponents<SEnvironmentLightComponent>(), scene);
			const SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(closestEnvironmentLightEntity);
			if (SComponent::IsValid(environmentLightComp))
			{
				SRenderCommand command;
				command.U32s.push_back(environmentLightComp->AssetReference.UID);
				command.Type = ERenderCommandType::Skybox;
				RenderManager->PushRenderCommand(command);
			}
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::PostBaseLightingPass;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::VolumetricBufferBlurPass;
			RenderManager->PushRenderCommand(command);
		}

		for (const SSpriteComponent* spriteComp : scene->GetComponents<SSpriteComponent>())
		{
			if (!SComponent::IsValid(spriteComp))
				continue;

			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(spriteComp);
			const STransform2DComponent* transform2DComp = scene->GetComponent<STransform2DComponent>(spriteComp);
			GEngine::GetAssetRegistry()->RequestAsset(spriteComp->AssetReference, spriteComp->Owner.GUID);

			if (SComponent::IsValid(transformComp))
			{
				if (!RenderManager->IsSpriteInWorldSpaceInstancedRenderList(spriteComp->AssetReference.UID))
				{
					// NR: Don't push a command every time
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferSpriteInstanced;
					command.U32s.push_back(spriteComp->AssetReference.UID);
					RenderManager->PushRenderCommand(command);
				}

				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(spriteComp->AssetReference.UID, transformComp, spriteComp);
			}
			else if (SComponent::IsValid(transform2DComp))
			{
				if (!RenderManager->IsSpriteInScreenSpaceInstancedRenderList(spriteComp->AssetReference.UID))
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::ScreenSpaceSprite;
					command.U32s.push_back(spriteComp->AssetReference.UID);
					RenderManager->PushRenderCommand(command);
				}

				RenderManager->AddSpriteToScreenSpaceInstancedRenderList(spriteComp->AssetReference.UID, transform2DComp, spriteComp);
			}
		}

		for (const SPhysics3DComponent* physics3DComponent : scene->GetComponents<SPhysics3DComponent>())
		{
			if (!SComponent::IsValid(physics3DComponent))
				continue;

			if (isInPlayingPlayState || !physics3DComponent->IsTrigger)
				continue;

			// TODO.NW: Render boundaries with line drawer?

			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(physics3DComponent);
			GEngine::GetAssetRegistry()->RequestAsset(ColliderWidgetReference.UID, transformComp->Owner.GUID);
			RenderManager->AddSpriteToWorldSpaceInstancedRenderList(ColliderWidgetReference.UID, transformComp, scene->GetComponent<STransformComponent>(scene->MainCameraEntity));

			SRenderCommand command;
			command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
			command.U32s.push_back(ColliderWidgetReference.UID);
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::Bloom;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::Tonemapping;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::AntiAliasing;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::GammaCorrection;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::RendererDebug;
			RenderManager->PushRenderCommand(command);
		}
	}

	void CRenderSystem::OnEntityPreDestroy(const SEntity entity)
	{
		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		assetRegistry->UnrequestAsset(CameraWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(ColliderWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(DecalWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(DirectionalLightWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(EnvironmentLightWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(PointLightWidgetReference, entity.GUID);
		assetRegistry->UnrequestAsset(SpotlightWidgetReference, entity.GUID);
	}
}
