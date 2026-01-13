// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "InputTypes.h"

#include <bitset>
#include <thread>
#include <mutex>

#include <GameInput.h>

namespace Havtorn
{
	constexpr U8 MaxNumUsers = 1;
	constexpr U8 PrimaryUser = 0;

	using namespace GameInput::v3;

	class CInputMapper;
	class CPlatformManager;

	enum class EInputDeviceType
	{
		Keyboard = 0,
		Gamepad = 1,
		Count = 2
	};

	enum class EMouseButton
	{
		Left = 1,
		Right = 2,
		Middle = 4,
		Mouse4 = 5,
		Mouse5 = 6
	};

	class CInput
	{
	public:
		friend CInputMapper;

		void MonitorDeviceConnectionChanges() noexcept;
		[[nodiscard]] std::array<std::array<IGameInputDevice*, STATIC_U8(EInputDeviceType::Count)>, MaxNumUsers>& GetActiveInputDevices();

	private:
		CInput();
		~CInput();
		
		static CInput* GetInstance();
		bool Init(CPlatformManager* platformManager);

		void UpdateEvents(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
		void UpdateState();

		[[nodiscard]] const std::bitset<3>& GetKeyInputModifiers() const;
		
		[[nodiscard]] U16 GetMouseX() const; 
		[[nodiscard]] U16 GetMouseY() const; 
		[[nodiscard]] I16 GetMouseDeltaX() const;
		[[nodiscard]] I16 GetMouseDeltaY() const;
		[[nodiscard]] I16 GetMouseWheelDelta() const;
		[[nodiscard]] std::map<WPARAM, SInputActionPayload>& GetKeyInputBuffer();

		void HandleKeyDown(const WPARAM& wParam);
		void HandleKeyUp(const WPARAM& wParam);

	private:
		std::map<WPARAM, SInputActionPayload> KeyInputBuffer;
		std::array<std::array<IGameInputDevice*, STATIC_U8(EInputDeviceType::Count)>, MaxNumUsers> ActiveInputDevices;

		GameInputGamepadState PreviousPrimaryUserGamepadState = {};
		GameInputGamepadState PrimaryUserGamepadState = {};
		GameInputCallbackToken KeyboardConnectionChangeHandle = 0;
		GameInputCallbackToken GamepadConnectionChangeHandle = 0;
		IGameInput* GameInputInstance = nullptr;

		std::bitset<256> KeyDownLast;
		std::bitset<256> KeyDown;
		std::bitset<3> KeyInputModifiers;

		U16 MouseX = 0;
		U16 MouseY = 0;
		U16 MouseScreenX = 0;
		U16 MouseScreenY = 0;
		U16 MouseLastX = 0;
		U16 MouseLastY = 0;
		U16 MouseRawDeltaX = 0;
		U16 MouseRawDeltaY = 0;
		I16 MouseWheelDelta = 0;

		F32 Horizontal = 0.0f;
		F32 Vertical = 0.0f;
		bool HorizontalPressed = false;
		bool VerticalPressed = false;
	};
}
