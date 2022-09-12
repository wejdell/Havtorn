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
		const auto& staticMeshComponents = scene->GetStaticMeshComponents();
		const auto& transformComponents = scene->GetTransformComponents();
		const auto& cameraComponents = scene->GetCameraComponents();
		const auto& materialComponents = scene->GetMaterialComponents();
		const auto& environmentLightComponents = scene->GetEnvironmentLightComponents();
		const auto& directionalLightComponents = scene->GetDirectionalLightComponents();
		const auto& pointLightComponents = scene->GetPointLightComponents();
		const auto& spotLightComponents = scene->GetSpotLightComponents();
		const auto& volumetricLightComponents = scene->GetVolumetricLightComponents();
		const auto& decalComponents = scene->GetDecalComponents();

		RenderManager->ClearSystemStaticMeshInstanceTransforms();

		if (!cameraComponents.empty())
		{
			const I64 transformCompIndex = cameraComponents[0]->Entity->GetComponentIndex(EComponentType::TransformComponent);
			auto& transformComp = transformComponents[transformCompIndex];

			std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
			components[static_cast<U8>(EComponentType::TransformComponent)] = transformComp;
			components[static_cast<U8>(EComponentType::CameraComponent)] = cameraComponents[0];
			SRenderCommand command(components, ERenderCommandType::CameraDataStorage);
			RenderManager->PushRenderCommand(command);
		}
		else
			return;

		for (auto& staticMeshComponent : staticMeshComponents)
		{
			if (!staticMeshComponent->Entity->HasComponent(EComponentType::TransformComponent))
				continue;

			if (!staticMeshComponent->Entity->HasComponent(EComponentType::MaterialComponent))
				continue;

			const I64 transformCompIndex = staticMeshComponent->Entity->GetComponentIndex(EComponentType::TransformComponent);
			auto& transformComp = transformComponents[transformCompIndex];

			const I64 materialCompIndex = staticMeshComponent->Entity->GetComponentIndex(EComponentType::MaterialComponent);
			auto& materialComp = materialComponents[materialCompIndex];

			if (!RenderManager->IsStaticMeshInInstancedRenderList(staticMeshComponent->Name)) // if static, if instanced
			{		
				if (!directionalLightComponents.empty())
				{
					std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
					components[static_cast<U8>(EComponentType::TransformComponent)] = transformComp;
					components[static_cast<U8>(EComponentType::StaticMeshComponent)] = staticMeshComponent;
					components[static_cast<U8>(EComponentType::DirectionalLightComponent)] = directionalLightComponents[0];
					SRenderCommand command(components, ERenderCommandType::ShadowAtlasPrePassDirectional);
					RenderManager->PushRenderCommand(command);
				}

				if (!pointLightComponents.empty())
				{
					std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
					components[static_cast<U8>(EComponentType::TransformComponent)] = transformComp;
					components[static_cast<U8>(EComponentType::StaticMeshComponent)] = staticMeshComponent;
					components[static_cast<U8>(EComponentType::PointLightComponent)] = pointLightComponents[0];
					SRenderCommand command(components, ERenderCommandType::ShadowAtlasPrePassPoint);
					RenderManager->PushRenderCommand(command);
				}

				if (!spotLightComponents.empty())
				{
					std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
					components[static_cast<U8>(EComponentType::TransformComponent)] = transformComp;
					components[static_cast<U8>(EComponentType::StaticMeshComponent)] = staticMeshComponent;
					components[static_cast<U8>(EComponentType::SpotLightComponent)] = spotLightComponents[0];
					SRenderCommand command(components, ERenderCommandType::ShadowAtlasPrePassSpot);
					RenderManager->PushRenderCommand(command);
				}

				std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
				components[static_cast<U8>(EComponentType::TransformComponent)] = transformComp;
				components[static_cast<U8>(EComponentType::StaticMeshComponent)] = staticMeshComponent;
				components[static_cast<U8>(EComponentType::MaterialComponent)] = materialComp;
				SRenderCommand command(components, ERenderCommandType::GBufferDataInstanced);
				RenderManager->PushRenderCommand(command);
			}

			RenderManager->AddStaticMeshToInstancedRenderList(staticMeshComponent->Name, transformComp->Transform.GetMatrix());
		}

		{
			SRenderCommand command(std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)>{}, ERenderCommandType::DecalDepthCopy);
			RenderManager->PushRenderCommand(command);
		}

		for (auto& decalComponent : decalComponents)
		{
			if (!decalComponent->Entity->HasComponent(EComponentType::TransformComponent))
				continue;

			const I64 transformCompIndex = decalComponent->Entity->GetComponentIndex(EComponentType::TransformComponent);
			auto& transformComp = transformComponents[transformCompIndex];

			std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
			components[static_cast<U8>(EComponentType::TransformComponent)] = transformComp;
			components[static_cast<U8>(EComponentType::DecalComponent)] = decalComponent;
			SRenderCommand command(components, ERenderCommandType::DeferredDecal);
			RenderManager->PushRenderCommand(command);
		}

		{
			SRenderCommand command(std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)>{}, ERenderCommandType::PreLightingPass);
			RenderManager->PushRenderCommand(command);
		}

		if (!directionalLightComponents.empty())
		{
			const I64 transformCompIndex = directionalLightComponents[0]->Entity->GetComponentIndex(EComponentType::TransformComponent);
			auto& transformComp = transformComponents[transformCompIndex];

			std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
			components[static_cast<U8>(EComponentType::TransformComponent)] = transformComp;
			components[static_cast<U8>(EComponentType::EnvironmentLightComponent)] = environmentLightComponents[0];
			components[static_cast<U8>(EComponentType::DirectionalLightComponent)] = directionalLightComponents[0];
			SRenderCommand command(components, ERenderCommandType::DeferredLightingDirectional);
			RenderManager->PushRenderCommand(command);

			if (directionalLightComponents[0]->Entity->HasComponent(EComponentType::VolumetricLightComponent))
			{
				const I64 volumetricCompIndex = directionalLightComponents[0]->Entity->GetComponentIndex(EComponentType::VolumetricLightComponent);
				auto& volumetricLightComp = volumetricLightComponents[volumetricCompIndex];

				if (volumetricLightComp->IsActive)
				{
					components[static_cast<U8>(EComponentType::VolumetricLightComponent)] = volumetricLightComp;
					SRenderCommand volumetricCommand(components, ERenderCommandType::VolumetricLightingDirectional);
					RenderManager->PushRenderCommand(volumetricCommand);
				}
			}
		}

		if (!pointLightComponents.empty())
		{
			const I64 transformCompIndex = pointLightComponents[0]->Entity->GetComponentIndex(EComponentType::TransformComponent);
			auto& transformComp = transformComponents[transformCompIndex];

			std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
			components[static_cast<U8>(EComponentType::TransformComponent)] = transformComp;
			components[static_cast<U8>(EComponentType::PointLightComponent)] = pointLightComponents[0];
			SRenderCommand command(components, ERenderCommandType::DeferredLightingPoint);
			RenderManager->PushRenderCommand(command);

			if (pointLightComponents[0]->Entity->HasComponent(EComponentType::VolumetricLightComponent))
			{
				const I64 volumetricCompIndex = pointLightComponents[0]->Entity->GetComponentIndex(EComponentType::VolumetricLightComponent);
				auto& volumetricLightComp = volumetricLightComponents[volumetricCompIndex];

				if (volumetricLightComp->IsActive)
				{
					components[static_cast<U8>(EComponentType::VolumetricLightComponent)] = volumetricLightComp;
					SRenderCommand volumetricCommand(components, ERenderCommandType::VolumetricLightingPoint);
					RenderManager->PushRenderCommand(volumetricCommand);
				}
			}
		}

		if (!spotLightComponents.empty())
		{
			const I64 transformCompIndex = spotLightComponents[0]->Entity->GetComponentIndex(EComponentType::TransformComponent);
			auto& transformComp = transformComponents[transformCompIndex];

			std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)> components;
			components[static_cast<U8>(EComponentType::TransformComponent)] = transformComp;
			components[static_cast<U8>(EComponentType::SpotLightComponent)] = spotLightComponents[0];
			SRenderCommand command(components, ERenderCommandType::DeferredLightingSpot);
			RenderManager->PushRenderCommand(command);

			if (spotLightComponents[0]->Entity->HasComponent(EComponentType::VolumetricLightComponent))
			{
				const I64 volumetricCompIndex = spotLightComponents[0]->Entity->GetComponentIndex(EComponentType::VolumetricLightComponent);
				auto& volumetricLightComp = volumetricLightComponents[volumetricCompIndex];

				if (volumetricLightComp->IsActive)
				{
					components[static_cast<U8>(EComponentType::VolumetricLightComponent)] = volumetricLightComp;
					SRenderCommand volumetricCommand(components, ERenderCommandType::VolumetricLightingSpot);
					RenderManager->PushRenderCommand(volumetricCommand);
				}
			}
		}

		{
			SRenderCommand command(std::array<Ref<SComponent>, static_cast<size_t>(EComponentType::Count)>{}, ERenderCommandType::VolumetricBufferBlurPass);
			RenderManager->PushRenderCommand(command);
		}
	}
}
