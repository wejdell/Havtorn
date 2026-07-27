// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ComponentView.h"

namespace Havtorn
{
	struct EDITOR_API SSkeletalAnimationComponentView : public SComponentView
	{
		SComponentViewResult View(const SEntity& entityOwner, CScene* scene) const override;
		virtual const char* GetComponentName() const override { return "Skeletal Animation"; };
	};
}
