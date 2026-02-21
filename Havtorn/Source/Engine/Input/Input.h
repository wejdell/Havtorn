// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "InputTypes.h"

#include <bitset>
#include <thread>
#include <mutex>

union SDL_Event;
struct SDL_Keyboard;
struct SDL_Gamepad;

namespace Havtorn
{
	constexpr U8 MaxNumUsers = 1;
	constexpr U8 PrimaryUser = 0;

	class CInputMapper;
	class CPlatformManager;

	class CInput
	{
	public:
		friend CInputMapper;

	private:
		CInput();
		~CInput();
		
		static CInput* GetInstance();
		bool Init(CPlatformManager* platformManager);

		void ProcessEvent(const SDL_Event* event);
		void EndFrameUpdate();

		[[nodiscard]] const std::map<U32, SInputActionPayload>& GetKeyInputBuffer() const;
		[[nodiscard]] const std::array<F32, STATIC_U64(EInputAxis::Count)>& GetAxisInputValues() const;
		[[nodiscard]] const std::bitset<3>& GetKeyInputModifiers() const;

		void HandleKeyDown(const U32& keyCode);
		void HandleKeyUp(const U32& keyCode);
		void HandleAxisEvent(const EInputAxis axis, const F32 value);
		void UpdateModifiers(const U32& modifiers, const bool pressedKey);

	private:
		// TODO.NW: Associate these with Users too
		std::map<U32, SInputActionPayload> KeyInputBuffer;
		std::array<F32, STATIC_U64(EInputAxis::Count)> AxisInputValues;
		std::array<SDL_Gamepad*, MaxNumUsers> ActiveGamepadDevices;
		std::bitset<3> KeyInputModifiers;
	};
}
