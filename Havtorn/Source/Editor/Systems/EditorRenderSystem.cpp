// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "EditorRenderSystem.h"

#include "EditorManager.h"

#include "ECS/ECSInclude.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderCommand.h"
#include "ECS/ComponentAlgo.h"
#include "Assets/AssetRegistry.h"
#include "Scene/Scene.h"

#include <hvpch.h>

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

	CEditorRenderSystem::CEditorRenderSystem(CRenderManager* renderManager, CWorld* world, CEditorManager* editorManager)
		: ISystem()
		, RenderManager(renderManager)
		, Manager(editorManager)
		, World(world)
	{
	}

	void CEditorRenderSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		for (Ptr<CScene>& scene : scenes)
		{
			if (World->GetWorldPlayState() == EWorldPlayState::Playing)
				return;

			if (!scene->OnEntityPreDestroy.IsBoundTo(Handle))
			{
				Handle = scene->OnEntityPreDestroy.AddMember(this, &CEditorRenderSystem::OnEntityPreDestroy);
			}

			auto tryAddComponentWidgets = [&]<typename T>(T&& /*emptyComponent*/, const SAssetReference& assetReference)
			{
				for (const T* component : scene->GetComponents<T>())
				{
					if (!SComponent::IsValid(component))
						continue;

					const STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(component);

					GEngine::GetAssetRegistry()->RequestAsset(assetReference.UID, transformComponent->Owner.GUID);
					RenderManager->AddSpriteToWorldSpaceInstancedRenderList(assetReference.UID, transformComponent, scene->GetComponent<STransformComponent>(scene->MainCameraEntity), 0);

					SRenderCommand command;
					command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
					command.U32s.push_back(assetReference.UID);
					RenderManager->PushRenderCommand(command, 0);
				}
			};

			tryAddComponentWidgets(SCameraComponent(), CameraWidgetReference);
			tryAddComponentWidgets(SDecalComponent(), DecalWidgetReference);
			tryAddComponentWidgets(SEnvironmentLightComponent(), EnvironmentLightWidgetReference);
			tryAddComponentWidgets(SDirectionalLightComponent(), DirectionalLightWidgetReference);
			tryAddComponentWidgets(SPointLightComponent(), PointLightWidgetReference);
			tryAddComponentWidgets(SSpotLightComponent(), SpotlightWidgetReference);

			for (const SPhysics3DComponent* physics3DComponent : scene->GetComponents<SPhysics3DComponent>())
			{
				if (!SComponent::IsValid(physics3DComponent))
					continue;

				// NW: Write this out by hand so regular colliders don't get this widget
				if (!physics3DComponent->IsTrigger)
					continue;

				const STransformComponent* transformComp = scene->GetComponent<STransformComponent>(physics3DComponent);
				GEngine::GetAssetRegistry()->RequestAsset(ColliderWidgetReference.UID, transformComp->Owner.GUID);
				RenderManager->AddSpriteToWorldSpaceInstancedRenderList(ColliderWidgetReference.UID, transformComp, scene->GetComponent<STransformComponent>(scene->MainCameraEntity), 0);

				SRenderCommand command;
				command.Type = ERenderCommandType::WorldSpaceSpriteEditorWidget;
				command.U32s.push_back(ColliderWidgetReference.UID);
				RenderManager->PushRenderCommand(command, 0);
			}
		}
	}

	void CEditorRenderSystem::OnEntityPreDestroy(const SEntity entity)
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
