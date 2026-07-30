// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "PrefabComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <Engine.h>
#include <Assets/AssetRegistry.h>
#include <Assets/AssetReference.h>
#include <ECS/Components/PrefabComponent.h>
#include <ECS/Components/TransformComponent.h>
#include <Scene/Scene.h>

#include <GUI.h>

namespace Havtorn
{
	void UnloadPrefabEntities(const SEntity& entityOwner, CScene* scene)
	{
		STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(entityOwner);
		if (!SComponent::IsValid(transformComponent) || transformComponent->AttachedEntities.empty())
			return;
		
		std::vector<SEntity> childrenToRemove;
		scene->GetAttachedEntities(entityOwner, childrenToRemove);

		// NW: Pop out the included Owner that we get from GetAttachedEntities
		childrenToRemove.pop_back();

		for (const SEntity& child : childrenToRemove)
			scene->RemoveEntity(child);		
	}

	void LoadPrefabEntities(const SPrefabComponent* component, CScene* scene)
	{
		STransformComponent* prefabTransform = scene->GetComponent<STransformComponent>(component->Owner);
		if (!SComponent::IsValid(prefabTransform))
		{
			HV_LOG_WARN("PrefabComponentView: Could not load prefab entities, missing prefab TransformComponent!");
			return;
		}

		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		assetRegistry->UnrequestAsset(component->AssetReference, component->Owner.GUID);

		const SPrefabAsset* prefabAsset = GEngine::GetAssetRegistry()->RequestAssetData<SPrefabAsset>(component->AssetReference, component->Owner.GUID);
		if (prefabAsset == nullptr)
			return;

		std::vector<SEntity> newEntities = scene->CopyEntities(prefabAsset->Scene.get(), {});

		for (const SEntity& entity : newEntities)
		{
			// TODO.NW: Maybe make attachment function taking attachment rules? Keeping absolute position or not
			// NW: Prefab entities have already been placed in the scene "parented" to the origin. Shift them over to the component space before attaching
			STransformComponent* childTransform = scene->GetComponent<STransformComponent>(entity);
			SMatrix matrix = childTransform->Transform.GetMatrix();
			matrix *= prefabTransform->Transform.GetMatrix();
			childTransform->Transform.SetMatrix(matrix);
			prefabTransform->Attach(childTransform);
		}
	}

    void SPrefabComponentView::View(const SEntity& entityOwner, CScene* scene) const
    {
		SPrefabComponent* prefabComponent = scene->GetComponent<SPrefabComponent>(entityOwner);
		if (!SComponent::IsValid(prefabComponent))
			return;

		GUI::Text("Prefab Mode");
		GUI::SameLine();
		GUI::TextDisabled(magic_enum::enum_name<EPrefabMode>(prefabComponent->PrefabMode).data());
		
		if (prefabComponent->PrefabMode == EPrefabMode::Packed)
		{
			if (GUI::Button("Unpack"))
			{
				STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(prefabComponent->Owner);
				if (SComponent::IsValid(transformComponent) && !transformComponent->AttachedEntities.empty())
				{
					std::vector<SEntity> childrenToRemove;
					scene->GetAttachedEntities(prefabComponent->Owner, childrenToRemove);

					// NW: Pop out the included Owner that we get from GetAttachedEntities
					childrenToRemove.pop_back();

					for (const SEntity& child : childrenToRemove)
					{
						STransformComponent* childTransform = scene->GetComponent<STransformComponent>(child);
						if (SComponent::IsValid(childTransform))
							transformComponent->Detach(childTransform);
					}
				}

				scene->RemoveEntity(prefabComponent->Owner);
				return;
			}

			GUI::SameLine();

			if (GUI::Button("Convert to Spawner"))
			{
				UnloadPrefabEntities(prefabComponent->Owner, scene);
				prefabComponent->PrefabMode = EPrefabMode::Spawner;
			}

			GUI::SameLine();
		
			if (GUI::Button("Refresh"))
			{
				UnloadPrefabEntities(prefabComponent->Owner, scene);
				LoadPrefabEntities(prefabComponent, scene);
			}
		}
		else
		{
			if (GUI::Button("Debug Spawn"))
			{
				UnloadPrefabEntities(prefabComponent->Owner, scene);
				LoadPrefabEntities(prefabComponent, scene);
			}

			GUI::SameLine();

			if (GUI::Button("Convert to Packed Prefab"))
			{
				UnloadPrefabEntities(prefabComponent->Owner, scene);
				LoadPrefabEntities(prefabComponent, scene);
				prefabComponent->PrefabMode = EPrefabMode::Packed;
			}

			GUI::SameLine();

			if (GUI::Button("Clear Debug Spawn"))
			{
				UnloadPrefabEntities(prefabComponent->Owner, scene);
			}
		}

		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
		inspector->InspectAssetComponent(prefabComponent, EAssetType::Prefab, SAssetReference::ConvertToPointers(prefabComponent->AssetReference));
    }

	U8 SPrefabComponentView::GetSortingPriority() const
	{
		return 4;
	}
}
