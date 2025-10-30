// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "TimerNodeEditorContext.h"
#include "HexRune/TimerNode.h"

namespace Havtorn
{
	namespace HexRune
	{
		STimerNodeEditorContext STimerNodeEditorContext::Context = {};
		STimerNodeEditorContext::STimerNodeEditorContext()
		{
			Name = "Timer";
			Category = "Game";
		}

		SNode* STimerNodeEditorContext::AddNode(SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<STimerNode>(existingID, TypeID);
			script->AddEditorContext<STimerNodeEditorContext>(node->UID);
			return node;
		}

	}
}

