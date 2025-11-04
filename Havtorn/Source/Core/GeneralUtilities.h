// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "CoreTypes.h"
#include "MathTypes/EngineMath.h"

#include <string>
#include <algorithm>
#include <concepts>

namespace Havtorn
{
	namespace UGeneralUtils
	{
		static std::string ExtractFileExtensionFromPath(const std::string& filePath)
		{
			U64 startIndex = filePath.find_last_of(".") + 1;
			std::string extension = filePath.substr(startIndex, filePath.length() - startIndex);
			std::for_each(extension.begin(), extension.end(), [](char& c) { c = static_cast<char>(std::tolower(c)); });
			return extension;
		}

		static std::string ExtractFileNameFromPath(const std::string& filePath)
		{
			U64 forwardIndex = filePath.find_last_of("/") + 1;
			U64 backwardIndex = filePath.find_last_of("\\") + 1;
			U64 startIndex = UMath::Max(forwardIndex, backwardIndex);
			return filePath.substr(startIndex, filePath.length() - startIndex);
		}

		static std::string ExtractFileBaseNameFromPath(const std::string& filePath)
		{
			U64 forwardIndex = filePath.find_last_of("/") + 1;
			U64 backwardIndex = filePath.find_last_of("\\") + 1;
			U64 startIndex = UMath::Max(forwardIndex, backwardIndex);
			return filePath.substr(startIndex, filePath.length() - startIndex - (ExtractFileExtensionFromPath(filePath).length() + 1));
		}

		static std::string ExtractParentDirectoryFromPath(const std::string& filePath)
		{
			std::string pathCopy = filePath;
			U64 forwardIndex = pathCopy.find_last_of("/") + 1;
			U64 backwardIndex = pathCopy.find_last_of("\\") + 1;
			if (forwardIndex == pathCopy.size() || backwardIndex == pathCopy.size())
			{
				pathCopy.pop_back();
				forwardIndex = pathCopy.find_last_of("/") + 1;
				backwardIndex = pathCopy.find_last_of("\\") + 1;
			}

			U64 endIndex = UMath::Max(forwardIndex, backwardIndex);
			return pathCopy.substr(0, endIndex);
		}

		static std::string ConvertToPlatformAgnosticPath(const std::string& filePath)
		{
			std::string newPath = filePath;
			std::replace(newPath.begin(), newPath.end(), '\\', '/');
			return newPath;
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

		template<typename T, std::invocable<const T&> P>
		static std::string GetNonCollidingString(const std::string& startingString, const std::vector<T>& collection, P&& projection)
		{
			std::string newString = startingString;
			bool foundCollision = true;
			I64 duplicates = 0;
			while (foundCollision)
			{
				foundCollision = false;
				for (const T& element : collection)
				{
					if (std::invoke(projection, element) == newString)
					{
						newString = startingString + std::to_string(++duplicates);
						foundCollision = true;
						break;
					}
				}
			}

			return newString;
		}
	}
}
