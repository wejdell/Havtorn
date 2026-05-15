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
#include <ECS/ComponentAlgo.h>
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
		CInputMapper* mapper = GEngine::GetInput();
		mapper->GetActionDelegate(EInputActionEvent::PickEditorEntity).AddMember(this, &CPickingSystem::OnMouseClick);
		mapper->GetActionDelegate(EInputActionEvent::ContextPickEditorEntity).AddMember(this, &CPickingSystem::OnMouseClick);
		mapper->GetActionDelegate(EInputActionEvent::ControlPickEditorEntity).AddMember(this, &CPickingSystem::OnMouseClick);
		mapper->GetActionDelegate(EInputActionEvent::ShiftPickEditorEntity).AddMember(this, &CPickingSystem::OnMouseClick);
		mapper->GetAxisDelegate(EInputAxisEvent::MousePositionHorizontal).AddMember(this, &CPickingSystem::OnMouseMove);
		mapper->GetAxisDelegate(EInputAxisEvent::MousePositionVertical).AddMember(this, &CPickingSystem::OnMouseMove);
	}

	CPickingSystem::~CPickingSystem()
	{
		CInputMapper* mapper = GEngine::GetInput();
		mapper->GetActionDelegate(EInputActionEvent::PickEditorEntity).RemoveObject(this);
		mapper->GetActionDelegate(EInputActionEvent::ContextPickEditorEntity).RemoveObject(this);
		mapper->GetActionDelegate(EInputActionEvent::ControlPickEditorEntity).RemoveObject(this);
		mapper->GetActionDelegate(EInputActionEvent::ShiftPickEditorEntity).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::MousePositionHorizontal).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::MousePositionVertical).RemoveObject(this);
	}

	void CPickingSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		SEntity mainCamera = GEngine::GetWorld()->GetMainCamera();
		if (SComponent::IsValid(EditorCameraTransform) && EditorCameraTransform->Owner == mainCamera)
			return;

		SCameraData cameraData = UComponentAlgo::GetCameraData(mainCamera, scenes);
		EditorCameraTransform = cameraData.TransformComponent;
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

	void CPickingSystem::WorldSpacePick(const bool modifierHeld) const
	{
		CWorld* world = GEngine::GetWorld();
		CViewportWindow* viewport = Manager->GetEditorWindow<CViewportWindow>();
		SCameraData cameraData = UComponentAlgo::GetCameraData(world->GetMainCamera(), world->GetActiveScenes());

		if (Manager->GetIsOverGizmo() || Manager->GetIsWorldPlaying() || !viewport->GetIsHovered() || Manager->GetIsModalOpen() || !cameraData.IsValid())
			return;

		SEntity candidate = viewport->GetEntityOnPixel();
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
		viewport->SetContextMenuEntity(viewport->GetEntityOnPixel());
	}
}
