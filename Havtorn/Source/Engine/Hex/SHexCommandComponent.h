// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/Component.h"
#include "SHexCommand.h"
#include <stack>

namespace Havtorn
{
	struct SHexCommandComponent : public SComponent
	{
		SHexCommandComponent() = default;
		SHexCommandComponent(const SEntity& entityOwner)
			: SComponent(entityOwner)
		{
		}

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);

		SGameplayTagContainer TagsToListenFor;
		
		// Runtime data
		std::stack<SHexCommand> HexCommands{};
	};
}
