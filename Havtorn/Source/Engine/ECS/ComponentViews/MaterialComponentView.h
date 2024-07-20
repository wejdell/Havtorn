// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsMaterial.h"

namespace Havtorn
{
	struct SMaterialComponent : public SComponent
	{
		SMaterialComponent() = default;
		SMaterialComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		std::vector<SEngineGraphicsMaterial> Materials;
		std::vector<U64> AssetRegistryKeys = {};
	};
}