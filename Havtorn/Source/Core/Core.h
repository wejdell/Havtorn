// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#ifdef HV_PLATFORM_WINDOWS

#ifdef HV_CORE_DLL
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

#ifdef HV_PLATFORM_DLL
#define PLATFORM_API __declspec(dllexport)
#else
#define PLATFORM_API __declspec(dllimport)
#endif

#ifdef HV_GUI_DLL
#define GUI_API __declspec(dllexport)
#else
#define GUI_API __declspec(dllimport)
#endif

#ifdef HV_ENGINE_DLL
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif

#ifdef HV_GAME_DLL
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __declspec(dllimport)
#endif

#ifdef HV_EDITOR_DLL
#define EDITOR_API __declspec(dllexport)
#else
#define EDITOR_API __declspec(dllimport)
#endif

#ifndef HV_DIRECTX_11
	#define HV_DIRECTX_11
#endif

#else
	#error Havtorn currently only supports 64 bit Windows
#endif

#ifdef HV_ENABLE_ASSERTS
	#define HV_ASSERT(x, ...) { if(!(x)) { HV_LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define HV_ASSERT(x, ...)
#endif

#define SAFE_DELETE(x) delete x; x = nullptr;
#define SAFE_ARRAY_DELETE(x) delete[] x; x = nullptr;

#define HV_ASSERT_BUFFER(x) static_assert((sizeof(x) % 16) == 0, "CB size not padded correctly");

#define BIT(x) (1 << x)
#define CACHE_LINE 32
#define CACHE_ALIGN __declspec(align(CACHE_LINE))

#define MAX_STRING_LEN 256
#define ENTITY_LIMIT 45

#define ARRAY_SIZE(x) ((int)(sizeof(x) / sizeof(*(x)))) // Size of a static C-style array. Don't use on pointers!

#define HAVTORN_VERSION "0.0.1"
