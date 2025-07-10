// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ScriptComponentEditorContext.h"

#include "ECS/Components/ScriptComponent.h"
#include "Scene/Scene.h"

#include <GUI.h>
#include <ECS/Components/MetaDataComponent.h>

namespace Havtorn
{
	SScriptComponentEditorContext SScriptComponentEditorContext::Context = {};

	SComponentViewResult SScriptComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("Script"))
			return SComponentViewResult();

		SScriptComponent* component = scene->GetComponent<SScriptComponent>(entityOwner);
		if (!component || (component && !component->Owner.IsValid()))
			return SComponentViewResult();

		HexRune::SScript* script = component->Script;
		if (!script)
			return { EComponentViewResultLabel::InspectAssetComponent, component, 0 };

		if (script->DataBindings.empty())
			GUI::TextDisabled("No Data Bindings");
		else
		{
			GUI::Text("Data Bindings");
			GUI::Separator();

			for (auto& db : component->DataBindings)
			{
				SVector2 available = GUI::GetContentRegionAvail();

				GUI::BeginHorizontal(db.Name.c_str(), { available.X, 12.0f });
				GUI::Text(db.Name.c_str());

				GUI::SameLine();

				GUI::TextDisabled("Type: %s", "Entity");
				GUI::SameLine();

				SEntity entity{};

				if (std::holds_alternative<SEntity>(db.Data))
					entity = std::get<SEntity>(db.Data);

				if (auto metaDataComponent = scene->GetComponent<SMetaDataComponent>(entity))
					GUI::Text("%s", metaDataComponent->Name.Data());
				else
					GUI::Text("Not Set");

				if (GUI::BeginDragDropTarget())
				{
					SGuiPayload payload = GUI::AcceptDragDropPayload("EntityAssignmentDrag", { EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNoDrawDefaultRect, EDragDropFlag::AcceptNopreviewTooltip });
					if (payload.Data != nullptr)
					{
						SEntity* draggedEntity = reinterpret_cast<SEntity*>(payload.Data);
						const SMetaDataComponent* draggedMetaDataComp = scene->GetComponent<SMetaDataComponent>(*draggedEntity);
						const std::string draggedEntityName = draggedMetaDataComp->IsValid() ? draggedMetaDataComp->Name.AsString() : "UNNAMED";
						GUI::SetTooltip(draggedEntityName.c_str());

						if (draggedEntity->IsValid())
						{
							GUI::SetTooltip("Assign %s to Data Binding '%s'?", draggedEntityName.c_str(), db.Name.c_str());

							if (payload.IsDelivery)
								db.Data = *draggedEntity;
						}
					}

					GUI::EndDragDropTarget();
				}

				// TODO.NW: Make component/entity dropper
				GUI::EndHorizontal();
			}
		}

		GUI::Checkbox("Trigger", component->TriggerScript);
	
		return { EComponentViewResultLabel::InspectAssetComponent, component, 0 };
	}

	bool SScriptComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("Script Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SScriptComponent>(entity);
		scene->AddComponentEditorContext(entity, &SScriptComponentEditorContext::Context);
		return true;
	}

	bool SScriptComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		// TODO.NW: Push IDs here instead of having to this labeling
		if (!GUI::Button("X##11"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SScriptComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SScriptComponentEditorContext::Context);
		return true;
	}

	U8 SScriptComponentEditorContext::GetSortingPriority() const
	{
		return 1;
	}
}
