// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "FileSystem/FileHeaderDeclarations.h"
#include <Color.h>

namespace Havtorn
{
	struct SSpriteComponent : public SComponent
	{
		SSpriteComponent() = default;
		SSpriteComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}
		SSpriteComponent(const SEntity& entityOwner, const std::string& assetPath)
			: SComponent(entityOwner)
			, AssetReference(SAssetReference(assetPath))
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		SColor Color = SColor::White;
		SVector4 UVRect = { 0.f, 0.f, 1.f, 1.f };
		SAssetReference AssetReference;
	};
}
