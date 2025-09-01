// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "FileSystem/FileHeaderDeclarations.h"

namespace Havtorn
{
	struct SEnvironmentLightComponent : public SComponent
	{
		SEnvironmentLightComponent() = default;
		SEnvironmentLightComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}
		SEnvironmentLightComponent(const SEntity& entityOwner, const std::string& assetPath)
			: SComponent(entityOwner)
			, AssetReference(SAssetReference(assetPath))
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		bool IsActive = false;
		SAssetReference AssetReference;
	};
}
