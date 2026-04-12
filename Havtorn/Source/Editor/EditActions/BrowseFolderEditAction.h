// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "EditAction.h"

#include <filesystem>

namespace Havtorn
{
	struct SBrowseFolderEditAction : public SEditAction
	{
		static SMetaCommand MakeEditActionCommand(const std::filesystem::path& from, const std::filesystem::path& to);
		SBrowseFolderEditAction(const SMetaCommand& command);
		void ResolveAction(CEditorManager* manager, const bool inverted) override;
		std::string ResolveCompactName(const SMetaCommand& command) override;
	};
}
