// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "GhostySystem.h"
#include "GhostyComponent.h"
#include "Scene/Scene.h"
#include "Scene/World.h"
#include "ECS/Systems/SpriteAnimatorGraphSystem.h"
#include "Input/InputMapper.h"

#include <Engine.h>

namespace Havtorn
{
	CGhostySystem::CGhostySystem()
		: ISystem()
	{
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Right).AddMember(this, &CGhostySystem::HandleAxisInput);

		EvaluateIdleFunc = std::bind(&CGhostySystem::EvaluateIdle, this, std::placeholders::_1, std::placeholders::_2);
		EvaluateLocomotionFunc = std::bind(&CGhostySystem::EvaluateLocomotion, this, std::placeholders::_1, std::placeholders::_2);

		CSpriteAnimatorGraphSystem* animatorGraphSystem = GEngine::GetWorld()->GetSystem<CSpriteAnimatorGraphSystem>();
		
		animatorGraphSystem->BindEvaluateFunction(EvaluateIdleFunc, "CGhostySystem::EvaluateIdle");
		animatorGraphSystem->BindEvaluateFunction(EvaluateLocomotionFunc, "CGhostySystem::EvaluateLocomotion");
	}

	void CGhostySystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		for (Ptr<CScene>& scene : scenes)
		{
			for (SGhostyComponent* ghostyComponent : scene->GetComponents<SGhostyComponent>())
			{
				if (!SComponent::IsValid(ghostyComponent))
					continue;

				ghostyComponent->State.Input = input;

				STransformComponent* transform = scene->GetComponent<STransformComponent>(ghostyComponent);
				if (!SComponent::IsValid(transform))
					continue;

				SPhysics2DComponent* physComponent = scene->GetComponent<SPhysics2DComponent>(ghostyComponent);
				if (!SComponent::IsValid(physComponent))
					continue;

				if (ghostyComponent->State.IsInWalkingAnimationState)
				{
					physComponent->Velocity.X = ghostyComponent->State.Input.X * ghostyComponent->State.MoveSpeed;
					GEngine::GetWorld()->Update2DPhysicsData(transform, physComponent);
				}
			}
		}
		
		ResetInput();
	}

	I16 CGhostySystem::EvaluateIdle(CScene* scene, const SEntity& entity)
	{
		SGhostyComponent* ghostyComponent = scene->GetComponent<SGhostyComponent>(entity);
		if (ghostyComponent->State.Input.LengthSquared() > 0.0f)
			return 1;

		ghostyComponent->State.IsInWalkingAnimationState = false;
		return 0;
	}

	I16 CGhostySystem::EvaluateLocomotion(CScene* scene, const SEntity& entity)
	{
		SGhostyComponent* ghostyComponent = scene->GetComponent<SGhostyComponent>(entity);
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