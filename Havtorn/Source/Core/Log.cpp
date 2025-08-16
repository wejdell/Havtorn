// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Log.h"

namespace Havtorn
{
	ULog* ULog::Instance = nullptr;

	void ULog::Print(const ELogCategory category, const char* message, ...)
	{
		if (!Instance)
			Instance = new ULog();

		const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		const EConsoleColor color = GetColorFromCategory(category);
		const bool isDefault = color == EConsoleColor::White;

		// Set Console Color
		if (!isDefault)
			SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));

		// VA Args, combined message
		va_list argptr;
		va_start(argptr, message);
		const std::string combinedMessage{ StringVsprintf(message, argptr) };

		// Timestamp
		tm newtime{};
		const time_t now = time(nullptr);
		localtime_s(&newtime, &now);
		char timeStamp[16];
		strftime(timeStamp, sizeof timeStamp, "%H:%M:%S", &newtime);

		// Printout
		const std::string categoryName = GetCategoryName(category);
		const std::string finalString = "[" + std::string(timeStamp) + "]" + " " + categoryName + combinedMessage;
		std::cout << finalString << std::endl;

		for (ILogContext* context : Instance->Contexts)
			context->Log(category, finalString);

		// Reset Console Color
		if (!isDefault)
			SetConsoleTextAttribute(hConsole, static_cast<WORD>(EConsoleColor::White));
	}

	std::string ULog::StringVsprintf(const char* format, const std::va_list args)
	{
		va_list tmpArgs; //unfortunately you cannot consume a va_list twice
		va_copy(tmpArgs, args); //so we have to copy it
		const int requiredLen = _vscprintf(format, tmpArgs) + 1;
		va_end(tmpArgs);

		char buff[4096];
		memset(buff, 0, requiredLen);

		if (vsnprintf_s(buff, requiredLen, format, args) < 0)
			return "StringVsprintf encoding error";

		return { buff };
	}

	EConsoleColor ULog::GetColorFromCategory(const ELogCategory category)
	{
		switch (category)
		{
		case ELogCategory::Trace:
			return EConsoleColor::White;
		case ELogCategory::Debug:
			return EConsoleColor::DarkGreen;
		case ELogCategory::Info:
			return EConsoleColor::DarkAqua;
		case ELogCategory::Warning:
			return EConsoleColor::Yellow;
		case ELogCategory::Error:
			return EConsoleColor::Red;
		case ELogCategory::Fatal:
		default:
			return EConsoleColor::WhiteRedBackground;
		};
	}

	std::string ULog::GetCategoryName(const ELogCategory category)
	{
		switch (category)
		{
		case ELogCategory::Trace:
			return "TRACE: ";
		case ELogCategory::Debug:
			return "DEBUG: ";
		case ELogCategory::Info:
			return "INFO:  ";
		case ELogCategory::Warning:
			return "WARN:  ";
		case ELogCategory::Error:
			return "ERROR: ";
		case ELogCategory::Fatal:
		default:
			return "FATAL: ";
		};
	}

	void ULog::AddLogContext(ILogContext* context)
	{
		if (!Instance)
			Instance = new ULog();

		Instance->Contexts.push_back(context);
	}

	void ULog::RemoveLogContext(ILogContext* context)
	{
		if (!Instance)
			return;
		
		std::erase(Instance->Contexts, context);
	}
}
