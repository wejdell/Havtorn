// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "ViewportWindow.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"
#include "Graphics/RenderManager.h"
#include "Graphics/RenderingPrimitives/RenderTexture.h"

#include "EditActions/RemoveEntityEditAction.h"

#include <Scene/Scene.h>
#include <ECS/ComponentAlgo.h>
#include <Assets/AssetRegistry.h>
#include <MathTypes/MathUtilities.h>
#include <PlatformManager.h>
#include <Input/InputMapper.h>

namespace Havtorn
{
	CViewportWindow::CViewportWindow(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager)
	{
		SnappingOptions.push_back({});
		SnappingOptions.emplace_back(SVector(0.01f), "0.01");
		SnappingOptions.emplace_back(SVector(0.05f), "0.05");
		SnappingOptions.emplace_back(SVector(0.1f), "0.1");
		SnappingOptions.emplace_back(SVector(0.5f), "0.5");
		SnappingOptions.emplace_back(SVector(1.0f), "1.0");
	}

	CViewportWindow::~CViewportWindow()
	{
	}

	void CViewportWindow::OnEnable()
	{
	}

	void CViewportWindow::OnInspectorGUI()
	{
		SEditorLayout& layout = Manager->GetEditorLayout();

		const SVector2<F32> layoutPosition = SVector2<F32>(layout.ViewportPosition.X, layout.ViewportPosition.Y);
		const SVector2<F32> layoutSize = SVector2<F32>(layout.ViewportSize.X, layout.ViewportSize.Y);
		GUI::SetNextWindowPos(layoutPosition);
		GUI::SetNextWindowSize(layoutSize);

		GUI::PushStyleVar(EStyleVar::WindowPadding, SVector2<F32>(0.0f));
		GUI::PushStyleVar(EStyleVar::ItemSpacing, SVector2<F32>(0.0f));

		const CEditorResourceManager* resourceManager = Manager->GetResourceManager();
		intptr_t playButtonID = resourceManager->GetStaticEditorTextureResource(EEditorTexture::PlayIcon);
		intptr_t pauseButtonID = resourceManager->GetStaticEditorTextureResource(EEditorTexture::PauseIcon);
		intptr_t stopButtonID = resourceManager->GetStaticEditorTextureResource(EEditorTexture::StopIcon);
		intptr_t moveButtonID = resourceManager->GetStaticEditorTextureResource(EEditorTexture::MoveGizmoIcon);
		intptr_t rotateButtonID = resourceManager->GetStaticEditorTextureResource(EEditorTexture::RotateGizmoIcon);
		intptr_t scaleButtonID = resourceManager->GetStaticEditorTextureResource(EEditorTexture::ScaleGizmoIcon);
		intptr_t settingsButtonID = resourceManager->GetStaticEditorTextureResource(EEditorTexture::EnvironmentLightIcon);

		// TODO.NW: Make toggle to unlock windows (allow moving)
		// TODO.NW: Make button to snap back to 16:9 aspect ratio (offer black bars?)
		// TODO.NW: Make button to reset to default layout, save layouts
		if (GUI::Begin(Name(), nullptr, { EWindowFlag::NoMove, EWindowFlag::NoCollapse, EWindowFlag::NoBringToFrontOnFocus }))
		{
			IsHovered = IsEnabled && GUI::IsWindowHovered();

			CWorld* world = GEngine::GetWorld();
			EWorldPlayState playState = world->GetWorldPlayState();
			IsPlayButtonEngaged = playState == EWorldPlayState::Playing;
			IsPauseButtonEngaged = playState == EWorldPlayState::Paused;
			ETransformGizmo currentGizmo = Manager->GetCurrentGizmo();

			CRenderManager* renderManager = Manager->GetRenderManager();
			ERenderPass renderPass = renderManager->GetRenderPass();
			constexpr F32 renderPassDropdownWidth = 136.0f;
			GUI::PushItemWidth(renderPassDropdownWidth);
			if (GUI::ComboEnum("View Mode", renderPass, { ERenderPass::Count }))
			{
				renderManager->SetRenderPass(renderPass);
				IsHovered = false;
			}
			if (GUI::IsItemHovered())
				GUI::SetTooltip("The render passes shown in the viewport\n(F7) toggles backwards\n(F8) toggles forwards\n(F9) resets to 'All'");
			GUI::PopItemWidth();

			SVector2<F32> buttonSize = { 16.0f, 16.0f };
			std::vector<SAlignedButtonData> buttonData;
			buttonData.push_back({ [&]() { world->BeginPlay(); }, playButtonID, IsPlayButtonEngaged, "Play (Alt+P)"});
			buttonData.push_back({ [&]() { world->PausePlay(); }, pauseButtonID, IsPauseButtonEngaged, "Pause"});
			buttonData.push_back({ [&]() { world->StopPlay(); }, stopButtonID, false, "Stop (Esc)"});
			buttonData.push_back({ [&]() { Manager->SetCurrentGizmo(ETransformGizmo::Translate); }, moveButtonID, currentGizmo == ETransformGizmo::Translate, "Move (W)"});
			buttonData.push_back({ [&]() { Manager->SetCurrentGizmo(ETransformGizmo::Rotate); }, rotateButtonID, currentGizmo == ETransformGizmo::Rotate, "Rotate (E)"});
			buttonData.push_back({ [&]() { Manager->SetCurrentGizmo(ETransformGizmo::Scale); }, scaleButtonID, currentGizmo == ETransformGizmo::Scale, "Scale (R)"});
			GUI::AddViewportButtons(buttonData, buttonSize, layout.ViewportSize.X);
			
			// TODO.NW: Fix size of this button
			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f + 64.0f);
			std::string playDimensionLabel = world->GetWorldPlayDimensions() == EWorldPlayDimensions::World3D ? "3D" : "2D";
			if (GUI::Button(playDimensionLabel.c_str(), buttonSize + GUI::GetStyleVar(EStyleVar::FramePadding) * 2.0f))
			{
				world->ToggleWorldPlayDimensions();
			}
			if (GUI::IsItemHovered())
				GUI::SetTooltip("Physics World Dimensions\nWhat physics engine to use");

			GUI::SameLine(layout.ViewportSize.X * 0.5f - 8.0f + 96.0f);
			if (GUI::ImageButton("ViewportSettingsButton", settingsButtonID, buttonSize))
				GUI::OpenPopup("ViewportSettings");

			if (GUI::BeginPopup("ViewportSettings")) 
			{
				auto spaceLabels = magic_enum::enum_names<ETransformGizmoSpace>();
				auto currentLabel = magic_enum::enum_name(Manager->GetCurrentGizmoSpace());
				if (GUI::BeginCombo("Gizmo Space", currentLabel.data()))
				{
					for (auto& label : spaceLabels)
					{
						bool isSelected = label == currentLabel;
						if (GUI::Selectable(label.data(), isSelected))
							Manager->SetGizmoSpace(magic_enum::enum_cast<ETransformGizmoSpace>(label).value());

						if (isSelected)
							GUI::SetItemDefaultFocus();
					}
					GUI::EndCombo();
				}

				// TODO.NW: Maybe we'd like to place this somewhere else?
				SSnappingOption currentSnapping = Manager->GetCurrentGizmoSnapping();
				if (GUI::BeginCombo("Snapping", currentSnapping.Label.c_str()))
				{
					for (auto& option : SnappingOptions)
					{
						bool isSelected = option == currentSnapping;
						if (GUI::Selectable(option.Label.c_str(), isSelected))
							Manager->SetGizmoSnapping(option);

						if (isSelected)
							GUI::SetItemDefaultFocus();
					}
					GUI::EndCombo();
				}

				std::string pivotText = "Unlock Pivot (Hold C)";
				const bool isPivotUnlocked = Manager->GetIsPivotMovingActive();
				if (!isPivotUnlocked && Manager->GetIsPivotOffsetSet())
					pivotText = "Reset Pivot (C)";
				else if (isPivotUnlocked)
					pivotText = "Set Pivot (Release C)";
				GUI::Text(pivotText.c_str());

				bool isGridSnappingActive = Manager->GetIsGridSnappingActive();
				if (GUI::Checkbox("Enable Grid Snapping (G)", isGridSnappingActive))
				{
					Manager->SetGridSnapping(isGridSnappingActive);
				}

				bool isVertexSnappingActive = Manager->GetIsVertexSnappingActive();
				if (GUI::Checkbox("Enable Vertex Snapping (V)", isVertexSnappingActive))
				{
					Manager->SetVertexSnapping(isVertexSnappingActive);
				}

				GUI::EndPopup();
			}
			
			if (GUI::IsItemHovered())
				GUI::SetTooltip("Snapping & Gizmo Settings");

			CCameraSystem* cameraSystem = GEngine::GetWorld()->GetSystem<CCameraSystem>();
			if (cameraSystem)
			{
				constexpr F32 cameraSpeedDraggerWidth = 100.0f;
				constexpr F32 spacing = 12.0f;
				const char* label = "Camera Speed ";
				const F32 labelWidth = GUI::CalculateTextSize(label).X;

				GUI::SameLine(GUI::GetContentRegionAvail().X - cameraSpeedDraggerWidth - labelWidth - spacing);
				GUI::PushItemWidth(cameraSpeedDraggerWidth);
				
				F32 cameraSpeed = cameraSystem->GetCameraSpeed();
				if (GUI::DragFloat(label, cameraSpeed, 0.5f, 0.2f, 20.0f, "%.2f"))
					cameraSystem->SetCameraSpeed(cameraSpeed);
				
				GUI::PopItemWidth();
			}

			const SEntity& mainCamera = world->GetMainCamera();
			Render(Manager->GetCurrentWorkingScene(), mainCamera.IsValid() ? mainCamera.GUID : 0);
		}

