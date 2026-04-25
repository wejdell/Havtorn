// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "EditAction.h"

#include <filesystem>
#include <MathTypes/Matrix.h>

namespace Havtorn
{
	struct SEditorAssetRepresentation;
	struct STransformComponent;

	struct SMoveTransformEditAction : public SEditAction
	{
		static SMetaCommand MakeEditActionCommand(CEditorManager* manager, STransformComponent* transformComp, SMatrix fullDeltaMatrix);
		SMoveTransformEditAction(const SMetaCommand& command);
		void ResolveAction(CEditorManager* manager, const bool inverted) override;
		std::string ResolveCompactName(const SMetaCommand& command) override;
	};
}
