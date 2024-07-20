// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "Entity.h"

namespace Havtorn
{
	struct HAVTORN_API SComponent
	{
		SComponent() = default;
		SComponent(const SEntity& entity);
		virtual ~SComponent() = default;

		bool IsValid() const;

		SEntity EntityOwner = SEntity::Null;

		//SComponent() = default;

		//explicit SComponent(EComponentType type)
		//	: Type(type)
		//{}

		//virtual ~SComponent() = default;

		//EComponentType Type = EComponentType::Count;
		//bool IsInUse = false;
	};
}
