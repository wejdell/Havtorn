// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Input.h"

#include <windowsx.h>

#include "Engine.h"

#include <ranges>

#include <../Platform/PlatformManager.h>

namespace Havtorn
{
	CInput* CInput::GetInstance()
	{
		static auto input = new CInput();
		return input;
	}

	CInput::CInput()
		: MouseX(0)
		, MouseY(0)
		, MouseScreenX(0)
		, MouseScreenY(0)
		, MouseLastX(0)
		, MouseLastY(0)
		, MouseRawDeltaX(0)
		, MouseRawDeltaY(0)
		, MouseWheelDelta(0)
		, Horizontal(0)
		, Vertical(0)
		, HorizontalPressed(false)
		, VerticalPressed(false)
	{

		// TODO.NW: Move all raw input stuff to using GameInput
		RAWINPUTDEVICE rid;
		rid.usUsagePage = 0x01; // For mouse
		rid.usUsage = 0x02; // For mouse
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;
		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
		{
			ENGINE_BOOL_POPUP(false, "Mouse could not be registered as Raw Input Device")
		}

		for (U8 userIndex = 0; userIndex < MaxNumUsers; userIndex++)
		{
			for (U8 deviceTypeIndex = 0; deviceTypeIndex < STATIC_U8(EInputDeviceType::Count); deviceTypeIndex++)
				ActiveInputDevices[userIndex][deviceTypeIndex] = nullptr;
		}
	}

	CInput::~CInput()
	{
		GameInputInstance->UnregisterCallback(KeyboardConnectionChangeHandle);
		GameInputInstance->UnregisterCallback(GamepadConnectionChangeHandle);

		// TODO.NW: Figure out why we crash if we try to release all resources.
		// It seems to be something internal to the Release call on the instance 
		// where it accesses invalid memory
		
		//for (U8 userIndex = 0; userIndex < MaxNumUsers; userIndex++)
		//{
		//	for (U8 deviceTypeIndex = 0; deviceTypeIndex < STATIC_U8(EInputDeviceType::Count); deviceTypeIndex++)
		//	{
		//		if (ActiveInputDevices[userIndex][deviceTypeIndex] != nullptr)
		//			ActiveInputDevices[userIndex][deviceTypeIndex]->Release();

		//		ActiveInputDevices[userIndex][deviceTypeIndex] = nullptr;
		//	}
		//}

		if (GameInputInstance != nullptr)
			GameInputInstance->Release();
	}

	void CALLBACK OnAnyKeyboardDeviceConnectionChanged(
		_In_ GameInputCallbackToken /*callbackToken*/,
		_In_ void* context,
		_In_ IGameInputDevice* device,
		_In_ uint64_t /*timestamp*/,
		_In_ GameInputDeviceStatus currentStatus,
		_In_ GameInputDeviceStatus /*previousStatus*/
	)
	{
		if (currentStatus & GameInputDeviceConnected)
		{
			HV_LOG_TRACE("Keyboard device connected!");

			CInput* inputInstance = reinterpret_cast<CInput*>(context);
			if (inputInstance != nullptr)
			{
				inputInstance->ActiveInputDevices[PrimaryUser][STATIC_U8(EInputDeviceType::Keyboard)] = device;
			}
		}
		else
		{
			HV_LOG_TRACE("Keyboard device disconnected!");

			CInput* inputInstance = reinterpret_cast<CInput*>(context);
			if (inputInstance != nullptr)
			{
				IGameInputDevice* existingDevice = inputInstance->ActiveInputDevices[PrimaryUser][STATIC_U8(EInputDeviceType::Keyboard)];
				if (device == existingDevice && existingDevice != nullptr)
				{
					existingDevice->Release();
					inputInstance->ActiveInputDevices[PrimaryUser][STATIC_U8(EInputDeviceType::Keyboard)] = nullptr;
					HV_LOG_WARN("Primary User keyboard disconnected!");
				}
			}
		}
	}

