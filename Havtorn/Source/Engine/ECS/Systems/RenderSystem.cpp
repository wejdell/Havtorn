// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "RenderSystem.h"
#include "Scene/World.h"
#include "Scene/Scene.h"
#include "ECS/ECSInclude.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderCommand.h"
#include "ECS/ComponentAlgo.h"
#include "Input/Input.h"

namespace Havtorn
{
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
		RenderManager->ClearSystemWorldSpaceSpriteInstanceData();
		RenderManager->ClearSystemScreenSpaceSpriteInstanceData();

		bool sceneHasActiveCamera = false;
		const bool isInPlayingPlayState = World->GetWorldPlayState() == EWorldPlayState::Playing;

		// TODO.NR: Could probably merge all of these loops into one
		// NR: Not worth doing right now

		for (const SCameraComponent* cameraComponent : scene->GetComponents<SCameraComponent>())
		{
			if (!cameraComponent->IsValid())
				continue;

			sceneHasActiveCamera = true;

			const STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(cameraComponent);

			SRenderCommand command;
			command.Type = ERenderCommandType::CameraDataStorage;
			command.Matrices.push_back(transformComponent->Transform.GetMatrix());
			command.Matrices.push_back(cameraComponent->ProjectionMatrix);
			RenderManager->PushRenderCommand(command);
		}

		if (!sceneHasActiveCamera)
			return;

		for (const SStaticMeshComponent* staticMeshComponent : scene->GetComponents<SStaticMeshComponent>())
		{
			if (!staticMeshComponent->IsValid())
				continue;
		
			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(staticMeshComponent);
			const SMaterialComponent* materialComp = scene->GetComponent<SMaterialComponent>(staticMeshComponent);

			if (!RenderManager->IsStaticMeshInInstancedRenderList(staticMeshComponent->Name.AsString())) // if static, if instanced
			{		
				for (const SDirectionalLightComponent* directionalLightComp : directionalLightComponents)
				{
					if (directionalLightComp->IsValid())
					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassDirectional;
						command.ShadowmapViews.push_back(directionalLightComp->ShadowmapView);
						command.Matrices.push_back(transformComp->Transform.GetMatrix());
						command.Strings.push_back(staticMeshComponent->Name.AsString());
						command.U8s.push_back(staticMeshComponent->TopologyIndex);
						command.DrawCallData = staticMeshComponent->DrawCallData;
						RenderManager->PushRenderCommand(command);
					}
				}

				for (const SPointLightComponent* pointLightComp : pointLightComponents)
				{
					if (pointLightComp->IsValid())
					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassPoint;
						command.Matrices.push_back(transformComp->Transform.GetMatrix());
						command.Strings.push_back(staticMeshComponent->Name.AsString());
						command.U8s.push_back(staticMeshComponent->TopologyIndex);
						command.DrawCallData = staticMeshComponent->DrawCallData;
						command.SetShadowMapViews(pointLightComp->ShadowmapViews);
						RenderManager->PushRenderCommand(command);
					}
				}

