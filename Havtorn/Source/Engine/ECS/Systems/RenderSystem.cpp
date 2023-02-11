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

			const I64 transformCompIndex = i;
			//const STransformComponent& transformComp = transformComponents[transformCompIndex];

			// Figure out why the memory is suddenly invalidated here after deserialization? Can't even preview the memory. Happens randomly.

			//STransformComponent& transformComp = transformComponents[i];

			//SComponentArray components;
			//components[static_cast<U8>(EComponentType::TransformComponent)] = &transformComp;
			//components[static_cast<U8>(EComponentType::CameraComponent)] = &cameraComponent;
			//components[static_cast<U8>(EComponentType::TransformComponent)] = &transformComponents[i];
			//components[static_cast<U8>(EComponentType::CameraComponent)] = &cameraComponents[i];
			//SRenderCommand command(components, ERenderCommandType::CameraDataStorage);
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
			//if (!staticMeshComponent->Entity->HasComponent(EComponentType::TransformComponent))
			//	continue;

			//if (!staticMeshComponent->Entity->HasComponent(EComponentType::MaterialComponent))
			//	continue;

			const SStaticMeshComponent& staticMeshComponent = staticMeshComponents[i];
			if (!staticMeshComponent.IsInUse)
				continue;

			const I64 transformCompIndex = i;
			const STransformComponent& transformComp = transformComponents[transformCompIndex];

			const I64 materialCompIndex = i;
			auto& materialComp = materialComponents[materialCompIndex];

			if (!RenderManager->IsStaticMeshInInstancedRenderList(staticMeshComponent.Name.AsString())) // if static, if instanced
			{		
				if (directionalLightComponents[i].IsInUse)
				{
					//SComponentArray components;
					//components[static_cast<U8>(EComponentType::TransformComponent)] = &transformComp;
					//components[static_cast<U8>(EComponentType::StaticMeshComponent)] = &staticMeshComponent;
					//components[static_cast<U8>(EComponentType::DirectionalLightComponent)] = &directionalLightComponents[i];
					//SRenderCommand command(components, ERenderCommandType::ShadowAtlasPrePassDirectional);
					SRenderCommand command;
					command.Type = ERenderCommandType::ShadowAtlasPrePassDirectional;
					command.TransformComponent = transformComp;
					command.StaticMeshComponent = staticMeshComponent;
					command.DirectionalLightComponent = directionalLightComponents[i];
					RenderManager->PushRenderCommand(command);
				}

				if (pointLightComponents[i].IsInUse)
				{
					//SComponentArray components;
					//components[static_cast<U8>(EComponentType::TransformComponent)] = &transformComp;
					//components[static_cast<U8>(EComponentType::StaticMeshComponent)] = &staticMeshComponent;
					//components[static_cast<U8>(EComponentType::PointLightComponent)] = &pointLightComponents[i];
					//SRenderCommand command(components, ERenderCommandType::ShadowAtlasPrePassPoint);
					SRenderCommand command;
					command.Type = ERenderCommandType::ShadowAtlasPrePassPoint;
					command.TransformComponent = transformComp;
					command.StaticMeshComponent = staticMeshComponent;
					command.PointLightComponent = pointLightComponents[i];
					RenderManager->PushRenderCommand(command);
				}

				if (spotLightComponents[i].IsInUse)
				{
					//SComponentArray components;
					//components[static_cast<U8>(EComponentType::TransformComponent)] = &transformComp;
					//components[static_cast<U8>(EComponentType::StaticMeshComponent)] = &staticMeshComponent;
					//components[static_cast<U8>(EComponentType::SpotLightComponent)] = &spotLightComponents[i];
					//SRenderCommand command(components, ERenderCommandType::ShadowAtlasPrePassSpot);
					SRenderCommand command;
					command.Type = ERenderCommandType::ShadowAtlasPrePassSpot;
					command.TransformComponent = transformComp;
					command.StaticMeshComponent = staticMeshComponent;
					command.SpotLightComponent = spotLightComponents[i];
					RenderManager->PushRenderCommand(command);
				}

				//SComponentArray components;
				//components[static_cast<U8>(EComponentType::TransformComponent)] = &transformComp;
				//components[static_cast<U8>(EComponentType::StaticMeshComponent)] = &staticMeshComponent;
				//components[static_cast<U8>(EComponentType::MaterialComponent)] = &materialComp;
				//SRenderCommand command(components, ERenderCommandType::GBufferDataInstanced);
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
			//SRenderCommand command(SComponentArray{}, ERenderCommandType::DecalDepthCopy);
			SRenderCommand command;
			command.Type = ERenderCommandType::DecalDepthCopy;
			RenderManager->PushRenderCommand(command);
		}
		
		for (U64 i = 0; i < decalComponents.size(); i++)
		{
			//if (!decalComponent->Entity->HasComponent(EComponentType::TransformComponent))
			//	continue;

			const SDecalComponent& decalComponent = decalComponents[i];
			if (!decalComponent.IsInUse)
				continue;

			//const I64 transformCompIndex = decalComponent->Entity->GetComponentIndex(EComponentType::TransformComponent);
			const I64 transformCompIndex = i;
			const STransformComponent& transformComp = transformComponents[transformCompIndex];

			//SComponentArray components;
			//components[static_cast<U8>(EComponentType::TransformComponent)] = &transformComp;
			//components[static_cast<U8>(EComponentType::DecalComponent)] = &decalComponent;
			//SRenderCommand command(components, ERenderCommandType::DeferredDecal);
			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredDecal;
			command.TransformComponent = transformComp;
			command.DecalComponent = decalComponent;
			RenderManager->PushRenderCommand(command);
		}

		{
			//SRenderCommand command(SComponentArray{}, ERenderCommandType::PreLightingPass);
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

			//SComponentArray components;
			//components[static_cast<U8>(EComponentType::TransformComponent)] = &transformComp;
			//components[static_cast<U8>(EComponentType::EnvironmentLightComponent)] = &environmentLightComp;
			//components[static_cast<U8>(EComponentType::DirectionalLightComponent)] = &directionalLightComp;
			//SRenderCommand command(components, ERenderCommandType::DeferredLightingDirectional);
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
		
		//if (!pointLightComponents.empty())
		//{
		//	const I64 transformCompIndex = pointLightComponents[0]->Entity->GetComponentIndex(EComponentType::TransformComponent);
		//	auto& transformComp = transformComponents[transformCompIndex];

		//	std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
		//	components[static_cast<U8>(EComponentType::TransformComponent)] = transformComp;
		//	components[static_cast<U8>(EComponentType::PointLightComponent)] = pointLightComponents[0];
		//	SRenderCommand command(components, ERenderCommandType::DeferredLightingPoint);
		//	RenderManager->PushRenderCommand(command);

		//	if (pointLightComponents[0]->Entity->HasComponent(EComponentType::VolumetricLightComponent))
		//	{
		//		const I64 volumetricCompIndex = pointLightComponents[0]->Entity->GetComponentIndex(EComponentType::VolumetricLightComponent);
		//		auto& volumetricLightComp = volumetricLightComponents[volumetricCompIndex];

		//		if (volumetricLightComp->IsActive)
		//		{
		//			components[static_cast<U8>(EComponentType::VolumetricLightComponent)] = volumetricLightComp;
		//			SRenderCommand volumetricCommand(components, ERenderCommandType::VolumetricLightingPoint);
		//			RenderManager->PushRenderCommand(volumetricCommand);
		//		}
		//	}
		//}

		//if (!spotLightComponents.empty())
		//{
		//	const I64 transformCompIndex = spotLightComponents[0]->Entity->GetComponentIndex(EComponentType::TransformComponent);
		//	auto& transformComp = transformComponents[transformCompIndex];

		//	std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
		//	components[static_cast<U8>(EComponentType::TransformComponent)] = transformComp;
		//	components[static_cast<U8>(EComponentType::SpotLightComponent)] = spotLightComponents[0];
		//	SRenderCommand command(components, ERenderCommandType::DeferredLightingSpot);
		//	RenderManager->PushRenderCommand(command);

		//	if (spotLightComponents[0]->Entity->HasComponent(EComponentType::VolumetricLightComponent))
		//	{
		//		const I64 volumetricCompIndex = spotLightComponents[0]->Entity->GetComponentIndex(EComponentType::VolumetricLightComponent);
		//		auto& volumetricLightComp = volumetricLightComponents[volumetricCompIndex];

		//		if (volumetricLightComp->IsActive)
		//		{
		//			components[static_cast<U8>(EComponentType::VolumetricLightComponent)] = volumetricLightComp;
		//			SRenderCommand volumetricCommand(components, ERenderCommandType::VolumetricLightingSpot);
		//			RenderManager->PushRenderCommand(volumetricCommand);
		//		}
		//	}
		//}

		{
			//SRenderCommand command(SComponentArray{}, ERenderCommandType::VolumetricBufferBlurPass);
			SRenderCommand command;
			command.Type = ERenderCommandType::VolumetricBufferBlurPass;
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::Bloom;
			//SRenderCommand command(SComponentArray{}, ERenderCommandType::Bloom);
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::Tonemapping;
			//SRenderCommand command(SComponentArray{}, ERenderCommandType::Tonemapping);
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::AntiAliasing;
			//SRenderCommand command(SComponentArray{}, ERenderCommandType::AntiAliasing);
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::GammaCorrection;
			//SRenderCommand command(SComponentArray{}, ERenderCommandType::GammaCorrection);
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command;
			command.Type = ERenderCommandType::RendererDebug;
			//SRenderCommand command(SComponentArray{}, ERenderCommandType::RendererDebug);
			RenderManager->PushRenderCommand(command);
		}
	}
}
