// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "ComponentView.h"

namespace Havtorn
{
	struct EDITOR_API SPhysics3DComponentView : public SComponentView
	{
		SComponentViewResult View(const SEntity& entityOwner, CScene* scene) const override;
		virtual const char* GetComponentName() const override { return "Physics 3D"; };
	};
}
