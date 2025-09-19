// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsMaterial.h"
#include "Assets/FileHeaderDeclarations.h"

namespace Havtorn
{
	struct SMaterialComponent : public SComponent
	{
		SMaterialComponent() = default;
		SMaterialComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}
		SMaterialComponent(const SEntity& entityOwner, const std::vector<std::string>& assetPaths)
			: SComponent(entityOwner)
			, AssetReferences(SAssetReference::MakeVectorFromPaths(assetPaths))
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		std::vector<SAssetReference> AssetReferences;
	};
}
