// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Input.h"

#include "Engine.h"

#include <ranges>

#include <../Platform/PlatformManager.h>

// TODO.NW: Move this system to core or platform?
#include <SDL3/SDL.h>
#include <SDL3/SDL_gamepad.h>

namespace Havtorn
{
	CInput* CInput::GetInstance()
	{
		static auto input = new CInput();
		return input;
	}

	CInput::CInput()
	{
		ActiveGamepadDevices.fill(nullptr);
		AxisInputValues.fill(0.0f);
	}

	CInput::~CInput()
	{
		if (ActiveGamepadDevices[PrimaryUser])
			SDL_CloseGamepad(ActiveGamepadDevices[PrimaryUser]);
	}

	bool CInput::Init(CPlatformManager* platformManager)
	{
		if (platformManager == nullptr)
			return false;

		platformManager->OnProcessEvent.AddMember(this, &CInput::ProcessEvent);	
		return true;
	}

	void CInput::ProcessEvent(const SDL_Event* event)
	{
		switch (event->type)
		{
		case SDL_EVENT_KEYBOARD_ADDED:
			break;
		case SDL_EVENT_KEYBOARD_REMOVED:
			break;

		case SDL_EVENT_GAMEPAD_ADDED:
		{
			/* this event is sent for each hotplugged stick, but also each already-connected gamepad during SDL_Init(). */
			if (ActiveGamepadDevices[PrimaryUser])
				SDL_CloseGamepad(ActiveGamepadDevices[PrimaryUser]);

			const SDL_JoystickID which = event->gdevice.which;
			ActiveGamepadDevices[PrimaryUser] = SDL_OpenGamepad(which);
			if (!ActiveGamepadDevices[PrimaryUser])
			{
				HV_LOG_ERROR("Gamepad #%u add, but not opened: %s", STATIC_U32(which), SDL_GetError());
			}
			else
			{
				char* mapping = SDL_GetGamepadMapping(ActiveGamepadDevices[PrimaryUser]);
				HV_LOG_INFO("Gamepad #%u ('%s') added", STATIC_U32(which), SDL_GetGamepadName(ActiveGamepadDevices[PrimaryUser]));
				if (mapping)
				{
					HV_LOG_INFO("Gamepad #%u mapping: %s", STATIC_U32(which), mapping);
					SDL_free(mapping);
				}
			}
		}
		break;

		case SDL_EVENT_GAMEPAD_REMOVED:              /**< A gamepad has been removed */
		{
			const SDL_JoystickID which = event->gdevice.which;
			ActiveGamepadDevices[PrimaryUser] = SDL_GetGamepadFromID(which);
			if (ActiveGamepadDevices[PrimaryUser])
			{
				SDL_CloseGamepad(ActiveGamepadDevices[PrimaryUser]);  /* the gamepad was unplugged. */
			}
			HV_LOG_INFO("Gamepad #%u removed", STATIC_U32(which));
		}
		break;

		case SDL_EVENT_KEY_DOWN:			
			UpdateModifiers(event->key.mod, true);
			HandleKeyDown(event->key.key);
			break;

		case SDL_EVENT_KEY_UP:
			UpdateModifiers(event->key.mod, false);
			HandleKeyUp(event->key.key);
			break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			HandleKeyDown(STATIC_U32(event->button.button));
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			HandleKeyUp(STATIC_U32(event->button.button));
			break;

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			HandleKeyDown(event->gbutton.button + STATIC_U32(EInputKey::GamepadRegionStart));
			break;

		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			HandleKeyUp(event->gbutton.button + STATIC_U32(EInputKey::GamepadRegionStart));
			break;

		case SDL_EVENT_MOUSE_MOTION:
			HandleAxisEvent(EInputAxis::MousePositionHorizontal, event->motion.x);
			HandleAxisEvent(EInputAxis::MousePositionVertical, event->motion.y);
			HandleAxisEvent(EInputAxis::MouseDeltaHorizontal, event->motion.xrel);
			HandleAxisEvent(EInputAxis::MouseDeltaVertical, event->motion.yrel);
			break;

		case SDL_EVENT_MOUSE_WHEEL:
			HandleAxisEvent(EInputAxis::MouseWheel, event->wheel.y);
			break;

		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		{
			const F32 axisValue = STATIC_F32(event->gaxis.value) / 32767.0f;
			HandleAxisEvent(static_cast<EInputAxis>(event->gaxis.axis + STATIC_U8(EInputAxis::GamepadRegionStart)), axisValue);
		}
		break;

		case SDL_EVENT_GAMEPAD_REMAPPED:             /**< The gamepad mapping was updated */
			break;
		case SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN:        /**< Gamepad touchpad was touched */
			break;
		case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION:      /**< Gamepad touchpad finger was moved */
			break;
		case SDL_EVENT_GAMEPAD_TOUCHPAD_UP:          /**< Gamepad touchpad finger was lifted */
			break;
		case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:        /**< Gamepad sensor was updated */
			break;
		case SDL_EVENT_GAMEPAD_UPDATE_COMPLETE:      /**< Gamepad update is complete */
			break;
		case SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED: /**< Gamepad Steam handle has changed */
			break;

		default:
			break;
		}
	}

