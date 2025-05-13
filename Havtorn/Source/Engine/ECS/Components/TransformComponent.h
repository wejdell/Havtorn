// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	class CScene;

	struct STransformComponent : public SComponent
	{
		STransformComponent() = default;
		STransformComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		ENGINE_API void Attach(STransformComponent* child);
		ENGINE_API void Detach(STransformComponent* child);

		STransform Transform;
		
		SEntity ParentEntity = SEntity::Null;
		std::vector<SEntity> AttachedEntities;

		// TODO.NW: Add Static/Dynamic modifiers
	};
}