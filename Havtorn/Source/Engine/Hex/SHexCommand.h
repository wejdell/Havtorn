// Copyright 2026 Team Havtorn. All Rights Reserved.
#pragma once
#include <GameplayTags\GameplayTag.h>


namespace Havtorn
{
#define HEXTYPES bool, Havtorn::F32, Havtorn::SVector2<F32>

	enum class EHexCommandDataType
	{
		Bool,
		Float,
		Vector2
	};

	struct SHexCommand
	{
		SGameplayTag Tag;
		EHexCommandDataType DataType;
		std::variant<HEXTYPES> Data;
	};
}
