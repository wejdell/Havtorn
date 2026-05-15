// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SAudioListenerComponent: public SComponent
	{
		SAudioListenerComponent() = default;
		SAudioListenerComponent(const SEntity& entityOwner);

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		SVector LocalOffset = SVector::Zero;
		U64 AudioObjectID = 0;
	};
}
