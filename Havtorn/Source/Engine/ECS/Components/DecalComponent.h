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

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		// TODO.NR: Figure out what to do about this. Unify with MaterialComp? have single material as property?
		std::vector<U16> TextureReferences = {0, 0, 0};
		bool ShouldRenderAlbedo = false;
		bool ShouldRenderMaterial = false;
		bool ShouldRenderNormal = false;
	};
}
