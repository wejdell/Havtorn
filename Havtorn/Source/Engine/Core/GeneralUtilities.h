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

		// Returns a string with the bytes converted to expected unit with symbol: B, KB or MB. I.e: 1024 will return "1 KB"
		static std::string BytesAsString(const size_t bytes)
		{
			std::string s;
			if (bytes >= (1024 * 1024))
			{
				double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
				s.append(std::to_string(mb));
				s.append(" MB");
			}
			else if (bytes >= 1024)
			{
				double kb = static_cast<double>(bytes) / 1024.0;
				s.append(std::to_string(kb));
				s.append(" kB");
			}
			else
			{
				s.append(std::to_string(bytes));
				s.append(" B");
			}
			return s;
		}
	}
}