		GUI::PopStyleVar();
		GUI::PopStyleVar();

		// Context Menu
		if (ContextMenuEntity.IsValid())
		{
			if (GUI::BeginPopupContextWindow())
			{
				OpenedEntityContextMenu = true;

				std::string entityName = "UNNAMED";
				CScene* containingScene = UComponentAlgo::GetContainingScene(ContextMenuEntity, GEngine::GetWorld()->GetActiveScenes());
				if (containingScene == nullptr)
				{
					GUI::CloseCurrentPopup();
				}
				else
				{
					SMetaDataComponent* metaData = containingScene->GetComponent<SMetaDataComponent>(ContextMenuEntity);
					if (SComponent::IsValid(metaData))
						entityName = metaData->Name.AsString();

					GUI::TextDisabled(entityName.c_str());
					GUI::Separator();

					if (GUI::MenuItem("Copy Focus Entity Link"))
						GUI::CopyToClipboard(Manager->GetEntityFocusLink(ContextMenuEntity).data());
					if (GUI::IsItemHovered())
						GUI::SetTooltip("Copies a shareable link for focusing this entity");

					if (GUI::MenuItem("Copy Camera View Link"))
						GUI::CopyToClipboard(Manager->GetCameraFocusLink().data());
					if (GUI::IsItemHovered())
						GUI::SetTooltip("Copies a shareable link for focusing the current camera view");
				}
				GUI::EndPopup();
			}
			else if (OpenedEntityContextMenu)
			{
				ContextMenuEntity = SEntity::Null;
				OpenedEntityContextMenu = false;
			}
		}

