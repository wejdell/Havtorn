// Copyright 2025 Team Havtorn. All Rights Reserved.

#include <HexRune/HexRune.h>
//#include <HexRune/Pin.h>

namespace Havtorn
{
	namespace HexRune
	{
		struct SGhostyNode : public SNode
		{
			GAME_API SGhostyNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual GAME_API I8 OnExecute() override;
		};
		
		struct SGhostyPositionNode : public SNode
		{
			GAME_API SGhostyPositionNode(const U64 id, const U32 typeID, SScript* owningScript);
			virtual GAME_API I8 OnExecute() override;
		};
		
	}
}