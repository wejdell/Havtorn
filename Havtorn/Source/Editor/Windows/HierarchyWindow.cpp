// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "HierarchyWindow.h"
#include "EditorManager.h"

#include <Engine.h>
#include <Scene/Scene.h>
#include <ECS/Entity.h>
#include <MathTypes/EngineMath.h>
#include <CoreTypes.h>
#include <ECS/Components/MetaDataComponent.h>
#include <ECS/Components/TransformComponent.h>

#include <ECS/Systems/DebugShapeSystem.h>

namespace Havtorn
{
	CHierarchyWindow::CHierarchyWindow(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
	{
	}

	CHierarchyWindow::~CHierarchyWindow()
	{
	}

	void CHierarchyWindow::OnEnable()
	{
	}

	void CHierarchyWindow::OnInspectorGUI()
	{
		const SEditorLayout& layout = Manager->GetEditorLayout();
		
		const SVector2<F32>& viewportWorkPos = GUI::GetViewportWorkPos();
		GUI::SetNextWindowPos(SVector2<F32>(viewportWorkPos.X + layout.HierarchyViewPosition.X, viewportWorkPos.Y + layout.HierarchyViewPosition.Y));
		GUI::SetNextWindowSize(SVector2<F32>(layout.HierarchyViewSize.X, layout.HierarchyViewSize.Y));
		
		if (GUI::Begin(Name(), nullptr, { EWindowFlag::NoMove, EWindowFlag::NoResize, EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus }))
		{
			 // Top Bar
			GUI::BeginChild("SearchBar", SVector2<F32>(0.0f, 54.0f));
			Filter.Draw("Search");
			GUI::Separator();

			CScene* scene = Manager->GetCurrentScene();
			if (!scene)
			{
				GUI::EndChild();
				GUI::End();
				return;
			}

			GUI::Text(scene->GetSceneName().c_str());
			GUI::Separator();
			GUI::EndChild();

			// Hierarchy View
			GUI::BeginChild("Hierarchy View");
			const std::vector<SEntity>& entities = GEngine::GetWorld()->GetEntities();
			SGuiMultiSelectIO io = GUI::BeginMultiSelect({});

			// Filter pre pass
			std::vector<SEntity> activeEntities = {};

			if (Filter.IsActive())
			{
				for (const SEntity& entity : entities)
				{
					if (!entity.IsValid())
						continue;

					const SMetaDataComponent* metaDataComp = scene->GetComponent<SMetaDataComponent>(entity);
					const std::string entryString = metaDataComp->IsValid() ? metaDataComp->Name.AsString() : "UNNAMED";

					if (Filter.PassFilter(entryString.c_str()))
						activeEntities.emplace_back(entity);
				}
			}
			else
			{
				activeEntities = entities;
			}

			// Filter out children from main list
			for (const SEntity& entity : activeEntities)
			{
				const STransformComponent* transform = scene->GetComponent<STransformComponent>(entity);
				if (transform->IsValid())
				{
					for (SEntity child : transform->AttachedEntities)
					{
						if (auto it = std::ranges::find(activeEntities, child); it != activeEntities.end())
							activeEntities.erase(it);
					}
				}
			}

			if (GUI::BeginTable("HierarchyEntityTable", 1))
			{
				InspectEntities(scene, activeEntities);
				GUI::EndTable();
			}

			GUI::EndMultiSelect();
		}
		GUI::EndChild();
		GUI::End();
	}

	void CHierarchyWindow::OnDisable()
	{
	}

	void CHierarchyWindow::InspectEntities(const CScene* scene, const std::vector<SEntity>& entities)
	{
		for (const SEntity& entity : entities)
		{
			GUI::TableNextRow();
			GUI::TableNextColumn();
			GUI::PushID(STATIC_I32(entity.GUID));

			const SMetaDataComponent* metaDataComp = scene->GetComponent<SMetaDataComponent>(entity);
			const std::string entryString = metaDataComp->IsValid() ? metaDataComp->Name.AsString() : "UNNAMED";

			std::vector<ETreeNodeFlag> flags = { ETreeNodeFlag::SpanAvailWidth, ETreeNodeFlag::DefaultOpen };

			if (entity == Manager->GetSelectedEntity())
				flags.emplace_back(ETreeNodeFlag::Selected);
			
			STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(entity);
			if (transformComponent->IsValid() && transformComponent->AttachedEntities.empty())
				flags.emplace_back(ETreeNodeFlag::Leaf);

			const bool isOpen = GUI::TreeNodeEx(entryString.c_str(), flags);

			if (GUI::BeginDragDropSource())
			{
				SGuiPayload payload = GUI::GetDragDropPayload();
				if (payload.Data == nullptr)
				{
					GUI::SetDragDropPayload("EntityHierarchyDrag", &entity, sizeof(SEntity));
				}
				GUI::Text(entryString.c_str());

				GUI::EndDragDropSource();
			}

			if (GUI::BeginDragDropTarget())
			{
				SGuiPayload payload = GUI::AcceptDragDropPayload("EntityHierarchyDrag", { EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNopreviewTooltip });
				if (payload.Data != nullptr)
				{
					SEntity* draggedEntity = reinterpret_cast<SEntity*>(payload.Data);
					const SMetaDataComponent* draggedMetaDataComp = scene->GetComponent<SMetaDataComponent>(*draggedEntity);
					const std::string draggedEntityName = draggedMetaDataComp->IsValid() ? draggedMetaDataComp->Name.AsString() : "UNNAMED";
					
					if (!transformComponent->IsValid())
					{
						GUI::SetTooltip("Cannot attach to entity %s, it has no transform!", entryString.c_str());
					}
					else
					{
						// TODO.NW: Fix bug where you attach the same entity to the same parent twice. Unparent in this case? Also unparent if dragged to another entity or to empty space.
						GUI::SetTooltip("Attach %s to %s?", draggedEntityName.c_str(), entryString.c_str());

						if (payload.IsDelivery)
						{
							STransformComponent* draggedTransformComponent = scene->GetComponent<STransformComponent>(*draggedEntity);

							// TODO.NW: Need very clear way to set up attachment and detachment. Have not done detachment yet.
							draggedTransformComponent->Transform.SetParent(&transformComponent->Transform);
							transformComponent->AttachedEntities.emplace_back(*draggedEntity);
							transformComponent->Transform.AddAttachment(&draggedTransformComponent->Transform);
						}
					}
				}

				GUI::EndDragDropTarget();
			}

			if (GUI::IsItemClicked())
				Manager->SetSelectedEntity(entity);

			if (isOpen)
			{
				InspectEntities(scene, transformComponent->AttachedEntities);
				GUI::TreePop();
			}

			GUI::PopID();
		}
	}
}
