// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <HavtornDelegate.h>

namespace Havtorn
{
	enum class EInputModifier
	{
		None		= 0,
		Shift		= BIT(0),
		Ctrl		= BIT(1),
		Alt			= BIT(2),
	};

	enum class EInputContext
	{
		Editor		= BIT(0),
		InGame		= BIT(1),
	};

	inline constexpr U32 operator&(U32 mask, EInputContext context)
	{
		return mask & STATIC_U32(context);
	}

	inline constexpr U32 operator&(EInputContext context, U32 mask)
	{
		return mask & context;
	}

	inline constexpr U32 operator|(EInputContext x, EInputContext y)
	{
		return (STATIC_U32(x) | STATIC_U32(y));
	}

	inline constexpr U32 operator^(EInputContext x, EInputContext y)
	{
		return (STATIC_U32(x) ^ STATIC_U32(y));
	}

	inline constexpr U32 operator~(EInputContext x)
	{
		return ~STATIC_U32(x);
	}

	inline U32& operator&=(U32& mask, EInputContext context)
	{
		mask &= STATIC_U32(context);
		return mask;
	}

	inline U32& operator|=(U32& mask, EInputContext context)
	{
		mask |= context;
		return mask;
	}

	inline U32& operator^=(U32& mask, EInputContext context)
	{
		mask ^= context;
		return mask;
	}

	enum class EInputKey
	{
		None		= 0x00,
		Mouse1		= 0x01, // Left
		Mouse3		= 0x02, // Middle
		Mouse2		= 0x03, // Right
		Mouse4		= 0x04,
		Mouse5		= 0x05,
		Return		= 0x0000000du,	// Enter
		Esc			= 0x0000001bu,	// Escape
		Backspace	= 0x00000008u,
		Tab			= 0x00000009u,
		Space		= 0x00000020u,
		Key0		= 0x00000030u,
		Key1		= 0x00000031u,
		Key2		= 0x00000032u,
		Key3		= 0x00000033u,
		Key4		= 0x00000034u,
		Key5		= 0x00000035u,
		Key6		= 0x00000036u,
		Key7		= 0x00000037u,
		Key8		= 0x00000038u,
		Key9		= 0x00000039u,
		Shift		= 0x400000e1u,
		Ctrl		= 0x400000e0u,
		Alt			= 0x400000e2u, // TODO.NW: See if we get inconsistent behavior here because we don't use the higher valued keycodes?
		Pause		= 0x40000048u,
		Caps		= 0x40000039u,	// Caps Lock
		PageUp		= 0x4000004bu,
		PageDown	= 0x4000004eu,
		End			= 0x4000004du,
		Home		= 0x4000004au,
		Left		= 0x40000050u, // Left Arrow
		Up			= 0x40000052u, // Up Arrow
		Right		= 0x4000004fu, // Right Arrow
		Down		= 0x40000051u, // Down
		PrtSc		= 0x40000046u, // Print Screen
		Insert		= 0x40000049u,
		Delete		= 0x0000007fu,
		KeyA		= 0x00000061u,
		KeyB		= 0x00000062u,
		KeyC		= 0x00000063u,
		KeyD		= 0x00000064u,
		KeyE		= 0x00000065u,
		KeyF		= 0x00000066u,
		KeyG		= 0x00000067u,
		KeyH		= 0x00000068u,
		KeyI		= 0x00000069u,
		KeyJ		= 0x0000006au,
		KeyK		= 0x0000006bu,
		KeyL		= 0x0000006cu,
		KeyM		= 0x0000006du,
		KeyN		= 0x0000006eu,
		KeyO		= 0x0000006fu,
		KeyP		= 0x00000070u,
		KeyQ		= 0x00000071u,
		KeyR		= 0x00000072u,
		KeyS		= 0x00000073u,
		KeyT		= 0x00000074u,
		KeyU		= 0x00000075u,
		KeyV		= 0x00000076u,
		KeyW		= 0x00000077u,
		KeyX		= 0x00000078u,
		KeyY		= 0x00000079u,
		KeyZ		= 0x0000007au,
		LWin		= 0x400000e3u, // Left Windows key
		RWin		= 0x400000e7u, // Right Windows key
		KeyNum0		= 0x40000062u, // Numeric keypad 0 key
		KeyNum1		= 0x40000059u, // Numeric keypad 1 key
		KeyNum2		= 0x4000005au, // Numeric keypad 2 key
		KeyNum3		= 0x4000005bu, // Numeric keypad 3 key
		KeyNum4		= 0x4000005cu, // Numeric keypad 4 key
		KeyNum5		= 0x4000005du, // Numeric keypad 5 key
		KeyNum6		= 0x4000005eu, // Numeric keypad 6 key
		KeyNum7		= 0x4000005fu, // Numeric keypad 7 key
		KeyNum8		= 0x40000060u, // Numeric keypad 8 key
		KeyNum9		= 0x40000061u, // Numeric keypad 9 key
		KeyNumMult	= 0x40000055u, // Numeric keypad Multiply key
		KeyNumAdd	= 0x40000057u, // Numeric keypad Add key
		Pipe		= 0x4000009fu, // Separator key
		KeyNumSub	= 0x40000056u, // Numeric keypad Subtract key
		KeyNumDec	= 0x40000063u, // Numeric keypad Decimal key
		KeyNumDiv	= 0x40000054u, // Numeric keypad Divide key
		KeyNumEnter = 0x40000058u, // Numeric keypad Enter key
		F1			= 0x4000003au,
		F2			= 0x4000003bu,
		F3			= 0x4000003cu,
		F4			= 0x4000003du,
		F5			= 0x4000003eu,
		F6			= 0x4000003fu,
		F7			= 0x40000040u,
		F8			= 0x40000041u,
		F9			= 0x40000042u,
		F10			= 0x40000043u,
		F11			= 0x40000044u,
		F12			= 0x40000045u,
		NumLk		= 0x40000053u,	// Num Lock key
		ScrLk		= 0x40000047u,	// Scroll Lock key
		GamepadInvalid = 257,
		GamepadSouth,				/**< Bottom face button (e.g. Xbox A button) */
		GamepadEast,				/**< Right face button (e.g. Xbox B button) */
		GamepadWest,				/**< Left face button (e.g. Xbox X button) */
		GamepadNorth,				/**< Top face button (e.g. Xbox Y button) */
		GamepadBack,
		GamepadGuide,
		GamepadStart,
		GamepadL3,
		GamepadR3,
		GamepadL1,
		GamepadR1,
		GamepadDPadUp,
		GamepadDPadDown,
		GamepadDPadLeft,
		GamepadDPadRight,
		GamepadMisc1,				/**< Additional button (e.g. Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button, Google Stadia capture button) */
		GamepadR4,				/**< Upper or primary paddle, under your right hand (e.g. Xbox Elite paddle P1, DualSense Edge RB button, Right Joy-Con SR button) */
		GamepadL4,				/**< Upper or primary paddle, under your left hand (e.g. Xbox Elite paddle P3, DualSense Edge LB button, Left Joy-Con SL button) */
		GamepadR5,				/**< Lower or secondary paddle, under your right hand (e.g. Xbox Elite paddle P2, DualSense Edge right Fn button, Right Joy-Con SL button) */
		GamepadL5,				/**< Lower or secondary paddle, under your left hand (e.g. Xbox Elite paddle P4, DualSense Edge left Fn button, Left Joy-Con SR button) */
		GamepadTouchPad,			/**< PS4/PS5 touchpad button */
		GamepadMisc2,				/**< Additional button */
		GamepadMisc3,				/**< Additional button (e.g. Nintendo GameCube left trigger click) */
		GamepadMisc4,				/**< Additional button (e.g. Nintendo GameCube right trigger click) */
		GamepadMisc5,				/**< Additional button */
		GamepadMisc6,				/**< Additional button */
		GamepadRegionStart = GamepadSouth
	};

