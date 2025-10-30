// Copyright 2025 Team Havtorn. All Rights Reserved.


#include "TimerNode.h"

namespace Havtorn
{
	namespace HexRune
	{
		STimerNode::STimerNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow, "Tick");
			AddInput(UGUIDManager::Generate(), EPinType::Float, "InTime");
			AddInput(UGUIDManager::Generate(), EPinType::Float, "Duration");

			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "OnTick");
			AddOutput(UGUIDManager::Generate(), EPinType::Float, "OutTime");
			AddOutput(UGUIDManager::Generate(), EPinType::Float, "T");
		}

		I8 STimerNode::OnExecute()
		{
			F32 duration{};
			GetDataOnPin(EPinDirection::Input, 2, duration);

			if (duration == 0.0f)
				return -1;

			F32 time{};
			GetDataOnPin(EPinDirection::Input, 1, time);
	
			F32 newTime{};
			newTime = time + GTime::Dt();
			SetDataOnPin(EPinDirection::Output, 1, newTime);

			F32 normalizedTime{};
			normalizedTime = newTime / duration;
			SetDataOnPin(EPinDirection::Output, 2, normalizedTime);

			if (newTime < duration)
				return 0;

			return -1;
		}
	}
}
