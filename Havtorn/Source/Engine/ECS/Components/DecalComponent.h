// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SDecalComponent : public SComponent
	{
		SDecalComponent()
			: SComponent(EComponentType::DecalComponent)
		{}

		U32 Serialize(char* toData, U32& bufferPosition);
		U32 Deserialize(const char* fromData, U32& bufferPosition);
		[[nodiscard]] U32 GetSize() const;

		// TODO.NR: Figure out what to do about this. Unify with MaterialComp? have single material as property?
		std::vector<U16> TextureReferences = {0, 0, 0};
		bool ShouldRenderAlbedo = false;
		bool ShouldRenderMaterial = false;
		bool ShouldRenderNormal = false;
	};
}
