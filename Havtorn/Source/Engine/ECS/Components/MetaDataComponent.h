// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include <HavtornString.h>

namespace Havtorn
{
	struct SMetaDataComponent : public SComponent
	{
		SMetaDataComponent() = default;
		SMetaDataComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}
		SMetaDataComponent(const SEntity& entityOwner, const std::string& nameInEditor)
			: SComponent(entityOwner)
			, Name(nameInEditor)
		{}

		// TODO.NR: Decrease this, surely the names don't require 255 chars
		CHavtornStaticString<255> Name;
	};
}
