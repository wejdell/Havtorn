// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "RenderSystem.h"
#include "Scene/Scene.h"
#include "ECS/ECSInclude.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderCommand.h"
#include "Input/Input.h"

namespace Havtorn
{
	CRenderSystem::CRenderSystem(CRenderManager* renderManager)
		: ISystem()
		, RenderManager(renderManager)
	{
	}

	void CRenderSystem::Update(CScene* scene)
	{
		std::vector<SStaticMeshComponent>& staticMeshComponents = scene->GetStaticMeshComponents();
		std::vector<STransformComponent>& transformComponents = scene->GetTransformComponents();
		std::vector<SCameraComponent>& cameraComponents = scene->GetCameraComponents();
		std::vector<SMaterialComponent>& materialComponents = scene->GetMaterialComponents();
		std::vector<SEnvironmentLightComponent>& environmentLightComponents = scene->GetEnvironmentLightComponents();
		std::vector<SDirectionalLightComponent>& directionalLightComponents = scene->GetDirectionalLightComponents();
		std::vector<SPointLightComponent>& pointLightComponents = scene->GetPointLightComponents();
		std::vector<SSpotLightComponent>& spotLightComponents = scene->GetSpotLightComponents();

		// Unused right now
		//const auto& volumetricLightComponents = scene->GetVolumetricLightComponents();

		const auto& decalComponents = scene->GetDecalComponents();

		RenderManager->ClearSystemStaticMeshInstanceTransforms();

		bool sceneHasActiveCamera = false;

		for (U64 i = 0; i < cameraComponents.size(); i++)
		{
			const SCameraComponent& cameraComponent = cameraComponents[i];
			if (!cameraComponent.IsInUse)
				continue;

			sceneHasActiveCamera = true;

			SRenderCommand command;
			command.Type = ERenderCommandType::CameraDataStorage;
			command.TransformComponent = transformComponents[i];
			command.CameraComponent = cameraComponents[i];
			RenderManager->PushRenderCommand(command);
		}

		if (!sceneHasActiveCamera)
			return;

		for (U64 i = 0; i < staticMeshComponents.size(); i++)
		{
			const SStaticMeshComponent& staticMeshComponent = staticMeshComponents[i];
			if (!staticMeshComponent.IsInUse)
				continue;
		
			const I64 transformCompIndex = i;
			const STransformComponent& transformComp = transformComponents[transformCompIndex];

			const I64 materialCompIndex = i;
			auto& materialComp = materialComponents[materialCompIndex];

			if (!RenderManager->IsStaticMeshInInstancedRenderList(staticMeshComponent.Name.AsString())) // if static, if instanced
			{		
				for (U64 j = 0; j < staticMeshComponents.size(); j++)
				{
					if (directionalLightComponents[j].IsInUse)
					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassDirectional;
						command.TransformComponent = transformComp;
						command.StaticMeshComponent = staticMeshComponent;
						command.DirectionalLightComponent = directionalLightComponents[j];
						RenderManager->PushRenderCommand(command);
					}

					if (pointLightComponents[j].IsInUse)
					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassPoint;
						command.TransformComponent = transformComp;
						command.StaticMeshComponent = staticMeshComponent;
						command.PointLightComponent = pointLightComponents[j];
						RenderManager->PushRenderCommand(command);
					}

					if (spotLightComponents[j].IsInUse)
					{
						SRenderCommand command;
						command.Type = ERenderCommandType::ShadowAtlasPrePassSpot;
						command.TransformComponent = transformComp;
						command.StaticMeshComponent = staticMeshComponent;
						command.SpotLightComponent = spotLightComponents[j];
						RenderManager->PushRenderCommand(command);
					}
				}

				SRenderCommand command;
				command.Type = ERenderCommandType::GBufferDataInstanced;
				command.TransformComponent = transformComp;
				command.StaticMeshComponent = staticMeshComponent;
				command.MaterialComponent = materialComp;
				RenderManager->PushRenderCommand(command);
			}

