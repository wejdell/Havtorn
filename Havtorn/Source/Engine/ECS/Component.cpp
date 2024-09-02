// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Component.h"

namespace Havtorn
{
	SComponent::SComponent(const SEntity& entity)
		: Owner(entity)
	{}

	bool SComponent::IsValid() const
	{
		return this != nullptr && Owner.IsValid();
	};
}
