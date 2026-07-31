// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ScriptComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/ScriptComponent.h>
#include <ECS/Components/MetaDataComponent.h>
#include <Assets/AssetRegistry.h>
#include <Scene/Scene.h>
#include <Engine.h>

#include <GUI.h>

namespace Havtorn
{
	void SScriptComponentView::ViewDataBinding(CScene* scene, HexRune::SScriptDataBinding& dataBinding) const
	{
		constexpr F32 dataBindingIndentation = 16.0f;
		GUI::Indent(dataBindingIndentation);

		switch (dataBinding.Type)
		{
		case HexRune::EPinType::Bool:
		{
			GUI::Text(dataBinding.Name.c_str());
			GUI::SameLine();
			GUI::TextDisabled(" |%s| ", "Bool");
			GUI::SameLine();
			GUI::Checkbox("", std::get<bool>(dataBinding.Data));
		}
		break;
		case HexRune::EPinType::Int:
		{
			GUI::Text(dataBinding.Name.c_str());
			GUI::SameLine();
			GUI::TextDisabled(" |%s| ", "Int");
			GUI::SameLine();
			GUI::InputInt("", std::get<I32>(dataBinding.Data));
		}
		break;
		case HexRune::EPinType::Float:
		{
			GUI::Text(dataBinding.Name.c_str());
			GUI::SameLine();
			GUI::TextDisabled(" |%s| ", "Float");
			GUI::SameLine();
			GUI::InputFloat("", std::get<F32>(dataBinding.Data));
		}
		break;
		case HexRune::EPinType::String:
		{
			GUI::Text(dataBinding.Name.c_str());
			GUI::SameLine();
			GUI::TextDisabled(" |%s| ", "String");
			GUI::SameLine();
			GUI::InputText("##edit", std::get<std::string>(dataBinding.Data));	
		}
		break;
		case HexRune::EPinType::Vector:
		{
			GUI::Text(dataBinding.Name.c_str());
			GUI::SameLine();
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

			GUI::Text(dataBinding.Name.c_str());
			GUI::SameLine();
			GUI::TextDisabled(" |%s| ", "Entity");
			GUI::SameLine();

			SEntity entity{};
			if (std::holds_alternative<SEntity>(dataBinding.Data))
				entity = std::get<SEntity>(dataBinding.Data);

			if (auto metaDataComponent = scene->GetComponent<SMetaDataComponent>(entity))
				GUI::Text("%s", metaDataComponent->Name.Data());
			else
				GUI::Text("Not Set");

			auto result = CEditorManager::EntityDragData.TryDeliver({ EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNoDrawDefaultRect, EDragDropFlag::AcceptNopreviewTooltip });
			if (result.Payload != nullptr)
			{
				SEntity* draggedEntity = result.Payload;
				const SMetaDataComponent* draggedMetaDataComp = scene->GetComponent<SMetaDataComponent>(*draggedEntity);
				const std::string draggedEntityName = SComponent::IsValid(draggedMetaDataComp) ? draggedMetaDataComp->Name.AsString() : "UNNAMED";
				GUI::SetTooltip(draggedEntityName.c_str());

				if (draggedEntity->IsValid())
				{
					GUI::SetTooltip("Assign %s to Data Binding '%s'?", draggedEntityName.c_str(), dataBinding.Name.c_str());

					if (result.Result == EDragDeliverResult::Delivered)
						dataBinding.Data = *draggedEntity;
				}
			}
		}
		break;
		case HexRune::EPinType::Asset:
		{
			SEditorAssetRepresentation* assetRep = Manager->GetAssetRepFromName(UGeneralUtils::ExtractFileBaseNameFromPath(std::get<std::string>(dataBinding.Data))).get();
			std::string pickerLabel = dataBinding.Name; 
			pickerLabel.append(" |Asset| ");
			pickerLabel.append(assetRep->Name);
		
			SAssetPickResult assetPickResult = Manager->AssetPickerDropdown(pickerLabel.c_str(), dataBinding.AssetType, assetRep);
			
			if (assetPickResult.State == EAssetPickerState::AssetPicked)
			{
				SEditorAssetRepresentation* newAssetRep = Manager->GetAssetRepFromDirEntry(assetPickResult.PickedEntry).get();
				dataBinding.Data = SAssetReference(newAssetRep->DirectoryEntry.path().string()).FilePath;
			}

			auto result = CEditorManager::AssetDragData.TryDeliver({ EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNopreviewTooltip });
			if (result.Payload != nullptr)
			{
				SEditorAssetRepresentation* payloadAssetRep = result.Payload;
				if (payloadAssetRep->AssetType == dataBinding.AssetType)
				{
					GUI::SetTooltip("Assign %s to Data Binding '%s'?", payloadAssetRep->Name.c_str(), dataBinding.Name.c_str());

					if (result.Result == EDragDeliverResult::Delivered)
					{
						dataBinding.Data = SAssetReference(payloadAssetRep->DirectoryEntry.path().string()).FilePath;
					}
				}
				else
				{
					GUI::SetTooltip("Can't assign asset of type '%s' to Data Binding '%s'!\nIt is expecting a '%s'.", GetAssetTypeName(payloadAssetRep->AssetType).c_str(), dataBinding.Name.c_str(), GetAssetTypeName(dataBinding.AssetType).c_str());
				}
			}
		}
		break;
		}

		GUI::Separator();
		GUI::Unindent(dataBindingIndentation);
	}

	void SScriptComponentView::View(const SEntity& entityOwner, CScene* scene) const
	{
		SScriptComponent* component = scene->GetComponent<SScriptComponent>(entityOwner);
		if (!SComponent::IsValid(component))
			return;

		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();
		inspector->InspectAssetComponent(component, EAssetType::Script, SAssetReference::ConvertToPointers(component->AssetReference));

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
	}

	U8 SScriptComponentView::GetSortingPriority() const
	{
		return 1;
	}
}
