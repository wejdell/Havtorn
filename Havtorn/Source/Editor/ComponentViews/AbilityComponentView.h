// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "ComponentView.h"

namespace Havtorn
{
	struct EDITOR_API SAbilityComponentView : public SComponentView
	{
		void View(const SEntity& entityOwner, CScene* scene) const override;
		virtual const char* GetComponentName() const override { return "Ability"; };
	};
}
