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
	CRenderSystem::CRenderSystem(CRenderManager* renderManager, CWorld* world)
		: ISystem()
		, RenderManager(renderManager)
		, World(world)
	{
	}

	void CRenderSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		for (Ptr<CScene>& scene : scenes)
		{
			const std::vector<SDirectionalLightComponent*>& directionalLightComponents = scene->GetComponents<SDirectionalLightComponent>();
			const std::vector<SPointLightComponent*>& pointLightComponents = scene->GetComponents<SPointLightComponent>();
			const std::vector<SSpotLightComponent*>& spotLightComponents = scene->GetComponents<SSpotLightComponent>();

			RenderManager->ClearRenderViewInstanceData();

			const bool isInPlayingPlayState = World->GetWorldPlayState() == EWorldPlayState::Playing;

			// TODO.NR: Could probably merge all of these loops into one
			// NR: Not worth doing right now

			std::vector<SCameraComponent*> allCameras = scene->GetComponents<SCameraComponent>();
			std::vector<SCameraComponent*> activeCameras;
			for (auto& camera : allCameras)
			{
				if (camera->IsActive)
					activeCameras.emplace_back(camera);
			}

			U16 numberOfActiveCameras = STATIC_U16(activeCameras.size());
			RenderManager->PrepareRenderViews(numberOfActiveCameras);

			for (U16 i = STATIC_U16(0); i < numberOfActiveCameras; i++)
			{
				SCameraComponent* cameraComponent = activeCameras[i];
				SEntity& cameraEntity = cameraComponent->Owner;

				if (!SComponent::IsValid(cameraComponent) || !cameraComponent->IsActive)
					continue;

				const STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(cameraComponent);

				{
					SRenderCommand command;
					command.Type = ERenderCommandType::CameraDataStorage;
					command.Matrices.push_back(transformComponent->Transform.GetMatrix());
					command.Matrices.push_back(cameraComponent->ProjectionMatrix);
					RenderManager->PushRenderCommand(command, i);
				}

				for (const SStaticMeshComponent* staticMeshComponent : scene->GetComponents<SStaticMeshComponent>())
				{
					const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(staticMeshComponent);
					SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(staticMeshComponent);

					if (!SComponent::IsValid(staticMeshComponent) || !SComponent::IsValid(transformComp) || !SComponent::IsValid(materialComp))
						continue;

					if (!RenderManager->IsStaticMeshInInstancedRenderList(staticMeshComponent->AssetReference.UID, i)) // if static, if instanced
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
								RenderManager->PushRenderCommand(command, i);
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
								RenderManager->PushRenderCommand(command, i);
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
								RenderManager->PushRenderCommand(command, i);
							}
						}

						if (materialComp->AssetReferences.size() != asset->NumberOfMaterials)
							materialComp->AssetReferences.resize(asset->NumberOfMaterials, SAssetReference("Resources/M_MeshPreview.hva"));

						std::vector<SGraphicsMaterialAsset*> materialAssets = GEngine::GetAssetRegistry()->RequestAssetData<SGraphicsMaterialAsset>(materialComp->AssetReferences, materialComp->Owner.GUID);

						if (isInPlayingPlayState || i > 0)
						{
							SRenderCommand command;
							command.Type = ERenderCommandType::GBufferDataInstanced;
							command.U32s.push_back(staticMeshComponent->AssetReference.UID);
							command.DrawCallData = asset->DrawCallData;

							for (SGraphicsMaterialAsset* materialAsset : materialAssets)
								command.Materials.push_back(materialAsset->Material);

							RenderManager->PushRenderCommand(command, i);
						}
						else
						{
							SRenderCommand command;
							command.Type = ERenderCommandType::GBufferDataInstancedEditor;
							command.U32s.push_back(staticMeshComponent->AssetReference.UID);
							command.DrawCallData = asset->DrawCallData;

							for (SGraphicsMaterialAsset* materialAsset : materialAssets)
								command.Materials.push_back(materialAsset->Material);

							RenderManager->PushRenderCommand(command, i);
						}
					}

					RenderManager->AddStaticMeshToInstancedRenderList(staticMeshComponent->AssetReference.UID, transformComp, i);
				}

				for (const SSkeletalMeshComponent* skeletalMeshComponent : scene->GetComponents<SSkeletalMeshComponent>())
				{
					const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(skeletalMeshComponent);
					SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(skeletalMeshComponent);

					if (!SComponent::IsValid(skeletalMeshComponent) || !SComponent::IsValid(transformComp) || !SComponent::IsValid(materialComp))
						continue;

					if (!RenderManager->IsSkeletalMeshInInstancedRenderList(skeletalMeshComponent->AssetReference.UID, i))
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
						//		RenderManager->PushRenderCommand(command, i);
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
						//		RenderManager->PushRenderCommand(command, i);
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
						//		RenderManager->PushRenderCommand(command, i);
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

						if (isInPlayingPlayState || i > 0)
						{
							SRenderCommand command;
							command.Type = ERenderCommandType::GBufferSkeletalInstanced;
							command.Matrices.push_back(transformComp->Transform.GetMatrix());
							command.U32s.push_back(skeletalMeshComponent->AssetReference.UID);
							command.DrawCallData = asset->DrawCallData;

							for (SGraphicsMaterialAsset* materialAsset : materialAssets)
								command.Materials.push_back(materialAsset->Material);

							RenderManager->PushRenderCommand(command, i);
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

							RenderManager->PushRenderCommand(command, i);
						}
					}

					RenderManager->AddSkeletalMeshToInstancedRenderList(skeletalMeshComponent->AssetReference.UID, transformComp, scene->GetComponent<SSkeletalAnimationComponent>(transformComp), i);
				}

				{
					SRenderCommand command;
					command.Type = ERenderCommandType::DecalDepthCopy;
					RenderManager->PushRenderCommand(command, i);
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
					RenderManager->PushRenderCommand(command, i);
				}

				{
					SRenderCommand command;
					command.Type = ERenderCommandType::PreLightingPass;
					RenderManager->PushRenderCommand(command, i);
				}

				for (const SDirectionalLightComponent* directionalLightComp : directionalLightComponents)
				{
					if (!SComponent::IsValid(directionalLightComp))
						continue;

					const SEntity& closestEnvironmentLightEntity = UComponentAlgo::GetClosestEntity3D(directionalLightComp->Owner, scene->GetComponents<SEnvironmentLightComponent>(), scene.get());
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
						RenderManager->PushRenderCommand(command, i);
					}

					if (const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(directionalLightComp))
					{
						if (volumetricLightComp->IsActive)
						{
							command.Type = ERenderCommandType::VolumetricLightingDirectional;
							command.SetVolumetricDataFromComponent(*volumetricLightComp);
							RenderManager->PushRenderCommand(command, i);
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
						RenderManager->PushRenderCommand(command, i);
					}

					if (const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(pointLightComp))
					{
						if (volumetricLightComp->IsActive)
						{
							command.Type = ERenderCommandType::VolumetricLightingPoint;
							command.SetVolumetricDataFromComponent(*volumetricLightComp);
							RenderManager->PushRenderCommand(command, i);
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
						RenderManager->PushRenderCommand(command, i);
					}

					if (const SVolumetricLightComponent* volumetricLightComp = scene->GetComponent<SVolumetricLightComponent>(spotLightComp))
					{
						if (volumetricLightComp->IsActive)
						{
							command.Type = ERenderCommandType::VolumetricLightingSpot;
							command.SetVolumetricDataFromComponent(*volumetricLightComp);
							RenderManager->PushRenderCommand(command, i);
						}
					}
				}

				{
					const SEntity& closestEnvironmentLightEntity = UComponentAlgo::GetClosestEntity3D(cameraEntity, scene->GetComponents<SEnvironmentLightComponent>(), scene.get());
					const SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(closestEnvironmentLightEntity);
					if (SComponent::IsValid(environmentLightComp))
					{
						SRenderCommand command;
						command.U32s.push_back(environmentLightComp->AssetReference.UID);
						command.Type = ERenderCommandType::Skybox;
						RenderManager->PushRenderCommand(command, i);
					}
				}

				{
					SRenderCommand command;
					command.Type = ERenderCommandType::PostBaseLightingPass;
					RenderManager->PushRenderCommand(command, i);
				}

				{
					SRenderCommand command;
					command.Type = ERenderCommandType::VolumetricBufferBlurPass;
					RenderManager->PushRenderCommand(command, i);
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
						if (!RenderManager->IsSpriteInWorldSpaceInstancedRenderList(spriteComp->AssetReference.UID, i))
						{
							// NR: Don't push a command every time
							SRenderCommand command;
							command.Type = ERenderCommandType::GBufferSpriteInstanced;
							command.U32s.push_back(spriteComp->AssetReference.UID);
							RenderManager->PushRenderCommand(command, i);
						}

						RenderManager->AddSpriteToWorldSpaceInstancedRenderList(spriteComp->AssetReference.UID, transformComp, spriteComp, i);
					}
					else if (SComponent::IsValid(transform2DComp))
					{
						if (!RenderManager->IsSpriteInScreenSpaceInstancedRenderList(spriteComp->AssetReference.UID, i))
						{
							SRenderCommand command;
							command.Type = ERenderCommandType::ScreenSpaceSprite;
							command.U32s.push_back(spriteComp->AssetReference.UID);
							RenderManager->PushRenderCommand(command, i);
						}

						RenderManager->AddSpriteToScreenSpaceInstancedRenderList(spriteComp->AssetReference.UID, transform2DComp, spriteComp, i);
					}
				}

				{
					SRenderCommand command;
					command.Type = ERenderCommandType::Bloom;
					RenderManager->PushRenderCommand(command, i);
				}

				{
					SRenderCommand command;
					command.Type = ERenderCommandType::Tonemapping;
					RenderManager->PushRenderCommand(command, i);
				}

				{
					SRenderCommand command;
					command.Type = ERenderCommandType::AntiAliasing;
					RenderManager->PushRenderCommand(command, i);
				}

				{
					SRenderCommand command;
					command.Type = ERenderCommandType::GammaCorrection;
					RenderManager->PushRenderCommand(command, i);
				}

				{
					SRenderCommand command;
					command.Type = ERenderCommandType::RendererDebug;
					RenderManager->PushRenderCommand(command, i);
				}
			}
		}
	}
}
