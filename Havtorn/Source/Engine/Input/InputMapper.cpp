// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "InputMapper.h"

#include <ranges>

#include "Input.h"

namespace Havtorn
{
	CInputMapper::CInputMapper()
		: Input(CInput::GetInstance())
		, CurrentInputContext(EInputContext::Editor)
	{}

	bool CInputMapper::Init()
	{
		// TODO.NR: Load from .ini file

		const SInputAxis forwardAxis = { EInputAxis::Key, EInputKey::KeyW, EInputKey::KeyS, EInputContext::Editor };
		MapEvent(EInputAxisEvent::Forward, forwardAxis);

		const SInputAxis rightAxis = { EInputAxis::Key, EInputKey::KeyD, EInputKey::KeyA, EInputContext::Editor };
		MapEvent(EInputAxisEvent::Right, rightAxis);

		const SInputAxis upAxis = { EInputAxis::Key, EInputKey::KeyE, EInputKey::KeyQ, EInputContext::Editor };
		MapEvent(EInputAxisEvent::Up, upAxis);

		const SInputAxis mouseDeltaHorizontal = { EInputAxis::MouseDeltaHorizontal, EInputContext::Editor };
		MapEvent(EInputAxisEvent::MouseDeltaHorizontal, mouseDeltaHorizontal);

		const SInputAxis mouseDeltaVertical = { EInputAxis::MouseDeltaVertical, EInputContext::Editor };
		MapEvent(EInputAxisEvent::MouseDeltaVertical, mouseDeltaVertical);

		const SInputAxis mousePositionHorizontal = { EInputAxis::MousePositionHorizontal, EInputContext::Editor };
		MapEvent(EInputAxisEvent::MousePositionHorizontal, mousePositionHorizontal);

		const SInputAxis mousePositionVertical = { EInputAxis::MousePositionVertical, EInputContext::Editor };
		MapEvent(EInputAxisEvent::MousePositionVertical, mousePositionVertical);

		const SInputAction translateTransform = { EInputKey::KeyW, EInputContext::Editor };
		MapEvent(EInputActionEvent::TranslateTransform, translateTransform);

		const SInputAction rotateTransform = { EInputKey::KeyE, EInputContext::Editor };
		MapEvent(EInputActionEvent::RotateTransform, rotateTransform);

		const SInputAction scaleTransform = { EInputKey::KeyR, EInputContext::Editor };
		MapEvent(EInputActionEvent::ScaleTransform, scaleTransform);

		const SInputAction toggleFreeCam = { EInputKey::Mouse2, EInputContext::Editor };
		MapEvent(EInputActionEvent::ToggleFreeCam, toggleFreeCam);

		const SInputAction renderPassForward = { EInputKey::F8, EInputContext::Editor };
		MapEvent(EInputActionEvent::CycleRenderPassForward, renderPassForward);

		const SInputAction renderPassBackward = { EInputKey::F7, EInputContext::Editor };
		MapEvent(EInputActionEvent::CycleRenderPassBackward, renderPassBackward);

		const SInputAction renderPassReset = { EInputKey::F9, EInputContext::Editor };
		MapEvent(EInputActionEvent::CycleRenderPassReset, renderPassReset);

		const SInputAction pickEntity = { EInputKey::Mouse1, EInputContext::Editor };
		MapEvent(EInputActionEvent::PickEditorEntity, pickEntity);

		const SInputAction focusEntity = { EInputKey::KeyF, EInputContext::Editor };
		MapEvent(EInputActionEvent::FocusEditorEntity, focusEntity);

		return true;
	}

	void CInputMapper::Update()
	{
		UpdateKeyboardInput();
		UpdateMouseInput();
		Input->UpdateState();
	}

	CMulticastDelegate<const SInputActionPayload>& CInputMapper::GetActionDelegate(EInputActionEvent event)
	{
		HV_ASSERT(BoundActionEvents.contains(event), "There is no such Input Action Event bound!");
		return BoundActionEvents[event].Delegate;
	}

