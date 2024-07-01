// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core/Core.h"
#include <cstdarg>

namespace Havtorn
{
	enum class EConsoleColor
	{
		DarkGreen = 2,
		DarkAqua = 3,
		White = 7,
		Red = 12,
		Yellow = 14,
		WhiteRedBackground = 79
	};

	struct HAVTORN_API ULog
	{
		static void Print(const EConsoleColor& color, const char* category, const char* message, ...);

		// TODO.NR: Figure out another place for this. This is the exact same as the one used in EngineException.h
		static std::string StringVsprintf(const char* format, const std::va_list args);
	};
}

#define HV_LOG_TRACE(...)		::Havtorn::ULog::Print(::Havtorn::EConsoleColor::White, "TRACE: ", __VA_ARGS__)
#define HV_LOG_DEBUG(...)		::Havtorn::ULog::Print(::Havtorn::EConsoleColor::DarkGreen, "DEBUG: ", __VA_ARGS__)
#define HV_LOG_INFO(...)		::Havtorn::ULog::Print(::Havtorn::EConsoleColor::DarkAqua, "INFO:  ", __VA_ARGS__)
#define HV_LOG_WARN(...)		::Havtorn::ULog::Print(::Havtorn::EConsoleColor::Yellow, "WARN:  ", __VA_ARGS__)
#define HV_LOG_ERROR(...)		::Havtorn::ULog::Print(::Havtorn::EConsoleColor::Red, "ERROR: ", __VA_ARGS__)
#define HV_LOG_FATAL(...)		::Havtorn::ULog::Print(::Havtorn::EConsoleColor::WhiteRedBackground, "FATAL: ", __VA_ARGS__)
