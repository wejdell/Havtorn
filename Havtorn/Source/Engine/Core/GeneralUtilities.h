// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <string>
#include "Core/MathTypes/HavtornMath.h"

namespace Havtorn
{
	namespace UGeneralUtils
	{
		static std::string ExtractFileNameFromPath(const std::string& filePath)
		{
			U64 forwardIndex = filePath.find_last_of("/") + 1;
			U64 backwardIndex = filePath.find_last_of("\\") + 1;
			U64 startIndex = UMath::Max(forwardIndex, backwardIndex);
			return filePath.substr(startIndex, filePath.length() - startIndex - 4);
		}
	}
}