			RenderManager->AddStaticMeshToInstancedRenderList(staticMeshComponent.Name.AsString(), transformComp.Transform.GetMatrix());
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::DecalDepthCopy;
			RenderManager->PushRenderCommand(command);
		}
		
		for (U64 i = 0; i < decalComponents.size(); i++)
		{
			const SDecalComponent& decalComponent = decalComponents[i];
			if (!decalComponent.IsInUse)
				continue;

			const STransformComponent& transformComp = transformComponents[i];

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredDecal;
			command.TransformComponent = transformComp;
			command.DecalComponent = decalComponent;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::PreLightingPass;
			RenderManager->PushRenderCommand(command);
		}

		for (U64 i = 0; i < directionalLightComponents.size(); i++)
		{
			const SDirectionalLightComponent& directionalLightComp = directionalLightComponents[i];
			if (!directionalLightComp.IsInUse)
				continue;

			// TODO: Fix hard coded index, maybe store index of environmentlight entity that the light source should use/closest?
			const SEnvironmentLightComponent& environmentLightComp = environmentLightComponents[1];
			if (!environmentLightComp.IsInUse)
				continue;

			const STransformComponent& transformComp = transformComponents[i];

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredLightingDirectional;
			command.TransformComponent = transformComp;
			command.EnvironmentLightComponent = environmentLightComp;
			command.DirectionalLightComponent = directionalLightComp;
			RenderManager->PushRenderCommand(command);

			//if (directionalLightComponents[0]->Entity->HasComponent(EComponentType::VolumetricLightComponent))
			//{
			//	const I64 volumetricCompIndex = directionalLightComponents[0]->Entity->GetComponentIndex(EComponentType::VolumetricLightComponent);
			//	auto& volumetricLightComp = volumetricLightComponents[volumetricCompIndex];

			//	if (volumetricLightComp->IsActive)
			//	{
			//		components[static_cast<U8>(EComponentType::VolumetricLightComponent)] = volumetricLightComp;
			//		SRenderCommand volumetricCommand(components, ERenderCommandType::VolumetricLightingDirectional);
			//		RenderManager->PushRenderCommand(volumetricCommand);
			//	}
			//}
		}

		// TODO: Fix ECS View functionality, going to be a mess to keep hard coded indices
		
		for (U64 i = 0; i < pointLightComponents.size(); i++)
		{
			const SPointLightComponent& pointLightComp = pointLightComponents[i];
			if (!pointLightComp.IsInUse)
				continue;

			const STransformComponent& transformComp = transformComponents[i];

			SRenderCommand command;
			command.TransformComponent = transformComp;
			command.PointLightComponent = pointLightComp;
			command.Type = ERenderCommandType::DeferredLightingPoint;
			RenderManager->PushRenderCommand(command);

			//if (pointLightComponents[0]->Entity->HasComponent(EComponentType::VolumetricLightComponent))
			//{
			//	const I64 volumetricCompIndex = pointLightComponents[0]->Entity->GetComponentIndex(EComponentType::VolumetricLightComponent);
			//	auto& volumetricLightComp = volumetricLightComponents[volumetricCompIndex];

			//	if (volumetricLightComp->IsActive)
			//	{
			//		components[static_cast<U8>(EComponentType::VolumetricLightComponent)] = volumetricLightComp;
			//		SRenderCommand volumetricCommand(components, ERenderCommandType::VolumetricLightingPoint);
			//		RenderManager->PushRenderCommand(volumetricCommand);
			//	}
			//}
		}

		for (U64 i = 0; i < spotLightComponents.size(); i++)
		{
			const SSpotLightComponent& spotLightComp = spotLightComponents[i];
			if (!spotLightComp.IsInUse)
				continue;

			const STransformComponent& transformComp = transformComponents[i];

			SRenderCommand command;
			command.TransformComponent = transformComp;
			command.SpotLightComponent = spotLightComp;
			command.Type = ERenderCommandType::DeferredLightingSpot;
			RenderManager->PushRenderCommand(command);

			//if (spotLightComponents[0]->Entity->HasComponent(EComponentType::VolumetricLightComponent))
			//{
			//	const I64 volumetricCompIndex = spotLightComponents[0]->Entity->GetComponentIndex(EComponentType::VolumetricLightComponent);
			//	auto& volumetricLightComp = volumetricLightComponents[volumetricCompIndex];

			//	if (volumetricLightComp->IsActive)
			//	{
			//		components[static_cast<U8>(EComponentType::VolumetricLightComponent)] = volumetricLightComp;
			//		SRenderCommand volumetricCommand(components, ERenderCommandType::VolumetricLightingSpot);
			//		RenderManager->PushRenderCommand(volumetricCommand);
			//	}
			//}
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::VolumetricBufferBlurPass;
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
}