	enum class EGamepadType
	{
		Unknown = 0,
		Standard,
		Xbox360,
		XboxOne,
		PS3,
		PS4,
		PS5,
		NintendoSwitchPro,
		NintendoSwitchJoyconLeft,
		NintendoSwitchJoyconRight,
		NintendoSwitchJoyconPair,
		GameCube,
		Count
	};

	/**
	 * The set of gamepad button labels
	 *
	 * This isn't a complete set, just the face buttons to make it easy to show
	 * button prompts.
	 *
	 * For a complete set, you should look at the button and gamepad type and have
	 * a set of symbols that work well with your art style.
	 *
	 * \since This enum is available since SDL 3.2.0.
	 */
	enum class EGamepadButtonLabel
	{
		Unknown,
		A,
		B,
		X,
		Y,
		Cross,
		Circle,
		Square,
		Triangle
	};

	/**
	 * The list of axes available on a gamepad
	 *
	 * Thumbstick axis values range from SDL_JOYSTICK_AXIS_MIN to
	 * SDL_JOYSTICK_AXIS_MAX, and are centered within ~8000 of zero, though
	 * advanced UI will allow users to set or autodetect the dead zone, which
	 * varies between gamepads.
	 *
	 * Trigger axis values range from 0 (released) to SDL_JOYSTICK_AXIS_MAX (fully
	 * pressed) when reported by SDL_GetGamepadAxis(). Note that this is not the
	 * same range that will be reported by the lower-level SDL_GetJoystickAxis().
	 *
	 * \since This enum is available since SDL 3.2.0.
	 */
	enum class EInputAxis
	{
		Key,
		MouseWheel,
		MouseDeltaHorizontal,
		MouseDeltaVertical,
		MousePositionHorizontal,
		MousePositionVertical,
		GamepadInvalid,
		GamepadLeftStickHorizontal,
		GamepadLeftStickVertical,
		GamepadRightStickHorizontal,
		GamepadRightStickVertical,
		GamepadLeftTrigger,
		GamepadRightTrigger,
		Count = GamepadRightTrigger + 1,
		GamepadRegionStart = GamepadLeftStickHorizontal
	};

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
		Rename,
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
		EInputKey Key = EInputKey::None;
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
		SInputAction(EInputKey key, EInputContext context, EInputModifier modifier)
			: Key(key)
			, Contexts(STATIC_U32(context))
			, Modifiers(STATIC_U32(modifier))
		{}

