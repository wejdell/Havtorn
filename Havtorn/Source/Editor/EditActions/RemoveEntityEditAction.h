// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "EditAction.h"

namespace Havtorn
{
	struct SEditorAssetRepresentation;
	struct SEntity;

	struct SRemoveEntityEditAction : public SEditAction
	{
		static SMetaCommand MakeEditActionCommand(CEditorManager* manager, const SEntity& entity, const bool removed);
		SRemoveEntityEditAction(const SMetaCommand& command);
		void ResolveAction(CEditorManager* manager, const bool inverted) override;
		std::string ResolveCompactName(const SMetaCommand& command) override;
	};
}
