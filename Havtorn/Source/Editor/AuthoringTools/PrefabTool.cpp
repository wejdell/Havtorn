// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"

#include <FileSystem.h>
#include <Graphics/RenderManager.h>
#include <Graphics/GeometryPrimitives.h>
#include <Input/InputMapper.h>
#include <Input/InputTypes.h>
#include <Timer.h>
#include <Assets/AssetRegistry.h>
#include <ECS/Systems/CameraSystem.h>
#include <Assets/FileHeaders/PrefabAssetFileHeader.h>
#include <Graphics/Debug/DebugDrawUtility.h>

#include "Windows/HierarchyWindow.h"
#include "Windows/InspectorWindow.h"
#include "Windows/ViewportWindow.h"
#include "Systems/EditorRenderSystem.h"

#include "PrefabTool.h"

using Havtorn::I32;
using Havtorn::F32;
using Havtorn::U64;

namespace Havtorn
{
	using namespace HexRune;

	CPrefabTool::CPrefabTool(const char* displayName, CEditorManager* manager)
		: CWindow(displayName, manager, false)
	{}

	void CPrefabTool::OnEnable()
	{
		CInputMapper* mapper = GEngine::GetInput();
		mapper->GetAxisDelegate(EInputAxisEvent::Up).AddMember(this, &CPrefabTool::HandleAxisInput);
		mapper->GetAxisDelegate(EInputAxisEvent::Right).AddMember(this, &CPrefabTool::HandleAxisInput);
		mapper->GetAxisDelegate(EInputAxisEvent::Forward).AddMember(this, &CPrefabTool::HandleAxisInput);
		mapper->GetAxisDelegate(EInputAxisEvent::MouseDeltaHorizontal).AddMember(this, &CPrefabTool::HandleAxisInput);
		mapper->GetAxisDelegate(EInputAxisEvent::MouseDeltaVertical).AddMember(this, &CPrefabTool::HandleAxisInput);
		mapper->GetAxisDelegate(EInputAxisEvent::Zoom).AddMember(this, &CPrefabTool::HandleAxisInput);
		mapper->GetActionDelegate(EInputActionEvent::ToggleFreeCam).AddMember(this, &CPrefabTool::ToggleFreeCam);
	}

