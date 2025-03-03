// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>

namespace Havtorn
{
	struct ENGINE_API SEntity
	{
		const static SEntity Null;

		U64 GUID = SEntity::Null.GUID;

		bool IsValid() const;
		auto operator<=>(const SEntity& other) const = default;
	};
}
