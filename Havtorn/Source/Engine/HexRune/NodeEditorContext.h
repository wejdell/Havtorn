// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include "Color.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SScript;
		struct SNode;

		struct SNodeEditorContext
		{
			virtual SNode* AddNode(SScript* /*script*/, const U64 /*existingID = 0*/) const { return nullptr; };
			virtual U8 GetSortingPriority() const { return UMath::MaxU8; };

			U32 TypeID = 0;

			std::string Name = "";
			std::string Category = "";
			SColor Color = SColor(1.0f, 1.0f, 1.0f, 0.1f);
			
			SVector2<F32> Position = SVector2<F32>();
			bool HasBeenInitialized = false;
		};
	}
}
