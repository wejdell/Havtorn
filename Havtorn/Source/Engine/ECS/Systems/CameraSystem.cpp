// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CameraSystem.h"
#include "Engine.h"
#include "Scene/Scene.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/CameraComponent.h"
#include "Input/InputMapper.h"

namespace Havtorn
{
	CCameraSystem::CCameraSystem()
		: ISystem()
	{
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Up).AddMember(this, &CCameraSystem::HandleAxisInput);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Right).AddMember(this, &CCameraSystem::HandleAxisInput);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Forward).AddMember(this, &CCameraSystem::HandleAxisInput);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Pitch).AddMember(this, &CCameraSystem::HandleAxisInput);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Yaw).AddMember(this, &CCameraSystem::HandleAxisInput);
		GEngine::GetInput()->GetActionDelegate(EInputActionEvent::ToggleFreeCam).AddMember(this, &CCameraSystem::ToggleFreeCam);
	}

	CCameraSystem::~CCameraSystem()
	{
	}

	void CCameraSystem::Update(CScene* scene)
	{
		const auto& transformComponents = scene->GetTransformComponents();
		const auto& cameraComponents = scene->GetCameraComponents();
		if (cameraComponents.empty())
			return;

		if (IsFreeCamActive)
		{
			const I64 transformCompIndex = cameraComponents[0]->Entity->GetComponentIndex(EComponentType::TransformComponent);
			auto& transformComp = transformComponents[transformCompIndex];

			const F32 dt = GTimer::Dt();
			transformComp->Transform.Translate(CameraMoveInput * dt);
			transformComp->Transform.Rotate(CameraRotateInput * dt);
		}

		CameraMoveInput = SVector::Zero;
		CameraRotateInput = SVector::Zero;
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
			case EInputAxisEvent::Pitch:
				CameraRotateInput.X += UMath::DegToRad(90.0f) * payload.AxisValue;
				return;
			case EInputAxisEvent::Yaw:
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
}
