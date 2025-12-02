// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "InspectorWindow.h"

#include <ECS/ECSInclude.h>
#include <ECS/ComponentAlgo.h>
#include <Engine.h>
#include <EditorManager.h>
#include <Graphics/RenderManager.h>
#include <Graphics/TextureBank.h>
#include <Scene/Scene.h>
#include <Assets/AssetRegistry.h>
#include <GeneralUtilities.h>

#include "Windows/ViewportWindow.h"
#include "Windows/SpriteAnimatorGraphNodeWindow.h"
#include "EditorResourceManager.h"

#include <FileSystem.h>
#include <GUI.h>

namespace Havtorn
{
	CInspectorWindow::CInspectorWindow(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
	{}

	CInspectorWindow::~CInspectorWindow()
	{
	}

	void CInspectorWindow::OnEnable()
	{
	}

	void CInspectorWindow::OnInspectorGUI()
	{
		const SEditorLayout& layout = Manager->GetEditorLayout();

		const SVector2<F32>& viewportWorkPos = GUI::GetViewportWorkPos();
		GUI::SetNextWindowPos(SVector2<F32>(viewportWorkPos.X + layout.InspectorPosition.X, viewportWorkPos.Y + layout.InspectorPosition.Y));
		GUI::SetNextWindowSize(SVector2<F32>(layout.InspectorSize.X, layout.InspectorSize.Y));

		if (!GUI::Begin(Name(), nullptr, { EWindowFlag::NoMove, EWindowFlag::NoResize, EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus }))
		{
			GUI::End();
			return;
		}

		std::vector<SEntity> selectedEntities = Manager->GetSelectedEntities();

		if (selectedEntities.empty())
		{
			GUI::TextDisabled("Select an Entity to modify");
			GUI::End();
			return;
		}

		const std::vector<Ptr<CScene>>& scenes = GEngine::GetWorld()->GetActiveScenes();

		// TODO.NW: Go through and make sure everything in the inspector gets unique ID, maybe based on entity GUID. 
		// Don't want same IDs over a frame when multiple entities are selected
		for (const SEntity& selectedEntity : selectedEntities)
		{
			if (GUI::BeginDragDropSource())
			{
				SGuiPayload payload = GUI::GetDragDropPayload();
				if (payload.Data == nullptr)
				{
					GUI::SetDragDropPayload("EntityDrag", &selectedEntity, sizeof(SEntity));
				}
				GUI::Text("entityName");

				GUI::EndDragDropSource();
			}

			CScene* currentScene = UComponentAlgo::GetContainingScene(selectedEntity, scenes);
			if (currentScene == nullptr)
			{
				GUI::TextDisabled("Could not find scene of selected entity");
				continue;
			}

			SMetaDataComponent* metaDataComp = currentScene->GetComponent<SMetaDataComponent>(selectedEntity);
			if (metaDataComp != nullptr)
			{
				GUI::InputText("##MetaDataCompName", &metaDataComp->Name);
				GUI::SameLine();
				GUI::TextDisabled("GUID %u", metaDataComp->Owner.GUID);
				if (GUI::IsItemHovered())
					GUI::SetTooltip("GUID %u", metaDataComp->Owner.GUID);
			}

			for (SComponentEditorContext* context : currentScene->GetComponentEditorContexts(selectedEntity))
			{
				GUI::Separator();

				if (context->RemoveComponent(selectedEntity, currentScene))
					continue;

				GUI::SameLine();
				SComponentViewResult result = context->View(selectedEntity, currentScene);

				// TODO.NR: Could make this a enum-function map, but would be good to set up clear rules for how this should work.
				switch (result.Label)
				{
				case EComponentViewResultLabel::UpdateTransformGizmo:
					UpdateTransformGizmo(result);
					break;
				case EComponentViewResultLabel::InspectAssetComponent:
					InspectAssetComponent(result);
					break;
				case EComponentViewResultLabel::OpenAssetTool:
					OpenAssetTool(result);
					break;
				case EComponentViewResultLabel::RenderPreview:
					RenderPreview(result);
					break;
				case EComponentViewResultLabel::PassThrough:
				default:
					break;
				}

				GUI::Dummy({ GUI::DummySizeX, GUI::DummySizeY });
			}

			UpdateAssetContextMenu();

			GUI::Separator();
			if (GUI::Button("Add Component", SVector2<F32>(GUI::GetContentRegionAvail().X, 0)))
				GUI::OpenPopup("Add Component Modal");

			OpenAddComponentModal(selectedEntity);
		}

		GUI::End();
	}

	void CInspectorWindow::OnDisable()
	{
	}

	void CInspectorWindow::UpdateTransformGizmo(const SComponentViewResult& result)
	{
		if (Manager->GetIsFreeCamActive())
			return;

		STransformComponent* viewedTransformComp = static_cast<STransformComponent*>(result.ComponentViewed);
		if (viewedTransformComp == nullptr)
			return;

		CWorld* world = GEngine::GetWorld();
		SEntity mainCamera = world->GetMainCamera();
		SCameraData mainCameraData = UComponentAlgo::GetCameraData(mainCamera, world->GetActiveScenes());

		if (!mainCameraData.IsValid())
			return;

		CViewportWindow* viewportWindow = Manager->GetEditorWindow<CViewportWindow>();
		SVector2<F32> viewportWindowDimensions = viewportWindow->GetRenderedSceneDimensions();
		SVector2<F32> viewportWindowPosition = viewportWindow->GetRenderedScenePosition();

		GUI::SetRect(viewportWindowPosition, viewportWindowDimensions);
		
		SMatrix viewMatrix = mainCameraData.TransformComponent->Transform.GetMatrix();
		SMatrix inverseView = viewMatrix.Inverse();

		ViewManipulation(viewMatrix, viewportWindowPosition, viewportWindowDimensions);

		CScene* currentScene = Manager->GetCurrentWorkingScene();
		if (Manager->GetIsDragCopyActive() && viewedTransformComp->Owner != currentScene->CopiedEntity && DeltaMatrix != SMatrix::Identity)
		{
			if (currentScene != nullptr && !currentScene->CopiedEntity.IsValid())
			{
				// TODO.NW: Make multi copy?
				SEntity newEntity = currentScene->CopyEntity(viewedTransformComp->Owner);
				Manager->SetSelectedEntity(newEntity);
				currentScene->CopiedEntity = newEntity;
			}
		}

		if (!Manager->GetIsDragCopyActive() && currentScene != nullptr)
			currentScene->CopiedEntity = SEntity::Null;

		SMatrix transformMatrix = viewedTransformComp->Transform.GetMatrix();

		GUI::PushID(0);
		// NW: We can choose GetSelectedEntity (which returns the first selected entity) to base the gizmo on if we want. 
		// I think it feels nicer to get the gizmo on the latest selected entity though, even though it will apply the 
		// delta matrix one frame later on all the other entities. This probably doesn't matter in editor.
		if (viewedTransformComp->Owner == Manager->GetSelectedEntities().back())
		{
			SVector gizmoSnapping = Manager->GetCurrentGizmoSnapping().Snapping;
			F32 snappingData[] = { gizmoSnapping.X, gizmoSnapping.Y, gizmoSnapping.Z };
			GUI::GizmoManipulate(inverseView.data, mainCameraData.CameraComponent->ProjectionMatrix.data, Manager->GetCurrentGizmo(), Manager->GetCurrentGizmoSpace(), transformMatrix.data, DeltaMatrix.data, snappingData);
		}
		else
		{
			transformMatrix *= DeltaMatrix;
		}
		viewedTransformComp->Transform.SetMatrix(transformMatrix);
		
		GUI::PopID();
		
		
		mainCameraData.TransformComponent->Transform.SetMatrix(viewMatrix);
	}

	void CInspectorWindow::ViewManipulation(SMatrix& outCameraView, const SVector2<F32>& windowPosition, const SVector2<F32>& windowSize)
	{
		const F32 viewManipulateRight = windowPosition.X + windowSize.X;
		const F32 viewManipulateTop = windowPosition.Y + 28.0f; // TODO.NR: Figure out all the menu bar sizes and stuff

		// TODO.NR: Focus on object, maybe make use of Focus function. Maybe add support for local and world View manipulation.

		const F32 camDistance = 1.0f;
		const F32 size = 128.0f;
		const U32 backgroundColor = 0x10101010;
		GUI::ViewManipulate(outCameraView.data, camDistance, SVector2<F32>(viewManipulateRight - size, viewManipulateTop), SVector2<F32>(size, size), SColor::FromPackedU32(backgroundColor));
	}

	void CInspectorWindow::InspectAssetComponent(SComponentViewResult& result)
	{
		std::vector<std::string> assetNames = {};

		if (result.AssetReferences != nullptr)
		{
			for (const SAssetReference& ref : *result.AssetReferences)
				assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(ref.FilePath));
		}
		if (result.AssetReference != nullptr)
		{
			assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(result.AssetReference->FilePath));
		}

