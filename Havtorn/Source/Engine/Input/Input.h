// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "InputTypes.h"

#include <bitset>

union SDL_Event;
struct SDL_Keyboard;
struct SDL_Gamepad;

namespace Havtorn
{
	constexpr U8 MaxNumUsers = 1;
	constexpr U8 PrimaryUser = 0;

	class CInputMapper;
	class CInputPoller;
	class CPlatformManager;

	class CInput
	{
	public:
		friend CInputMapper;
		friend CInputPoller;

	private:
		CInput();
		~CInput();
		
		static CInput* GetInstance();
		bool Init(CPlatformManager* platformManager);

		void ProcessEvent(const SDL_Event* event);
		void EndFrameUpdate();

		[[nodiscard]] const std::map<U32, SInputActionPayload>& GetKeyInputBuffer() const;
		[[nodiscard]] const std::array<F32, STATIC_U64(EInputAxis::Count)>& GetAxisInputValues() const;
		[[nodiscard]] const std::bitset<11>& GetKeyInputModifiers() const;

		void HandleKeyDown(const U32& keyCode);
		void HandleKeyUp(const U32& keyCode);
		void HandleAxisEvent(const EInputAxis axis, const F32 value);
		void SetModifiers(const U32& modifiers);

	private:
		// TODO.NW: Associate these with Users too
		std::map<U32, SInputActionPayload> KeyInputBuffer;
		std::array<F32, STATIC_U64(EInputAxis::Count)> AxisInputValues;
		std::array<SDL_Gamepad*, MaxNumUsers> ActiveGamepadDevices;
		std::bitset<11> KeyInputModifiers;
	};
}
