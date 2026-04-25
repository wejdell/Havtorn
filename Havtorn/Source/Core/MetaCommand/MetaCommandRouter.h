// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "Core.h"
#include "MetaCommand.h"
#include <map>

namespace Havtorn
{
	class CORE_API IMetaCommandParser
	{
	public:
		virtual void Parse(const SMetaCommand& command) = 0;
		U32 ParserID = 0;
	};

	struct UMetaCommandRouter
	{
		static void CORE_API Push(const SMetaCommand& command);
		static void CORE_API AddParser(IMetaCommandParser* parser);
		static void CORE_API RemoveParser(IMetaCommandParser* parser);

	private:
		static UMetaCommandRouter* Instance;
		std::map<U32, IMetaCommandParser*> Parsers;
	};
}
