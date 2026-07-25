// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include "ComponentView.h"
#include "HexRune/HexRune.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SScriptDataBinding;
	}

	struct EDITOR_API SScriptComponentView : public SComponentView
	{
		SComponentViewResult View(const SEntity& entityOwner, CScene* scene) const override;
		virtual const char* GetComponentName() const override { return "Script"; };
		U8 GetSortingPriority() const override;

		void ViewDataBinding(CScene* scene, HexRune::SScriptDataBinding& dataBinding) const;

		static SScriptComponentView Context;
	};
}