	void CInput::EndFrameUpdate()
	{
		for (auto& keyInput : KeyInputBuffer | std::views::values)
		{
			if (keyInput.IsPressed)
			{
				keyInput.IsPressed = false;
				keyInput.IsHeld = true;
			}
		}

		for (auto it = KeyInputBuffer.cbegin(); it != KeyInputBuffer.cend();)
		{
			auto& keyInput = it->second;

			if (keyInput.IsReleased)
				it = KeyInputBuffer.erase(it);

			else
				++it;
		}

		HandleAxisEvent(EInputAxis::MouseWheel, 0.0f);
		HandleAxisEvent(EInputAxis::MouseDeltaHorizontal, 0.0f);
		HandleAxisEvent(EInputAxis::MouseDeltaVertical, 0.0f);

		constexpr F32 deadzone = 0.07f;
		for (EInputAxis axis = EInputAxis::GamepadRegionStart; axis < EInputAxis::Count; axis = static_cast<EInputAxis>(STATIC_U8(axis) + 1))
		{
			const F32 currentValue = AxisInputValues[STATIC_U64(axis)];

			if (axis >= EInputAxis::GamepadRegionStart && UMath::Abs(currentValue) > deadzone)
				continue;

			HandleAxisEvent(axis, 0.0f);
		}
	}

	const std::map<U32, SInputActionPayload>& CInput::GetKeyInputBuffer() const
	{
		return KeyInputBuffer;
	}

	const std::array<F32, STATIC_U64(EInputAxis::Count)>& CInput::GetAxisInputValues() const
	{
		return AxisInputValues;
	}

	const std::bitset<3>& CInput::GetKeyInputModifiers() const
	{
		return KeyInputModifiers;
	}

	void CInput::HandleKeyDown(const U32& keyCode)
	{
		if (KeyInputBuffer.contains(keyCode))
		{
			if (KeyInputBuffer[keyCode].IsPressed)
			{
				KeyInputBuffer[keyCode].IsPressed = false;
				KeyInputBuffer[keyCode].IsHeld = true;
			}
			else if (!KeyInputBuffer[keyCode].IsHeld)
			{
				KeyInputBuffer[keyCode].IsPressed = true;
			}
		}
		else
		{
			KeyInputBuffer.emplace(keyCode, SInputActionPayload());
			KeyInputBuffer[keyCode].Key = static_cast<EInputKey>(keyCode);
			KeyInputBuffer[keyCode].IsPressed = true;
		}
	}

	void CInput::HandleKeyUp(const U32& keyCode)
	{
		KeyInputBuffer[keyCode].IsPressed = false;
		KeyInputBuffer[keyCode].IsHeld = false;
		KeyInputBuffer[keyCode].IsReleased = true;
	}

	void CInput::HandleAxisEvent(const EInputAxis axis, const F32 value)
	{		
		AxisInputValues[STATIC_U64(axis)] = value;
	}

	void CInput::UpdateModifiers(const U32& modifiers, const bool pressedKey)
	{
		// TODO.NW: Add super (GUI/Windows) key mod
		const U32 modValue = modifiers - 4096;

		if ((modValue & SDL_KMOD_SHIFT) != 0)
			KeyInputModifiers[0] = pressedKey;
		if ((modValue & SDL_KMOD_CTRL) != 0)
			KeyInputModifiers[1] = pressedKey;
		if ((modValue & SDL_KMOD_ALT) != 0)
			KeyInputModifiers[2] = pressedKey;
	}
}
