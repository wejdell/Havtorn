// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "PickingSystem.h"

#include "EditorManager.h"
#include "Windows/ViewportWindow.h"

#include <hvpch.h>
#include <Engine.h>
#include <CoreTypes.h>
#include <MathTypes/MathUtilities.h>
#include <MathTypes/Matrix.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/CameraComponent.h>
#include <Graphics/Debug/DebugDrawUtility.h>
#include <Input/InputMapper.h>
#include <Input/InputTypes.h>
#include <Scene/Scene.h>
#include <Graphics/RenderManager.h>
#include <PlatformManager.h>
#include <GUI.h>

namespace Havtorn
{
	CPickingSystem::CPickingSystem(CEditorManager* editorManager)
		: Manager(editorManager)
	{
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::PickEditorEntity).AddMember(this, &CPickingSystem::OnMouseClick);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::ContextPickEditorEntity).AddMember(this, &CPickingSystem::OnMouseClick);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::ControlPickEditorEntity).AddMember(this, &CPickingSystem::OnMouseClick);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::ShiftPickEditorEntity).AddMember(this, &CPickingSystem::OnMouseClick);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::MousePositionHorizontal).AddMember(this, &CPickingSystem::OnMouseMove);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::MousePositionVertical).AddMember(this, &CPickingSystem::OnMouseMove);
	}

	void CPickingSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		SEntity mainCamera = GEngine::GetWorld()->GetMainCamera();
		for (Ptr<CScene>& scene : scenes)
		{
			SCameraComponent* sceneCamera = scene->GetComponent<SCameraComponent>(mainCamera);
			STransformComponent* sceneCameraTransform = scene->GetComponent<STransformComponent>(mainCamera);

			if (sceneCamera != nullptr && sceneCameraTransform != nullptr)
			{
				EditorCameraComponent = sceneCamera;
				EditorCameraTransform = sceneCameraTransform;
			}
		}
	}

	void CPickingSystem::OnMouseClick(const SInputActionPayload payload)
	{
		if (payload.Event == EInputActionEvent::ContextPickEditorEntity)
		{
			if (!SComponent::IsValid(EditorCameraTransform))
				return;

			if (payload.IsPressed)
				ContextPickStartingCameraMatrix = EditorCameraTransform->Transform.GetMatrix();

			if (payload.IsReleased)
			{
				// NW: Track whether FreeCam was meaningfully used during press and release of context pick button
				SMatrix newCameraMatrix = EditorCameraTransform->Transform.GetMatrix();
				if (ContextPickStartingCameraMatrix.NearlyEqual(newCameraMatrix))
					WorldSpaceContextPick();
			}

			return;
		}

		if (payload.IsPressed)
			WorldSpacePick(payload.Event == EInputActionEvent::ControlPickEditorEntity || payload.Event == EInputActionEvent::ShiftPickEditorEntity);
	}

	void CPickingSystem::OnMouseMove(const SInputAxisPayload payload)
	{
		if (payload.Event == EInputAxisEvent::MousePositionHorizontal)
			MousePosition.X = payload.AxisValue;

		if (payload.Event == EInputAxisEvent::MousePositionVertical)
			MousePosition.Y = payload.AxisValue;
	}

	SEntity CPickingSystem::FindEntityInViewport() const
	{
		const CViewportWindow* viewport = Manager->GetEditorWindow<CViewportWindow>();
		if (Manager->GetIsOverGizmo() || Manager->GetIsWorldPlaying() || !viewport->GetIsHovered() || Manager->GetIsModalOpen() || !SComponent::IsValid(EditorCameraComponent) || !SComponent::IsValid(EditorCameraTransform))
			return SEntity::Null;

		const SVector2<F32> renderedSceneDimensions = viewport->GetRenderedSceneDimensions();
		const SVector2<F32> renderedScenePosition = viewport->GetRenderedScenePosition();

		const SVector2<U16> resolution = Manager->GetPlatformManager()->GetResolution();
		const SVector2<F32> rectRelativeMousePos = SVector2((MousePosition.X - renderedScenePosition.X) / renderedSceneDimensions.X, (MousePosition.Y - renderedScenePosition.Y) / renderedSceneDimensions.Y);

		const SVector2<F32> fullscreenMousePos = { UMath::Ceil(STATIC_F32(resolution.X) * rectRelativeMousePos.X), UMath::Ceil(STATIC_F32(resolution.Y) * rectRelativeMousePos.Y - 12.0f) };

		if (!UMath::IsWithin(fullscreenMousePos.X, 0.0f, STATIC_F32(resolution.X)) || !UMath::IsWithin(fullscreenMousePos.Y, 0.0f, STATIC_F32(resolution.Y)))
			return SEntity::Null;

		const U64 dataIndex = STATIC_U64(fullscreenMousePos.X) + STATIC_U64(fullscreenMousePos.Y) * STATIC_U64(resolution.X);
		const U64 pickedEntityGUID = Manager->GetRenderManager()->GetEntityGUIDFromData(dataIndex);

		return SEntity(pickedEntityGUID);
	}

	void CPickingSystem::WorldSpacePick(const bool modifierHeld) const
	{
		SEntity candidate = FindEntityInViewport();
		if (!candidate.IsValid())
			return;

		if (modifierHeld && Manager->IsEntitySelected(candidate))
			Manager->RemoveSelectedEntity(candidate);
		else if (modifierHeld)
			Manager->AddSelectedEntity(candidate);
		else if (!Manager->IsEntitySelected(candidate))
			Manager->SetSelectedEntity(candidate);
	}

	void CPickingSystem::WorldSpaceContextPick()
	{
		// TODO.NW: Might make sense to move the base functionality of this system to the viewport window? And let this system handle the input layer only
		CViewportWindow* viewport = Manager->GetEditorWindow<CViewportWindow>();
		viewport->SetContextMenuEntity(FindEntityInViewport());
	}
}