	void CPrefabTool::OnInspectorGUI()
	{
		if (Manager->GetIsOverGizmo())
			GUI::SetNextWindowPos(WindowPos);

		// TODO.NW: Make ON_SCOPE_EXIT equivalent?
		if (!GUI::Begin(Name(), &IsEnabled))
		{
			GUI::End();
			return;
		}

		IsHovered = IsEnabled && GUI::IsWindowHovered();

		if (PrefabData == nullptr)
		{
			GUI::End();
			return;
		}

		const bool isWindowHovered = GUI::IsWindowHovered();
		if (!IsHoveringWindow && isWindowHovered)
			GEngine::GetWorld()->BlockSystem<CCameraSystem>(this);
		else if (IsHoveringWindow && !isWindowHovered)
			GEngine::GetWorld()->UnblockSystem<CCameraSystem>(this);

		IsHoveringWindow = isWindowHovered;

		CHierarchyWindow* hierarchyWindow = Manager->GetEditorWindow<CHierarchyWindow>();
		if (hierarchyWindow == nullptr)
			return;

		CInspectorWindow* inspectorWindow = Manager->GetEditorWindow<CInspectorWindow>();
		if (inspectorWindow == nullptr)
			return;

		CViewportWindow* viewportWindow = Manager->GetEditorWindow<CViewportWindow>();
		if (viewportWindow == nullptr)
			return;

		GUI::Text(UGeneralUtils::ExtractFileBaseNameFromPath(CurrentPrefabAssetRef.FilePath).c_str());
		GUI::SameLine();
		if (GUI::Button("Save"))
		{
			SPrefabFileHeader fileHeader;
			fileHeader.Name = PrefabData->Scene->GetSceneName();
			fileHeader.Scene = PrefabData->Scene.get();
			// TODO.NW: Check redirections?
			GEngine::GetAssetRegistry()->SaveAsset(UGeneralUtils::ExtractParentDirectoryFromPath(CurrentPrefabAssetRef.FilePath) + "/", fileHeader);			
		}

		{ // Preview Settings
			RenderPreviewSettings();
		}

		GUI::Separator();

		CScene* prefabScene = PrefabData->Scene.get();

		SVector2<F32> available = GUI::GetContentRegionAvail();
		available.X *= 0.33f;
		GUI::BeginChild("Hierarchy", available);
		{ // Hierarchy
			hierarchyWindow->Header();

			GUI::Text(prefabScene->GetSceneName().c_str());
			GUI::SameLine();

			auto createNewEntity = [prefabScene]() 
			{
				std::string newEntityName = UGeneralUtils::GetNonCollidingString("NewEntity", prefabScene->Entities, [prefabScene](const SEntity& entity)
					{
						const SMetaDataComponent* metaDataComp = prefabScene->GetComponent<SMetaDataComponent>(entity);
						return SComponent::IsValid(metaDataComp) ? metaDataComp->Name.AsString() : "UNNAMED";
					}
				);
				prefabScene->AddEntity(newEntityName);
			};

			if (GUI::Button("Create New Entity"))
			{
				createNewEntity();
			}

			hierarchyWindow->InspectScene(prefabScene);

			if (GUI::BeginPopupContextWindow())
			{
				if (GUI::MenuItem("Create New Entity"))
					createNewEntity();

				GUI::EndPopup();
			}
		}
		GUI::EndChild();

		GUI::SameLine();
		WindowPos = GUI::GetWindowPos();
		PreviewWindowPosition = WindowPos + GUI::GetCursorPos();
		PreviewWindowDimensions = available;
		GUI::BeginChild("Viewport", available);
		{ // Viewport
			RenderPrefab();
			GDebugDraw::AddGrid(SVector(), SVector(), SColor::Grey, -1.0f, false, 0.005f, false, PrefabToolRenderID);
			IsHoveringViewport = viewportWindow->Render(prefabScene, PrefabToolRenderID);
		}
		GUI::EndChild();

		GUI::SameLine();
		GUI::BeginChild("Inspector", available);
		{ // Inspector
			std::vector<SEntity> selectedEntities = Manager->GetSelectedEntities();

			if (selectedEntities.empty())
				GUI::TextDisabled("Select an Entity to modify");

			// TODO.NW: Go through and make sure everything in the inspector gets unique ID, maybe based on entity GUID. 
			// Don't want same IDs over a frame when multiple entities are selected
			for (const SEntity& selectedEntity : selectedEntities)
			{
				// TODO.NW: Fix bug where selecting an entity in prefab editor and then dragging the window makes the drag source lose its ID somehow. Wasn't enough to not run the InspectorWindow logic
				//if (GUI::BeginDragDropSource())
				//{
				//	SGuiPayload payload = GUI::GetDragDropPayload();
				//	if (payload.Data == nullptr)
				//	{
				//		GUI::SetDragDropPayload("EntityDrag", &selectedEntity, sizeof(SEntity));
				//	}
				//	GUI::Text("entityName");

				//	GUI::EndDragDropSource();
				//}

				inspectorWindow->InspectEntity(selectedEntity, prefabScene);
			}
		}
		GUI::EndChild();

		GUI::End();
	}

	void CPrefabTool::OnDeferredExit()
	{
		ClosePrefab();
	}

