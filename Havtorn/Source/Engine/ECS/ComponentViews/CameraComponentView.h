// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/ComponentView.h"

namespace Havtorn
{
	struct SCameraComponentView : public SComponentView
	{
		void View(const SEntity& entityOwner, CScene* scene) override;
	};
}
