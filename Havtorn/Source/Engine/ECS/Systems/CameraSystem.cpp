// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraSystem.h"
#include "Engine.h"
#include "Scene/World.h"
#include "Scene/Scene.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/CameraControllerComponent.h"
#include "ECS/ComponentAlgo.h"
#include "Input/InputMapper.h"

namespace Havtorn
{
	CCameraSystem::CCameraSystem()
		: ISystem()
	{
		CInputMapper* mapper = GEngine::GetInput();
		mapper->GetAxisDelegate(EInputAxisEvent::Up).AddMember(this, &CCameraSystem::HandleAxisInput);
		mapper->GetAxisDelegate(EInputAxisEvent::Right).AddMember(this, &CCameraSystem::HandleAxisInput);
		mapper->GetAxisDelegate(EInputAxisEvent::Forward).AddMember(this, &CCameraSystem::HandleAxisInput);
		mapper->GetAxisDelegate(EInputAxisEvent::MouseDeltaHorizontal).AddMember(this, &CCameraSystem::HandleAxisInput);
		mapper->GetAxisDelegate(EInputAxisEvent::MouseDeltaVertical).AddMember(this, &CCameraSystem::HandleAxisInput);
		mapper->GetAxisDelegate(EInputAxisEvent::Zoom).AddMember(this, &CCameraSystem::HandleAxisInput);
		mapper->GetActionDelegate(EInputActionEvent::ToggleFreeCam).AddMember(this, &CCameraSystem::ToggleFreeCam);
		
		CWorld* world = GEngine::GetWorld();
		world->OnBeginPlayDelegate.AddMember(this, &CCameraSystem::OnBeginPlay);
		world->OnPausePlayDelegate.AddMember(this, &CCameraSystem::OnPausePlay);
		world->OnEndPlayDelegate.AddMember(this, &CCameraSystem::OnEndPlay);
	}

