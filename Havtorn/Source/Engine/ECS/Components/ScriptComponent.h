// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SScript;
	}

	struct SScriptComponent : public SComponent
	{
		SScriptComponent() = default;
		SScriptComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{}

		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);
		[[nodiscard]] U32 GetSize() const;

		virtual bool IsValid() const override;

		HexRune::SScript* Script = nullptr;
		U64 AssetRegistryKey = 0;
	};
}