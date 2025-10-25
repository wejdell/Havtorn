#include "GhostyNode.h"
// Copyright 2025 Team Havtorn. All Rights Reserved.
namespace Havtorn
{
	namespace HexRune
	{
		SGhostyNode::SGhostyNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow, "In");
			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Out");
		}
		
		I8 SGhostyNode::OnExecute()
		{
			return 0;
		}

		SGhostyPositionNode::SGhostyPositionNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddOutput(UGUIDManager::Generate(), EPinType::Vector, "Position");
		}

		GAME_API I8 SGhostyPositionNode::OnExecute()
		{
			SetDataOnPin(EPinDirection::Output, 0, SVector(24, 25, 26));
			return 0;
		}

		/*
		SetDataOnPin(EPinDirection::Output, 1, component);
		SetDataOnPin(EPinDirection::Output, 2, i);
		*/
	}
}


