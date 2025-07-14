// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/ComponentEditorContext.h"
#include "HexRune/HexRune.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SScriptDataBinding;
	}

	struct ENGINE_API SScriptComponentEditorContext : public SComponentEditorContext
	{
		SComponentViewResult View(const SEntity& entityOwner, CScene* scene) const override;
		bool AddComponent(const SEntity& entity, CScene* scene) const override;
		bool RemoveComponent(const SEntity& entity, CScene* scene) const override;
		U8 GetSortingPriority() const override;

		void ViewDataBinding(CScene* scene, HexRune::SScriptDataBinding& dataBinding) const;

		static SScriptComponentEditorContext Context;
	};
}
