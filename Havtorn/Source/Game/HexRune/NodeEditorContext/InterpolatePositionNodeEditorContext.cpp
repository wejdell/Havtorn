// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "InterpolatePositionNodeEditorContext.h"
#include "HexRune/InterpolatePosition.h"

namespace Havtorn
{
	namespace HexRune
	{
		SInterpolatePositionNodeEditorContext SInterpolatePositionNodeEditorContext::Context = {};
		SInterpolatePositionNodeEditorContext::SInterpolatePositionNodeEditorContext()
		{
			Name = "Interpolate Position";
			Category = "Game";
		}

		SNode* SInterpolatePositionNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SInterpolatePosition>(existingID, TypeID);
			script->AddEditorContext<SInterpolatePositionNodeEditorContext>(node->UID);
			return node;
		}
	}
}


