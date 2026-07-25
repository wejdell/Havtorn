// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "ComponentView.h"

namespace Havtorn
{
	struct EDITOR_API SStaticMeshComponentView : public SComponentView
	{
		SComponentViewResult View(const SEntity& entityOwner, CScene* scene) const override;
		virtual const char* GetComponentName() const override { return "Static Mesh"; };
		U8 GetSortingPriority() const override;

		static SStaticMeshComponentView Context;
	};
}
