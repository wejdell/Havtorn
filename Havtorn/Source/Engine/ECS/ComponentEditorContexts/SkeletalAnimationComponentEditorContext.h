// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/ComponentEditorContext.h"

namespace Havtorn
{
	struct ENGINE_API SSkeletalAnimationComponentEditorContext : public SComponentEditorContext
	{
		SComponentViewResult View(const SEntity& entityOwner, CScene* scene) const override;
		bool AddComponent(const SEntity& entity, CScene* scene) const override;
		bool RemoveComponent(const SEntity& entity, CScene* scene) const override;

		static SSkeletalAnimationComponentEditorContext Context;
	};
}
