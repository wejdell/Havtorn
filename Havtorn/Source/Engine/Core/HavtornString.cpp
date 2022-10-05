// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "hvpch.h"

#include "HavtornString.h"

namespace Havtorn
{
	bool CHavtornString::Contains(const char* str) const
	{
		// https://en.wikipedia.org/wiki/Boyer%E2%80%93Moore%E2%80%93Horspool_algorithm
		// Boyer–Moore–Horspool algorithm
		// O(n) best case
		// O(nm) worst case
		
		const U16 needleLength = static_cast<U16>(strlen(str));
		if (needleLength >= Size)
			return false;

		// 55 fps
		const std::array<U16, 256> charTable = CharacterTableForComparison(str);
		U16 skip = 0;
		while ((static_cast<U16>(strlen(Characters)) - skip) >= needleLength)
		{
			if (Same(&Characters[skip], str, needleLength))
			{
				return true;
			}
			skip += charTable[static_cast<int>(Characters[skip + needleLength - 1])];
		}
		return false;

		// Is probably better? 46fps
		//std::string string(Characters);
		//return string.find(str, strlen(str)) != std::string::npos;
	}

	std::array<U16, 256> CHavtornString::CharacterTableForComparison(const char* str) const
	{
		std::array<U16, 256> characterTable = { 0 };
		U16 strLength = static_cast<U16>(strlen(str));
		for (U16 i = 0; i < 256; i++)
		{
			characterTable[i] = strLength;
		}
		for (U16 i = 0; i < strLength - 1; i++)
		{
			characterTable[static_cast<int>(str[i])] = strLength - 1 - i;
		}
		return characterTable;
	}

	bool CHavtornString::Same(const char* str1, const char* str2, const U16 count) const
	{
		U16 i = count - 1;
		while (str1[i] == str2[i])
		{
			if (i == 0)
				return true;
			i -= 1;
		}
		return false;
	}
}
