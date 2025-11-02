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

#include <../Game/GameScene.h>

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
			SEditData editData;

			std::vector<Ptr<CScene>>& scenes = Manager->GetScenes();
			if (!scenes.empty())
			{
				Header();
				Body(scenes, editData);
			}

			Footer(scenes, editData);
			Edit(editData);
		}

		GUI::End();
	}

	void CHierarchyWindow::OnDisable()
	{
	}

	void CHierarchyWindow::FilterChildrenFromList(const CScene* scene, const std::vector<SEntity>& children, std::vector<SEntity>& filteredEntities)
	{
		for (SEntity child : children)
		{
			const STransformComponent* transform = scene->GetComponent<STransformComponent>(child);
			if (!SComponent::IsValid(transform))
				continue;

			if (!transform->AttachedEntities.empty())
				FilterChildrenFromList(scene, transform->AttachedEntities, filteredEntities);

			if (auto it = std::ranges::find(filteredEntities, child); it == filteredEntities.end())
				filteredEntities.emplace_back(child);
		}
	}

	void CHierarchyWindow::InspectEntities(const CScene* scene, const std::vector<SEntity>& entities)
	{
		for (const SEntity& entity : entities)
		{
			GUI::TableNextRow();
			GUI::TableNextColumn();
			GUI::PushID(STATIC_I32(entity.GUID));

			const SMetaDataComponent* metaDataComp = scene->GetComponent<SMetaDataComponent>(entity);
			const std::string entryString = SComponent::IsValid(metaDataComp) ? metaDataComp->Name.AsString() : "UNNAMED";

			std::vector<ETreeNodeFlag> flags = { ETreeNodeFlag::SpanAvailWidth, ETreeNodeFlag::DefaultOpen };

			if (Manager->IsEntitySelected(entity))
				flags.emplace_back(ETreeNodeFlag::Selected);

			STransformComponent* transformComponent = scene->GetComponent<STransformComponent>(entity);

			if (transformComponent == nullptr)
				flags.emplace_back(ETreeNodeFlag::Leaf);
			else if (SComponent::IsValid(transformComponent) && transformComponent->AttachedEntities.empty())
				flags.emplace_back(ETreeNodeFlag::Leaf);

			const bool isOpen = GUI::TreeNodeEx(entryString.c_str(), flags);
			if (GUI::BeginDragDropSource())
			{
				SGuiPayload payload = GUI::GetDragDropPayload();
				if (payload.Data == nullptr)
				{
					GUI::SetDragDropPayload("EntityDrag", &entity, sizeof(SEntity));				
				}
				GUI::Text(entryString.c_str());

				GUI::EndDragDropSource();
			}

			if (GUI::BeginDragDropTarget())
			{
				SGuiPayload payload = GUI::AcceptDragDropPayload("EntityDrag", { EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNopreviewTooltip });
				if (payload.Data != nullptr)
				{
					SEntity* draggedEntity = reinterpret_cast<SEntity*>(payload.Data);
					const SMetaDataComponent* draggedMetaDataComp = scene->GetComponent<SMetaDataComponent>(*draggedEntity);
					const std::string draggedEntityName = SComponent::IsValid(draggedMetaDataComp) ? draggedMetaDataComp->Name.AsString() : "UNNAMED";
					GUI::SetTooltip(draggedEntityName.c_str());

					if (!SComponent::IsValid(transformComponent))
					{
						GUI::SetTooltip("Cannot attach to entity %s, it has no transform!", entryString.c_str());
					}
					else
					{
						STransformComponent* draggedTransform = scene->GetComponent<STransformComponent>(*draggedEntity);
						if (!SComponent::IsValid(draggedTransform))
						{
							GUI::SetTooltip("Cannot attach %s to entity, it has no transform!", draggedEntityName.c_str());
						}
						else
						{
							GUI::SetTooltip("Attach %s to %s?", draggedEntityName.c_str(), entryString.c_str());

							if (payload.IsDelivery)
							{
								if (draggedTransform->Transform.HasParent())
								{
									STransformComponent* existingParentComponent = scene->GetComponent<STransformComponent>(draggedTransform->ParentEntity);
									existingParentComponent->Detach(draggedTransform);
								}
								transformComponent->Attach(draggedTransform);
							}
						}
					}
				}

				GUI::EndDragDropTarget();
			}

			if (GUI::IsItemHovered() && GUI::IsMouseReleased(0))
			{
				// TODO.NW: Simplify this
				if (GUI::IsShiftHeld())
				{
					// TODO.NW: Figure out when to clear rest of selection so that shift clicking only selects between
					// latest selection and current entity.
					SEntity latestSelection = Manager->GetLastSelectedEntity();
					if (!latestSelection.IsValid())
					{
						Manager->SetSelectedEntity(entity);
					}
					else
					{
						if (Manager->IsEntitySelected(entity))
						{
							auto currentEntityIt = std::ranges::find(entities, entity);
							auto latestSelectionIt = std::ranges::find(entities, latestSelection);
							U64 currentEntityIndex = std::ranges::distance(entities.begin(), currentEntityIt);
							U64 latestSelectionIndex = std::ranges::distance(entities.begin(), latestSelectionIt);
							U64 startIndex = UMath::Min(currentEntityIndex, latestSelectionIndex);
							U64 endIndex = UMath::Max(currentEntityIndex, latestSelectionIndex);
							for (U64 i = startIndex; i != endIndex; i++)
								Manager->RemoveSelectedEntity(entities[i]);
						}
						else
						{
							auto currentEntityIt = std::ranges::find(entities, entity);
							auto latestSelectionIt = std::ranges::find(entities, latestSelection);
							U64 currentEntityIndex = std::ranges::distance(entities.begin(), currentEntityIt);
							U64 latestSelectionIndex = std::ranges::distance(entities.begin(), latestSelectionIt);
							U64 startIndex = UMath::Min(currentEntityIndex, latestSelectionIndex);
							U64 endIndex = UMath::Max(currentEntityIndex, latestSelectionIndex);
							for (U64 i = startIndex; i != endIndex; i++)
								Manager->AddSelectedEntity(entities[i]);
						}
					}
				}
				else if (GUI::IsControlHeld() && Manager->IsEntitySelected(entity))
					Manager->RemoveSelectedEntity(entity);
				else if (GUI::IsControlHeld())
					Manager->AddSelectedEntity(entity);
				else if (!Manager->IsEntitySelected(entity))
					Manager->SetSelectedEntity(entity);
			}

			if (isOpen && SComponent::IsValid(transformComponent))
			{
				InspectEntities(scene, transformComponent->AttachedEntities);
				GUI::TreePop();
			}

			if (!SComponent::IsValid(transformComponent))
				GUI::TreePop();

			GUI::PopID();
		}
	}

	void CHierarchyWindow::Header()
	{
		GUI::BeginChild("SearchBar", SVector2<F32>(0.0f, 30.0f));
		Filter.Draw("Search");
		GUI::EndChild();
		GUI::Separator();
	}

	void CHierarchyWindow::Body(std::vector<Ptr<CScene>>& scenes, SEditData& editData)
	{
		GUI::BeginChild("HierarchyView", SVector2<F32>(0.0f, GUI::GetContentRegionAvail().Y - 36.0f));
		for (U64 sceneIndex = 0; sceneIndex < scenes.size(); sceneIndex++)
		{
			Ptr<CScene>& scene = scenes[sceneIndex];

			const bool isCurrentScene = scene.get() == Manager->GetCurrentScene();
			GUI::Selectable(scene->GetSceneName().c_str(), isCurrentScene);

			if (GUI::IsDoubleClick() && GUI::IsItemHovered())
			{
				if (editData.DoubleClickIndex == -1)
					editData.DoubleClickIndex = sceneIndex;
			}

			GUI::Separator();

			// Filter pre pass
			std::vector<SEntity> activeEntities = {};

			if (Filter.IsActive())
			{
				for (const SEntity& entity : scene->Entities)
				{
					if (!entity.IsValid())
						continue;

					const SMetaDataComponent* metaDataComp = scene->GetComponent<SMetaDataComponent>(entity);
					const std::string entryString = SComponent::IsValid(metaDataComp) ? metaDataComp->Name.AsString() : "UNNAMED";

					if (Filter.PassFilter(entryString.c_str()))
						activeEntities.emplace_back(entity);
				}
			}
			else
			{
				activeEntities = scene->Entities;
			}

			// Filter out children from main list
			std::vector<SEntity> childEntities;
			for (const SEntity& entity : activeEntities)
			{
				const SMetaDataComponent* draggedMetaDataComp = scene->GetComponent<SMetaDataComponent>(entity);
				const std::string draggedEntityName = SComponent::IsValid(draggedMetaDataComp) ? draggedMetaDataComp->Name.AsString() : "UNNAMED";

				const STransformComponent* transform = scene->GetComponent<STransformComponent>(entity);
				if (!SComponent::IsValid(transform))
					continue;

				if (!transform->AttachedEntities.empty())
					FilterChildrenFromList(scene.get(), transform->AttachedEntities, childEntities);
			}
			for (const SEntity& child : childEntities)
			{
				if (auto it = std::ranges::find(activeEntities, child); it != activeEntities.end())
					activeEntities.erase(it);
			}

			if (GUI::BeginTable("HierarchyEntityTable", 1))
			{
				InspectEntities(scene.get(), activeEntities);
				GUI::EndTable();
			}

			// Detachment drop
			if (GUI::BeginDragDropTarget())
			{
				SGuiPayload payload = GUI::AcceptDragDropPayload("EntityHierarchyDrag", { EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNoDrawDefaultRect, EDragDropFlag::AcceptNopreviewTooltip });
				if (payload.Data != nullptr)
				{
					SEntity* draggedEntity = reinterpret_cast<SEntity*>(payload.Data);
					const SMetaDataComponent* draggedMetaDataComp = scene->GetComponent<SMetaDataComponent>(*draggedEntity);
					const std::string draggedEntityName = SComponent::IsValid(draggedMetaDataComp) ? draggedMetaDataComp->Name.AsString() : "UNNAMED";
					GUI::SetTooltip(draggedEntityName.c_str());

					STransformComponent* draggedTransform = scene->GetComponent<STransformComponent>(*draggedEntity);
					if (!SComponent::IsValid(draggedTransform))
					{
						GUI::SetTooltip("Cannot detach entity %s, it has no transform!", draggedEntityName.c_str());
					}
					else
					{
						const SEntity& parentEntity = draggedTransform->ParentEntity;
						if (parentEntity.IsValid())
						{
							STransformComponent* parentTransform = scene->GetComponent<STransformComponent>(parentEntity);
							if (SComponent::IsValid(parentTransform))
							{
								const SMetaDataComponent* parentMetaDataComp = scene->GetComponent<SMetaDataComponent>(parentEntity);
								const std::string parentEntityName = SComponent::IsValid(parentMetaDataComp) ? parentMetaDataComp->Name.AsString() : "UNNAMED";
								GUI::SetTooltip("Detach %s from %s?", draggedEntityName.c_str(), parentEntityName.c_str());

								if (payload.IsDelivery)
									parentTransform->Detach(draggedTransform);
							}
						}
					}
				}

				GUI::EndDragDropTarget();
			}

			if (GUI::BeginPopupContextWindow())
			{
				if (GUI::MenuItem("Create New Entity"))
				{
					CScene* scenePointer = scene.get();
					std::string newEntityName = UGeneralUtils::GetNonCollidingString("NewEntity", scenePointer->Entities, [scenePointer](const SEntity& entity)
						{
							const SMetaDataComponent* metaDataComp = scenePointer->GetComponent<SMetaDataComponent>(entity);
							return SComponent::IsValid(metaDataComp) ? metaDataComp->Name.AsString() : "UNNAMED";
						}
					);
					scene->AddEntity(newEntityName);
				}

				if (GUI::MenuItem("Remove Scene"))
					editData.QueuedRemovalIndex = STATIC_I64(sceneIndex);

				GUI::EndPopup();
			}

			GUI::Separator();
		}
		GUI::EndChild();
		SceneAssetDrag();
	}

	void CHierarchyWindow::Footer(std::vector<Ptr<CScene>>& scenes, SEditData& editData)
	{
		// TODO.NW: Center align this. Wrap it in a world function call?
		GUI::BeginChild("CreateButton", SVector2<F32>(0.0f, 30.0f));
		GUI::Separator();
		if (!scenes.empty())
		{
			if (GUI::Button("Add Empty Scene"))
			{
				std::string newSceneName = UGeneralUtils::GetNonCollidingString("NewScene", scenes, [](const Ptr<CScene>& scene) { return scene->GetSceneName(); });

				GEngine::GetWorld()->CreateScene<CGameScene>();
				CScene* activeScene = Manager->GetScenes().back().get();
				activeScene->Init(newSceneName);
				Manager->SetCurrentScene(STATIC_I64(Manager->GetScenes().size()) - 1);
			}
			GUI::SameLine();
			if (GUI::Button("Clear Scenes"))
			{
				editData.QueuedRemovalIndex = -1;
				GEngine::GetWorld()->ClearScenes();
				Manager->SetCurrentScene(-1);
			}
		}
		else
		{
			if (GUI::Button("Create New Scene"))
			{
				std::string newSceneName = UGeneralUtils::GetNonCollidingString("NewScene", scenes, [](const Ptr<CScene>& scene) { return scene->GetSceneName(); });

				GEngine::GetWorld()->CreateScene<CGameScene>();
				CScene* activeScene = Manager->GetScenes().back().get();
				activeScene->Init(newSceneName);
				activeScene->Init3DDefaults();
				Manager->SetCurrentScene(STATIC_I64(Manager->GetScenes().size()) - 1);
			}
		}
		GUI::EndChild();
		SceneAssetDrag();
	}

	void CHierarchyWindow::SceneAssetDrag()
	{
		// Asset drop
		if (GUI::BeginDragDropTarget())
		{
			SGuiPayload payload = GUI::AcceptDragDropPayload("AssetDrag", { EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNoDrawDefaultRect, EDragDropFlag::AcceptNopreviewTooltip });
			if (payload.Data != nullptr)
			{
				SEditorAssetRepresentation* payloadAssetRep = reinterpret_cast<SEditorAssetRepresentation*>(payload.Data);

				if (payloadAssetRep->AssetType == EAssetType::Scene)
					GUI::SetTooltip("Add Scene?");

				if (payload.IsDelivery)
				{
					GEngine::GetWorld()->AddScene<CGameScene>(payloadAssetRep->DirectoryEntry.path().string());
					Manager->SetCurrentScene(STATIC_I64(Manager->GetScenes().size()) - 1);
				}
			}

			GUI::EndDragDropTarget();
		}
	}

	void CHierarchyWindow::Edit(const SEditData& editData)
	{
		if (editData.QueuedRemovalIndex >= 0)
		{
			GEngine::GetWorld()->RemoveScene(editData.QueuedRemovalIndex);
			std::vector<Ptr<CScene>>& remainingScenes = Manager->GetScenes();
			if (!remainingScenes.empty())
			{
				Manager->SetCurrentScene(0);
			}
		}

		if (editData.DoubleClickIndex >= 0)
		{
			std::vector<Ptr<CScene>>& remainingScenes = Manager->GetScenes();
			I64 sceneIndex = UMath::Clamp(editData.DoubleClickIndex, STATIC_I64(0), STATIC_I64(remainingScenes.size()) - 1);
			Manager->SetCurrentScene(sceneIndex);
		}
	}
}
