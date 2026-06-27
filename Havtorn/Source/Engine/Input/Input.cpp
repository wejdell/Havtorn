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
		constexpr U32 gamepadButtonInvalid = STATIC_U32(EInputButton::GamepadInvalid);

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
		{
			const U32 keyScanCode = STATIC_U32(event->key.scancode);
			if (keyScanCode >= gamepadButtonInvalid)
				return;

			SetModifiers(event->key.mod);
			HandleButtonDown(event->key.scancode);
		}
		break;

		case SDL_EVENT_KEY_UP:
		{
			const U32 keyScanCode = STATIC_U32(event->key.scancode);
			if (keyScanCode >= gamepadButtonInvalid)
				return;

			HandleButtonUp(event->key.scancode);
			SetModifiers(event->key.mod);
		}
		break;

		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			HandleButtonDown(STATIC_U32(event->button.button));
			break;

		case SDL_EVENT_MOUSE_BUTTON_UP:
			HandleButtonUp(STATIC_U32(event->button.button));
			break;

		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			HandleButtonDown(event->gbutton.button + STATIC_U32(EInputButton::GamepadRegionStart));
			break;

		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			HandleButtonUp(event->gbutton.button + STATIC_U32(EInputButton::GamepadRegionStart));
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
		for (auto& keyInput : ButtonInputBuffer | std::views::values)
		{
			if (keyInput.IsPressed)
			{
				keyInput.IsPressed = false;
				keyInput.IsHeld = true;
			}
		}

		for (auto it = ButtonInputBuffer.cbegin(); it != ButtonInputBuffer.cend();)
		{
			auto& keyInput = it->second;

			if (keyInput.IsReleased)
				it = ButtonInputBuffer.erase(it);

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

	const std::map<U32, SInputActionPayload>& CInput::GetButtonInputBuffer() const
	{
		return ButtonInputBuffer;
	}

	const std::array<F32, STATIC_U64(EInputAxis::Count)>& CInput::GetAxisInputValues() const
	{
		return AxisInputValues;
	}

	const std::bitset<11>& CInput::GetKeyInputModifiers() const
	{
		return KeyInputModifiers;
	}

	void CInput::HandleButtonDown(const U32& scanCode)
	{
		if (ButtonInputListener.has_value())
		{
			ButtonInputListener.value()(static_cast<EInputButton>(scanCode));
			ButtonInputListener.reset();
		}

		if (ButtonInputBuffer.contains(scanCode))
		{
			if (ButtonInputBuffer[scanCode].IsPressed)
			{
				ButtonInputBuffer[scanCode].IsPressed = false;
				ButtonInputBuffer[scanCode].IsHeld = true;
			}
			else if (!ButtonInputBuffer[scanCode].IsHeld)
			{
				ButtonInputBuffer[scanCode].IsPressed = true;
			}
		}
		else
		{
			ButtonInputBuffer.emplace(scanCode, SInputActionPayload());
			ButtonInputBuffer[scanCode].Key = static_cast<EInputButton>(scanCode);
			ButtonInputBuffer[scanCode].IsPressed = true;
		}
	}

	void CInput::HandleButtonUp(const U32& scanCode)
	{
		ButtonInputBuffer[scanCode].IsPressed = false;
		ButtonInputBuffer[scanCode].IsHeld = false;
		ButtonInputBuffer[scanCode].IsReleased = true;
	}

	void CInput::HandleAxisEvent(const EInputAxis axis, const F32 value)
	{		
		AxisInputValues[STATIC_U64(axis)] = value;
	}

	void CInput::SetModifiers(const U32& modifiers)
	{
		const U32 modValue = modifiers - 4096;
		KeyInputModifiers = modValue;
	}
}
