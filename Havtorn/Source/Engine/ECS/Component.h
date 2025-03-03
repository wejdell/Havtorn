// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Entity.h"

namespace Havtorn
{
	struct ENGINE_API SComponent
	{
		SComponent() = default;
		SComponent(const SEntity& entity);
		virtual ~SComponent() = default;

		bool IsValid() const;

		SEntity Owner = SEntity::Null;
	};
}
