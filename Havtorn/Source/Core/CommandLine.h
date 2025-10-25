// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core.h"

namespace Havtorn
{	
	class UCommandLine
	{
	public:
		CORE_API static void Parse(const std::string& commandLine);
		CORE_API static std::string GetOptionParameter(const std::string& option);
		CORE_API static std::vector<std::string> GetFreeParameters();
		CORE_API static bool IsOptionParameterValid(const std::string& parameter);
		CORE_API static bool HasFreeParameter(const std::string& parameter);
		CORE_API static std::string GetDeepLinkCommand();

	private:
		static I64 PickIndex(I64 whitespace, I64 nullTermination);

		const std::string InvalidParameter = "INVALID_PARAMETER";
		const std::string DeepLinkURL = "havtorn://";

		UCommandLine() = default;
		static UCommandLine* Instance;
		std::vector<std::string> FreeParameters;
		std::map<std::string, std::string> Options;
		std::string DeepLinkCommand;
	};
}
