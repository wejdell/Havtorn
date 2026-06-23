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
					if (!GGameplayTagManager::ContainsTag(inputAction.Tag, hexCommandComponent->TagsToListenFor))
						continue;

					for (auto& mapping : inputAction.InputMappings)
					{
						const U32 typeIndex = STATIC_U32(mapping.Data.index());
						switch (typeIndex)
						{
						case 0:
						{
							const SAxis& axis = std::get<SAxis>(mapping.Data);
							const F32 axisValue = GetAxisValue(axis, input);

							if (UMath::Abs(axisValue) > 0.0f)
							{
								const SHexCommand axisCommand = { .Tag = inputAction.Tag, .DataType = EHexCommandDataType::Float, .Data = axisValue };
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
							if (mapping.ActivationType == EInputActivationType::Continuous && (isInputHeld || isInputPressed))
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
						case 2:
						{
							const S2DAxis& data = std::get<S2DAxis>(mapping.Data);
							const F32 horizontalAxisValue = GetAxisValue(data.HorizontalAxis, input);
							const F32 verticalAxisValue = GetAxisValue(data.VerticalAxis, input);

							if (UMath::Abs(horizontalAxisValue) > 0.0f || UMath::Abs(verticalAxisValue) > 0.0f)
							{
								const SHexCommand axisCommand = { .Tag = inputAction.Tag, .DataType = EHexCommandDataType::Vector2, .Data = SVector2<F32>(horizontalAxisValue, verticalAxisValue) };
								hexCommandComponent->HexCommands.push(axisCommand);
							}
						}
						break;
						}
					}

				}
			}
		}
	}

	F32 CInputSystem::GetAxisValue(const SAxis& axis, const CInputMapper* inputMapper) const
	{
		F32 axisValue = 0.0f;

		if (axis.Axis == EInputAxis::Key)
		{
			// NW: Only continuous axis detection is allowed for now
			const bool isPositiveHeld = inputMapper->IsHeld(axis.AxisPositiveKey, axis.Modifiers) || inputMapper->IsPressed(axis.AxisPositiveKey, axis.Modifiers);
			const bool isNegativeHeld = inputMapper->IsHeld(axis.AxisNegativeKey, axis.Modifiers) || inputMapper->IsPressed(axis.AxisNegativeKey, axis.Modifiers);

			// NW: I think it makes sense to cancel the axis out if both keys are held, in which case we keep the initial 0.0f value from initialization. 
			// This way we get less issues with floating point precision

			if (isPositiveHeld && !isNegativeHeld)
				axisValue = 1.0f;
			else if (isNegativeHeld && !isPositiveHeld)
				axisValue = -1.0f;
		}
		else
		{
			axisValue = inputMapper->GetAxisValue(axis.Axis, axis.Modifiers);
		}

		return axisValue;
	}
}
