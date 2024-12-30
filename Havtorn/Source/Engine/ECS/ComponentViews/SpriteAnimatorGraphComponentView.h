// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/ComponentView.h"

namespace Havtorn
{
	struct SSpriteAnimatorGraphComponentView : public SComponentEditorContext
	{
		HAVTORN_API SComponentViewResult View(const SEntity& entityOwner, CScene* scene) override;

		static SSpriteAnimatorGraphComponentView Context;
	};
}
