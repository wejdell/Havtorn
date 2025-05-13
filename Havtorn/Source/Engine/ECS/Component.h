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

		virtual bool IsValid() const;
		virtual void IsDeleted(CScene* fromScene);

		SEntity Owner = SEntity::Null;
	};
}
