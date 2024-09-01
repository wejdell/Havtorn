// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	struct SDecalComponent : public SComponent
	{
		SDecalComponent() = default;
		SDecalComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		std::vector<U64> AssetRegistryKeys = {};
		// TODO.NR: Figure out what to do about this. Unify with MaterialComp? have single material as property?
		// TODO.NR: Change to U32 so that it matches with TextureBank
		std::vector<U16> TextureReferences = {0, 0, 0};
		bool ShouldRenderAlbedo = false;
		bool ShouldRenderMaterial = false;
		bool ShouldRenderNormal = false;
	};
}
