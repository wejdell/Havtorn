// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraSystem.h"
#include "Engine.h"
#include "Scene/Scene.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/CameraComponent.h"
#include "ECS/Components/CameraControllerComponent.h"
#include "Input/InputMapper.h"

namespace Havtorn
{
	CCameraSystem::CCameraSystem()
		: ISystem()
	{
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Up).AddMember(this, &CCameraSystem::HandleAxisInput);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Right).AddMember(this, &CCameraSystem::HandleAxisInput);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Forward).AddMember(this, &CCameraSystem::HandleAxisInput);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::MouseHorizontal).AddMember(this, &CCameraSystem::HandleAxisInput);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::MouseVertical).AddMember(this, &CCameraSystem::HandleAxisInput);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::ToggleFreeCam).AddMember(this, &CCameraSystem::ToggleFreeCam);
	}

	CCameraSystem::~CCameraSystem()
	{
	}

	void CCameraSystem::Update(CScene* scene)
	{	
		if (!scene->MainCameraEntity.IsValid())
			return;

		SCameraControllerComponent& controllerComp = *scene->GetComponent<SCameraControllerComponent>(scene->MainCameraEntity);
		STransformComponent& transformComp = *scene->GetComponent<STransformComponent>(scene->MainCameraEntity);

		const F32 dt = GTime::Dt();

		if (!IsFreeCamActive)
		{
			// Decelerate
			controllerComp.CurrentAccelerationFactor = UMath::Clamp(controllerComp.CurrentAccelerationFactor - (1.0f / controllerComp.AccelerationDuration) * dt);
			transformComp.Transform.Translate(controllerComp.AccelerationDirection * controllerComp.CurrentAccelerationFactor * controllerComp.MaxMoveSpeed * dt);

			ResetInput();
			return;
		}

		// === Rotation ===
		controllerComp.CurrentPitch = UMath::Clamp(controllerComp.CurrentPitch + (CameraRotateInput.X * controllerComp.RotationSpeed * dt), -SCameraControllerComponent::MaxPitchDegrees + 0.01f, SCameraControllerComponent::MaxPitchDegrees - 0.01f);
		controllerComp.CurrentYaw = UMath::WrapAngle(controllerComp.CurrentYaw + (CameraRotateInput.Y * controllerComp.RotationSpeed * dt));
		SMatrix newMatrix = transformComp.Transform.GetMatrix();
		newMatrix.SetRotation({ controllerComp.CurrentPitch, controllerComp.CurrentYaw, 0.0f });
		transformComp.Transform.SetMatrix(newMatrix);

		// === Translation ===
		if (CameraMoveInput.IsNearlyZero())
		{
			// Decelerate
			controllerComp.CurrentAccelerationFactor = UMath::Clamp(controllerComp.CurrentAccelerationFactor - (1.0f / controllerComp.AccelerationDuration) * dt);
			transformComp.Transform.Translate(controllerComp.AccelerationDirection * controllerComp.CurrentAccelerationFactor * controllerComp.MaxMoveSpeed * dt);

			ResetInput();
			return;
		}

		// Jerk
		if (controllerComp.AccelerationDirection != CameraMoveInput.GetNormalized())
			controllerComp.CurrentAccelerationFactor = UMath::Min(controllerComp.CurrentAccelerationFactor, 0.5f);
		 
		// Accelerate
		controllerComp.CurrentAccelerationFactor = UMath::Clamp(controllerComp.CurrentAccelerationFactor + (1.0f / controllerComp.AccelerationDuration) * dt);
		controllerComp.AccelerationDirection = CameraMoveInput.GetNormalized();	
		
		transformComp.Transform.Translate(controllerComp.AccelerationDirection * controllerComp.CurrentAccelerationFactor * controllerComp.MaxMoveSpeed * dt);

		ResetInput();
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
			case EInputAxisEvent::MouseVertical:
				CameraRotateInput.X += 90.0f * payload.AxisValue;
				return;
			case EInputAxisEvent::MouseHorizontal:
				CameraRotateInput.Y += 90.0f * payload.AxisValue;
				return;
			default: 
				return;
		}
	}
	
	void CCameraSystem::ToggleFreeCam(const SInputActionPayload payload)
	{
		IsFreeCamActive = payload.IsHeld;
	}
	
	void CCameraSystem::ResetInput()
	{
		CameraMoveInput = SVector::Zero;
		CameraRotateInput = SVector::Zero;
	}
}
