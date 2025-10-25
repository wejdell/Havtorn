// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "SGhostyNodeEditorContext.h"
#include "GhostyNode.h"

namespace Havtorn
{
	namespace HexRune
	{
		SGhostyNodeEditorContext SGhostyNodeEditorContext::Context = {};
		SGhostyNodeEditorContext::SGhostyNodeEditorContext()
		{
			Name = "Ghosty";
			Category = "Game";
		}

		HexRune::SNode* SGhostyNodeEditorContext::AddNode(HexRune::SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SGhostyNode>(existingID, TypeID);
			script->AddEditorContext<SGhostyNodeEditorContext>(node->UID);
			return node;
		}

		SGhostyNodePositionEditorContex SGhostyNodePositionEditorContex::Context = {};
		SGhostyNodePositionEditorContex::SGhostyNodePositionEditorContex()
		{
			Name = "Ghosty Position";
			Category = "Game";
		}

		GAME_API HexRune::SNode* SGhostyNodePositionEditorContex::AddNode(HexRune::SScript* script, const U64 existingID) const
		{
			if (script == nullptr)
				return nullptr;

			SNode* node = script->AddNode<SGhostyPositionNode>(existingID, TypeID);
			script->AddEditorContext<SGhostyNodePositionEditorContex>(node->UID);
			return node;
		}
	}
}