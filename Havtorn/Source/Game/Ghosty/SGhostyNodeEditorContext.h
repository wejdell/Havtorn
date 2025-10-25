// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include <HexRune/NodeEditorContext.h>

namespace Havtorn
{
	namespace HexRune
	{
		struct SGhostyNodeEditorContext : public HexRune::SNodeEditorContext
		{
			GAME_API SGhostyNodeEditorContext();
			GAME_API virtual HexRune::SNode* AddNode(HexRune::SScript* script, const U64 existingID = 0) const override;
			GAME_API static SGhostyNodeEditorContext Context;
		};

		struct SGhostyNodePositionEditorContex : public SNodeEditorContext
		{
			GAME_API SGhostyNodePositionEditorContex();
			GAME_API virtual HexRune::SNode* AddNode(HexRune::SScript* script, const U64 existingID = 0) const override;
			GAME_API static SGhostyNodePositionEditorContex Context;
		};
	}
}