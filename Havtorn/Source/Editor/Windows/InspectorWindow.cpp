// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "InspectorWindow.h"

#include <ECS/ECSInclude.h>
#include <ECS/ComponentAlgo.h>
#include <Engine.h>
#include <EditorManager.h>
#include <Graphics/RenderManager.h>
#include <Scene/Scene.h>
#include <Assets/AssetRegistry.h>
#include <GeneralUtilities.h>
#include <Graphics/Debug/DebugDrawUtility.h>

#include "Windows/ViewportWindow.h"
#include "Windows/HierarchyWindow.h"
#include "Windows/AssetBrowserWindow.h"
#include "AuthoringTools/PrefabTool.h"
#include "Windows/SpriteAnimatorGraphNodeWindow.h"
#include "EditorResourceManager.h"

#include "EditActions/MoveTransformEditAction.h"

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
		SEditorLayout& layout = Manager->GetEditorLayout();

		const SVector2<F32> layoutPosition = SVector2<F32>(layout.InspectorPosition.X, layout.InspectorPosition.Y);
		const SVector2<F32> layoutSize = SVector2<F32>(layout.InspectorSize.X, layout.InspectorSize.Y);
		GUI::SetNextWindowPos(layoutPosition);
		GUI::SetNextWindowSize(layoutSize);

		if (!GUI::Begin(Name(), nullptr, { EWindowFlag::NoMove, EWindowFlag::NoResize, EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus }))
		{
			GUI::End();
			return;
		}

		IsHovered = IsEnabled && GUI::IsWindowHovered();

		std::vector<SEntity> selectedEntities = Manager->GetSelectedEntities();

		if (selectedEntities.empty())
		{
			GUI::TextDisabled("Select an Entity to modify");
			GUI::End();
			return;
		}

		const std::vector<Ptr<CScene>>& worldScenes = GEngine::GetWorld()->GetActiveScenes();

		// TODO.NW: Go through and make sure everything in the inspector gets unique ID, maybe based on entity GUID. 
		// Don't want same IDs over a frame when multiple entities are selected
		for (const SEntity& selectedEntity : selectedEntities)
		{
			// NW: The main inspector logic does not care what gets inspected outside of the world scenes
			CScene* currentScene = UComponentAlgo::GetContainingScene(selectedEntity, worldScenes);

			InspectEntity(selectedEntity, currentScene);
		}

		const SVector2<F32> newPosition = GUI::GetWindowPos();
		const SVector2<F32> newSize = GUI::GetWindowSize();
		if (layoutPosition != newPosition || layoutPosition != newSize)
		{
			layout.InspectorPosition.X = STATIC_I16(newPosition.X);
			layout.InspectorPosition.Y = STATIC_I16(newPosition.Y);
			layout.InspectorSize.X = STATIC_U16(newSize.X);
			layout.InspectorSize.Y = STATIC_U16(newSize.Y);
			layout.InspectorChanged = true;
		}

		GUI::End();
	}

	void CInspectorWindow::OnDisable()
	{
	}

	void CInspectorWindow::InspectEntity(const SEntity& entity, CScene* owningScene)
	{
		if (owningScene == nullptr || !owningScene->HasEntity(entity.GUID))
		{
			GUI::TextDisabled("Could not find scene of selected entity");
			return;
		}

		SMetaDataComponent* metaDataComp = owningScene->GetComponent<SMetaDataComponent>(entity);
		if (metaDataComp != nullptr)
		{
			GUI::InputText("##MetaDataCompName", &metaDataComp->Name);
			GUI::SameLine();
			GUI::TextDisabled("GUID %llu", metaDataComp->Owner.GUID);
			if (GUI::IsItemHovered())
				GUI::SetTooltip("GUID %llu", metaDataComp->Owner.GUID);

			CEditorManager::EntityDragData.TrySet(metaDataComp->Owner, metaDataComp->Name.AsString().c_str(), { EDragDropFlag::SourceAllowNullID });
		}

		for (const U64& runtimeHash : owningScene->EntityComponentRuntimeHashes.at(entity.GUID))
		{
			const std::map<U64, Ptr<SComponentView>>& componentViewsMap = Manager->GetComponentViewsMap();
			if (!componentViewsMap.contains(runtimeHash))
				continue;

			const Ptr<SComponentView>& componentView = componentViewsMap.at(runtimeHash);
			GUI::Separator();

			GUI::PushID(componentView->GetComponentName());
			if (GUI::Button("X"))
			{
				if (owningScene != nullptr && entity.IsValid())
				{
					owningScene->ComponentSerializers.at(owningScene->TypeHashToTypeID.at(runtimeHash)).ComponentRemover(entity, owningScene);
					GUI::PopID();
					continue;
				}
			}
			GUI::PopID();

			GUI::SameLine();

			if (!GUI::TryOpenComponentView(componentView->GetComponentName()))
			{
				GUI::Dummy({ GUI::DummySizeX, GUI::DummySizeY });
				continue;
			}

			componentView->View(entity, owningScene);

			GUI::Dummy({ GUI::DummySizeX, GUI::DummySizeY });
		}
		UpdateAssetContextMenu();

		GUI::Separator();

		AddComponentPopup(entity, owningScene);
	}

	void CInspectorWindow::UpdateTransformGizmo(STransformComponent* viewedTransformComp)
	{
		if (Manager->GetIsFreeCamActive() || Manager->GetRenderManager()->GetRenderPass() == ERenderPass::Game)
			return;

		if (!SComponent::IsValid(viewedTransformComp))
			return;

		CWorld* world = GEngine::GetWorld();
		const SEntity mainCamera = world->GetMainCamera();

		SMatrix viewMatrix = SMatrix::Identity;
		SMatrix projectionMatrix = SMatrix::Identity;

		CScene* currentScene = Manager->GetContainingScene(viewedTransformComp->Owner);
		const CPrefabTool* prefabTool = Manager->GetEditorWindow<CPrefabTool>();
		const bool workingInPrefabScene = prefabTool != nullptr && prefabTool->GetWorkingScene() == currentScene;
		SCameraData mainCameraData = UComponentAlgo::GetCameraData(mainCamera, world->GetActiveScenes());
		
		if (workingInPrefabScene)
		{
			viewMatrix = prefabTool->GetViewMatrix();
			projectionMatrix = prefabTool->GetProjectionMatrix();
			GUI::SetGizmoRect(prefabTool->GetPreviewWindowPosition(), prefabTool->GetPreviewWindowDimensions());
		}
		else if (mainCameraData.IsValid())
		{
			viewMatrix = mainCameraData.TransformComponent->Transform.GetMatrix();
			projectionMatrix = mainCameraData.CameraComponent->ProjectionMatrix;

			CViewportWindow* viewportWindow = Manager->GetEditorWindow<CViewportWindow>();
			SVector2<F32> viewportWindowDimensions = viewportWindow->GetRenderedSceneDimensions();
			SVector2<F32> viewportWindowPosition = viewportWindow->GetRenderedScenePosition();

			GUI::SetGizmoRect(viewportWindowPosition, viewportWindowDimensions);
			ViewManipulation(viewMatrix, viewportWindowPosition, viewportWindowDimensions);
		}
		else
		{
			return;
		}

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
			SMatrix originalTransform = transformMatrix;

			if (Manager->GetIsPivotMovingActive())
			{
				// Move pivot
				CViewportWindow* viewport = Manager->GetEditorWindow<CViewportWindow>();
				const SVector vertexWorldPosition = viewport->GetClosestVertexPositionOnPixel(viewedTransformComp->Owner);
				PivotWorldSpace = vertexWorldPosition;
				PivotOffset = PivotWorldSpace - originalTransform.GetTranslation();
			}
			else if (!Manager->GetIsPivotOffsetSet())
			{
				// No offset pivot
				PivotWorldSpace = originalTransform.GetTranslation();
				PivotOffset = SVector::Zero;
			}

			SMatrix gizmoMatrix = transformMatrix;
			gizmoMatrix.SetTranslation(PivotWorldSpace);

			const SVector gizmoSnapping = Manager->GetCurrentGizmoSnapping().Snapping;
			const F32 snappingData[] = { gizmoSnapping.X, gizmoSnapping.Y, gizmoSnapping.Z };
			GUI::GizmoManipulate(viewMatrix.Inverse().data, projectionMatrix.data, Manager->GetCurrentGizmo(), Manager->GetCurrentGizmoSpace(), gizmoMatrix.data, DeltaMatrix.data, snappingData);

			// TODO.NW: Do this generally, and make links for all selected entities
			IsUsingGizmo = GUI::IsUsingGizmo();
			if (IsUsingGizmo && !WasUsingGizmo)
			{
				// Start accumulating delta
				FullDeltaMatrix = SMatrix::Identity;

				InitialTranslation = originalTransform.GetTranslation();
				InitialRotation = SQuaternion(originalTransform.GetRotationMatrix());
				
				// NW: Scale is not additive
				const SVector scale = originalTransform.GetScale();
				FullDeltaMatrix.SetScale(scale);
				
				PivotOffset = InitialRotation * PivotOffset * scale.Reciprocal();
				InitialOffset = PivotOffset;
			}

			if (IsUsingGizmo)
				FullDeltaMatrix *= DeltaMatrix;

			if (IsUsingGizmo)
			{
				// TODO.NW: Let quats be the ground truth of rotation. Move away from Matrices in Transforms and cache them only for sending to the GPU. Transforms should be scale+quat+trans

				const SMatrix rotationMatrix = FullDeltaMatrix.GetRotationMatrix();
				const SQuaternion newRotation = (SQuaternion(rotationMatrix) * InitialRotation).Inverse();
				PivotOffset = newRotation * InitialOffset * FullDeltaMatrix.GetScale();

				ETransformGizmo gizmo = Manager->GetCurrentGizmo();
				if (gizmo == ETransformGizmo::Translate)
				{
					PivotWorldSpace = InitialTranslation + FullDeltaMatrix.GetTranslation() + PivotOffset;

					if (Manager->GetIsVertexSnappingActive())
						PivotWorldSpace = VertexSnap(PivotWorldSpace, viewedTransformComp->Owner);
					else if (Manager->GetIsGridSnappingActive())
						PivotWorldSpace = GridSnap(PivotWorldSpace);

					transformMatrix.SetTranslation(PivotWorldSpace - PivotOffset);
				}
				else if (gizmo == ETransformGizmo::Rotate)
				{
					const SVector newPosition = PivotWorldSpace - PivotOffset;
					SMatrix::Recompose(newPosition, newRotation, transformMatrix.GetScale(), transformMatrix);
				}
				else if (gizmo == ETransformGizmo::Scale)
				{
					SMatrix::Recompose(transformMatrix.GetTranslation(), newRotation, FullDeltaMatrix.GetScale(), transformMatrix);
				}
			}

			if (Manager->GetIsGridSnappingActive())
			{
				const SVector pos = transformMatrix.GetTranslation() + PivotOffset;
				const SVector roundedPos = { UMath::Round(pos.X, 1.0f), UMath::Round(pos.Y, 1.0f), UMath::Round(pos.Z, 1.0f) };
				GDebugDraw::AddGrid(roundedPos, SVector::Zero, SColor::Grey, -1.0f, false, 0.005f, false);
			}
		}
		else
		{
			transformMatrix *= DeltaMatrix;
		}
		viewedTransformComp->Transform.SetMatrix(transformMatrix);

		GUI::PopID();

		if (!IsUsingGizmo && WasUsingGizmo && !FullDeltaMatrix.NearlyEqual(SMatrix::Identity))
		{
			PivotWorldSpace = transformMatrix.GetTranslation() + PivotOffset;
			UMetaCommandRouter::Push(SMoveTransformEditAction::MakeEditActionCommand(Manager, viewedTransformComp, FullDeltaMatrix));
		}

		WasUsingGizmo = IsUsingGizmo;

		if (mainCameraData.IsValid() && !workingInPrefabScene)
			mainCameraData.TransformComponent->Transform.SetMatrix(viewMatrix);
	}

	SVector CInspectorWindow::VertexSnap(const SVector& pivotPosition, const SEntity& viewedEntity)
	{
		CViewportWindow* viewport = Manager->GetEditorWindow<CViewportWindow>();
		SEntity hoveredEntity = viewport->GetEntityOnPixel();
		if (!hoveredEntity.IsValid() || hoveredEntity == viewedEntity)
			return pivotPosition;

		SVector vertexPos = viewport->GetClosestVertexPositionOnPixel(hoveredEntity);
		if (vertexPos.IsEqual(SVector::Zero))
			return pivotPosition;

		return vertexPos;
	}

	SVector CInspectorWindow::GridSnap(const SVector& pivotPosition)
	{
		const SVector snapping = Manager->GetCurrentGizmoSnapping().Snapping;
		return { UMath::Round(pivotPosition.X, snapping.X), UMath::Round(pivotPosition.Y, snapping.Y), UMath::Round(pivotPosition.Z, snapping.Z) };
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

	void CInspectorWindow::InspectAssetComponent(SComponent* viewedComponent, const EAssetType assetType, std::vector<SAssetReference*> assetReferences)
	{
		std::vector<std::string> assetNames = {};

		for (const SAssetReference* ref : assetReferences)
			assetNames.push_back(UGeneralUtils::ExtractFileBaseNameFromPath(ref->FilePath));

		for (U8 index = 0; index < static_cast<U8>(assetNames.size()); index++)
		{
			std::string assetName = assetNames[index];

			const Ptr<SEditorAssetRepresentation>& assetRep = Manager->GetAssetRepFromName(assetName);
			GUI::Separator();

			std::string id = assetName;
			id.append(std::to_string(index));
			GUI::PushID(id.c_str());

			SAssetPickResult assetPickResult = Manager->AssetPickerDropdown(assetName.c_str(), assetType, assetRep.get());
			SAssetReference* currentReference = (assetReferences)[AssetPickedIndex];

			if (assetPickResult.State == EAssetPickerState::Active)
				Manager->SetIsModalOpen(true);
			else if (assetPickResult.State == EAssetPickerState::Cancelled)
				Manager->SetIsModalOpen(false);
			else if (assetPickResult.State == EAssetPickerState::ContextMenu)
			{
				ContextMenuAssetRef = currentReference;
				ContextMenuAssetRequester = viewedComponent->Owner.GUID;

				Manager->SetIsModalOpen(false);
			}
			else if (assetPickResult.State == EAssetPickerState::AssetPicked || assetPickResult.State == EAssetPickerState::GetFromSelected)
			{
				AssetPickedIndex = index;

				SEditorAssetRepresentation* pickedAsset = nullptr;
				if (assetPickResult.State == EAssetPickerState::AssetPicked)
					pickedAsset = Manager->GetAssetRepFromDirEntry(assetPickResult.PickedEntry).get();
				else if (assetPickResult.State == EAssetPickerState::GetFromSelected)
				{
					SEditorAssetRepresentation* selectedAssetInBrowser = Manager->GetSelectedAsset();
					if (selectedAssetInBrowser != nullptr && (selectedAssetInBrowser->AssetType == assetRep->AssetType || selectedAssetInBrowser->AssetType == assetType))
						pickedAsset = Manager->GetSelectedAsset();
				}

				if (pickedAsset != nullptr)
				{
					ReassignAssetRef(viewedComponent->Owner.GUID, assetReferences, AssetPickedIndex, pickedAsset->DirectoryEntry.path().string());
				}

				AssetPickedIndex = 0;
				Manager->SetIsModalOpen(false);
			}
			else if (assetPickResult.State == EAssetPickerState::FindInBrowser)
			{
				Manager->GetEditorWindow<CAssetBrowserWindow>()->BrowseTo(assetRep.get());
			}
			else if (assetPickResult.State == EAssetPickerState::Inactive)
			{
				Manager->SetIsModalOpen(false);
				
				auto result = CEditorManager::AssetDragData.TryDeliver({ EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNopreviewTooltip });
				if (result.Payload != nullptr)
				{
					SEditorAssetRepresentation* payloadAssetRep = result.Payload;
					if (payloadAssetRep->AssetType == assetType)
					{
						GUI::SetTooltip("Assign %s?", payloadAssetRep->Name.c_str());

						if (result.Result == EDragDeliverResult::Delivered)
						{
							ReassignAssetRef(viewedComponent->Owner.GUID, assetReferences, index, payloadAssetRep->DirectoryEntry.path().string());
						}
					}
					else
					{
						GUI::SetTooltip("Invalid asset type: '%s'", GetAssetTypeName(payloadAssetRep->AssetType).c_str());
					}
				}
			}

			GUI::PopID();
		}
	}

	void CInspectorWindow::OpenAssetTool(SComponent* viewedComponent)
	{
		SSpriteAnimatorGraphComponent* component = static_cast<SSpriteAnimatorGraphComponent*>(viewedComponent);
		if (component == nullptr)
			return;

		Manager->GetEditorWindow<CSpriteAnimatorGraphNodeWindow>()->Inspect(*component);
	}

	void CInspectorWindow::RenderPreview(const SComponent* viewedComponent)
	{
		GUI::TextDisabled("Preview");
		GUI::Separator();

		const SEditorLayout& layout = Manager->GetEditorLayout();
		// TODO.NW: Centralize layout padding for stuff like this
		const F32 previewWidth = layout.InspectorSize.X - 16.0f;
		CRenderTexture* previewRenderTexture = Manager->GetRenderManager()->GetRenderTargetTexture(viewedComponent->Owner.GUID);
		if (previewRenderTexture != nullptr)
			GUI::Image((intptr_t)previewRenderTexture->GetShaderResourceView(), SVector2<F32>(previewWidth, previewWidth * (9.0f / 16.0f)));
		
		GUI::Separator();
	}

	void CInspectorWindow::AddComponentPopup(const SEntity& entity, CScene* owningScene)
	{
		if (owningScene == nullptr || !entity.IsValid())
			return;

		if (GUI::Button("Add Component", SVector2<F32>(GUI::GetContentRegionAvail().X, 0)))
			GUI::OpenPopup("Add Component Popup");

		if (!GUI::BeginPopup("Add Component Popup"))
		{
			ComponentFilter = SGuiTextFilter();
			return;
		}

		ComponentFilter.Draw("Search", 0); // TODO.NW: Figure out a nicer way of setting the width

		if (GUI::BeginChild("NewComponentTypeTable", SVector2<F32>(0.0f, 200.0f)))
		{
			for (const Ptr<SComponentView>& view : Manager->GetComponentViewsVector())
			{
				const U64 viewHash = view->GetRuntimeHash();
				const std::vector<U64>& existingComponentHashes = owningScene->EntityComponentRuntimeHashes.at(entity.GUID);
				
				bool hasComponent = false;
				for (const U64& componentHash : existingComponentHashes)
				{
					if (viewHash == componentHash)
					{
						hasComponent = true;
						break;
					}
				}

				// TODO.NW: Should make a choice here whether to allow multiple components of the same type, 
				// or continue working under the assumption that every component can handle all the data it needs for 
				// its owning entity.
				if (hasComponent)
					continue;
				
				const char* newComponentName = view->GetComponentName();

				if (!ComponentFilter.PassFilter(newComponentName))
					continue;

				if (GUI::Selectable(newComponentName))
				{
					// Add component dependencies if needed
					for (const U64& dependencyHash : Manager->GetComponentDependencies(viewHash))
					{
						if (std::ranges::find(existingComponentHashes, dependencyHash) == existingComponentHashes.end())
							owningScene->ComponentSerializers.at(owningScene->TypeHashToTypeID.at(dependencyHash)).ComponentAdder(entity, owningScene);
					}

					owningScene->ComponentSerializers.at(owningScene->TypeHashToTypeID.at(viewHash)).ComponentAdder(entity, owningScene);

					Manager->SetIsModalOpen(false);
					GUI::CloseCurrentPopup();
				}
			}
			GUI::EndChild();
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

	void CInspectorWindow::ReassignAssetRef(const U64 assetRequester, const std::vector<SAssetReference*>& references, const U8 index, const std::string& newPath)
	{
		std::string newAssetPath = UGeneralUtils::ConvertToPlatformAgnosticPath(newPath);

		std::vector<std::string> paths = SAssetReference::GetPaths(references);

		if (paths[index] != newAssetPath)
			GEngine::GetAssetRegistry()->UnrequestAsset(SAssetReference(paths[index]), assetRequester);

		*(references)[index] = SAssetReference(newAssetPath);
	}
}
