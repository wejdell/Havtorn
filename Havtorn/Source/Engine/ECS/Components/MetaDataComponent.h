// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Core/HavtornString.h"

namespace Havtorn
{
	struct SMetaDataComponent : public SComponent
	{
		SMetaDataComponent()
			: SComponent(EComponentType::MetaDataComponent)
		{}

		CHavtornStaticString<255> Name;
	};
}
