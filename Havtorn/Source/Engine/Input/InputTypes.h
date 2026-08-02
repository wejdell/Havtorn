// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <HavtornDelegate.h>
#include <InputStructs.h>

namespace Havtorn
{
	// TODO.NW: Events and onward should be extendable in a game project
	enum class EInputActionEvent
	{
		None,
		TranslateTransform,
		RotateTransform,
		ScaleTransform,
		ToggleFreeCam,
		CycleRenderPassForward,
		CycleRenderPassBackward,
		CycleRenderPassReset,
		PickEditorEntity,
		ContextPickEditorEntity,
		ControlPickEditorEntity,
		ShiftPickEditorEntity,
		FocusEditorEntity,
		DeleteEvent,
		ToggleFullscreen,
		StartPlay,
		StopPlay,
		AltPress,
		AltRelease,
		Copy,
		Paste,
		Undo,
		Redo,
		Rename,
		MovePivot,
		VertexSnapping,
		GridSnapping,
		ToggleCursor,
		ClearSelection,
		Count
	};

	enum class EInputAxisEvent
	{
		Right,		// X-axis
		Up,			// Y-axis
		Forward,	// Z-axis
		Pitch,		// X-axis
		Yaw,		// Y-axis
		Roll,		// Z-axis
		MouseDeltaHorizontal,
		MouseDeltaVertical,
		MousePositionHorizontal,
		MousePositionVertical,
		Zoom,
		Count
	};

	struct SInputActionPayload
	{
		EInputActionEvent Event = EInputActionEvent::Count;
		EInputButton Key = EInputButton::None;
		bool IsPressed = false;
		bool IsHeld = false;
		bool IsReleased = false;
	};

	struct SInputAxisPayload
	{
		EInputAxisEvent Event = EInputAxisEvent::Count;
		F32 AxisValue = 0.0f;
	};

	struct SInputAction
	{
		SInputAction(EInputButton key, EInputContext context, EInputModifier modifier)
			: Key(key)
			, Contexts(STATIC_U32(context))
			, Modifiers(STATIC_U32(modifier))
		{}

		SInputAction(EInputButton key, U32 contexts, EInputModifier modifier)
			: Key(key)
			, Contexts(STATIC_U32(contexts))
			, Modifiers(STATIC_U32(modifier))
		{}

		SInputAction(EInputButton key, std::initializer_list<EInputContext> contexts, std::initializer_list<EInputModifier> modifiers = {})
			: Key(key)
			, Contexts(STATIC_U32(EInputContext::Editor))
			, Modifiers(0)
		{
			SetContexts(contexts);
			SetModifiers(modifiers);
		}

		SInputAction(EInputButton key, EInputContext context)
			: Key(key)
			, Contexts(STATIC_U32(context))
			, Modifiers(0)
		{}

		SInputAction(EInputButton key, U32 contexts)
			: Key(key)
			, Contexts(contexts)
			, Modifiers(0)
		{}

		// Pass in the number of modifiers the SInputAction should have
		// followed by that number of EInputModifier entries, separated by comma
		void SetModifiers(U32 numberOfModifiers, ...)
		{
			Modifiers = 0;

			va_list args;
			va_start(args, numberOfModifiers);

			for (U32 index = 0; index < numberOfModifiers; index++)
			{
				Modifiers += STATIC_U32(va_arg(args, EInputModifier));
			}

			va_end(args);
		}

		void SetModifiers(std::initializer_list<EInputModifier> modifiers)
		{
			Modifiers = 0;
			for (auto modifier : modifiers)
				Modifiers += STATIC_U32(modifier);
		}

		void SetContexts(std::initializer_list<EInputContext> contexts)
		{
			Contexts = 0;
			for (auto context : contexts)
				Contexts += STATIC_U32(context);
		}

		EInputButton Key = EInputButton::None;
		U32 Contexts = STATIC_U32(EInputContext::Editor);
		U32 Modifiers = STATIC_U32(EInputModifier::None);
	};

	struct SInputActionEvent
	{
		SInputActionEvent() = default;

		explicit SInputActionEvent(SInputAction action)
			: Delegate(CMulticastDelegate<const SInputActionPayload>())
		{
			Actions.push_back(action);
		}

		[[nodiscard]] bool HasKey(const EInputButton& key) const
		{
			return std::ranges::any_of(Actions.begin(), Actions.end(),
				[key](const SInputAction& action) {return action.Key == key; });
		}

		[[nodiscard]] bool HasContext(U32 context) const
		{
			return std::ranges::any_of(Actions.begin(), Actions.end(),
				[context](const SInputAction& action) {return (action.Contexts & context) != 0; });
		}

		[[nodiscard]] bool HasModifiers(U32 modifiers) const
		{
			return std::ranges::any_of(Actions.begin(), Actions.end(),
				[modifiers](const SInputAction& action) {return ((action.Modifiers == 0 && modifiers == 0) || (action.Modifiers & modifiers) != 0); });
		}

		[[nodiscard]] bool Has(const EInputButton& key, U32 context, U32 modifiers) const
		{
			return std::ranges::any_of(Actions.begin(), Actions.end(),
				[key, context, modifiers](const SInputAction& action)
				{
					return action.Key == key && (action.Contexts & context) != 0 && ((action.Modifiers == 0 && modifiers == 0) || (action.Modifiers & modifiers) != 0);
				});
		}

		CMulticastDelegate<const SInputActionPayload> Delegate;
		std::vector<SInputAction> Actions;
	};