	CCameraSystem::~CCameraSystem()
	{
		CInputMapper* mapper = GEngine::GetInput();
		mapper->GetAxisDelegate(EInputAxisEvent::Up).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::Right).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::Forward).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::MouseDeltaHorizontal).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::MouseDeltaVertical).RemoveObject(this);
		mapper->GetAxisDelegate(EInputAxisEvent::Zoom).RemoveObject(this);
		mapper->GetActionDelegate(EInputActionEvent::ToggleFreeCam).RemoveObject(this);

		CWorld* world = GEngine::GetWorld();
		world->OnBeginPlayDelegate.RemoveObject(this);
		world->OnPausePlayDelegate.RemoveObject(this);
		world->OnEndPlayDelegate.RemoveObject(this);
	}

	void CCameraSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{	
		SEntity mainCamera = GEngine::GetWorld()->GetMainCamera();

		for (Ptr<CScene>& scene : scenes)
		{
			SCameraControllerComponent* controllerComp = scene->GetComponent<SCameraControllerComponent>(mainCamera);
			STransformComponent* transformComp = scene->GetComponent<STransformComponent>(mainCamera);

			if (controllerComp == nullptr || transformComp == nullptr)
				continue;

			const F32 dt = GTime::Dt();

			// === Speed ===
			if (CameraSpeedInput < 0.0f)
				controllerComp->MaxMoveSpeed = UMath::Remap(-5.0f, -1.0f, 0.2f, 2.0f, CameraSpeedInput);
			else
				controllerComp->MaxMoveSpeed = UMath::Remap(0.0f, 5.0f, 3.0f, 10.0f, CameraSpeedInput);

			if (!IsFreeCamActive)
			{
				// Decelerate
				controllerComp->CurrentAccelerationFactor = UMath::Clamp(controllerComp->CurrentAccelerationFactor - (1.0f / controllerComp->AccelerationDuration) * dt);
				transformComp->Transform.Translate(controllerComp->AccelerationDirection * controllerComp->CurrentAccelerationFactor * controllerComp->MaxMoveSpeed * dt);

				ResetInput();
				continue;
			}

			// === Rotation ===
			controllerComp->CurrentPitch = UMath::Clamp(controllerComp->CurrentPitch + (CameraRotateInput.X * controllerComp->RotationSpeed * dt), -SCameraControllerComponent::MaxPitchDegrees + 0.01f, SCameraControllerComponent::MaxPitchDegrees - 0.01f);
			controllerComp->CurrentYaw = UMath::WrapAngle(controllerComp->CurrentYaw + (CameraRotateInput.Y * controllerComp->RotationSpeed * dt));
			SMatrix newMatrix = transformComp->Transform.GetMatrix();
			newMatrix.SetRotation({ controllerComp->CurrentPitch, controllerComp->CurrentYaw, 0.0f });
			transformComp->Transform.SetMatrix(newMatrix);

			// === Translation ===
			if (CameraMoveInput.IsNearlyZero())
			{
				// Decelerate
				controllerComp->CurrentAccelerationFactor = UMath::Clamp(controllerComp->CurrentAccelerationFactor - (1.0f / controllerComp->AccelerationDuration) * dt);
				transformComp->Transform.Translate(controllerComp->AccelerationDirection * controllerComp->CurrentAccelerationFactor * controllerComp->MaxMoveSpeed * dt);

				ResetInput();
				continue;
			}

			// Jerk
			if (controllerComp->AccelerationDirection != CameraMoveInput.GetNormalized())
				controllerComp->CurrentAccelerationFactor = UMath::Min(controllerComp->CurrentAccelerationFactor, 0.5f);
		 
			// Accelerate
			controllerComp->CurrentAccelerationFactor = UMath::Clamp(controllerComp->CurrentAccelerationFactor + (1.0f / controllerComp->AccelerationDuration) * dt);
			controllerComp->AccelerationDirection = CameraMoveInput.GetNormalized();	
		
			transformComp->Transform.Translate(controllerComp->AccelerationDirection * controllerComp->CurrentAccelerationFactor * controllerComp->MaxMoveSpeed * dt);

			ResetInput();
		}
	}

	void CCameraSystem::HandleAxisInput(const SInputAxisPayload payload)
	{
		switch (payload.Event)
		{
			case EInputAxisEvent::Right: 
				CameraMoveInput += SVector::Right * payload.AxisValue;
				return;
			case EInputAxisEvent::Up:
				CameraMoveInput += SVector::Up * payload.AxisValue;
				return;
			case EInputAxisEvent::Forward:
				CameraMoveInput += SVector::Forward * payload.AxisValue;
				return;
			case EInputAxisEvent::MouseDeltaVertical:
				CameraRotateInput.X += 90.0f * payload.AxisValue;
				return;
			case EInputAxisEvent::MouseDeltaHorizontal:
				CameraRotateInput.Y += 90.0f * payload.AxisValue;
				return;
			case EInputAxisEvent::Zoom:
			{
				if (IsFreeCamActive)
					CameraSpeedInput = UMath::Clamp(CameraSpeedInput + payload.AxisValue, -5.0f, 5.0f);
			}
				return;
			default: 
				return;
		}
	}
	
	void CCameraSystem::ToggleFreeCam(const SInputActionPayload payload)
	{
		IsFreeCamActive = payload.IsHeld;
	}

	void CCameraSystem::OnBeginPlay(std::vector<Ptr<CScene>>& scenes)
	{
		const SEntity& mainCamera = GEngine::GetWorld()->GetMainCamera();
		SCameraData previousCameraData = UComponentAlgo::GetCameraData(mainCamera, scenes);
		SEntity startingCamera = SEntity::Null;
		PreviousMainCamera = mainCamera;

		for (Ptr<CScene>& scene : scenes)
		{
			for (SCameraComponent* camera : scene->GetComponents<SCameraComponent>())
			{
				// TODO: Loud errors if multiple cameras are marked as starting cameras
				if (camera->IsStartingCamera)
				{
					camera->IsActive = true;
					startingCamera = camera->Owner;
					break;
				}
			}
		}

		if (mainCamera != startingCamera)
		{
			if (previousCameraData.IsValid())
				previousCameraData.CameraComponent->IsActive = false;

			GEngine::GetWorld()->SetMainCamera(startingCamera);
		}
	}

	void CCameraSystem::OnPausePlay(std::vector<Ptr<CScene>>& /*scenes*/)
	{
	}

	void CCameraSystem::OnEndPlay(std::vector<Ptr<CScene>>& scenes)
	{
		const SEntity& startingCamera = GEngine::GetWorld()->GetMainCamera();
		SCameraData startingCameraData = UComponentAlgo::GetCameraData(startingCamera, scenes);
		SCameraData previousCameraData = UComponentAlgo::GetCameraData(PreviousMainCamera, scenes);

		if (PreviousMainCamera != startingCamera && PreviousMainCamera.IsValid())
		{
			if (startingCameraData.IsValid())
				startingCameraData.CameraComponent->IsActive = false;
			if (previousCameraData.IsValid())
				previousCameraData.CameraComponent->IsActive = true;

			GEngine::GetWorld()->SetMainCamera(PreviousMainCamera);
		}

		PreviousMainCamera = SEntity::Null;
	}
	
	void CCameraSystem::SetCameraSpeed(const F32 speed)
	{
		if (speed < 3.0f)
			CameraSpeedInput = UMath::Remap(0.2f, 2.99f, -5.0f, -0.0f, speed);
		else
			CameraSpeedInput = UMath::Remap(3.0f, 10.0f, 0.0f, 5.0f, speed);

		HV_LOG_INFO("Speed Input: %f", CameraSpeedInput);
	}

	F32 CCameraSystem::GetCameraSpeed() const
	{
		if (CameraSpeedInput < 0.0f)
			return UMath::Remap(-5.0f, -1.0f, 0.2f, 2.0f, CameraSpeedInput);
		else
			return UMath::Remap(0.0f, 5.0f, 3.0f, 10.0f, CameraSpeedInput);
	}

	void CCameraSystem::ResetInput()
	{
		CameraMoveInput = SVector::Zero;
		CameraRotateInput = SVector::Zero;
	}
}
