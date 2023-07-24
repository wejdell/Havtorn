// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"

#include "Color.h"

namespace Havtorn
{
	const F32 SColor::F32Max = 1.0f;
	const F32 SColor::F32Min = 0.0f;
	const F32 SColor::U8MaxAsF32 = 255.0f;
	const F32 SColor::U8MaxAsF32Reciprocal = 1.0f / 255.0f;

	 const SColor SColor::Red = SColor(255, 0, 0);
	 const SColor SColor::Green = SColor(0, 255, 0);
	 const SColor SColor::Blue = SColor(0, 0, 255);
	 const SColor SColor::Black = SColor(0);
	 const SColor SColor::White = SColor(255);
	 const SColor SColor::Grey = SColor(127);
	 const SColor SColor::Teal = SColor(50, 255, 255);
	 const SColor SColor::Orange = SColor(255, 125, 15);
	 const SColor SColor::Magenta = SColor(200, 0, 225);
	 const SColor SColor::Yellow = SColor(255, 225, 25);
}