// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "CommandLine.h"

namespace Havtorn
{
	UCommandLine* UCommandLine::Instance = nullptr;

	void UCommandLine::Parse(const std::string& commandLine)
	{
		if (Instance == nullptr)
			Instance = new UCommandLine();

		Instance->FreeParameters.clear();
		Instance->Options.clear();
		Instance->DeepLinkCommand = std::string();

		std::string commandLineCopy = commandLine;
		if (commandLineCopy.back() != ' ' && commandLineCopy.back() != '\0')
			commandLineCopy.push_back(' ');

		I64 firstIndex = 0;
		I64 whitespaceIndex = commandLineCopy.find_first_of(' ');
		I64 nullTerminationIndex = commandLineCopy.find_first_of('\0');
		I64 separatorIndex = PickIndex(whitespaceIndex, nullTerminationIndex);

		// Extract all params
		while (separatorIndex != -1)
		{
			I64 paramLength = separatorIndex - firstIndex;
			Instance->FreeParameters.push_back(commandLineCopy.substr(firstIndex, paramLength));
			
			std::string& newestParam = Instance->FreeParameters.back();
			std::string wrappingSequence = std::string("/\"");
			I64 wrapping = newestParam.find(wrappingSequence);
			while (wrapping != -1)
			{
				newestParam.erase(wrapping, wrappingSequence.size());
				wrapping = newestParam.find(wrappingSequence);
			}

			firstIndex = separatorIndex + 1;
			whitespaceIndex = commandLineCopy.find_first_of(' ', separatorIndex + 1);
			nullTerminationIndex = commandLineCopy.find_first_of('\0', separatorIndex + 1);
			separatorIndex = PickIndex(whitespaceIndex, nullTerminationIndex);
		}

		std::vector<I64> indicesOfParamsToDelete = {};

		// Extract DeepLink command
		if (Instance->FreeParameters.size() > 1)
		{
			std::string command = Instance->FreeParameters[1];
			I64 urlSeparator = command.find(Instance->DeepLinkURL);
			if (urlSeparator != -1)
			{
				U64 urlSize = Instance->DeepLinkURL.size();
				urlSeparator += urlSize;
				Instance->DeepLinkCommand = command.substr(urlSeparator, command.size() - urlSize);
				indicesOfParamsToDelete.push_back(1);
			}
		}

		// Extract option params
		for (I64 i = 0; i < STATIC_I64(Instance->FreeParameters.size()); i++)
		{
			const std::string& param = Instance->FreeParameters[i];

			if (param[0] != '-')
				continue;

			I64 equalityIndex = param.find_first_of('=');
			if (equalityIndex == -1)
				continue;

			std::string key = param.substr(1, equalityIndex - 1);
			std::string value = param.substr(equalityIndex + 1, param.size() - equalityIndex - 1);

			Instance->Options.emplace(key, value);
			indicesOfParamsToDelete.push_back(i);
		}

		// Remove option parameters from FreeParameters
		for (I64 i = indicesOfParamsToDelete.size() - 1; i >= 0; i--)
		{
			I64 offset = indicesOfParamsToDelete[i];
			Instance->FreeParameters.erase(Instance->FreeParameters.begin() + offset);
		}
	}

	std::string UCommandLine::GetOptionParameter(const std::string& option)
	{
		if (Instance == nullptr)
			return "INVALID_PARAMETER";

		if (Instance->Options.contains(option))
			return Instance->Options[option];

		return "INVALID_PARAMETER";
	}

	std::vector<std::string> UCommandLine::GetFreeParameters()
	{
		if (Instance == nullptr)
			return {};

		return Instance->FreeParameters;
	}

	bool UCommandLine::IsOptionParameterValid(const std::string& parameter)
	{
		if (Instance == nullptr)
			return false;

		return parameter != Instance->InvalidParameter;
	}

	bool UCommandLine::HasFreeParameter(const std::string& parameter)
	{
		if (Instance == nullptr)
			return false;

		return std::ranges::find(Instance->FreeParameters, parameter) != Instance->FreeParameters.end();
	}

	std::string UCommandLine::GetDeepLinkCommand()
	{
		if (Instance == nullptr)
			return "";

		return Instance->DeepLinkCommand;
	}

	I64 UCommandLine::PickIndex(I64 whitespace, I64 nullTermination)
	{
		if (whitespace >= 0 && nullTermination >= 0)
			return UMath::Min(whitespace, nullTermination);

		if (whitespace >= 0)
			return whitespace;

		if (nullTermination >= 0)
			return nullTermination;

		return -1;
	}
}
