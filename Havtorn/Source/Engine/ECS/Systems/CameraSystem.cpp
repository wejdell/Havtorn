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
		const auto& cameraComponents = scene->GetCameraComponents();
		
		if (cameraComponents.empty())
			return;

		const auto& transformComponents = scene->GetTransformComponents();
		const auto& cameraControllerComponents = scene->GetCameraControllerComponents();
	
		const I64 controllerCompIndex = cameraComponents[0]->Entity->GetComponentIndex(EComponentType::CameraControllerComponent);
		auto& controllerComp = cameraControllerComponents[controllerCompIndex];

		const I64 transformCompIndex = cameraComponents[0]->Entity->GetComponentIndex(EComponentType::TransformComponent);
		auto& transformComp = transformComponents[transformCompIndex];

		const F32 dt = GTimer::Dt();

		if (!IsFreeCamActive)
		{
			// Decelerate
			controllerComp->CurrentAccelerationFactor = UMath::Clamp(controllerComp->CurrentAccelerationFactor - (1.0f / controllerComp->AccelerationDuration) * dt);
			F32 easedFactor = UMath::EaseInOutCubic(controllerComp->CurrentAccelerationFactor);
			transformComp->Transform.Translate(controllerComp->AccelerationDirection * easedFactor * controllerComp->MaxMoveSpeed * dt);

			ResetInput();
			return;
		}

		// === Rotation ===
		controllerComp->CurrentPitch = UMath::Clamp(controllerComp->CurrentPitch + (CameraRotateInput.X * controllerComp->RotationSpeed * dt), (-UMath::Pi * 0.5f) + 0.01f, (UMath::Pi * 0.5f) - 0.01f);
		controllerComp->CurrentYaw = UMath::WrapAngle(controllerComp->CurrentYaw + (CameraRotateInput.Y * controllerComp->RotationSpeed * dt));
		transformComp->Transform.GetMatrix().SetRotation({ controllerComp->CurrentPitch, controllerComp->CurrentYaw, 0.0f });

		// === Translation ===
		if (CameraMoveInput.IsNearlyZero())
		{
			// Decelerate
			controllerComp->CurrentAccelerationFactor = UMath::Clamp(controllerComp->CurrentAccelerationFactor - (1.0f / controllerComp->AccelerationDuration) * dt);
			F32 easedFactor = UMath::EaseInOutCubic(controllerComp->CurrentAccelerationFactor);
			transformComp->Transform.Translate(controllerComp->AccelerationDirection * easedFactor * controllerComp->MaxMoveSpeed * dt);

			ResetInput();
			return;
		}

		// Jerk
		if (controllerComp->AccelerationDirection != CameraMoveInput.GetNormalized())
			controllerComp->CurrentAccelerationFactor = UMath::Min(controllerComp->CurrentAccelerationFactor, 0.4f);
		 
		// Accelerate
		controllerComp->CurrentAccelerationFactor = UMath::Clamp(controllerComp->CurrentAccelerationFactor + (1.0f / controllerComp->AccelerationDuration) * dt);
		controllerComp->AccelerationDirection = CameraMoveInput.GetNormalized();	
		
		F32 easedFactor = UMath::EaseInOutCubic(controllerComp->CurrentAccelerationFactor);
		transformComp->Transform.Translate(controllerComp->AccelerationDirection * easedFactor * controllerComp->MaxMoveSpeed * dt);

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
				CameraRotateInput.X += UMath::DegToRad(90.0f) * payload.AxisValue;
				return;
			case EInputAxisEvent::MouseHorizontal:
				CameraRotateInput.Y += UMath::DegToRad(90.0f) * payload.AxisValue;
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
