// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "HexRune/Pin.h"
#include "HexRune/HexRune.h"

namespace Havtorn
{
	namespace HexRune
	{
		struct SScript;
		struct SScriptDataBinding;
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

		HexRune::SScript* Script = nullptr;
		std::vector<HexRune::SScriptDataBinding> DataBindings;
		U64 AssetRegistryKey = 0;
		U64 TriggeringSourceNode = 0;
		bool TriggerScript = false;
	};
}