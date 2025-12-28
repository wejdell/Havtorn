// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/ECSInclude.h"

namespace Havtorn
{
	struct SCameraData
	{
		bool IsValid() const
		{
			return CameraComponent != nullptr && TransformComponent != nullptr;
		}
		SCameraComponent* CameraComponent = nullptr;
		STransformComponent* TransformComponent = nullptr;
	};

	struct UComponentAlgo
	{
		template<typename T>
		static SEntity GetClosestEntity3D(const SEntity& toEntity, const std::vector<T*>& fromComponents, const CScene* inScene);

		static SCameraData GetCameraData(const SEntity& cameraEntity, const std::vector<Ptr<CScene>>& scenes);

		static CScene* GetContainingScene(const SEntity& entity, const std::vector<Ptr<CScene>>& scenes);
	};

	template<typename T>
	inline SEntity UComponentAlgo::GetClosestEntity3D(const SEntity& toEntity, const std::vector<T*>& fromComponents, const CScene* inScene)
	{
		const STransformComponent* transformComponent = inScene->GetComponent<STransformComponent>(toEntity);
		if (!SComponent::IsValid(transformComponent))
			return SEntity::Null;

		F32 minDistanceSquared = UMath::MaxFloat;
		SEntity closestEntity = SEntity::Null;
		for (const T* component : fromComponents)
		{
			const STransformComponent* otherTransformComponent = inScene->GetComponent<STransformComponent>(component);
			if (!SComponent::IsValid(otherTransformComponent))
				continue;

			const F32 distanceSquared = transformComponent->Transform.GetMatrix().GetTranslation().DistanceSquared(otherTransformComponent->Transform.GetMatrix().GetTranslation());
			if (distanceSquared < minDistanceSquared)
			{
				minDistanceSquared = distanceSquared;
				closestEntity = otherTransformComponent->Owner;
			}
		}

		return closestEntity;
	}

	inline SCameraData UComponentAlgo::GetCameraData(const SEntity& cameraEntity, const std::vector<Ptr<CScene>>& scenes)
	{
		SCameraData data;

		for (const Ptr<CScene>& scene : scenes)
		{
			SCameraComponent* cameraComponent = scene->GetComponent<SCameraComponent>(cameraEntity);
			STransformComponent* cameraTransform = scene->GetComponent<STransformComponent>(cameraEntity);
			
			if (cameraComponent != nullptr && cameraTransform != nullptr)
			{
				data.CameraComponent = cameraComponent;
				data.TransformComponent = cameraTransform;
				return data;
			}
		}

		return data;
	}

	inline CScene* UComponentAlgo::GetContainingScene(const SEntity& entity, const std::vector<Ptr<CScene>>& scenes)
	{
		for (const Ptr<CScene>& scene : scenes)
		{
			if (scene->HasEntity(entity.GUID))
				return scene.get();
		}

		return nullptr;
	}
}
