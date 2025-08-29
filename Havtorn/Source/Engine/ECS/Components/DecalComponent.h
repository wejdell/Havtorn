// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "FileSystem/FileHeaderDeclarations.h"

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

		// TODO.NW: Figure out what to do about this. Unify with MaterialComp? have single material as property?
		std::vector<SAssetReference> AssetReferences;
		bool ShouldRenderAlbedo = false;
		bool ShouldRenderMaterial = false;
		bool ShouldRenderNormal = false;
	};
}
