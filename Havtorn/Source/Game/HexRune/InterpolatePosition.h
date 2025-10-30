// Copyright 2025 Team Havtorn. All Rights Reserved.


#pragma once
#include <HexRune/HexRune.h>

namespace Havtorn
{
	namespace HexRune
	{
		struct SInterpolatePosition : public SNode
		{
			GAME_API SInterpolatePosition(const U64 id, const U32 typeID, SScript* owningScript);
			virtual GAME_API I8 OnExecute() override;
		};
	}
}