	void CALLBACK OnAnyGamepadDeviceConnectionChanged(
		_In_ GameInputCallbackToken /*callbackToken*/,
		_In_ void* context,
		_In_ IGameInputDevice* device,
		_In_ uint64_t /*timestamp*/,
		_In_ GameInputDeviceStatus currentStatus,
		_In_ GameInputDeviceStatus /*previousStatus*/
		)
	{
		if (currentStatus & GameInputDeviceConnected)
		{
			const GameInputDeviceInfo* info = nullptr;
			device->GetDeviceInfo(&info);

			if (info != nullptr)
			{
				if (info->gamepadInfo->aButtonLabel == GameInputLabelIconCross)
					HV_LOG_TRACE("Device connected! Playstation Gamepad");
				else
					HV_LOG_TRACE("Device connected! Xbox Gamepad");
			}
			else
				HV_LOG_TRACE("Device connected! Unknown Gamepad type");

			CInput* inputInstance = reinterpret_cast<CInput*>(context);
			if (inputInstance != nullptr)
			{
				inputInstance->ActiveInputDevices[PrimaryUser][STATIC_U8(EInputDeviceType::Gamepad)] = device;
			}
		}
		else
		{
			HV_LOG_TRACE("Device disconnected!");

			CInput* inputInstance = reinterpret_cast<CInput*>(context);
			if (inputInstance != nullptr)
			{
				IGameInputDevice* existingDevice = inputInstance->ActiveInputDevices[PrimaryUser][STATIC_U8(EInputDeviceType::Gamepad)];
				if (device == existingDevice && existingDevice != nullptr)
				{
					existingDevice->Release();
					inputInstance->ActiveInputDevices[PrimaryUser][STATIC_U8(EInputDeviceType::Gamepad)] = nullptr;
					HV_LOG_WARN("Primary User gamepad disconnected!");
				}
			}
		}
	}

	void CInput::MonitorDeviceConnectionChanges() noexcept
	{
		if (FAILED(GameInputInstance->RegisterDeviceCallback(
			nullptr,                              // Don't filter to events from a specific device
			GameInputKindGamepad,				  // Listen for Gamepad changes
			GameInputDeviceConnected,             // Notify on changes to GameInputDeviceConnected status
			GameInputAsyncEnumeration,            // Enumerate initial devices asynchronously
			this,								  // Context
			OnAnyGamepadDeviceConnectionChanged,  // Callback function
			&GamepadConnectionChangeHandle)))
		{
			HV_LOG_ERROR("CInput::MonitorDeviceConnectionChanges: Could not register GameInput device callback for gamepads!");
		}

		if (FAILED(GameInputInstance->RegisterDeviceCallback(
			nullptr,                              // Don't filter to events from a specific device
			GameInputKindKeyboard,				  // Listen for Keyboard changes
			GameInputDeviceConnected,             // Notify on changes to GameInputDeviceConnected status
			GameInputAsyncEnumeration,            // Enumerate initial devices asynchronously
			this,								  // Context
			OnAnyKeyboardDeviceConnectionChanged, // Callback function
			&KeyboardConnectionChangeHandle)))                                    		
		{
			HV_LOG_ERROR("CInput::MonitorDeviceConnectionChanges: Could not register GameInput device callback for keyboard!");
		}
	}

	bool CInput::Init(CPlatformManager* platformManager)
	{
		platformManager->OnMessageHandled.AddMember(this, &CInput::UpdateEvents);
		
		ENGINE_HR_BOOL(GameInputCreate(&GameInputInstance));
		MonitorDeviceConnectionChanges();
		
		return true;
	}

