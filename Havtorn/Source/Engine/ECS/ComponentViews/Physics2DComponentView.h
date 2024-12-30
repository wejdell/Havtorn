// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/ComponentView.h"

namespace Havtorn
{
	struct SPhysics2DComponentView : public SComponentEditorContext
	{
		HAVTORN_API SComponentViewResult View(const SEntity& entityOwner, CScene* scene) override;

		static SPhysics2DComponentView Context;
	};
}
