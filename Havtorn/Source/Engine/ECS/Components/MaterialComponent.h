// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsMaterial.h"

namespace Havtorn
{
	struct SMaterialComponent : public SComponent
	{
		//SMaterialComponent(Ref<SEntity> entity, EComponentType type)
		//	: SComponent(std::move(entity), type)
		//{}

		U32 Serialize(char* toData, U32& bufferPosition);
		U32 Deserialize(const char* fromData, U32& bufferPosition);
		[[nodiscard]] U32 GetSize() const;

		std::vector<SEngineGraphicsMaterial> Materials;
	};
}