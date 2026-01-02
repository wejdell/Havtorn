// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ScriptComponentEditorContext.h"

#include "ECS/Components/ScriptComponent.h"
#include "Scene/Scene.h"
#include "Engine.h"

#include <GUI.h>
#include <ECS/Components/MetaDataComponent.h>
#include <Assets/RuntimeAssetDeclarations.h>
#include <Assets/AssetRegistry.h>

namespace Havtorn
{
	SScriptComponentEditorContext SScriptComponentEditorContext::Context = {};

	void SScriptComponentEditorContext::ViewDataBinding(CScene* scene, HexRune::SScriptDataBinding& dataBinding) const
	{
		GUI::Text(dataBinding.Name.c_str());

		GUI::SameLine();
		switch (dataBinding.Type)
		{
		case HexRune::EPinType::Bool:
		{
			GUI::TextDisabled(" |%s| ", "Bool");
			GUI::SameLine();
			GUI::Checkbox("", std::get<bool>(dataBinding.Data));
		}
		break;
		case HexRune::EPinType::Int:
		{
			GUI::TextDisabled(" |%s| ", "Int");
			GUI::SameLine();
			GUI::InputInt("", std::get<I32>(dataBinding.Data));
		}
		break;
		case HexRune::EPinType::Float:
		{
			GUI::TextDisabled(" |%s| ", "Float");
			GUI::SameLine();
			GUI::InputFloat("", std::get<F32>(dataBinding.Data));
		}
		break;
		case HexRune::EPinType::String:
		{
			GUI::TextDisabled(" |%s| ", "String");
			GUI::SameLine();
			GUI::InputText("##edit", std::get<std::string>(dataBinding.Data));	
		}
		break;
		case HexRune::EPinType::Vector:
		{
			GUI::TextDisabled(" |%s| ", "Vector");
			GUI::SameLine();
			GUI::DragFloat3("##edit", std::get<SVector>(dataBinding.Data));
		}
		break;
		case HexRune::EPinType::Matrix:
		{}
		break;
		case HexRune::EPinType::Quaternion:
		{}
		break;
		case HexRune::EPinType::Entity:
		{
			// TODO.NW: Handle Component type

			GUI::TextDisabled(" |%s| ", "Entity");
			GUI::SameLine();

			SEntity entity{};
			if (std::holds_alternative<SEntity>(dataBinding.Data))
				entity = std::get<SEntity>(dataBinding.Data);

			if (auto metaDataComponent = scene->GetComponent<SMetaDataComponent>(entity))
				GUI::Text("%s", metaDataComponent->Name.Data());
			else
				GUI::Text("Not Set");

			if (GUI::BeginDragDropTarget())
			{
				SGuiPayload payload = GUI::AcceptDragDropPayload("EntityDrag", { EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNoDrawDefaultRect, EDragDropFlag::AcceptNopreviewTooltip });
				if (payload.Data != nullptr)
				{
					SEntity* draggedEntity = reinterpret_cast<SEntity*>(payload.Data);
					const SMetaDataComponent* draggedMetaDataComp = scene->GetComponent<SMetaDataComponent>(*draggedEntity);
					const std::string draggedEntityName = SComponent::IsValid(draggedMetaDataComp) ? draggedMetaDataComp->Name.AsString() : "UNNAMED";
					GUI::SetTooltip(draggedEntityName.c_str());

					if (draggedEntity->IsValid())
					{
						GUI::SetTooltip("Assign %s to Data Binding '%s'?", draggedEntityName.c_str(), dataBinding.Name.c_str());

						if (payload.IsDelivery)
							dataBinding.Data = *draggedEntity;
					}
				}

				GUI::EndDragDropTarget();
			}
		}
		break;
		case HexRune::EPinType::Asset:
		{
			std::string assetPath = "";
			// TODO.NW: Need to figure out how to access editor reps from here. Maybe editor reps contain an engine asset and we can drag that instead. 
			//if (GUI::BeginDragDropTarget())
			//{
			//	SGuiPayload payload = GUI::AcceptDragDropPayload("AssetDrag", { EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNopreviewTooltip });
			//	if (payload.Data != nullptr)
			//	{
			//		std::string draggedString = *reinterpret_cast<std::string*>(payload.Data);
			//		GUI::SetTooltip(draggedString.c_str());

			//		if (!draggedString.empty())
			//		{
			//			GUI::SetTooltip("Assign %s to Data Binding '%s'?", draggedString.c_str(), dataBinding.Name.c_str());

			//			if (payload.IsDelivery)
			//				assetPath = draggedString;
			//		}
			//	}

			//	GUI::EndDragDropTarget();
			//}

			GUI::TextDisabled(" |%s| ", "Asset");
			GUI::SameLine();

			SAsset asset;
			if (std::holds_alternative<std::string>(dataBinding.Data))
				asset = *GEngine::GetAssetRegistry()->RequestAsset(SAssetReference(std::get<std::string>(dataBinding.Data)), 100);

			if (asset.Reference.FilePath.empty())
				asset.Reference.FilePath = assetPath;

			//GUI::Text(asset.AssetPath.c_str());
			GUI::InputText("##edit", asset.Reference.FilePath);

			if (GUI::BeginPopupContextWindow())
			{
				if (GUI::MenuItem("Paste Asset Path"))
					asset.Reference.FilePath = GUI::CopyFromClipboard();

				GUI::EndPopup();
			}

			if (!asset.Reference.FilePath.empty())
				asset.Reference = SAssetReference(asset.Reference.FilePath);

			dataBinding.Data = asset.Reference.FilePath;
			// TODO.NW: Simplify flow for assigning assets through a view result
			//if (dataBinding.AssetType == EAssetType::StaticMesh)
			//{
			//	GUI::AssetPicker("##edit", "Static Mesh", );
			//}
		}
		break;
		}
	}

	SComponentViewResult SScriptComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("Script"))
			return SComponentViewResult();

		SScriptComponent* component = scene->GetComponent<SScriptComponent>(entityOwner);
		if (!component || (component && !component->Owner.IsValid()))
			return SComponentViewResult();

		if (component->DataBindings.empty())
			GUI::TextDisabled("No Data Bindings");
		else
		{
			GUI::Text("Data Bindings");
			GUI::Separator();

			for (auto& db : component->DataBindings)
			{
				GUI::PushID(db.UID);
				ViewDataBinding(scene, db);
				GUI::PopID();

			}
		}

		GUI::Checkbox("Trigger", component->TriggerScript);
	
		return { EComponentViewResultLabel::InspectAssetComponent, component, SAssetReference::ConvertToPointers(component->AssetReference), EAssetType::Script};
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
