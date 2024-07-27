// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/ECSInclude.h"

namespace Havtorn
{
	struct UComponentAlgo
	{
		template<typename T>
		static SEntity GetClosestEntity3D(const SEntity& toEntity, const std::vector<T*>& fromComponents, const CScene* inScene);
	};

	template<typename T>
	inline SEntity UComponentAlgo::GetClosestEntity3D(const SEntity& toEntity, const std::vector<T*>& fromComponents, const CScene* inScene)
	{
		STransformComponent* transformComponent = inScene->GetComponent<STransformComponent>(toEntity);
		if (!transformComponent->IsValid())
			return SEntity::Null;

		F32 minDistanceSquared = UMath::MaxFloat;
		SEntity closestEntity = SEntity::Null;
		for (const T* component : fromComponents)
		{
			STransformComponent* otherTransformComponent = inScene->GetComponent<STransformComponent>(component);
			if (!otherTransformComponent->IsValid())
				continue;

			F32 distanceSquared = transformComponent->Transform.GetMatrix().GetTranslation().DistanceSquared(otherTransformComponent->Transform.GetMatrix().GetTranslation());
			if (distanceSquared < minDistanceSquared)
			{
				minDistanceSquared = distanceSquared;
				closestEntity = otherTransformComponent->EntityOwner;
			}
		}

		return closestEntity;
	}
}
