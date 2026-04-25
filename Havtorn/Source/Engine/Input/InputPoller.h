// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "InputTypes.h"
#include "../Engine/HexRune/Pin.h"

union SDL_Event;

namespace Havtorn
{
	class CInput;
	class CPlatformManager;

	struct SInputData
	{
		U64 UID;

		HexRune::EInputParamType ParamType;
		//std::variant<INPUT_PARAM_VARIANTS> Func;

		EInputKey Key;
		EInputAxis Axis;
	};

	class CInputPoller
	{
		friend class GEngine;

	private:
		CInputPoller();
		~CInputPoller();

		CInputPoller(const CInputPoller&) = delete;
		CInputPoller(CInputPoller&&) = delete;
		CInputPoller operator=(const CInputPoller&) = delete;
		CInputPoller operator=(CInputPoller&&) = delete;

	public:
		bool Init(CPlatformManager* platformManager);
		void Update();
		void ProcessEvent(const SDL_Event* event);

		//[[nodiscard]] ENGINE_API 
		//[[nodiscard]] ENGINE_API

	private:
		void UpdateKeyInput();
		void UpdateAxisInput();

		std::vector<SInputData> InputData;

		CInput* Input = nullptr;
	};
}