				for (const SSpotLightComponent* spotLightComp : spotLightComponents)
				{
					if (spotLightComp->IsValid())
					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassSpot;
						command.Matrices.push_back(transformComp->Transform.GetMatrix());
						command.Strings.push_back(staticMeshComponent->Name.AsString());
						command.U8s.push_back(staticMeshComponent->TopologyIndex);
						command.DrawCallData = staticMeshComponent->DrawCallData;
						command.ShadowmapViews.push_back(spotLightComp->ShadowmapView);
						RenderManager->PushRenderCommand(command);
					}
				}

				if (isInPlayingPlayState)
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferDataInstanced;
					command.Matrices.push_back(transformComp->Transform.GetMatrix());
					command.Strings.push_back(staticMeshComponent->Name.AsString());
					command.U8s.push_back(staticMeshComponent->TopologyIndex);
					command.U8s.push_back(staticMeshComponent->PixelShaderIndex);
					command.U8s.push_back(staticMeshComponent->SamplerIndex);
					command.DrawCallData = staticMeshComponent->DrawCallData;
					command.Materials = materialComp->Materials;
					RenderManager->PushRenderCommand(command);
				}
				else 
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferDataInstancedEditor;
					command.Matrices.push_back(transformComp->Transform.GetMatrix());
					command.Strings.push_back(staticMeshComponent->Name.AsString());
					command.U8s.push_back(staticMeshComponent->TopologyIndex);
					command.U8s.push_back(staticMeshComponent->PixelShaderIndex);
					command.U8s.push_back(staticMeshComponent->SamplerIndex);
					command.DrawCallData = staticMeshComponent->DrawCallData;
					command.Materials = materialComp->Materials;
					RenderManager->PushRenderCommand(command);
				}
			}

			RenderManager->AddStaticMeshToInstancedRenderList(staticMeshComponent->Name.AsString(), transformComp);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::DecalDepthCopy;
			RenderManager->PushRenderCommand(command);
		}
		
		for (const SDecalComponent* decalComponent : scene->GetComponents<SDecalComponent>())
		{
			if (!decalComponent->IsValid())
				continue;

			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(decalComponent);

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredDecal;
			command.Matrices.push_back(transformComp->Transform.GetMatrix());
			command.Flags.push_back(decalComponent->ShouldRenderAlbedo);
			command.Flags.push_back(decalComponent->ShouldRenderMaterial);
			command.Flags.push_back(decalComponent->ShouldRenderNormal);
			command.U16s = decalComponent->TextureReferences;
			RenderManager->PushRenderCommand(command);

			if (!isInPlayingPlayState)
			{
				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(decalComponent->EditorTextureIndex, transformComp, scene->GetComponent<STransformComponent>(scene->MainCameraEntity));

				command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
				command.U32s.push_back(decalComponent->EditorTextureIndex);
				RenderManager->PushRenderCommand(command);
			}
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::PreLightingPass;
			RenderManager->PushRenderCommand(command);
		}

		for (const SDirectionalLightComponent* directionalLightComp : directionalLightComponents)
		{
			if (!directionalLightComp->IsValid())
				continue;

			const SEntity& closestEnvironmentLightEntity = UComponentAlgo::GetClosestEntity3D(directionalLightComp->Owner, scene->GetComponents<SEnvironmentLightComponent>(), scene);
			const SEnvironmentLightComponent* environmentLightComp = scene->GetComponent<SEnvironmentLightComponent>(closestEnvironmentLightEntity);
			if (!environmentLightComp->IsValid())
				continue;

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredLightingDirectional;
			command.U16s.push_back(environmentLightComp->AmbientCubemapReference);
			command.Vectors.push_back(directionalLightComp->Direction);
			command.Colors.push_back(directionalLightComp->Color);
			command.ShadowmapViews.push_back(directionalLightComp->ShadowmapView);
			RenderManager->PushRenderCommand(command);

			if (!isInPlayingPlayState)
			{
				const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(directionalLightComp);
				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(directionalLightComp->EditorTextureIndex, transformComp, scene->GetComponent<STransformComponent>(scene->MainCameraEntity));

				command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
				command.U32s.push_back(directionalLightComp->EditorTextureIndex);
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
		}

		for (const SPointLightComponent* pointLightComp : pointLightComponents)
		{
			if (!pointLightComp->IsValid())
				continue;

			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(pointLightComp);

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredLightingPoint;
			command.Matrices.push_back(transformComp->Transform.GetMatrix());
			command.Colors.push_back(SColor(pointLightComp->ColorAndIntensity.X, pointLightComp->ColorAndIntensity.Y, pointLightComp->ColorAndIntensity.Z, 1.0f));
			command.F32s.push_back(pointLightComp->ColorAndIntensity.W);
			command.F32s.push_back(pointLightComp->Range);
			command.SetShadowMapViews(pointLightComp->ShadowmapViews);
			RenderManager->PushRenderCommand(command);

			if (!isInPlayingPlayState)
			{
				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(pointLightComp->EditorTextureIndex, transformComp, scene->GetComponent<STransformComponent>(scene->MainCameraEntity));

				command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
				command.U32s.push_back(pointLightComp->EditorTextureIndex);
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
		}

		for (const SSpotLightComponent* spotLightComp : spotLightComponents)
		{
			if (!spotLightComp->IsValid())
				continue;

			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(spotLightComp);

			SRenderCommand command;
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

			if (!isInPlayingPlayState)
			{
				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(spotLightComp->EditorTextureIndex, transformComp, scene->GetComponent<STransformComponent>(scene->MainCameraEntity));

				command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
				command.U32s.push_back(spotLightComp->EditorTextureIndex);
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
			if (!spriteComp->IsValid())
				continue;

			const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(spriteComp);
			const STransform2DComponent* transform2DComp = scene->GetComponent<STransform2DComponent>(spriteComp);

			if (transformComp->IsValid())
			{
				if (!RenderManager->IsSpriteInWorldSpaceInstancedRenderList(spriteComp->TextureIndex)) 
				{
					// NR: Don't push a command every time
					SRenderCommand command;
					command.Type = ERenderCommandType::GBufferSpriteInstanced;
					command.U32s.push_back(spriteComp->TextureIndex);
					RenderManager->PushRenderCommand(command);
				}

				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(spriteComp->TextureIndex, transformComp, spriteComp);
			}
			else if (transform2DComp->IsValid())
			{
				if (!RenderManager->IsSpriteInScreenSpaceInstancedRenderList(spriteComp->TextureIndex))
				{
					SRenderCommand command;
					command.Type = ERenderCommandType::ScreenSpaceSprite;
					command.U32s.push_back(spriteComp->TextureIndex);
					RenderManager->PushRenderCommand(command);
				}

				RenderManager->AddSpriteToScreenSpaceInstancedRenderList(spriteComp->TextureIndex, transform2DComp, spriteComp);
			}
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
}