	struct SInputAxis
	{
		SInputAxis(EInputAxis axis, EInputContext context)
			: Axis(axis)
			, AxisPositiveKey(EInputButton::KeyW)
			, AxisNegativeKey(EInputButton::KeyS)
			, Contexts(STATIC_U32(context))
			, Modifiers(0)
		{}

		SInputAxis(EInputAxis axis, U32 contexts)
			: Axis(axis)
			, AxisPositiveKey(EInputButton::KeyW)
			, AxisNegativeKey(EInputButton::KeyS)
			, Contexts(contexts)
			, Modifiers(0)
		{}

		SInputAxis(EInputAxis axis, EInputContext context, EInputModifier modifier)
			: Axis(axis)
			, AxisPositiveKey(EInputButton::KeyW)
			, AxisNegativeKey(EInputButton::KeyS)
			, Contexts(STATIC_U32(context))
			, Modifiers(STATIC_U32(modifier))
		{}

		SInputAxis(EInputAxis axis, EInputButton axisPositiveKey, EInputButton axisNegativeKey, EInputContext context)
			: Axis(axis)
			, AxisPositiveKey(axisPositiveKey)
			, AxisNegativeKey(axisNegativeKey)
			, Contexts(STATIC_U32(context))
			, Modifiers(0)
		{}

		SInputAxis(EInputAxis axis, EInputButton axisPositiveKey, EInputButton axisNegativeKey, U32 contexts)
			: Axis(axis)
			, AxisPositiveKey(axisPositiveKey)
			, AxisNegativeKey(axisNegativeKey)
			, Contexts(contexts)
			, Modifiers(0)
		{}

		SInputAxis(EInputAxis axis, std::initializer_list<EInputContext> contexts, std::initializer_list<EInputModifier> modifiers = {})
			: Axis(axis)
			, AxisPositiveKey(EInputButton::KeyW)
			, AxisNegativeKey(EInputButton::KeyS)
			, Contexts(STATIC_U32(EInputContext::Editor))
			, Modifiers(0)
		{
			SetContexts(contexts);
			SetModifiers(modifiers);
		}

		SInputAxis(EInputAxis axis, EInputButton axisPositiveKey, EInputButton axisNegativeKey, std::initializer_list<EInputContext> contexts, std::initializer_list<EInputModifier> modifiers = {})
			: Axis(axis)
			, AxisPositiveKey(axisPositiveKey)
			, AxisNegativeKey(axisNegativeKey)
			, Contexts(STATIC_U32(EInputContext::Editor))
			, Modifiers(0)
		{
			SetContexts(contexts);
			SetModifiers(modifiers);
		}

		// Pass in the number of modifiers the SInputAction should have
		// followed by that number of EInputModifier entries, separated by comma
		void SetModifiers(U32 numberOfModifiers, ...)
		{
			Modifiers = 0;

			va_list args;
			va_start(args, numberOfModifiers);

			for (U32 index = 0; index < numberOfModifiers; index++)
			{
				Modifiers += STATIC_U32(va_arg(args, EInputModifier));
			}

			va_end(args);
		}

		void SetModifiers(std::initializer_list<EInputModifier> modifiers)
		{
			Modifiers = 0;
			for (auto modifier : modifiers)
				Modifiers += STATIC_U32(modifier);
		}

		void SetContexts(std::initializer_list<EInputContext> contexts)
		{
			Contexts = 0;
			for (auto context : contexts)
				Contexts += STATIC_U32(context);
		}

		[[nodiscard]] F32 GetAxisValue(const EInputButton& key) const
		{
			if (AxisPositiveKey == key)
				return 1.0;

			if (AxisNegativeKey == key)
				return -1.0f;

			return 0.0f;
		}

		EInputAxis Axis = EInputAxis::Key;
		EInputButton AxisPositiveKey = EInputButton::None; // Optional
		EInputButton AxisNegativeKey = EInputButton::None; // Optional
		U32 Contexts = STATIC_U32(EInputContext::Editor);
		U32 Modifiers = STATIC_U32(EInputModifier::None);
	};

	struct SInputAxisEvent
	{
		SInputAxisEvent() = default;

		explicit SInputAxisEvent(SInputAxis axis)
			: Delegate(CMulticastDelegate<const SInputAxisPayload>())
		{
			Axes.push_back(axis);
		}

		[[nodiscard]] bool HasKeyAxis() const
		{
			return std::ranges::any_of(Axes.begin(), Axes.end(),
				[](const SInputAxis& axis) {return axis.Axis == EInputAxis::Key; });
		}

		[[nodiscard]] bool Has(const EInputButton& key, const U32 context, const U32 modifiers, F32& outAxisValue) const
		{
			return std::ranges::any_of(Axes.begin(), Axes.end(),
				[key, context, modifiers, &outAxisValue](const SInputAxis& axisAction)
				{
					if ((axisAction.AxisPositiveKey == key || axisAction.AxisNegativeKey == key)
						&& (axisAction.Contexts & context) != 0 && (axisAction.Modifiers ^ modifiers) == 0)
					{
						outAxisValue = axisAction.GetAxisValue(key);
						return true;
					}
					return false;
				});
		}

		[[nodiscard]] bool Has(const EInputAxis& axis, const U32 context, const U32 modifiers) const
		{
			return std::ranges::any_of(Axes.begin(), Axes.end(),
				[axis, context, modifiers](const SInputAxis& axisAction)
				{
					if (axisAction.Axis == axis && (axisAction.Contexts & context) != 0 && (axisAction.Modifiers ^ modifiers) == 0)
					{
						return true;
					}
					return false;
				});
		}

		CMulticastDelegate<const SInputAxisPayload> Delegate;
		std::vector<SInputAxis> Axes;
	};
}
