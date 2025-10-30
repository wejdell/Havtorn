// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include <HexRune/NodeEditorContext.h>
namespace Havtorn
{
	namespace HexRune
	{
		struct SInterpolatePositionNodeEditorContext : public SNodeEditorContext
		{
			SInterpolatePositionNodeEditorContext();
			virtual SNode* AddNode(SScript* script, const U64 existingID = 0) const override;
			static SInterpolatePositionNodeEditorContext Context;
		};
	}
}
