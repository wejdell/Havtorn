// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core.h"
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

	enum class CORE_API ELogCategory
	{
		Trace,
		Debug,
		Info,
		Warning,
		Error,
		Fatal
	};
	
	class CORE_API ILogContext
	{
	public:
		virtual void Log(const ELogCategory category, const std::string& message) = 0;
	};

	struct ULog
	{
		static void CORE_API Print(const ELogCategory category, const char* message, ...);
		static void CORE_API AddLogContext(ILogContext* context);
		static void CORE_API RemoveLogContext(ILogContext* context);

	private:
		// TODO.NR: Figure out another place for this. This is the exact same as the one used in EngineException.h
		static std::string StringVsprintf(const char* format, const std::va_list args);
		static EConsoleColor GetColorFromCategory(const ELogCategory category);
		static std::string GetCategoryName(const ELogCategory category);

	private:
		static ULog* Instance;
		std::vector<ILogContext*> Contexts;
	};
}

#define HV_LOG_TRACE(...)		::Havtorn::ULog::Print(::Havtorn::ELogCategory::Trace, __VA_ARGS__)
#define HV_LOG_DEBUG(...)		::Havtorn::ULog::Print(::Havtorn::ELogCategory::Debug, __VA_ARGS__)
#define HV_LOG_INFO(...)		::Havtorn::ULog::Print(::Havtorn::ELogCategory::Info, __VA_ARGS__)
#define HV_LOG_WARN(...)		::Havtorn::ULog::Print(::Havtorn::ELogCategory::Warning, __VA_ARGS__)
#define HV_LOG_ERROR(...)		::Havtorn::ULog::Print(::Havtorn::ELogCategory::Error, __VA_ARGS__)
#define HV_LOG_FATAL(...)		::Havtorn::ULog::Print(::Havtorn::ELogCategory::Fatal, __VA_ARGS__)
