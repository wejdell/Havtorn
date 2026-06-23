// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <variant>
#include <GameplayTags/GameplayTag.h>

namespace Havtorn
{
	enum class EInputModifier
	{
		None = 0,
		Shift = BIT(0) | BIT(1),
		Ctrl = BIT(6) | BIT(7),
		Alt = BIT(8) | BIT(9),
		Super = BIT(10) | BIT(11),
	};

	enum class EInputContext
	{
		Editor = BIT(0),
		InGame = BIT(1),
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

	enum class EInputButton
	{
		None                = 0,
		MouseLeft           = 1,
		MouseMiddle         = 2,
		MouseRight          = 3, 
		KeyA                = 4,
		KeyB                = 5,
		KeyC                = 6,
		KeyD                = 7,
		KeyE                = 8,
		KeyF                = 9,
		KeyG                = 10,
		KeyH                = 11,
		KeyI                = 12,
		KeyJ                = 13,
		KeyK                = 14,
		KeyL                = 15,
		KeyM                = 16,
		KeyN                = 17,
		KeyO                = 18,
		KeyP                = 19,
		KeyQ                = 20,
		KeyR                = 21,
		KeyS                = 22,
		KeyT                = 23,
		KeyU                = 24,
		KeyV                = 25,
		KeyW                = 26,
		KeyX                = 27,
		KeyY                = 28,
		KeyZ                = 29,
		Key1                = 30,
		Key2                = 31,
		Key3                = 32,
		Key4                = 33,
		Key5                = 34,
		Key6                = 35,
		Key7                = 36,
		Key8                = 37,
		Key9                = 38,
		Key0                = 39,
		Return              = 40, // Enter
		Esc                 = 41, // Escape
		Backspace           = 42,
		Tab                 = 43,
		Space               = 44,
        Minus               = 45, // + on SE keyboard
        Equals              = 46, // ´ on SE keyboard
        LBracket            = 47, // Å on SE keyboard
        RBracket            = 48, // ¨ on SE keyboard
        Semicolon           = 51, // Ö on SE keyboard
        AcuteAccent         = 52, // Ä on SE keyboard
        ANSIGraveAccent     = 53, // § on SE keyboard
        Comma               = 54,
        Period              = 55,
        ForwardSlash        = 56, // - on SE keyboard
		Caps                = 57, // Caps Lock
		F1                  = 58,
		F2                  = 59,
		F3                  = 60,
		F4                  = 61,
		F5                  = 62,
		F6                  = 63,
		F7                  = 64,
		F8                  = 65,
		F9                  = 66,
		F10                 = 67,
		F11                 = 68,
		F12                 = 69,
		PrtSc               = 70, // Print Screen
		ScrLk               = 71, // Scroll Lock key
		Pause               = 72,
		Insert              = 73,
		Home                = 74,
		PageUp              = 75,
		Delete              = 76,
		End                 = 77,
		PageDown            = 78,
		Right               = 79, // Right Arrow
		Left                = 80, // Left Arrow
		Down                = 81, // Down
		Up                  = 82, // Up Arrow
		NumLk               = 83, // Num Lock key
		KeyNumDiv           = 84, // Numeric keypad Divide key
		KeyNumMult          = 85, // Numeric keypad Multiply key
		KeyNumSub           = 86, // Numeric keypad Subtract key
		KeyNumAdd           = 87, // Numeric keypad Add key
		KeyNumEnter         = 88, // Numeric keypad Enter key
		KeyNum1             = 89, // Numeric keypad 1 key
		KeyNum2             = 90, // Numeric keypad 2 key
		KeyNum3             = 91, // Numeric keypad 3 key
		KeyNum4             = 92, // Numeric keypad 4 key
		KeyNum5             = 93, // Numeric keypad 5 key
		KeyNum6             = 94, // Numeric keypad 6 key
		KeyNum7             = 95, // Numeric keypad 7 key
		KeyNum8             = 96, // Numeric keypad 8 key
		KeyNum9             = 97, // Numeric keypad 9 key
		KeyNum0             = 98, // Numeric keypad 0 key
		KeyNumDec           = 99, // Numeric keypad Decimal key
        ISOGraveAccent      = 100, // < on SE keyboard
		LCtrl               = 224,
		LShift              = 225,
		LAlt                = 226, // TODO.NW: See if we get inconsistent behavior here because we don't use the higher valued keycodes?
		LGui                = 227, // Left Windows/Command key
		RCtrl               = 228,
		RShift              = 229,
		RAlt                = 230,
		RGui                = 231, // Right Windows/Command key
		GamepadInvalid      = 257,
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

	struct SAxis //Float
	{
		EInputAxis Axis = EInputAxis::Key;
		EInputButton AxisPositiveKey = EInputButton::None; // Optional
		EInputButton AxisNegativeKey = EInputButton::None; // Optional
		U32 Modifiers = STATIC_U32(EInputModifier::None);
	};

	struct SKey //Bool
	{
		EInputButton Key = EInputButton::None;
		U32 Modifiers = STATIC_U32(EInputModifier::None);
	};

	struct S2DAxis //2D Vector
	{
		SAxis HorizontalAxis;
		SAxis VerticalAxis;
	};

	enum class EInputActivationType : U8
	{
		Continuous,
		KeyDown,
		KeyUp,
	};

	struct SInputMapping
	{
		EInputActivationType ActivationType = EInputActivationType::Continuous;
		std::variant<SAxis, SKey, S2DAxis> Data;
	};

	struct SInputMapAction
	{
		SGameplayTag Tag;
		std::vector<SInputMapping> InputMappings;
	};
}

template <>
struct magic_enum::customize::enum_range<Havtorn::EInputButton> 
{
	static constexpr Havtorn::I32 min = 0;
	static constexpr Havtorn::I32 max = 512;
};
