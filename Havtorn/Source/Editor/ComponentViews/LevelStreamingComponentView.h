// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "ComponentView.h"

namespace Havtorn
{
	struct EDITOR_API SLevelStreamingComponentView : public SComponentView
	{
		void View(const SEntity& entityOwner, CScene* scene) const override;
		virtual const char* GetComponentName() const override { return "Level Streaming"; };
		U8 GetSortingPriority() const override;
	};
}