	WPARAM GetWParamFromGameInputButton(const GameInputGamepadButtons button)
	{
		if (button & GameInputGamepadMenu)
			return static_cast<WPARAM>(EInputKey::GamepadMenu);
		if (button & GameInputGamepadView)
			return static_cast<WPARAM>(EInputKey::GamepadView);
		if (button & GameInputGamepadA)
			return static_cast<WPARAM>(EInputKey::GamepadA);
		if (button & GameInputGamepadB)
			return static_cast<WPARAM>(EInputKey::GamepadB);
		//if (button & GameInputGamepadC)
		//	return static_cast<WPARAM>(EInputKey::GamepadX);
		if (button & GameInputGamepadX)
			return static_cast<WPARAM>(EInputKey::GamepadX);
		if (button & GameInputGamepadY)
			return static_cast<WPARAM>(EInputKey::GamepadY);
		//if (button & GameInputGamepadZ)
		//	return static_cast<WPARAM>(EInputKey::GamepadZ);
		if (button & GameInputGamepadDPadUp)
			return static_cast<WPARAM>(EInputKey::GamepadDPadUp);
		if (button & GameInputGamepadDPadDown)
			return static_cast<WPARAM>(EInputKey::GamepadDPadDown);
		if (button & GameInputGamepadDPadLeft)
			return static_cast<WPARAM>(EInputKey::GamepadDPadLeft);
		if (button & GameInputGamepadDPadRight)
			return static_cast<WPARAM>(EInputKey::GamepadDPadRight);
		if (button & GameInputGamepadLeftShoulder)
			return static_cast<WPARAM>(EInputKey::GamepadL1);
		if (button & GameInputGamepadRightShoulder)
			return static_cast<WPARAM>(EInputKey::GamepadR1);
		if (button & GameInputGamepadLeftTriggerButton)
			return static_cast<WPARAM>(EInputKey::GamepadL2);
		if (button & GameInputGamepadRightTriggerButton)
			return static_cast<WPARAM>(EInputKey::GamepadR2);
		if (button & GameInputGamepadLeftThumbstick)
			return static_cast<WPARAM>(EInputKey::GamepadL3);
		if (button & GameInputGamepadRightThumbstick)
			return static_cast<WPARAM>(EInputKey::GamepadR3);
		//if (button & GameInputGamepadRightThumbstickUp)
		//	return static_cast<WPARAM>(EInputKey::GamepadView);
		//if (button & GameInputGamepadRightThumbstickDown)
		//	return static_cast<WPARAM>(EInputKey::GamepadView);
		//if (button & GameInputGamepadRightThumbstickLeft)
		//	return static_cast<WPARAM>(EInputKey::GamepadView);
		//if (button & GameInputGamepadRightThumbstickRight)
		//	return static_cast<WPARAM>(EInputKey::GamepadView);
		//if (button & GameInputGamepadPaddleLeft1)
		//	return static_cast<WPARAM>(EInputKey::GamepadView);
		//if (button & GameInputGamepadPaddleLeft2)
		//	return static_cast<WPARAM>(EInputKey::GamepadView);
		//if (button & GameInputGamepadPaddleRight1)
		//	return static_cast<WPARAM>(EInputKey::GamepadView);
		//if (button & GameInputGamepadPaddleRight2)
		//	return static_cast<WPARAM>(EInputKey::GamepadView);
		return 0;
	};

	void CInput::UpdateEvents(HWND /*handle*/, UINT message, WPARAM wParam, LPARAM lParam)
	{
		std::vector<char> rawBuffer;

		switch (message)
		{
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			KeyDown[wParam] = true;
			HandleKeyDown(wParam);
			break;

		case WM_SYSKEYUP:
		case WM_KEYUP:
			KeyDown[wParam] = false;
			HandleKeyUp(wParam);
			break;

		case WM_MOUSEMOVE:
			MouseX = GET_X_LPARAM(lParam); // Returns x coordiante
			MouseY = GET_Y_LPARAM(lParam); // Returns y coordinate
			break;

		case WM_MOUSEWHEEL:
			MouseWheelDelta += GET_WHEEL_DELTA_WPARAM(wParam); // Returns difference in mouse wheel position
			break;

		case WM_LBUTTONDOWN:
			HandleKeyDown(STATIC_U32(EMouseButton::Left));
			break;

		case WM_LBUTTONUP:
			HandleKeyUp(STATIC_U32(EMouseButton::Left));
			break;

		case WM_RBUTTONDOWN:
			HandleKeyDown(STATIC_U32(EMouseButton::Right));
			break;

		case WM_RBUTTONUP:
			HandleKeyUp(STATIC_U32(EMouseButton::Right));
			break;

		case WM_MBUTTONDOWN:
			HandleKeyDown(STATIC_U32(EMouseButton::Middle));
			break;

		case WM_MBUTTONUP:
			HandleKeyUp(STATIC_U32(EMouseButton::Middle));
			break;

		case WM_XBUTTONDOWN:
			if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) 
				HandleKeyDown(STATIC_U32(EMouseButton::Mouse4));
			else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2) 
				HandleKeyDown(STATIC_U32(EMouseButton::Mouse5));
			break;

