// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include <ECS/ComponentEditorContext.h>

namespace Havtorn
{
	struct GAME_EDITOR_API STestEditorContext : public SComponentEditorContext
	{
		SComponentViewResult View(const SEntity& entityOwner, CScene* scene) const override;
		bool AddComponent(const SEntity& entity, CScene* scene) const override;
		bool RemoveComponent(const SEntity& entity, CScene* scene) const override;
		virtual const char* GetComponentName() const override { return "Ability"; };

		static STestEditorContext Context;
	};
}
