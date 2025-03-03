// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <map>

namespace Havtorn
{
	class CAssetRegistry
	{
		struct SReferenceCounter
		{
			SReferenceCounter() = default;
			SReferenceCounter(U64 guid) 
				: GUID(guid)
				, NumberOfReferences(0)
			{}

			U64 GUID = 0;
			U64 NumberOfReferences = 0;
		};

	public:
		ENGINE_API U64 Register(const std::string& assetPath);
		ENGINE_API std::vector<U64> Register(const std::vector<std::string>& assetPaths);
		ENGINE_API void Unregister(const std::string& assetPath);

		// TODO.NR: This is slow, and should only be done during loading/unloading
		const std::string& GetAssetPath(const U64& guid);
		std::vector<std::string> GetAssetPaths(const std::vector<U64>& guids);

		[[nodiscard]] U32 GetSize() const;
		void Serialize(char* toData, U64& pointerPosition) const;
		void Deserialize(const char* fromData, U64& pointerPosition);

	private:
		std::map<std::string, SReferenceCounter> Registry;
		const std::string InvalidPath = "INVALID_ASSET_PATH";
	};
}
