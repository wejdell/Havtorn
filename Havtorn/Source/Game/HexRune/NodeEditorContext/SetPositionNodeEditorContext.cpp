// Copyright 2025 Team Havtorn. All Rights Reserved.


#include "SetPositionNodeEditorContext.h"
#include "HexRune/SetPositionNode.h"
#include "TimerNodeEditorContext.h"

namespace Havtorn
{
	namespace HexRune
	{
		SSetPositionNodeEditorContext SSetPositionNodeEditorContext::Context = {};
		SSetPositionNodeEditorContext::SSetPositionNodeEditorContext()
		{
			Name = "Set Position";
			Category = "Game";
		}

		SNode* SSetPositionNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SSetPositionNode>(existingID, TypeID);
			script->AddEditorContext<STimerNodeEditorContext>(node->UID);
			return node;
		}

	}
}