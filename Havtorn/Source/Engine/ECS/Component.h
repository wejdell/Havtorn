// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Entity.h"

namespace Havtorn
{
	class CScene;

	struct ENGINE_API SComponent
	{
		SComponent() = default;
		SComponent(const SEntity& entity);
		virtual ~SComponent() noexcept {};

		virtual void IsDeleted(CScene* fromScene);

		static bool IsValid(const SComponent* component);

		SEntity Owner = SEntity::Null;
	};
}
