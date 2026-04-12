// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <MetaCommand/MetaCommand.h>

#include <string>

namespace Havtorn
{
	class CEditorManager;

	struct SEditAction
	{
		SEditAction(const SMetaCommand& command, const std::string& compactName);
		virtual void ResolveAction(CEditorManager* manager, const bool inverted);
		virtual std::string ResolveCompactName(const SMetaCommand& command);
		SMetaCommand Command;
		std::string CompactName;
	};
}
