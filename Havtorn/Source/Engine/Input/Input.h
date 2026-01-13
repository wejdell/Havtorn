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

	class CInput
	{
	public:
		friend CInputMapper;

		static ENGINE_API CInput* GetInstance();

		enum class EMouseButton
		{
			Left = 1,
			Right = 2,
			Middle = 4,
			Mouse4 = 5,
			Mouse5 = 6
		};

		CInput();
		~CInput();
		bool Init(CPlatformManager* platformManager);

		void UpdateEvents(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
		void UpdateState();

		[[nodiscard]] const std::bitset<3>& GetKeyInputModifiers() const;

		bool IsKeyDown(WPARAM wParam);
		[[nodiscard]] bool IsKeyPressed(WPARAM wParam) const;
		[[nodiscard]] bool IsKeyReleased(WPARAM wParam) const;
		
		// X coordiantes in application window
		[[nodiscard]] ENGINE_API U16 GetMouseX() const; 
		// Y coordiantes in application window
		[[nodiscard]] ENGINE_API U16 GetMouseY() const; 
		[[nodiscard]] ENGINE_API U16 GetMouseScreenX() const;
		[[nodiscard]] ENGINE_API U16 GetMouseScreenY() const;
		[[nodiscard]] I16 GetMouseDeltaX() const;
		[[nodiscard]] I16 GetMouseDeltaY() const;
		[[nodiscard]] I16 GetMouseRawDeltaX() const;
		[[nodiscard]] I16 GetMouseRawDeltaY() const;
		// Positive = away from user, negative = towards user
		[[nodiscard]] I16 GetMouseWheelDelta() const;

		static void SetMouseScreenPosition(U16 x, U16 y);

		void MonitorDeviceConnectionChanges() noexcept;

		std::array<std::array<IGameInputDevice*, STATIC_U8(EInputDeviceType::Count)>, MaxNumUsers> ActiveInputDevices;
	private:
		[[nodiscard]] std::map<WPARAM, SInputActionPayload>& GetKeyInputBuffer();

		void HandleKeyDown(const WPARAM& wParam);
		void HandleKeyUp(const WPARAM& wParam);

	private:
		std::map<WPARAM, SInputActionPayload> KeyInputBuffer;
		std::bitset<3> KeyInputModifiers;
		IGameInput* GameInputInstance = nullptr;
		GameInputCallbackToken KeyboardConnectionChangeHandle;
		GameInputCallbackToken GamepadConnectionChangeHandle;
		//std::bitset<5> MouseButtonLast;
		//std::bitset<5> MouseButton;

		GameInputGamepadState PreviousPrimaryUserGamepadState;
		GameInputGamepadState PrimaryUserGamepadState;

		std::bitset<256> KeyDownLast;
		std::bitset<256> KeyDown;

		U16 MouseX;
		U16 MouseY;
		U16 MouseScreenX;
		U16 MouseScreenY;
		U16 MouseLastX;
		U16 MouseLastY;
		U16 MouseRawDeltaX;
		U16 MouseRawDeltaY;
		I16 MouseWheelDelta;

		F32 Horizontal;
		F32 Vertical;
		bool HorizontalPressed;
		bool VerticalPressed;
	};
}
