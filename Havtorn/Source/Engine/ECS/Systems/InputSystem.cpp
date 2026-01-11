// Copyright 2025 Team Havtorn. All Rights Reserved.
#include "hvpch.h"

#include "InputSystem.h"
#include "Engine.h"
#include "Scene/Scene.h"

#include "Input/InputMapper.h"
#include "../Components/InputComponent.h"


namespace Havtorn
{
	CInputSystem::CInputSystem()
		: ISystem()
	{
		DelegateAxisForwardHandle = GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Forward).AddMember(this, &CInputSystem::HandleMoveInput);
		DelegateAxisRightHandle = GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Right).AddMember(this, &CInputSystem::HandleMoveInput);
	}

	CInputSystem::~CInputSystem()
	{
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Forward).RemoveHandle(DelegateAxisForwardHandle);
		GEngine::GetInput()->GetAxisDelegate(EInputAxisEvent::Right).RemoveHandle(DelegateAxisRightHandle);
	}

	void CInputSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		for (auto& scene : scenes)
		{
			std::vector<SInputComponent*> components = scene->GetComponents<SInputComponent>();
			for (auto& component : components)
			{
				if (component->IsActive)
				{
					component->MoveInput = GlobalMoveInput;
				}
				else
				{
					component->MoveInput = {};
				}			
			}
		}

		GlobalMoveInput = {};
	}

	void CInputSystem::HandleMoveInput(const SInputAxisPayload payload)
	{
		switch (payload.Event)
		{
		case EInputAxisEvent::Right:
			GlobalMoveInput.X = payload.AxisValue;
			break;
		case EInputAxisEvent::Forward:
			GlobalMoveInput.Z = payload.AxisValue;
			break;
		}
	}


}
