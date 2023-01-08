// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SMetaDataComponent : public SComponent
	{
		SMetaDataComponent()
			: SComponent(EComponentType::MetaDataComponent)
		{}

		// TODO: Define
		U32 Serialize(char* toData, U32& bufferPosition);
		U32 Deserialize(const char* fromData, U32& bufferPosition);
		[[nodiscard]] U32 GetSize() const;

		std::string Name = "";
	};
}
