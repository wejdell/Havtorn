// Copyright 2025 Team Havtorn. All Rights Reserved.


#include "InterpolatePosition.h"

namespace Havtorn
{
	namespace HexRune
	{
		SInterpolatePosition::SInterpolatePosition(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Vector, "A");
			AddInput(UGUIDManager::Generate(), EPinType::Vector, "B");
			AddInput(UGUIDManager::Generate(), EPinType::Float, "T");
			AddOutput(UGUIDManager::Generate(), EPinType::Vector, "Out");
		}

		I8 SInterpolatePosition::OnExecute()
		{
			SVector a{};
			GetDataOnPin(EPinDirection::Input, 1, a);
			SVector b{};
			GetDataOnPin(EPinDirection::Input, 2, b);
			F32 t{};
			GetDataOnPin(EPinDirection::Input, 3, t);

			SVector out = SVector::Lerp(a, b, t);
			SetDataOnPin(EPinDirection::Output, 0, out);
			return -1;
		}
	}
}