		for (U8 index = 0; index < static_cast<U8>(assetNames.size()); index++)
		{
			std::string assetName = assetNames[index];

			SEditorAssetRepresentation* assetRep = Manager->GetAssetRepFromName(assetName).get();
			GUI::Separator();

			std::string id = assetName;
			id.append(std::to_string(index));
			GUI::PushID(id.c_str());

			const F32 thumbnailPadding = 4.0f;
			const F32 cellWidth = GUI::TexturePreviewSizeX * 0.75f + thumbnailPadding;
			const F32 panelWidth = 256.0f;
			const I32 columnCount = static_cast<I32>(panelWidth / cellWidth);
			const std::string modalName = "Select " + GetAssetTypeName(result.AssetType) + " Asset";
			SAssetPickResult assetPickResult = GUI::AssetPickerFilter(assetName.c_str(), modalName.c_str(), (intptr_t)assetRep->TextureRef.GetShaderResourceView(), "Assets", columnCount, Manager->GetAssetFilteredInspectFunction(), result.AssetType);

			SAssetReference* currentReference = nullptr;
			if (result.AssetReferences != nullptr)
			{
				currentReference = &(*result.AssetReferences)[AssetPickedIndex];
			}
			else if (result.AssetReference != nullptr)
			{
				currentReference = result.AssetReference;
			}

			if (assetPickResult.State == EAssetPickerState::Active)
				Manager->SetIsModalOpen(true);
			else if (assetPickResult.State == EAssetPickerState::Cancelled)
				Manager->SetIsModalOpen(false);
			else if (assetPickResult.State == EAssetPickerState::ContextMenu)
			{
				ContextMenuAssetRef = currentReference;
				ContextMenuAssetRequester = result.ComponentViewed->Owner.GUID;

				Manager->SetIsModalOpen(false);
			}
			else if (assetPickResult.State == EAssetPickerState::AssetPicked)
			{
				AssetPickedIndex = index;
				auto pickedAsset = Manager->GetAssetRepFromDirEntry(assetPickResult.PickedEntry).get();
				std::string newAssetPath = UGeneralUtils::ConvertToPlatformAgnosticPath(pickedAsset->DirectoryEntry.path().string());

				if (result.AssetReferences != nullptr)
				{
					std::vector<std::string> paths = SAssetReference::GetPaths(*result.AssetReferences);
					
					if (paths[AssetPickedIndex] != newAssetPath)
						GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(paths[AssetPickedIndex]), result.ComponentViewed->Owner.GUID);
					
					paths[AssetPickedIndex] = newAssetPath;
					*result.AssetReferences = SAssetReference::MakeVectorFromPaths(paths);
				}
				if (result.AssetReference != nullptr)
				{
					if (result.AssetReference->FilePath != newAssetPath)
						GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(result.AssetReference->FilePath), result.ComponentViewed->Owner.GUID);

					*result.AssetReference = SAssetReference(newAssetPath);
				}

