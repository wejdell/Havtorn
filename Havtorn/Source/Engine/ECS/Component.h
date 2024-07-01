// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Entity.h"

namespace Havtorn
{
	struct SComponent
	{
		SComponent() = default;

		explicit SComponent(EComponentType type)
			: Type(type)
		{}

		virtual ~SComponent() = default;

		EComponentType Type = EComponentType::Count;
		bool IsInUse = false;
	};
}
