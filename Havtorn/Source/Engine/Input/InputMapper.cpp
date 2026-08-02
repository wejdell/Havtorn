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

	CInputMapper::~CInputMapper()
	{
		delete Input;
	}

	bool CInputMapper::Init(CPlatformManager* platformManager)
	{
		Input->Init(platformManager);

		// TODO.NW: Load from .ini file

		const SInputAxis forwardAxis = { EInputAxis::Key, EInputButton::KeyW, EInputButton::KeyS, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputAxisEvent::Forward, forwardAxis);

		const SInputAxis forwardAxisGamepad = { EInputAxis::GamepadLeftStickVertical, EInputContext::InGame };
		MapEvent(EInputAxisEvent::Forward, forwardAxisGamepad);

		const SInputAxis rightAxis = { EInputAxis::Key, EInputButton::KeyD, EInputButton::KeyA, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputAxisEvent::Right, rightAxis);

		const SInputAxis rightAxisGamepad = { EInputAxis::GamepadLeftStickHorizontal, EInputContext::InGame };
		MapEvent(EInputAxisEvent::Right, rightAxisGamepad);

		const SInputAxis upAxis = { EInputAxis::Key, EInputButton::KeyE, EInputButton::KeyQ, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputAxisEvent::Up, upAxis);

		const SInputAxis upAxisGamepad = { EInputAxis::Key, EInputButton::GamepadR1, EInputButton::GamepadL1, EInputContext::InGame };
		MapEvent(EInputAxisEvent::Up, upAxisGamepad);

		const SInputAxis mouseDeltaHorizontal = { EInputAxis::MouseDeltaHorizontal, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputAxisEvent::MouseDeltaHorizontal, mouseDeltaHorizontal);

		const SInputAxis gamepadDeltaHorizontal = { EInputAxis::GamepadRightStickHorizontal, EInputContext::InGame};
		MapEvent(EInputAxisEvent::MouseDeltaHorizontal, gamepadDeltaHorizontal);

		const SInputAxis mouseDeltaVertical = { EInputAxis::MouseDeltaVertical, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputAxisEvent::MouseDeltaVertical, mouseDeltaVertical);

		const SInputAxis gamepadDeltaVertical = { EInputAxis::GamepadRightStickVertical, EInputContext::InGame };
		MapEvent(EInputAxisEvent::MouseDeltaVertical, gamepadDeltaVertical);

		const SInputAxis mousePositionHorizontal = { EInputAxis::MousePositionHorizontal, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputAxisEvent::MousePositionHorizontal, mousePositionHorizontal);

		const SInputAxis mousePositionVertical = { EInputAxis::MousePositionVertical, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputAxisEvent::MousePositionVertical, mousePositionVertical);

		const SInputAxis mouseWheel = { EInputAxis::MouseWheel, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputAxisEvent::Zoom, mouseWheel);

		const SInputAction translateTransform = { EInputButton::KeyW, EInputContext::Editor };
		MapEvent(EInputActionEvent::TranslateTransform, translateTransform);

		const SInputAction rotateTransform = { EInputButton::KeyE, EInputContext::Editor };
		MapEvent(EInputActionEvent::RotateTransform, rotateTransform);

		const SInputAction scaleTransform = { EInputButton::KeyR, EInputContext::Editor };
		MapEvent(EInputActionEvent::ScaleTransform, scaleTransform);

		const SInputAction toggleFreeCam = { EInputButton::MouseRight, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputActionEvent::ToggleFreeCam, toggleFreeCam);

		const SInputAction toggleFreeCamGamepad = { EInputButton::GamepadWest, EInputContext::InGame };
		MapEvent(EInputActionEvent::ToggleFreeCam, toggleFreeCamGamepad);

		const SInputAction renderPassForward = { EInputButton::F8, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputActionEvent::CycleRenderPassForward, renderPassForward);

		const SInputAction renderPassBackward = { EInputButton::F7, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputActionEvent::CycleRenderPassBackward, renderPassBackward);

		const SInputAction renderPassReset = { EInputButton::F9, EInputContext::Editor | EInputContext::InGame };
		MapEvent(EInputActionEvent::CycleRenderPassReset, renderPassReset);

		const SInputAction pickEntity = { EInputButton::MouseLeft, EInputContext::Editor };
		MapEvent(EInputActionEvent::PickEditorEntity, pickEntity);

		const SInputAction contextPickEntity = { EInputButton::MouseRight, EInputContext::Editor };
		MapEvent(EInputActionEvent::ContextPickEditorEntity, contextPickEntity);

		const SInputAction ctrlPickEntity = { EInputButton::MouseLeft, EInputContext::Editor, EInputModifier::Ctrl };
		MapEvent(EInputActionEvent::ControlPickEditorEntity, ctrlPickEntity);

		const SInputAction shiftPickEntity = { EInputButton::MouseLeft, EInputContext::Editor, EInputModifier::Shift };
		MapEvent(EInputActionEvent::ShiftPickEditorEntity, shiftPickEntity);

		const SInputAction focusEntity = { EInputButton::KeyF, EInputContext::Editor };
		MapEvent(EInputActionEvent::FocusEditorEntity, focusEntity);

		const SInputAction deleteAction = { EInputButton::Delete, EInputContext::Editor };
		MapEvent(EInputActionEvent::DeleteEvent, deleteAction);

		const SInputAction toggleFullscreen = { EInputButton::F11, EInputContext::Editor };
		MapEvent(EInputActionEvent::ToggleFullscreen, toggleFullscreen);

		const SInputAction startPlay = { EInputButton::KeyP, EInputContext::Editor, EInputModifier::Alt };
		MapEvent(EInputActionEvent::StartPlay, startPlay);

		// TODO.NW: Figure out how to deal with editor vs in game context here, and how to construct the input action.
		// Esc by default in UE is end play. Shift+Esc could be used to Pause, or we decide to shift the role of those two.
		const SInputAction stopPlay = { EInputButton::Esc, EInputContext::InGame };
		MapEvent(EInputActionEvent::StopPlay, stopPlay);

		const SInputAction unlockCursor = { EInputButton::KeyC, EInputContext::InGame, EInputModifier::Alt };
		MapEvent(EInputActionEvent::ToggleCursor, unlockCursor);

		// NW: the Sys key is a bit different. We might need this workaround on other modifier keys as well
		// TODO.NW: See if this is better handled now that we use SDL
		const SInputAction altPress = { EInputButton::LAlt, EInputContext::Editor, EInputModifier::Alt };
		MapEvent(EInputActionEvent::AltPress, altPress);

		const SInputAction altRelease = { EInputButton::LAlt, EInputContext::Editor };
		MapEvent(EInputActionEvent::AltRelease, altRelease);

		const SInputAction copy = { EInputButton::KeyC, EInputContext::Editor, EInputModifier::Ctrl };
		MapEvent(EInputActionEvent::Copy, copy);

		const SInputAction paste = { EInputButton::KeyV, EInputContext::Editor, EInputModifier::Ctrl };
		MapEvent(EInputActionEvent::Paste, paste);

		const SInputAction undo = { EInputButton::KeyZ, EInputContext::Editor, EInputModifier::Ctrl };
		MapEvent(EInputActionEvent::Undo, undo);

		const SInputAction redo = { EInputButton::KeyY, EInputContext::Editor, EInputModifier::Ctrl };
		MapEvent(EInputActionEvent::Redo, redo);

		const SInputAction rename = { EInputButton::F2, EInputContext::Editor };
		MapEvent(EInputActionEvent::Rename, rename);

		const SInputAction movePivot = { EInputButton::KeyC, EInputContext::Editor };
		MapEvent(EInputActionEvent::MovePivot, movePivot);

		const SInputAction vertexSnapping = { EInputButton::KeyV, EInputContext::Editor };
		MapEvent(EInputActionEvent::VertexSnapping, vertexSnapping);

		const SInputAction gridSnapping = { EInputButton::KeyG, EInputContext::Editor };
		MapEvent(EInputActionEvent::GridSnapping, gridSnapping);

		const SInputAction clearSelection = { EInputButton::Esc, EInputContext::Editor };
		MapEvent(EInputActionEvent::ClearSelection, clearSelection);

		return true;
	}

	void CInputMapper::Update()
	{
		UpdateKeyInput();
		UpdateAxisInput();
	}

	void CInputMapper::EndFrame()
	{
		Input->EndFrameUpdate();
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

	SVector2<F32> CInputMapper::GetCurrentMousePosition() const
	{
		const std::array<F32, STATIC_U64(EInputAxis::Count)>& axisInputValues = Input->GetAxisInputValues();
		return SVector2<F32>(axisInputValues[STATIC_U64(EInputAxis::MousePositionHorizontal)], axisInputValues[STATIC_U64(EInputAxis::MousePositionVertical)]);
	}

	bool CInputMapper::IsPressed(const EInputButton key, const U32 modifiers) const
	{
		const std::map<U32, SInputActionPayload>& keyInputBuffer = Input->GetButtonInputBuffer();
		if (!keyInputBuffer.contains(STATIC_U32(key)))
			return false;

		const auto& activeModifiers = Input->GetKeyInputModifiers().to_ulong();
		return ((modifiers == 0) || (activeModifiers & modifiers) != 0) && keyInputBuffer.at(STATIC_U32(key)).IsPressed;
	}

	bool CInputMapper::IsReleased(const EInputButton key, const U32 modifiers) const
	{
		const std::map<U32, SInputActionPayload>& keyInputBuffer = Input->GetButtonInputBuffer();
		if (!keyInputBuffer.contains(STATIC_U32(key)))
			return false;

		const auto& activeModifiers = Input->GetKeyInputModifiers().to_ulong();
		return ((modifiers == 0) || (activeModifiers & modifiers) != 0) && keyInputBuffer.at(STATIC_U32(key)).IsReleased;
	}

	bool CInputMapper::IsHeld(const EInputButton key, const U32 modifiers) const
	{
		const std::map<U32, SInputActionPayload>& keyInputBuffer = Input->GetButtonInputBuffer();
		if (!keyInputBuffer.contains(STATIC_U32(key)))
			return false;

		const auto& activeModifiers = Input->GetKeyInputModifiers().to_ulong();
		return ((modifiers == 0) || (activeModifiers & modifiers) != 0) && keyInputBuffer.at(STATIC_U32(key)).IsHeld;
	}

	F32 CInputMapper::GetAxisValue(const EInputAxis axis, const U32 modifiers) const
	{
		const std::array<F32, STATIC_U64(EInputAxis::Count)>& axisInputValues = Input->GetAxisInputValues();
	
		const auto& activeModifiers = Input->GetKeyInputModifiers().to_ulong();
		return ((modifiers == 0) || (activeModifiers & modifiers) != 0) ? axisInputValues[STATIC_U64(axis)] : F32(0.0f);
	}

	void CInputMapper::StartListenForButtonInput(const std::function<void(const EInputButton)>& onNextButtonInput)
	{
		Input->ButtonInputListener = onNextButtonInput;
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

	void CInputMapper::UpdateKeyInput()
	{
		const auto& modifiers = Input->GetKeyInputModifiers().to_ulong();
		const auto& context = STATIC_U32(CurrentInputContext);

		for (auto& [param, actionPayload] : Input->GetButtonInputBuffer())
		{
			for (auto& [event, data] : BoundActionEvents)
			{
				if (data.Has(static_cast<EInputButton>(param), context, modifiers))
				{
					const SInputActionPayload payload = { event, actionPayload.Key, actionPayload.IsPressed, actionPayload.IsHeld, actionPayload.IsReleased };
					data.Delegate.Broadcast(payload);
				}
			}

			// Key Axes
			for (auto& [event, data] : BoundAxisEvents)
			{
				if (data.HasKeyAxis())
				{
					F32 axisValue = 0.0f;
					if (data.Has(static_cast<EInputButton>(param), context, modifiers, axisValue))
					{
						const SInputAxisPayload axisPayload = { event, axisValue };
						data.Delegate.Broadcast(axisPayload);
					}
				}
			}
		}
	}

	void CInputMapper::UpdateAxisInput()
	{
		const auto& modifiers = Input->GetKeyInputModifiers().to_ulong();
		const auto& context = STATIC_U32(CurrentInputContext);

		const std::array<F32, STATIC_U64(EInputAxis::Count)>& axisInputValues = Input->GetAxisInputValues();

		for (EInputAxis axis = EInputAxis::MouseWheel; axis < EInputAxis::Count; axis = static_cast<EInputAxis>(STATIC_U8(axis) + 1))
		{
			for (auto& [event, data] : BoundAxisEvents)
			{
				if (data.Has(axis, context, modifiers))
				{
					F32 axisValue = axisInputValues[STATIC_U64(axis)];

					if (axis == EInputAxis::MouseDeltaHorizontal && axisValue == 0.0f)
						continue;
					if (axis == EInputAxis::MouseDeltaVertical && axisValue == 0.0f)
						continue;
					if (axis == EInputAxis::MouseWheel && axisValue == 0.0f)
						continue;
					if (axis >= EInputAxis::GamepadRegionStart && axisValue == 0.0f)
						continue;

					const SInputAxisPayload payload = { event, axisValue };
					data.Delegate.Broadcast(payload);
				}
			}
		}
	}
}