		const SVector2<F32> newPosition = GUI::GetWindowPos();
		const SVector2<F32> newSize = GUI::GetWindowSize();
		if (layoutPosition != newPosition || layoutSize != newSize)
		{
			layout.ViewportPosition.X = STATIC_I16(newPosition.X);
			layout.ViewportPosition.Y = STATIC_I16(newPosition.Y);
			layout.ViewportSize.X = STATIC_U16(newSize.X);
			layout.ViewportSize.Y = STATIC_U16(newSize.Y);
			layout.ViewportChanged = true;
		}

		GUI::End();
	}

	void CViewportWindow::OnDisable()
	{
		ClearMaterialRefs(false);
	}
	
	const SVector2<F32> CViewportWindow::GetRenderedSceneDimensions() const
	{
		return RenderedSceneDimensions;
	}
	
	const SVector2<F32> CViewportWindow::GetRenderedScenePosition() const
	{
		return RenderedScenePosition;
	}

	bool CViewportWindow::Render(CScene* assetDragScene, const U64 renderTargetGUID)
	{
		// TODO.NW: All these broken out functions (in viewport, hierarchy, and inspector) that the prefab tool have license to use
		// should be const or static
		const bool workingInViewport = assetDragScene == Manager->GetCurrentWorkingScene();

		bool wasRenderHovered = false;

		CRenderTexture* mainRenderTexture = nullptr;
		if (renderTargetGUID != 0)
			mainRenderTexture = Manager->GetRenderManager()->GetRenderTargetTexture(renderTargetGUID);

		if (mainRenderTexture && mainRenderTexture->IsShaderResourceValid())
		{
			const SVector2<F32> vMin = GUI::GetWindowContentRegionMin();
			const SVector2<F32> vMax = GUI::GetWindowContentRegionMax();

			const F32 width = static_cast<F32>(vMax.X - vMin.X);
			const F32 height = static_cast<F32>(vMax.Y - vMin.Y - ViewportMenuHeight - 4.0f);

			if (workingInViewport)
			{
				const SVector2<F32> viewportWorkPos = GUI::GetViewportWorkPos();
				const SEditorLayout& layout = Manager->GetEditorLayout();
				SVector2<F32> windowPos = SVector2<F32>(viewportWorkPos.X + layout.ViewportPosition.X, viewportWorkPos.Y + layout.ViewportPosition.Y);
				windowPos.Y += ViewportMenuHeight - 4.0f;
				RenderedScenePosition.X = windowPos.X;
				RenderedScenePosition.Y = windowPos.Y;
				RenderedSceneDimensions = { width, height };
			}

			GUI::Image((intptr_t)mainRenderTexture->GetShaderResourceView(), SVector2<F32>(width, height));
			wasRenderHovered = GUI::IsItemHovered();
		}

		GUI::SetGizmoDrawList();

		// TODO.NW: Unnestle these ifs
		CScene* currentScene = assetDragScene;
		if (currentScene != nullptr)
		{
			if (GUI::BeginDragDropTarget())
			{
				SGuiPayload payload = GUI::AcceptDragDropPayload("AssetDrag", { EDragDropFlag::AcceptBeforeDelivery, EDragDropFlag::AcceptNopreviewTooltip });
				if (payload.Data != nullptr)
				{
					SEditorAssetRepresentation* payloadAssetRep = reinterpret_cast<SEditorAssetRepresentation*>(payload.Data);
					if (payloadAssetRep->AssetType == EAssetType::Material)
						UpdatePreviewMaterial(currentScene, payloadAssetRep);
					else
						UpdatePreviewEntity(currentScene, payloadAssetRep);

					if (currentScene->PreviewEntity.IsValid())
						GUI::SetTooltip("Create Entity?");
					else if (payloadAssetRep->AssetType == EAssetType::Material)
						GUI::SetTooltip("Assign Material?");
					else
						GUI::SetTooltip("Asset type not supported yet!");

					if (payload.IsDelivery)
					{
						DeliverAssetDrag(currentScene, payloadAssetRep);
					}
				}

				GUI::EndDragDropTarget();
			}
			else
			{
				if (currentScene->PreviewEntity.IsValid())
				{
					// TODO.NW: Figure out mismatch that happens with other components when we remove the preview, suddenly entity component indices are one too big. Is it a race condition or something?
					currentScene->RemoveEntity(currentScene->PreviewEntity);
					currentScene->PreviewEntity = SEntity::Null;
				}
				
				ClearMaterialRefs(true);
			}
		}

		return wasRenderHovered;
	}

	void CViewportWindow::UpdatePreviewEntity(CScene* scene, const SEditorAssetRepresentation* assetRepresentation)
	{
		if (assetRepresentation->AssetType != EAssetType::StaticMesh && 
			assetRepresentation->AssetType != EAssetType::SkeletalMesh && 
			assetRepresentation->AssetType != EAssetType::SkeletalAnimation && 
			assetRepresentation->AssetType != EAssetType::Prefab)
			return;

		if (scene->PreviewEntity.IsValid())
		{
			// TODO.NW: This is too annoying, we should have an easy time of setting the transform of entities
			STransformComponent& previewTransform = *scene->GetComponent<STransformComponent>(scene->PreviewEntity);
			SMatrix transformCopy = previewTransform.Transform.GetMatrix();
			
			transformCopy.SetTranslation(GetWorldPositionOnPixel());
			
			previewTransform.Transform.SetMatrix(transformCopy);
			return;
		}

		std::string newEntityName = UGeneralUtils::GetNonCollidingString(assetRepresentation->Name, scene->Entities, [scene](const SEntity& entity)
			{
				const SMetaDataComponent* metaDataComp = scene->GetComponent<SMetaDataComponent>(entity);
				return SComponent::IsValid(metaDataComp) ? metaDataComp->Name.AsString() : "UNNAMED";
			}
		);
		scene->PreviewEntity = scene->AddEntity(newEntityName, 8000);
		GEngine::GetWorld()->SetEditorRenderExemptEntity(scene->PreviewEntity);

		scene->AddComponent<STransformComponent>(scene->PreviewEntity)->Transform;
		scene->AddComponentEditorContext(scene->PreviewEntity, &STransformComponentEditorContext::Context);

		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();

		switch (assetRepresentation->AssetType)
		{
		case EAssetType::StaticMesh:
		{
			std::string staticMeshPath = assetRepresentation->DirectoryEntry.path().string();
			scene->AddComponent<SStaticMeshComponent>(scene->PreviewEntity, staticMeshPath);
			scene->AddComponentEditorContext(scene->PreviewEntity, &SStaticMeshComponentEditorContext::Context);
			SStaticMeshAsset* meshAsset = assetRegistry->RequestAssetData<SStaticMeshAsset>(SAssetReference(staticMeshPath), scene->PreviewEntity.GUID);

			std::vector<std::string> previewMaterials;
			previewMaterials.resize(meshAsset->NumberOfMaterials, CEditorManager::PreviewMaterial);
			scene->AddComponent<SMaterialComponent>(scene->PreviewEntity, previewMaterials);
			scene->AddComponentEditorContext(scene->PreviewEntity, &SMaterialComponentEditorContext::Context);
		}
			break;

		case EAssetType::SkeletalMesh:
		{
			std::string meshPath = assetRepresentation->DirectoryEntry.path().string();
			scene->AddComponent<SSkeletalMeshComponent>(scene->PreviewEntity, meshPath);
			scene->AddComponentEditorContext(scene->PreviewEntity, &SSkeletalMeshComponentEditorContext::Context);
			SSkeletalMeshAsset* meshAsset = assetRegistry->RequestAssetData<SSkeletalMeshAsset>(SAssetReference(meshPath), scene->PreviewEntity.GUID);
			
			// TODO.NW: Deal with different asset types, and figure out bind pose for skeletal meshes

			std::vector<std::string> previewMaterials;
			previewMaterials.resize(meshAsset->NumberOfMaterials, CEditorManager::PreviewMaterial);
			scene->AddComponent<SMaterialComponent>(scene->PreviewEntity, previewMaterials);
			scene->AddComponentEditorContext(scene->PreviewEntity, &SMaterialComponentEditorContext::Context);
		}
			break;

		case EAssetType::SkeletalAnimation:
		{
			const std::string animationPath = assetRepresentation->DirectoryEntry.path().string();
			SSkeletalAnimationAsset* animationAsset = assetRegistry->RequestAssetData<SSkeletalAnimationAsset>(SAssetReference(animationPath), scene->PreviewEntity.GUID);
			const std::vector<std::string> animationPaths = { animationPath };
			scene->AddComponent<SSkeletalAnimationComponent>(scene->PreviewEntity, animationPaths);
			scene->AddComponentEditorContext(scene->PreviewEntity, &SSkeletalAnimationComponentEditorContext::Context);

			const std::string meshPath = animationAsset->RigPath;
			scene->AddComponent<SSkeletalMeshComponent>(scene->PreviewEntity, meshPath);
			scene->AddComponentEditorContext(scene->PreviewEntity, &SSkeletalMeshComponentEditorContext::Context);
			SSkeletalMeshAsset* meshAsset = assetRegistry->RequestAssetData<SSkeletalMeshAsset>(SAssetReference(meshPath), scene->PreviewEntity.GUID);

			std::vector<std::string> previewMaterials;
			previewMaterials.resize(meshAsset->NumberOfMaterials, CEditorManager::PreviewMaterial);
			scene->AddComponent<SMaterialComponent>(scene->PreviewEntity, previewMaterials);
			scene->AddComponentEditorContext(scene->PreviewEntity, &SMaterialComponentEditorContext::Context);
		}
			break;

		case EAssetType::Prefab:
		{
			// TODO.NW: Handle prefab previs/preview
			
			scene->AddComponent<SPrefabComponent>(scene->PreviewEntity, assetRepresentation->DirectoryEntry.path().string());
			scene->AddComponentEditorContext(scene->PreviewEntity, &SPrefabComponentEditorContext::Context);
		}
			break;

		default :
			break;
		}
	}

	void CViewportWindow::UpdatePreviewMaterial(CScene* scene, const SEditorAssetRepresentation* assetRepresentation)
	{
		if (assetRepresentation->AssetType != EAssetType::Material)
			return;

		SEntity entityOnPixel = GetEntityOnPixel();
		SMaterialComponent* materialComponent = scene->GetComponent<SMaterialComponent>(entityOnPixel);
		if (!SComponent::IsValid(materialComponent))
			return;

		std::vector<SMaterialVertex> vertices = FindLocalVertices(scene, entityOnPixel);
		if (vertices.empty())
			return;
	
		STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOnPixel);
		SVector4 worldSpacePos = GetWorldPositionOnPixel();
		SVector localSpaceCursorPos = (worldSpacePos * transform->Transform.GetMatrix().FastInverse()).ToVector3();

		// TODO.NW: This would be nice to handle through a standard algorithm
		F32 minDist = UMath::MaxFloat;
		U16 closestMaterialIndex = 0;
		for (const SMaterialVertex& vertexPos : vertices)
		{
			const F32 currentDist = vertexPos.LocalVertex.DistanceSquared(localSpaceCursorPos);
			if (currentDist < minDist)
			{
				minDist = currentDist;
				closestMaterialIndex = vertexPos.MaterialIndex;
			}
		}

		if (materialComponent->AssetReferences.size() <= closestMaterialIndex)
			return;

		// Reassignment
		SAssetReference* hoveredMaterialRef = &materialComponent->AssetReferences[closestMaterialIndex];
		if (LastMaterialReference != nullptr && LastMaterialReference != hoveredMaterialRef)
			*LastMaterialReference = LastMaterialReferenceValue;
		
		if (LastMaterialReference != hoveredMaterialRef)
		{
			LastMaterialReference = hoveredMaterialRef;
			LastMaterialReferenceValue = *hoveredMaterialRef;
		}
		
		// New assignment
		SAssetReference assetRepValue = SAssetReference(assetRepresentation->DirectoryEntry.path().string());
		*hoveredMaterialRef = assetRepValue;
	}

	void CViewportWindow::DeliverAssetDrag(CScene* toScene, const SEditorAssetRepresentation* assetRepresentation)
	{
		if (assetRepresentation->AssetType == EAssetType::StaticMesh ||
			assetRepresentation->AssetType == EAssetType::SkeletalMesh ||
			assetRepresentation->AssetType == EAssetType::SkeletalAnimation)
		{
			SEntity copiedEntity = toScene->CopyEntity(toScene->PreviewEntity);
			toScene->RemoveEntity(toScene->PreviewEntity);
			Manager->SetSelectedEntity(copiedEntity);
			toScene->PreviewEntity = SEntity::Null;
			GEngine::GetWorld()->SetEditorRenderExemptEntity(SEntity::Null);
			UMetaCommandRouter::Push(SRemoveEntityEditAction::MakeEditActionCommand(Manager, copiedEntity, false));
		}

		if (assetRepresentation->AssetType == EAssetType::Material)
		{
			ClearMaterialRefs(false);
		}

		if (assetRepresentation->AssetType == EAssetType::Prefab)
		{
			const SEntity copiedEntity = toScene->CopyEntity(toScene->PreviewEntity);
			toScene->RemoveEntity(toScene->PreviewEntity);
			Manager->SetSelectedEntity(copiedEntity);
			toScene->PreviewEntity = SEntity::Null;

			const SPrefabAsset* prefab = GEngine::GetAssetRegistry()->RequestAssetData<SPrefabAsset>(SAssetReference(assetRepresentation->DirectoryEntry.path().string()), copiedEntity.GUID);
			std::vector<SEntity> newEntities = toScene->CopyEntities(prefab->Scene.get(), {});

			STransformComponent* parentTransform = toScene->GetComponent<STransformComponent>(copiedEntity);
			for (const SEntity& entity : newEntities)
			{
				// TODO.NW: Maybe make attachment function taking attachment rules? Keeping absolute position or not
				// NW: Prefab entities have already been placed in the scene "parented" to the origin. Shift them over to the prefab space before attaching
				STransformComponent* childTransform = toScene->GetComponent<STransformComponent>(entity);
				SMatrix matrix = childTransform->Transform.GetMatrix();
				matrix *= parentTransform->Transform.GetMatrix();
				childTransform->Transform.SetMatrix(matrix);
				parentTransform->Attach(childTransform);
			}
		}
	}

	SEntity CViewportWindow::GetEntityOnPixel() const
	{
		const U64 dataIndex = GetEditorDataIndexOnPixel();
		if (dataIndex == UMath::MaxU64)
			return SEntity::Null;

		const U64 pickedEntityGUID = Manager->GetRenderManager()->GetEntityGUIDFromData(dataIndex);

		return SEntity(pickedEntityGUID);
	}

	SVector4 CViewportWindow::GetWorldPositionOnPixel() const
	{
		const U64 dataIndex = GetEditorDataIndexOnPixel();
		if (dataIndex == UMath::MaxU64)
			return SVector4::Zero;

		SVector4 worldPos = Manager->GetRenderManager()->GetWorldPositionFromData(dataIndex);

		if (worldPos.W == 0.0f)
		{
			CWorld* world = GEngine::GetWorld();
			const std::vector<Ptr<CScene>>& scenes = world->GetActiveScenes();
			const SCameraData mainCameraData = UComponentAlgo::GetCameraData(world->GetMainCamera(), scenes);

			if (!mainCameraData.IsValid())
				return worldPos;

			const SMatrix viewMatrix = mainCameraData.TransformComponent->Transform.GetMatrix();
			const SMatrix projectionMatrix = mainCameraData.CameraComponent->ProjectionMatrix;

			const SVector2<F32> mousePosition = GEngine::GetInput()->GetCurrentMousePosition();
			const SRay worldRay = UMathUtilities::RaycastWorld(mousePosition, RenderedSceneDimensions, RenderedScenePosition, viewMatrix, projectionMatrix);
			constexpr F32 dragDistanceFromEditorCamera = 3.0f;
			worldPos = SVector4(worldRay.GetPointOnRay(dragDistanceFromEditorCamera), 1.0f);
		}

		return worldPos;
	}

	SVector CViewportWindow::GetClosestVertexPositionOnPixel(const SEntity& forEntity) const
	{
		CScene* owningScene = Manager->GetContainingScene(forEntity);
		if (owningScene == nullptr)
			return SVector::Zero;

		std::vector<SMaterialVertex> vertices = FindLocalVertices(owningScene, forEntity);
		if (vertices.empty())
			return SVector::Zero;

		STransformComponent* transform = owningScene->GetComponent<STransformComponent>(forEntity);
		SVector4 worldSpacePos = GetWorldPositionOnPixel();
		SVector localSpaceCursorPos = (worldSpacePos * transform->Transform.GetMatrix().FastInverse()).ToVector3();

		// TODO.NW: This would be nice to handle through a standard algorithm
		F32 minDist = UMath::MaxFloat;
		SVector closestPos = localSpaceCursorPos;
		for (const SMaterialVertex& vertexPos : vertices)
		{
			const F32 currentDist = vertexPos.LocalVertex.DistanceSquared(localSpaceCursorPos);
			if (currentDist < minDist)
			{
				minDist = currentDist;
				closestPos = vertexPos.LocalVertex;
			}
		}

		return (SVector4(closestPos, 1.0f) * transform->Transform.GetMatrix()).ToVector3();
	}

	void CViewportWindow::SetContextMenuEntity(const SEntity& entity)
	{
		if (!entity.IsValid())
			return;

		ContextMenuEntity = entity;

		// TODO.NW: Decide what feels best here, to clear all selections or select the picked one
		//Manager->SetSelectedEntity(entity);
		Manager->ClearSelectedEntities();
	}

	U64 CViewportWindow::GetEditorDataIndexOnPixel() const
	{
		const SVector2<F32> mousePosition = GEngine::GetInput()->GetCurrentMousePosition();
		const SVector2<U16> resolution = Manager->GetPlatformManager()->GetResolution();
		const SVector2<F32> rectRelativeMousePos = SVector2((mousePosition.X - RenderedScenePosition.X) / RenderedSceneDimensions.X, (mousePosition.Y - RenderedScenePosition.Y) / RenderedSceneDimensions.Y);

		const SVector2<F32> fullscreenMousePos = { UMath::Ceil(STATIC_F32(resolution.X) * rectRelativeMousePos.X), UMath::Ceil(STATIC_F32(resolution.Y) * rectRelativeMousePos.Y - 12.0f) };

		if (!UMath::IsWithin(fullscreenMousePos.X, 0.0f, STATIC_F32(resolution.X)) || !UMath::IsWithin(fullscreenMousePos.Y, 0.0f, STATIC_F32(resolution.Y)))
			return UMath::MaxU64;

		return STATIC_U64(fullscreenMousePos.X) + STATIC_U64(fullscreenMousePos.Y) * STATIC_U64(resolution.X);
	}

	void CViewportWindow::ClearMaterialRefs(const bool reassignLastMaterial)
	{
		if (reassignLastMaterial && LastMaterialReference != nullptr)
			*LastMaterialReference = LastMaterialReferenceValue;

		LastMaterialReference = nullptr;
		LastMaterialReferenceValue = SAssetReference();
	}

	std::vector<SMaterialVertex> CViewportWindow::FindLocalVertices(CScene* scene, const SEntity& entity) const
	{
		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		std::vector<SMaterialVertex> vertices = {};

		SStaticMeshComponent* staticMeshComponent = scene->GetComponent<SStaticMeshComponent>(entity);
		SSkeletalMeshComponent* skeletalMeshComponent = scene->GetComponent<SSkeletalMeshComponent>(entity);
		if (SComponent::IsValid(staticMeshComponent))
		{
			SStaticMeshAsset* meshAsset = assetRegistry->RequestAssetData<SStaticMeshAsset>(staticMeshComponent->AssetReference, 100);

			if (meshAsset != nullptr)
				vertices = meshAsset->MaterialVertexAssociations;

			assetRegistry->UnrequestAsset(staticMeshComponent->AssetReference, 100);
		}
		else if (SComponent::IsValid(skeletalMeshComponent))
		{
			SSkeletalMeshAsset* meshAsset = assetRegistry->RequestAssetData<SSkeletalMeshAsset>(skeletalMeshComponent->AssetReference, 100);

			if (meshAsset != nullptr)
				vertices = meshAsset->MaterialVertexAssociations;

			assetRegistry->UnrequestAsset(skeletalMeshComponent->AssetReference, 100);
		}

		return vertices;
	}
}
