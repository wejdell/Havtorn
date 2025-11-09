// Copyright 2025 Team Havtorn. All Rights Reserved.


#include "SetPositionNode.h"

namespace Havtorn
{
	namespace HexRune
	{
		SSetPositionNode::SSetPositionNode(const U64 id, const U32 typeID, SScript* owningScript)
			: SNode(id, typeID, owningScript, ENodeType::Standard)
		{
			AddInput(UGUIDManager::Generate(), EPinType::Flow, "In");
			AddInput(UGUIDManager::Generate(), EPinType::Vector, "Pos");
			AddInput(UGUIDManager::Generate(), EPinType::Entity, "Entity");

			AddOutput(UGUIDManager::Generate(), EPinType::Flow, "Out");
		}

		I8 SSetPositionNode::OnExecute()
		{
			SVector pos;
			SEntity entity;
			GetDataOnPin(EPinDirection::Input, 1, pos);
			GetDataOnPin(EPinDirection::Input, 2, entity);

			STransformComponent* transform = OwningScript->Scene->GetComponent<STransformComponent>(entity);

			SMatrix translationMatrix = transform->Transform.GetMatrix();
			translationMatrix.SetTranslation(pos);
			transform->Transform.SetMatrix(translationMatrix);

			return 0;
		}
	}
}
