// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "hvpch.h"

#include "InputSystem.h"

#include "Engine.h"
#include "ECS/ECSInclude.h"

#include "Input/InputMapper.h"
#include "Scene/Scene.h"

#include "GameplayTags/GameplayTagManager.h"

namespace Havtorn
{
	CInputSystem::CInputSystem()
		: ISystem()
	{
	}

	CInputSystem::~CInputSystem()
	{
	}

	void CInputSystem::Update(std::vector<Ptr<CScene>>& scenes)
	{
		const CInputMapper* input = GEngine::GetInput();

		for (auto& scene : scenes)
		{
			std::vector<SInputComponent*> inputComponents = scene->GetComponents<SInputComponent>();
			for (auto& inputComponent : inputComponents)
			{
				SHexCommandComponent* hexCommandComponent = scene->GetComponent<SHexCommandComponent>(inputComponent);

				if (!SComponent::IsValid(hexCommandComponent))
					continue;

				SInputAsset* inputAsset = GEngine::GetAssetRegistry()->RequestAssetData<SInputAsset>(inputComponent->AssetReference, inputComponent->Owner.GUID);
				if (inputAsset == nullptr)
					continue;
		
				for (auto& inputAction : inputAsset->InputActions)
				{
					if (GGameplayTagManager::ContainsTag(inputAction.Tag, hexCommandComponent->TagsToListenFor))
					{					
						for (auto& mapping : inputAction.InputMappings)
						{
							const U32 typeIndex = STATIC_U32(mapping.Data.index());
							switch (typeIndex)
							{
							case 0:
							{
								SAxis& axis = std::get<SAxis>(mapping.Data);

								// NW: Only continuous axis detection is allowed for now
								const F32 axisValue = input->GetAxisValue(axis.Axis, axis.Modifiers);
								const bool isPositiveHeld = input->IsPressed(axis.AxisPositiveKey, axis.Modifiers) || input->IsHeld(axis.AxisPositiveKey, axis.Modifiers);
								const bool isNegativeHeld = input->IsPressed(axis.AxisNegativeKey, axis.Modifiers) || input->IsHeld(axis.AxisNegativeKey, axis.Modifiers);

								F32 finalAxisValue = axisValue;
								
								if (isNegativeHeld)
									finalAxisValue = -1.0f;

								if (isPositiveHeld)
									finalAxisValue = 1.0f;

								if (UMath::Abs(finalAxisValue) > 0.0f)
								{
									const SHexCommand axisCommand = { .Tag = inputAction.Tag, .DataType = EHexCommandDataType::Float, .Data = finalAxisValue };
									hexCommandComponent->HexCommands.push(axisCommand);
								}
							}
							break;
							case 1:
							{
								SKey& key = std::get<SKey>(mapping.Data);

								const bool isInputPressed = input->IsPressed(key.Key, key.Modifiers);
								const bool isInputHeld = input->IsHeld(key.Key, key.Modifiers);
								const bool isInputReleased = input->IsReleased(key.Key, key.Modifiers);

								bool isActivated = false;
								if (mapping.ActivationType == EInputActivationType::Continuous && (isInputPressed || isInputHeld))
									isActivated = true;
								else if (mapping.ActivationType == EInputActivationType::KeyDown && isInputPressed)
									isActivated = true;
								else if (mapping.ActivationType == EInputActivationType::KeyUp && isInputReleased)
									isActivated = true;

								if (isActivated)
								{
									const SHexCommand keyCommand = { .Tag = inputAction.Tag, .DataType = EHexCommandDataType::Bool, .Data = isActivated };
									hexCommandComponent->HexCommands.push(keyCommand);
								}
							}
							break;
							}
						}
					}
				}
			}
		}
	}
}
