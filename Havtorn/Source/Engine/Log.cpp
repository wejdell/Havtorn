// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"

#include "Log.h"

namespace Havtorn
{
	void ULog::Print(const EConsoleColor& color, const char* category, const char* message, ...)
	{
		const HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
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
		std::cout << "[" << timeStamp << "]" << " " << category << combinedMessage << std::endl;

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
}
