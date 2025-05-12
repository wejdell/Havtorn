// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct STransformComponent : public SComponent
	{
		STransformComponent() = default;
		STransformComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		void Attach(STransformComponent* child)
		{
			child->Transform.SetParent(&Transform);
			child->ParentEntity = Owner;

			AttachedEntities.emplace_back(child->Owner);
			Transform.AddAttachment(&child->Transform);
		}
		void Detach(STransformComponent* child)
		{
			child->Transform.SetParent(nullptr);
			child->ParentEntity = SEntity::Null;

			if (auto it = std::find(AttachedEntities.begin(), AttachedEntities.end(), child->Owner); it != AttachedEntities.end())
				AttachedEntities.erase(it);
			Transform.RemoveAttachment(&child->Transform);
		}

		STransform Transform;
		
		SEntity ParentEntity = SEntity::Null;
		// TODO.NW: Serialize this
		std::vector<SEntity> AttachedEntities;

		// TODO.NW: Add Static/Dynamic modifiers
	};
}