		SInputAction(EInputKey key, U32 contexts, EInputModifier modifier)
			: Key(key)
			, Contexts(STATIC_U32(contexts))
			, Modifiers(STATIC_U32(modifier))
		{}

		SInputAction(EInputKey key, std::initializer_list<EInputContext> contexts, std::initializer_list<EInputModifier> modifiers = {})
			: Key(key)
			, Contexts(STATIC_U32(EInputContext::Editor))
			, Modifiers(0)
		{
			SetContexts(contexts);
			SetModifiers(modifiers);
		}

		SInputAction(EInputKey key, EInputContext context)
			: Key(key)
			, Contexts(STATIC_U32(context))
			, Modifiers(0)
		{}

		SInputAction(EInputKey key, U32 contexts)
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

		EInputKey Key = EInputKey::None;
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

		[[nodiscard]] bool HasKey(const EInputKey& key) const
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
				[modifiers](const SInputAction& action) {return (action.Modifiers ^ modifiers) == 0; });
		}

		[[nodiscard]] bool Has(const EInputKey& key, U32 context, U32 modifiers) const
		{
			return std::ranges::any_of(Actions.begin(), Actions.end(),
				[key, context, modifiers](const SInputAction& action)
				{
					return action.Key == key && (action.Contexts & context) != 0 && (action.Modifiers ^ modifiers) == 0;
				});
		}

		CMulticastDelegate<const SInputActionPayload> Delegate;
		std::vector<SInputAction> Actions;
	};

	struct SInputAxis
	{
		SInputAxis(EInputAxis axis, EInputContext context)
			: Axis(axis)
			, AxisPositiveKey(EInputKey::KeyW)
			, AxisNegativeKey(EInputKey::KeyS)
			, Contexts(STATIC_U32(context))
			, Modifiers(0)
		{}

		SInputAxis(EInputAxis axis, U32 contexts)
			: Axis(axis)
			, AxisPositiveKey(EInputKey::KeyW)
			, AxisNegativeKey(EInputKey::KeyS)
			, Contexts(contexts)
			, Modifiers(0)
		{}

		SInputAxis(EInputAxis axis, EInputContext context, EInputModifier modifier)
			: Axis(axis)
			, AxisPositiveKey(EInputKey::KeyW)
			, AxisNegativeKey(EInputKey::KeyS)
			, Contexts(STATIC_U32(context))
			, Modifiers(STATIC_U32(modifier))
		{}

		SInputAxis(EInputAxis axis, EInputKey axisPositiveKey, EInputKey axisNegativeKey, EInputContext context)
			: Axis(axis)
			, AxisPositiveKey(axisPositiveKey)
			, AxisNegativeKey(axisNegativeKey)
			, Contexts(STATIC_U32(context))
			, Modifiers(0)
		{}

		SInputAxis(EInputAxis axis, EInputKey axisPositiveKey, EInputKey axisNegativeKey, U32 contexts)
			: Axis(axis)
			, AxisPositiveKey(axisPositiveKey)
			, AxisNegativeKey(axisNegativeKey)
			, Contexts(contexts)
			, Modifiers(0)
		{}

		SInputAxis(EInputAxis axis, std::initializer_list<EInputContext> contexts, std::initializer_list<EInputModifier> modifiers = {})
			: Axis(axis)
			, AxisPositiveKey(EInputKey::KeyW)
			, AxisNegativeKey(EInputKey::KeyS)
			, Contexts(STATIC_U32(EInputContext::Editor))
			, Modifiers(0)
		{
			SetContexts(contexts);
			SetModifiers(modifiers);
		}

		SInputAxis(EInputAxis axis, EInputKey axisPositiveKey, EInputKey axisNegativeKey, std::initializer_list<EInputContext> contexts, std::initializer_list<EInputModifier> modifiers = {})
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

		[[nodiscard]] F32 GetAxisValue(const EInputKey& key) const
		{
			if (AxisPositiveKey == key)
				return 1.0;

			if (AxisNegativeKey == key)
				return -1.0f;

			return 0.0f;
		}

		EInputAxis Axis = EInputAxis::Key;
		EInputKey AxisPositiveKey = EInputKey::None; // Optional
		EInputKey AxisNegativeKey = EInputKey::None; // Optional
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

		[[nodiscard]] bool Has(const EInputKey& key, const U32 context, const U32 modifiers, F32& outAxisValue) const
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
