// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "Graphics/GraphicsStructs.h"
#include "Assets/FileHeaderDeclarations.h"

namespace Havtorn
{
	struct SStaticMeshComponent : public SComponent
	{
		SStaticMeshComponent() = default;
		SStaticMeshComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}
		SStaticMeshComponent(const SEntity& entityOwner, const std::string& assetPath)
			: SComponent(entityOwner)
			, AssetReference(SAssetReference(assetPath))
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		ENGINE_API void IsDeleted(CScene* fromScene) override;

		SAssetReference AssetReference;
	};
}
