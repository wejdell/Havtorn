// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "UICanvasComponentEditorContext.h"

#include "ECS/Components/UICanvasComponent.h"
#include "ECS/Components/MetaDataComponent.h"
#include "ECS/Systems/UISystem.h"
#include "Scene/Scene.h"
#include "Scene/World.h"
#include "Engine.h"
#include "Graphics/TextureBank.h"
#include "Graphics/Debug/DebugDrawUtility.h"

#include <GUI.h>

namespace Havtorn 
{
	SUICanvasComponentEditorContext SUICanvasComponentEditorContext::Context = {};

	SComponentViewResult SUICanvasComponentEditorContext::View(const SEntity& entityOwner, CScene* scene) const
	{
		if (!GUI::TryOpenComponentView("UI Canvas"))
			return SComponentViewResult();

		SUICanvasComponent* canvasComponent = scene->GetComponent<SUICanvasComponent>(entityOwner);

		GUI::TextDisabled("Elements");

		GUI::SameLine();
		if (GUI::Button("Add"))
			canvasComponent->Elements.push_back(SUIElement());

		GUI::SameLine();
		if (GUI::Button("Clear"))
			canvasComponent->Elements.clear();

		if (canvasComponent->Elements.empty())
			return SComponentViewResult();

		I32 elementToRemoveIndex = -1;

		// TODO.NW: Need preview for collision rect and UV
		// TODO.NW: Maybe calculate bounds for whole canvas?
		for (I32 i = 0; i < STATIC_I32(canvasComponent->Elements.size()); i++)
		{
			SUIElement& element = canvasComponent->Elements[i];
			GUI::PushID(i);
			GUI::Separator();

			if (GUI::Button("X"))
				elementToRemoveIndex = i;
			GUI::SameLine();

			std::string elementName = "Element ";
			elementName.append(std::to_string(i));
			if (GUI::TreeNode(elementName.c_str()))
			{
				// TODO.NW: Add references to list, but only if not marked for delete

				element.State = GUI::ComboEnum("Preview State", element.State);
				if (element.State == EUIElementState::Count)
					element.State = EUIElementState::Idle;

				GUI::DragFloat2("Local Position", element.LocalPosition, GUI::SliderSpeed);
				GUI::DragFloat2("Local Scale", element.LocalScale, GUI::SliderSpeed);
				GUI::DragFloat("Local Rotation (Degrees)", element.LocalDegreesRoll, GUI::SliderSpeed);
				GUI::DragFloat4("Collision Rect", element.CollisionRect, GUI::SliderSpeed);

				element.BindingType = GUI::ComboEnum("Binding Type", element.BindingType);
				if (element.BindingType == EUIBindingType::GenericFunction)
				{
					std::string boundFunctionName = "Function Not Found";
					if (CUISystem* uiSystem = GEngine::GetWorld()->GetSystem<CUISystem>())
						boundFunctionName = uiSystem->GetFunctionName(element.BoundData);

					// TODO.NW: Have combobox of all existing bound functions in system?
					// TODO.NW: Have ClassName::FunctionName in hint text
					if (GUI::InputText("(On Click) ClassName::FunctionName: ", boundFunctionName))
						element.BoundData = std::hash<std::string>{}(boundFunctionName);
				}
				else if (element.BindingType == EUIBindingType::OtherCanvas)
				{
					GUI::TextDisabled("(On Click) Activate Canvas: ");
					GUI::SameLine();

					if (auto metaDataComponent = scene->GetComponent<SMetaDataComponent>(SEntity{ element.BoundData }))
						GUI::Text("%s", metaDataComponent->Name.Data());
					else
						GUI::Text("Not Set");

					// TODO.NW: Generalize the drag drop functions, this is used (basically) in script component data binding as well.
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
								GUI::SetTooltip("Assign %s to UI Binding?", draggedEntityName.c_str());

								if (payload.IsDelivery)
									element.BoundData = draggedEntity->GUID;
							}
						}

						GUI::EndDragDropTarget();
					}
				}

				GUI::TextDisabled("UVs");
				GUI::DragFloat4("Idle UVRect", element.UVRects[0], GUI::SliderSpeed);
				GUI::DragFloat4("Hover UVRect", element.UVRects[1], GUI::SliderSpeed);
				GUI::DragFloat4("Active UVRect", element.UVRects[2], GUI::SliderSpeed);

				GDebugDraw::AddLine2D(SVector2<F32>(0.0f), SVector2<F32>(1.0f), SColor::Magenta, 1.0f, true, 0.1f);

				GUI::TreePop();
			}
			GUI::PopID();
		}

		if (elementToRemoveIndex != -1)
			canvasComponent->Elements.erase(canvasComponent->Elements.begin() + elementToRemoveIndex);

		// TODO.NW: This is not a nice solution for these ui elements. 
		// Can we figure out a way to send a bunch raw pointers to asset refs instead?
		return { EComponentViewResultLabel::InspectAssetComponent, canvasComponent, nullptr, &canvasComponent->Elements[0].StateAssetReferences, EAssetType::Texture };
	}

	bool SUICanvasComponentEditorContext::AddComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("UI Canvas Component"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->AddComponent<SUICanvasComponent>(entity);
		scene->AddComponentEditorContext(entity, &SUICanvasComponentEditorContext::Context);
		return true;
	}

	bool SUICanvasComponentEditorContext::RemoveComponent(const SEntity& entity, CScene* scene) const
	{
		if (!GUI::Button("X##17"))
			return false;

		if (scene == nullptr || !entity.IsValid())
			return false;

		scene->RemoveComponent<SUICanvasComponent>(entity);
		scene->RemoveComponentEditorContext(entity, &SUICanvasComponentEditorContext::Context);
		return true;
	}
}
