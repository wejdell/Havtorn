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
		SDecalComponent(const SEntity& entityOwner, const std::vector<std::string>& assetPaths)
			: SComponent(entityOwner)
			, AssetReferences(SAssetReference::MakeVectorFromPaths(assetPaths))
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		// TODO.NW: Figure out what to do about this. Unify with MaterialComp? have single material as property?
		std::vector<SAssetReference> AssetReferences = { SAssetReference(), SAssetReference(), SAssetReference() };
		bool ShouldRenderAlbedo = false;
		bool ShouldRenderMaterial = false;
		bool ShouldRenderNormal = false;
	};
}
