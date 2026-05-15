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

		// NW: This is offered as a guaranteed way of getting the current mouse mouse cursor position, regardless of other inputs.
		[[nodiscard]] ENGINE_API SVector2<F32> GetCurrentMousePosition() const;
		[[nodiscard]] ENGINE_API bool IsPressed(const EInputKey key, const U32 modifiers) const;
		[[nodiscard]] ENGINE_API bool IsReleased(const EInputKey key, const U32 modifiers) const;
		[[nodiscard]] ENGINE_API bool IsHeld(const EInputKey key, const U32 modifiers) const;
		[[nodiscard]] ENGINE_API F32 GetAxisValue(const EInputAxis axis, const U32 modifiers) const;

	private:
		void MapEvent(EInputActionEvent event, SInputAction action);
		void MapEvent(EInputAxisEvent event, SInputAxis axisAction);
		void UpdateKeyInput();
		void UpdateAxisInput();

		std::map<EInputActionEvent, SInputActionEvent> BoundActionEvents;
		std::map<EInputAxisEvent, SInputAxisEvent> BoundAxisEvents;
		CInput* Input = nullptr;

		EInputContext CurrentInputContext = EInputContext::InGame;
	};
}
