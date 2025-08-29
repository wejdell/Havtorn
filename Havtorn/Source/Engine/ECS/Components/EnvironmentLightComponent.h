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

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		bool IsActive = false;
		SAssetReference AssetReference;
	};
}
