// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/ComponentView.h"

namespace Havtorn
{
	struct SVolumetricLightComponentView : public SComponentEditorContext
	{
		HAVTORN_API SComponentViewResult View(const SEntity& entityOwner, CScene* scene) override;

		static SVolumetricLightComponentView Context;
	};
}
