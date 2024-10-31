// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "Entity.h"

namespace Havtorn
{
	const SEntity SEntity::Null = { 0 };

	bool SEntity::IsValid() const
	{
		return GUID != SEntity::Null.GUID;
	}
}