	void CPrefabTool::OnDisable()
	{
		RunDeferredExit = true;

		GEngine::GetWorld()->UnblockSystem<CCameraSystem>(this);

		CInputMapper* mapper = GEngine::GetInput();
		mapper->GetAxisDelegate(EInputAxisEvent::Up).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::Right).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::Forward).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::MouseDeltaHorizontal).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::MouseDeltaVertical).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::Zoom).RemoveObject(this);
		mapper->GetActionDelegate(EInputActionEvent::ToggleFreeCam).RemoveObject(this);
	}

	void CPrefabTool::OpenPrefab(SEditorAssetRepresentation* asset)
	{
		if (IsEnabled)
			ClosePrefab();

		CurrentPrefabAssetRef = SAssetReference(asset->DirectoryEntry.path().string());

		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		PrefabData = assetRegistry->RequestAssetData<SPrefabAsset>(CurrentPrefabAssetRef, PrefabToolRenderID);
		Manager->GetRenderManager()->RequestRenderView(PrefabToolRenderID);
		PreviewSkylight = assetRegistry->RequestAssetData<STextureCubeAsset>(PreviewSkylightAssetRef, PrefabToolRenderID);

		PreviewCameraViewMatrix = SMatrix::Identity;
		PreviewCameraViewMatrix.SetTranslation(SVector(0.0f, 1.0f, -1.0f));
		SetEnabled(true);
	}

	void CPrefabTool::ClosePrefab()
	{
		// TODO.NW: Both MaterialTool and this tool can't handle CurrentPrefabAssetRef being invalidated by editor actions, such as
		// deleting or renaming. We need a solid way of handling this

		if (PrefabData != nullptr && PrefabData->Scene->HasEntity(Manager->GetSelectedEntity().GUID))
			Manager->ClearSelectedEntities();

		CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
		assetRegistry->UnrequestAsset(CurrentPrefabAssetRef, PrefabToolRenderID);
		PrefabData = nullptr;
		
		assetRegistry->UnrequestAsset(PreviewSkylightAssetRef, PrefabToolRenderID);
		PreviewSkylight = nullptr;

		CurrentPrefabAssetRef = SAssetReference();
		Manager->GetRenderManager()->UnrequestRenderView(PrefabToolRenderID);
	}

	void CPrefabTool::HandleAxisInput(const SInputAxisPayload payload)
	{
		switch (payload.Event)
		{
		case EInputAxisEvent::Right:
			PreviewCameraMoveInput += SVector::Right * payload.AxisValue;
			return;
		case EInputAxisEvent::Up:
			PreviewCameraMoveInput += SVector::Up * payload.AxisValue;
			return;
		case EInputAxisEvent::Forward:
			PreviewCameraMoveInput += SVector::Forward * payload.AxisValue;
			return;
		case EInputAxisEvent::MouseDeltaVertical:
			PreviewCameraRotationInput.X += 90.0f * payload.AxisValue;
			return;
		case EInputAxisEvent::MouseDeltaHorizontal:
			PreviewCameraRotationInput.Y += 90.0f * payload.AxisValue;
			return;
		case EInputAxisEvent::Zoom:
		{
			if (IsFreeCamActive)
				PreviewCameraSpeedInput = UMath::Clamp(PreviewCameraSpeedInput + payload.AxisValue, -5.0f, 5.0f);
		}
		return;
		default:
			return;
		}
	}

	void CPrefabTool::ToggleFreeCam(const SInputActionPayload payload)
	{
		if (IsHoveringViewport && payload.IsHeld)
			IsFreeCamActive = true;
		else if (!payload.IsHeld)
			IsFreeCamActive = false;
	}

	CScene* CPrefabTool::GetWorkingScene() const
	{
		if (PrefabData != nullptr)
			return PrefabData->Scene.get();
		
		return nullptr;
	}

	const SMatrix& CPrefabTool::GetViewMatrix() const
	{
		return PreviewCameraViewMatrix;
	}

	const SMatrix& CPrefabTool::GetProjectionMatrix() const
	{
		return PreviewCameraProjectionMatrix;
	}

	const SVector2<F32>& CPrefabTool::GetPreviewWindowPosition() const
	{
		return PreviewWindowPosition;
	}

	const SVector2<F32>& CPrefabTool::GetPreviewWindowDimensions() const
	{
		return PreviewWindowDimensions;
	}

	void CPrefabTool::RenderPreviewSettings()
	{
		const SVector2<F32> settingsCategorySize = SVector2<F32>(256.0f, 146.0f);
		const F32 settingsPropertyWidth = 80.0f;

		GUI::BeginChild("Cubemap", settingsCategorySize);
		GUI::TextDisabled("Light Preview Settings");
		auto skyboxAssetRep = Manager->GetAssetRepFromName(UGeneralUtils::ExtractFileBaseNameFromPath(PreviewSkylightAssetRef.FilePath)).get();

		intptr_t assetPickerThumbnail = Manager->GetTextureResourceFromAssetRep(skyboxAssetRep);
		std::string pickerLabel = "Preview Skybox | ";
		if (skyboxAssetRep != nullptr)
			pickerLabel.append(skyboxAssetRep->Name);

		SAssetPickResult result = GUI::AssetPickerFilter(pickerLabel.c_str(), "Preview Skybox", assetPickerThumbnail, "Assets/Textures/Cubemaps", 4, Manager->GetAssetFilteredInspectFunction(), EAssetType::TextureCube);

		if (result.State == EAssetPickerState::AssetPicked)
		{
			CAssetRegistry* assetRegistry = GEngine::GetAssetRegistry();
			assetRegistry->UnrequestAsset(PreviewSkylightAssetRef, PrefabToolRenderID);
			skyboxAssetRep = Manager->GetAssetRepFromDirEntry(result.PickedEntry).get();
			PreviewSkylightAssetRef = SAssetReference(skyboxAssetRep->DirectoryEntry.path().string());
			PreviewSkylight = assetRegistry->RequestAssetData<STextureCubeAsset>(PreviewSkylightAssetRef, PrefabToolRenderID);
		}

		GUI::PushItemWidth(settingsPropertyWidth);
		GUI::DragFloat("Preview Light Intensity", PreviewLightIntensity, 0.01f);
		GUI::DragFloat3("Preview Light Direction", PreviewLightDirection, 0.05f, -1.0f, 1.0f, "%.1f");
		GUI::PopItemWidth();
		GUI::EndChild();

		GUI::SameLine();

		GUI::BeginChild("Light", settingsCategorySize);
		GUI::PushItemWidth(settingsPropertyWidth);
		GUI::ColorPicker3("Preview Light Color", PreviewLightColor);
		GUI::PopItemWidth();
		GUI::EndChild();

		GUI::SameLine();

		GUI::BeginChild("Camera", settingsCategorySize);
		GUI::TextDisabled("Camera Preview Settings");
		GUI::PushItemWidth(settingsPropertyWidth);
		GUI::DragFloat("Max Move Speed", PreviewCameraMaxMoveSpeed, GUI::SliderSpeed, 0.1f, 10.0f);
		GUI::DragFloat("Rotation Speed", PreviewCameraRotationSpeed, GUI::SliderSpeed, 0.1f, 5.0f);
		GUI::DragFloat("Acceleration Duration", PreviewCameraAccelerationDuration, GUI::SliderSpeed * 0.1f, 0.1f, 5.0f);
		GUI::PopItemWidth();
		GUI::EndChild();
	}

	void CPrefabTool::UpdateViewMatrix()
	{
		const F32 dt = GTime::Dt();

		// TODO.NW: Check sensitivity?

		auto applyLocalInput = [&]()
		{
			const SVector localInput = PreviewCameraAccelerationDirection * PreviewCameraCurrentAccelerationFactor * PreviewCameraMaxMoveSpeed * dt;
			SVector localMove = PreviewCameraViewMatrix.GetRight() * localInput.X;
			PreviewCameraViewMatrix.M[3][0] += localMove.X;
			PreviewCameraViewMatrix.M[3][1] += localMove.Y;
			PreviewCameraViewMatrix.M[3][2] += localMove.Z;
			localMove = PreviewCameraViewMatrix.GetUp() * localInput.Y;
			PreviewCameraViewMatrix.M[3][0] += localMove.X;
			PreviewCameraViewMatrix.M[3][1] += localMove.Y;
			PreviewCameraViewMatrix.M[3][2] += localMove.Z;
			localMove = PreviewCameraViewMatrix.GetForward() * localInput.Z;
			PreviewCameraViewMatrix.M[3][0] += localMove.X;
			PreviewCameraViewMatrix.M[3][1] += localMove.Y;
			PreviewCameraViewMatrix.M[3][2] += localMove.Z;
		};

		auto resetInput = [&]() 
		{
			PreviewCameraMoveInput = SVector::Zero;
			PreviewCameraRotationInput = SVector::Zero;
		};

		// === Speed ===
		if (PreviewCameraSpeedInput < 0.0f)
			PreviewCameraMaxMoveSpeed = UMath::Remap(-5.0f, -1.0f, 0.2f, 2.0f, PreviewCameraSpeedInput);
		else
			PreviewCameraMaxMoveSpeed = UMath::Remap(0.0f, 5.0f, 3.0f, 10.0f, PreviewCameraSpeedInput);

		if (!IsFreeCamActive)
		{
			// Decelerate
			PreviewCameraCurrentAccelerationFactor = UMath::Clamp(PreviewCameraCurrentAccelerationFactor - (1.0f / PreviewCameraAccelerationDuration) * dt);
			
			applyLocalInput();
			resetInput();
			return;
		}

		// === Rotation ===
		PreviewCameraCurrentPitch = UMath::Clamp(PreviewCameraCurrentPitch + (PreviewCameraRotationInput.X * PreviewCameraRotationSpeed * dt), -MaxPitchDegrees + 0.01f, MaxPitchDegrees - 0.01f);
		PreviewCameraCurrentYaw = UMath::WrapAngle(PreviewCameraCurrentYaw + (PreviewCameraRotationInput.Y * PreviewCameraRotationSpeed * dt));
		PreviewCameraViewMatrix.SetRotation({ PreviewCameraCurrentPitch, PreviewCameraCurrentYaw, 0.0f });

		// === Translation ===
		if (PreviewCameraMoveInput.IsNearlyZero())
		{
			// Decelerate
			PreviewCameraCurrentAccelerationFactor = UMath::Clamp(PreviewCameraCurrentAccelerationFactor - (1.0f / PreviewCameraAccelerationDuration) * dt);
			
			applyLocalInput();
			resetInput();
			return;
		}

		// Jerk
		if (PreviewCameraAccelerationDirection != PreviewCameraMoveInput.GetNormalized())
			PreviewCameraCurrentAccelerationFactor = UMath::Min(PreviewCameraCurrentAccelerationFactor, 0.5f);

		// Accelerate
		PreviewCameraCurrentAccelerationFactor = UMath::Clamp(PreviewCameraCurrentAccelerationFactor + (1.0f / PreviewCameraAccelerationDuration) * dt);
		PreviewCameraAccelerationDirection = PreviewCameraMoveInput.GetNormalized();

		applyLocalInput();
		resetInput();
	}

	void CPrefabTool::RenderPrefab()
	{
		CRenderManager* renderManager = Manager->GetRenderManager();

		if (PrefabData == nullptr)
			return;

		{
			// TODO.NW: Make preview cam projection settings?
			UpdateViewMatrix();

			SRenderCommand command = SRenderCommand(ERenderCommandType::CameraDataStorage);
			command.Matrices.emplace_back(PreviewCameraViewMatrix);
			command.Matrices.emplace_back(PreviewCameraProjectionMatrix);
			renderManager->PushRenderCommand(command, PrefabToolRenderID);
		}

		CScene* scene = PrefabData->Scene.get();

		//Add widgets from editor render system

		const CRenderSystem* renderSystem = GEngine::GetWorld()->GetSystem<CRenderSystem>();
		renderSystem->PushCommandsForScene(scene, PrefabToolRenderID, SEntity::Null, true);
		
		// NW: Custom directional light command
		{
			const SVector4 directionalLightDirection = SVector4(PreviewLightDirection, 0.0f);
			const SVector4 directionalLightColor = { PreviewLightColor.AsVector(), PreviewLightIntensity };

			SShadowmapViewData shadowmapView;
			shadowmapView.ShadowProjectionMatrix = SMatrix::OrthographicLH(8.0f, 8.0f, -8.0f, 8.0f);

			SRenderCommand command;
			command.Type = ERenderCommandType::DeferredLightingDirectional;
			command.Vectors.push_back(directionalLightDirection);
			command.Colors.push_back(directionalLightColor);
			command.ShadowmapViews.push_back(shadowmapView);
			command.RenderTextures.push_back(PreviewSkylight->RenderTexture);
			renderManager->PushRenderCommand(command, PrefabToolRenderID);
		}

		// NW: Custom skybox command
		{
			SRenderCommand command;
			command.RenderTextures.push_back(PreviewSkylight->RenderTexture);
			command.Type = ERenderCommandType::Skybox;
			renderManager->PushRenderCommand(command, PrefabToolRenderID);
		}

		renderSystem->PushUniqueCommands(PrefabToolRenderID);

		CEditorRenderSystem* editorRenderSystem = GEngine::GetWorld()->GetSystem<CEditorRenderSystem>();
		editorRenderSystem->PushCommandsForScene(scene, PrefabToolRenderID, PreviewCameraViewMatrix);
	}
}
