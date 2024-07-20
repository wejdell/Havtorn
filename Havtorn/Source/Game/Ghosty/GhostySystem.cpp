// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "GhostySystem.h"
#include "GhostyComponent.h"
#include "Scene/Scene.h"
#include "Input/InputMapper.h"

namespace Havtorn
{
	CGhostySystem::CGhostySystem()
		: ISystem()
	{
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Right).AddMember(this, &CGhostySystem::HandleAxisInput);

		EvaluateIdleFunc = std::bind(&CGhostySystem::EvaluateIdle, this, std::placeholders::_1, std::placeholders::_2);
		EvaluateLocomotionFunc = std::bind(&CGhostySystem::EvaluateLocomotion, this, std::placeholders::_1, std::placeholders::_2);
	}

	void CGhostySystem::Update(CScene* scene)
	{
		F32 deltaTime = GTime::Dt();
		
		for (SGhostyComponent* ghostyComponent : scene->GetComponents<SGhostyComponent>())
		{
			if (!ghostyComponent->IsValid())
				continue;

			ghostyComponent->State.Input = input;

			STransformComponent* transform = scene->GetComponent<STransformComponent>(ghostyComponent);
			if (ghostyComponent->State.IsInWalkingAnimationState)
			{
				transform->Transform.Move(ghostyComponent->State.Input * ghostyComponent->State.MoveSpeed * deltaTime);
			}
		}

		ResetInput();
	}

	I16 CGhostySystem::EvaluateIdle(CScene* scene, U64 entitySceneIndex)
	{
		SGhostyComponent* ghostyComponent = scene->GetComponents<SGhostyComponent>()[entitySceneIndex];
		if (ghostyComponent->State.Input.LengthSquared() > 0.0f)
			return 1;

		ghostyComponent->State.IsInWalkingAnimationState = false;
		return 0;
	}

	I16 CGhostySystem::EvaluateLocomotion(CScene* scene, U64 entitySceneIndex)
	{
		SGhostyComponent* ghostyComponent = scene->GetComponents<SGhostyComponent>()[entitySceneIndex];
		SVector stateInput = ghostyComponent->State.Input;

		ghostyComponent->State.IsInWalkingAnimationState = true;

		if (stateInput.X < 0)
			return 0;

		return 1;
	}

	void CGhostySystem::HandleAxisInput(const SInputAxisPayload payload)
	{
		input = SVector::Right * payload.AxisValue;
	}

	void CGhostySystem::ResetInput()
	{
		input.X = 0.0f;
		input.Y = 0.0f;
	}
}