				AssetPickedIndex = 0;
				Manager->SetIsModalOpen(false);
			}



			GUI::PopID();
		}
	}

	void CInspectorWindow::OpenAssetTool(const SComponentViewResult& result)
	{
		SSpriteAnimatorGraphComponent* component = static_cast<SSpriteAnimatorGraphComponent*>(result.ComponentViewed);
		if (component == nullptr)
			return;

		Manager->GetEditorWindow<CSpriteAnimatorGraphNodeWindow>()->Inspect(*component);
	}

	void CInspectorWindow::RenderPreview(const SComponentViewResult& result)
	{
		GUI::TextDisabled("Preview");
		GUI::Separator();

		const SEditorLayout& layout = Manager->GetEditorLayout();
		// TODO.NW: Centralize layout padding for stuff like this
		const F32 previewWidth = layout.InspectorSize.X - 16.0f;
		CRenderTexture* previewRenderTexture = Manager->GetRenderManager()->GetRenderTargetTexture(result.ComponentViewed->Owner.GUID);
		if (previewRenderTexture != nullptr)
			GUI::Image((intptr_t)previewRenderTexture->GetShaderResourceView(), SVector2<F32>(previewWidth, previewWidth * (9.0f / 16.0f)));
		
		GUI::Separator();
	}

	void CInspectorWindow::OpenAddComponentModal(const SEntity& entity)
	{
		if (!GUI::BeginPopupModal("Add Component Modal", NULL, { EWindowFlag::AlwaysAutoResize }))
			return;

		Manager->SetIsModalOpen(true);
		CScene* currentScene = UComponentAlgo::GetContainingScene(entity, GEngine::GetWorld()->GetActiveScenes());
		if (currentScene == nullptr)
			return;

		if (GUI::BeginTable("NewComponentTypeTable", 1))
		{
			for (const SComponentEditorContext* context : currentScene->GetComponentEditorContexts())
			{
				GUI::TableNextColumn();

				if (context->AddComponent(entity, currentScene))
				{
					Manager->SetIsModalOpen(false);
					GUI::CloseCurrentPopup();
				}
			}

			GUI::EndTable();
		}

		if (GUI::Button("Cancel", SVector2<F32>(GUI::GetContentRegionAvail().X, 0))) 
		{
			Manager->SetIsModalOpen(false);
			GUI::CloseCurrentPopup(); 
		}

		GUI::EndPopup();
	}

	void CInspectorWindow::UpdateAssetContextMenu()
	{
		if (ContextMenuAssetRef == nullptr)
			return;

		if (GUI::BeginPopupContextWindow())
		{
			if (GUI::MenuItem("Fix up redirectors"))
			{
				std::string oldPath = ContextMenuAssetRef->FilePath;

				CJsonDocument config = UFileSystem::OpenJson(UFileSystem::EngineConfig);
				std::string redirection = config.GetValueFromArray("Asset Redirectors", ContextMenuAssetRef->FilePath, "");

				GEngine::GetAssetRegistry()->UnrequestAsset(*ContextMenuAssetRef, ContextMenuAssetRequester);
				*ContextMenuAssetRef = SAssetReference(redirection);

				HV_LOG_INFO("Asset Redirector from %s to %s fixed.", oldPath.c_str(), ContextMenuAssetRef->FilePath.c_str());
				ContextMenuAssetRef = nullptr;
				ContextMenuAssetRequester = 0;
			}

			// TODO.NW: This is terrible. It would be nice if we could make a common solution for bringing up smart context menus for specific selectables
			// This is needed for the Asset Browser as well (IsContextMenuRefHovered is inspired by the Asset Browser solution)
			if (!GUI::IsItemClicked() && !IsContextMenuRefHovered && (GUI::IsMouseClicked(0) || GUI::IsMouseClicked(1)))
			{
				ContextMenuAssetRef = nullptr;
				ContextMenuAssetRequester = 0;
			}

			GUI::EndPopup();
		}		
	}
}
