// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include <ECS/ComponentEditorContext.h>

namespace Havtorn
{
	struct SGhostyComponentEditorContext : public SComponentEditorContext
	{
		SComponentViewResult View(const SEntity& entityOwner, CScene* scene) const override;
		bool AddComponent(const SEntity& entity, CScene* scene) const override;
		bool RemoveComponent(const SEntity& entity, CScene* scene) const override;
		virtual const char* GetComponentName() const override { return "Ghosty"; };
		static SGhostyComponentEditorContext Context;
	};
}
