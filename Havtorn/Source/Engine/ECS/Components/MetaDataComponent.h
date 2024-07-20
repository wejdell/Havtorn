// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Core/HavtornString.h"

namespace Havtorn
{
	struct SMetaDataComponent : public SComponent
	{
		SMetaDataComponent() = default;
		SMetaDataComponent(const SEntity& entityOwner, const std::string& nameInEditor)
			: SComponent(entityOwner)
			, Name(nameInEditor)
		{}

		CHavtornStaticString<255> Name;
	};
}
