// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include "Color.h"

namespace Havtorn
{
	class CScene;

	namespace HexRune
	{		
		struct SScript;
	}

	struct SNodeView
	{
		friend class CEditorManager;
			
		virtual void View(const HexRune::SScript* /*owningScript*/) const {};
		virtual U8 GetSortingPriority() const { return UMath::MaxU8; };
		U64 GetRuntimeHash() const { return RuntimeHash; }

		std::string Name = "";
		std::string Category = "";
		SColor Color = SColor(1.0f, 1.0f, 1.0f, 0.1f);	

		// TODO.NW: Add minimum node width?

	private:
		U64 RuntimeHash = 0;
	};
}