	CMulticastDelegate<const SInputAxisPayload>& CInputMapper::GetAxisDelegate(EInputAxisEvent event)
	{
		HV_ASSERT(BoundAxisEvents.contains(event), "There is no such Input Axis Event bound!");
		return BoundAxisEvents[event].Delegate;
	}

	void CInputMapper::SetInputContext(EInputContext context)
	{
		CurrentInputContext = context;
	}

	void CInputMapper::MapEvent(EInputActionEvent event, SInputAction action)
	{
		if (!BoundActionEvents.contains(event))
			BoundActionEvents.emplace(event, SInputActionEvent(action));

		else
			BoundActionEvents[event].Actions.push_back(action);
	}

	void CInputMapper::MapEvent(EInputAxisEvent event, SInputAxis axisAction)
	{
		if (!BoundAxisEvents.contains(event))
			BoundAxisEvents.emplace(event, SInputAxisEvent(axisAction));

		else
			BoundAxisEvents[event].Axes.push_back(axisAction);
	}

	void CInputMapper::UpdateKeyboardInput()
	{
		const auto& modifiers = Input->GetKeyInputModifiers().to_ulong();
		const auto& context = STATIC_U32(CurrentInputContext);

		for (auto& param : Input->GetKeyInputBuffer())
		{
			for (auto& val : BoundActionEvents)
			{
				if (val.second.Has(static_cast<EInputKey>(param.first), context, modifiers))
				{
					param.second.Event = val.first;
					val.second.Delegate.Broadcast(param.second);
				}
			}

			// Key Axes
			for (auto& val : BoundAxisEvents)
			{
				if (val.second.HasKeyAxis())
				{
					F32 axisValue = 0.0f;
					if (val.second.Has(static_cast<EInputKey>(param.first), context, modifiers, axisValue))
					{
						const SInputAxisPayload payload = { val.first, axisValue };
						val.second.Delegate.Broadcast(payload);
					}
				}
			}
		}
	}

	void CInputMapper::UpdateMouseInput()
	{
		SVector2<F32> rawMouseMovement = { STATIC_F32(Input->GetMouseDeltaX()), STATIC_F32(Input->GetMouseDeltaY()) };
		SVector2<F32> rawMousePosition = { STATIC_F32(Input->GetMouseX()), STATIC_F32(Input->GetMouseY()) };
		F32 mouseWheelDelta = STATIC_F32(Input->GetMouseWheelDelta());

		for (auto& val : BoundAxisEvents)
		{
			if (rawMouseMovement.X != 0.0f && val.second.Has(EInputAxis::MouseDeltaHorizontal, rawMouseMovement.X))
			{
				const F32 axisValue = rawMouseMovement.X;
				const SInputAxisPayload payload = { val.first, axisValue };
				val.second.Delegate.Broadcast(payload);
			}

			if (rawMouseMovement.Y != 0.0f && val.second.Has(EInputAxis::MouseDeltaVertical, rawMouseMovement.Y))
			{
				const F32 axisValue = rawMouseMovement.Y;
				const SInputAxisPayload payload = { val.first, axisValue };
				val.second.Delegate.Broadcast(payload);
			}

			if (val.second.Has(EInputAxis::MousePositionHorizontal, rawMousePosition.X))
			{
				const F32 axisValue = rawMousePosition.X;
				const SInputAxisPayload payload = { val.first, axisValue };
				val.second.Delegate.Broadcast(payload);
			}

			if (val.second.Has(EInputAxis::MousePositionVertical, rawMousePosition.Y))
			{
				const F32 axisValue = rawMousePosition.Y;
				const SInputAxisPayload payload = { val.first, axisValue };
				val.second.Delegate.Broadcast(payload);
			}

			if (mouseWheelDelta != 0.0f && val.second.Has(EInputAxis::MouseWheel, mouseWheelDelta))
			{
				const F32 axisValue = mouseWheelDelta;
				const SInputAxisPayload payload = { val.first, axisValue };
				val.second.Delegate.Broadcast(payload);
			}
		}
	}
}
