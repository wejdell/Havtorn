// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "InputTypes.h"
#include <map>

namespace Havtorn
{
	class CInput;
	class CPlatformManager;

	class CInputMapper
	{
		friend class GEngine;

	private:
		CInputMapper();
		~CInputMapper();
		CInputMapper(const CInputMapper&) = delete;
		CInputMapper(CInputMapper&&) = delete;
		CInputMapper operator=(const CInputMapper&) = delete;
		CInputMapper operator=(CInputMapper&&) = delete;

		bool Init(CPlatformManager* platformManager);
	
	public:
		void Update();

		[[nodiscard]] ENGINE_API CMulticastDelegate<const SInputActionPayload>& GetActionDelegate(EInputActionEvent event);
		[[nodiscard]] ENGINE_API CMulticastDelegate<const SInputAxisPayload>& GetAxisDelegate(EInputAxisEvent event);

		void SetInputContext(EInputContext context);

	private:
		void MapEvent(EInputActionEvent event, SInputAction action);
		void MapEvent(EInputAxisEvent event, SInputAxis axisAction);
		void UpdateKeyboardInput();
		void UpdateMouseInput();

		std::map<EInputActionEvent, SInputActionEvent> BoundActionEvents;
		std::map<EInputAxisEvent, SInputAxisEvent> BoundAxisEvents;
		CInput* Input = nullptr;

		EInputContext CurrentInputContext;
	};
}
