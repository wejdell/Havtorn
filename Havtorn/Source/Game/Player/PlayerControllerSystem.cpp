// Copyright 2026 Team Havtorn. All Rights Reserved.
#include "PlayerControllerSystem.h"

#include <ECS/Components/Physics3DControllerComponent.h>
#include <ECS/Components/InputComponent.h>

namespace Havtorn
{
	CPlayerControllerSystem::CPlayerControllerSystem()
		: ISystem()
	{
	}

	CPlayerControllerSystem::~CPlayerControllerSystem()
	{
	}

	void CPlayerControllerSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		F32 deltaTime = GTime::Dt();
		for (auto& scene : scenes)
		{
			std::vector<SPhysics3DControllerComponent*> components = scene->GetComponents<SPhysics3DControllerComponent>();
			for (SPhysics3DControllerComponent* component : components)
			{
				SEntity entity = component->Owner;
				SInputComponent* input = scene->GetComponent<SInputComponent>(entity);
				
				if (!SComponent::IsValid(input))
					continue;

				component->Displacement = input->MoveInput * 5.0f * deltaTime;
			}
		}
	}
}