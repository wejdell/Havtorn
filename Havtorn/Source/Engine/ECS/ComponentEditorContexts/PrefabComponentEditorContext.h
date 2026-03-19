// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/ComponentEditorContext.h"

namespace Havtorn
{
	struct ENGINE_API SPrefabComponentEditorContext : public SComponentEditorContext
	{
		SComponentViewResult View(const SEntity& entityOwner, CScene* scene) const override;
		bool AddComponent(const SEntity& entity, CScene* scene) const override;
		bool RemoveComponent(const SEntity& entity, CScene* scene) const override;
		virtual const char* GetComponentName() const override { return "Prefab"; };
		U8 GetSortingPriority() const override;

		static SPrefabComponentEditorContext Context;
	};
}
