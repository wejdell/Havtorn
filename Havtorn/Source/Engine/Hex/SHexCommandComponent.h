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

		std::stack<SHexCommand> HexCommands{};
		SGameplayTagContainer TagsToListenFor;

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);

	private:
		void DeserializeDataVariant(std::variant<HEXTYPES>& data, const EHexCommandDataType dataType, const char* fromData, Havtorn::U64& pointerPosition);

		template<typename T>
		void DeserializeVariant(std::variant<HEXTYPES>& data, const char* fromData, U64& pointerPosition)
		{
			T value;
			DeserializeData(value, fromData, pointerPosition);
			data = value;
		}
	};
}
