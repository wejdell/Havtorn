// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "GameScript.h"
#include <Scene/World.h>
namespace Havtorn
{
	[[nodiscard]] U32 SGameScript::GetSize() const
	{
		U32 size = HexRune::SScript::GetSize();
		return size;
	}

	void SGameScript::Serialize(char* toData, U64& pointerPosition) const
	{
		HexRune::SScript::Serialize(toData, pointerPosition);;
	}

	void SGameScript::Deserialize(const char* fromData, U64& pointerPosition)
	{
		HexRune::SScript::Deserialize(fromData, pointerPosition);
	}
}
