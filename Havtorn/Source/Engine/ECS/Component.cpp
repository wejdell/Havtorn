// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "Component.h"
#include "Scene/Scene.h"

namespace Havtorn
{
	SComponent::SComponent(const SEntity& entity)
		: Owner(entity)
	{}

	bool SComponent::IsValid(const SComponent* component)
	{
		return component != nullptr && component->Owner.IsValid();
	}

	void SComponent::IsDeleted(CScene* /*fromScene*/)
	{
	}
}