		case WM_XBUTTONUP:
			if (GET_XBUTTON_WPARAM(wParam) == XBUTTON1)
				HandleKeyUp(STATIC_U32(EMouseButton::Mouse4));
			else if (GET_XBUTTON_WPARAM(wParam) == XBUTTON2)
				HandleKeyUp(STATIC_U32(EMouseButton::Mouse5));
			break;

		// Raw Input
		case WM_INPUT:
		{
			UINT size = 0;
			if (GetRawInputData(
				reinterpret_cast<HRAWINPUT>(lParam),
				RID_INPUT,
				nullptr,
				&size,
				sizeof(RAWINPUTHEADER)) == -1)
			{
				// Error if inputdata == -1
				break;
			}
			rawBuffer.resize(size);

			if (GetRawInputData(
				reinterpret_cast<HRAWINPUT>(lParam),
				RID_INPUT,
				rawBuffer.data(),
				&size,
				sizeof(RAWINPUTHEADER)) != size)
			{
				// Probably an error if the size doesn't match up
				break;
			}

			auto& rawInput = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
			if (rawInput.header.dwType == RIM_TYPEMOUSE &&
				(rawInput.data.mouse.lLastX != 0 || rawInput.data.mouse.lLastY != 0))
			{
				// Register raw input
				MouseRawDeltaX = STATIC_U16(rawInput.data.mouse.lLastX);
				MouseRawDeltaY = STATIC_U16(rawInput.data.mouse.lLastY);
			}
		}
			break;

