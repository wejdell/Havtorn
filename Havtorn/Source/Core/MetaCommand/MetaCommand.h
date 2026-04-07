// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	struct SMetaCommand
	{
		CORE_API SMetaCommand(const std::string_view input);

		std::map<std::string, std::string> Parameters;
		std::string Name;
		U32 Parser = 0;
		U32 Domain = 0;
		U32 Command = 0;
	};
}