		default:
			break;
		}

		// Ask for the latest reading from devices that provide fixed-format
		// gamepad state. If a device has been assigned to GamepadInputDevice, filter
		// readings to just the ones coming from that device. Otherwise, if
		// GamepadInputDevice is null, it will allow readings from any device.
		IGameInputReading* reading = nullptr;
		IGameInputDevice* primaryGamepad = ActiveInputDevices[PrimaryUser][STATIC_U8(EInputDeviceType::Gamepad)];
		if (primaryGamepad != nullptr)
		{
			if (SUCCEEDED(GameInputInstance->GetCurrentReading(GameInputKindGamepad, primaryGamepad, &reading)))
			{
				GameInputGamepadState state;
				reading->GetGamepadState(&state);
				reading->Release();
		
				PrimaryUserGamepadState = state;
			}
		}
	
		{
			auto checkButtonState = [&](const GameInputGamepadButtons button)
			{
				if ((PrimaryUserGamepadState.buttons & button) != 0 && (PreviousPrimaryUserGamepadState.buttons & button) == 0)
				{
					WPARAM wParam = GetWParamFromGameInputButton(button);
					KeyDown[wParam] = true;
					HandleKeyDown(wParam);
				}

				if ((PrimaryUserGamepadState.buttons & button) == 0 && (PreviousPrimaryUserGamepadState.buttons & button) != 0)
				{
					WPARAM wParam = GetWParamFromGameInputButton(button);
					KeyDown[wParam] = false;
					HandleKeyUp(wParam);
				}
			};

			checkButtonState(GameInputGamepadMenu);
			checkButtonState(GameInputGamepadView);
			checkButtonState(GameInputGamepadA);
			checkButtonState(GameInputGamepadB);
			checkButtonState(GameInputGamepadX);
			checkButtonState(GameInputGamepadY);
			checkButtonState(GameInputGamepadDPadUp);
			checkButtonState(GameInputGamepadDPadDown);
			checkButtonState(GameInputGamepadDPadLeft);
			checkButtonState(GameInputGamepadDPadRight);
			checkButtonState(GameInputGamepadLeftShoulder);
			checkButtonState(GameInputGamepadRightShoulder);
			checkButtonState(GameInputGamepadLeftTriggerButton);
			checkButtonState(GameInputGamepadRightTriggerButton);
			checkButtonState(GameInputGamepadLeftThumbstick);
			checkButtonState(GameInputGamepadRightThumbstick);
		}
		PreviousPrimaryUserGamepadState = PrimaryUserGamepadState;

	}

	void CInput::UpdateState()
	{
		KeyDownLast = KeyDown;

		MouseLastX = MouseX;
		MouseLastY = MouseY;
		MouseRawDeltaX = 0;
		MouseRawDeltaY = 0;
		MouseWheelDelta = 0;
		//MouseButtonLast = MouseButton;

		POINT point;
		if (GetCursorPos(&point)) 
		{
			MouseScreenX = STATIC_U16(point.x);
			MouseScreenY = STATIC_U16(point.y);
		}

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
	}

	std::map<WPARAM, SInputActionPayload>& CInput::GetKeyInputBuffer()
	{
		return KeyInputBuffer;
	}

	const std::bitset<3>& CInput::GetKeyInputModifiers() const
	{
		return KeyInputModifiers;
	}

	bool CInput::IsKeyDown(WPARAM wParam) 
	{
		return KeyDown[wParam];
	}

	bool CInput::IsKeyPressed(WPARAM wParam) const
	{
		return KeyDown[wParam] && (!KeyDownLast[wParam]);
	}

	bool CInput::IsKeyReleased(WPARAM wParam) const
	{
		return (!KeyDown[wParam]) && KeyDownLast[wParam];
	}

	U16 CInput::GetMouseX() const
	{
		return MouseX;
	}

	U16 CInput::GetMouseY() const
	{
		return MouseY;
	}

	U16 CInput::GetMouseScreenX() const
	{
		return MouseScreenX;
	}

	U16 CInput::GetMouseScreenY() const
	{
		return MouseScreenY;
	}

	I16 CInput::GetMouseDeltaX() const
	{
		return static_cast<I16>(MouseX - MouseLastX);
	}

	I16 CInput::GetMouseDeltaY() const
	{
		return static_cast<I16>(MouseY - MouseLastY);
	}

	I16 CInput::GetMouseRawDeltaX() const
	{
		return static_cast<I16>(MouseRawDeltaX);
	}

	I16 CInput::GetMouseRawDeltaY() const
	{
		return static_cast<I16>(MouseRawDeltaY);
	}

	I16 CInput::GetMouseWheelDelta() const
	{
		return MouseWheelDelta;
	}

	void CInput::SetMouseScreenPosition(U16 x, U16 y)
	{
		SetCursorPos(x, y);
	}

	void CInput::HandleKeyDown(const WPARAM& wParam)
	{
		if (wParam == 0x10 || wParam == 0x11 || wParam == 0x12) // Handle Shift, Ctrl, and Alt input modifiers
			KeyInputModifiers[wParam - 0x10] = true;

		if (KeyInputBuffer.contains(wParam))
		{
			if (KeyInputBuffer[wParam].IsPressed)
			{
				KeyInputBuffer[wParam].IsPressed = false;
				KeyInputBuffer[wParam].IsHeld = true;
			}
			else if (!KeyInputBuffer[wParam].IsHeld)
			{
				KeyInputBuffer[wParam].IsPressed = true;
			}
		}
		else
		{
			KeyInputBuffer.emplace(wParam, SInputActionPayload());
			KeyInputBuffer[wParam].Key = static_cast<EInputKey>(wParam);
			KeyInputBuffer[wParam].IsPressed = true;
		}
	}

	void CInput::HandleKeyUp(const WPARAM& wParam)
	{
		if (wParam == 0x10 || wParam == 0x11 || wParam == 0x12) // Handle Shift, Ctrl, and Alt input modifiers
			KeyInputModifiers[wParam - 0x10] = false;

		KeyInputBuffer[wParam].IsPressed = false;
		KeyInputBuffer[wParam].IsHeld = false;
		KeyInputBuffer[wParam].IsReleased = true;
	}
}
