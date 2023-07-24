// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <map>

namespace Havtorn
{
	struct SAssetReferenceCounter
	{
		EComponentType ComponentType = EComponentType::Count;
		U16 ComponentIndex = 0;
		U8 ComponentSubIndex = 0;
		U8 SceneIndex = 0;

		bool HasSameComponent(const SAssetReferenceCounter& other) const;
		bool operator==(const SAssetReferenceCounter& other) const;
	};

	class CAssetRegistry
	{
	public:
		void Register(const std::string& assetPath, SAssetReferenceCounter&& counter);
		void Register(const std::vector<std::string>& assetPaths, const SAssetReferenceCounter& counter);
		void Unregister(const std::string& assetPath, const SAssetReferenceCounter& counter);

		// TODO.NR: This is slow, and should only be done during loading/unloading
		const std::string& GetAssetPath(const SAssetReferenceCounter& counter);
		std::vector<std::string> GetAssetPaths(const SAssetReferenceCounter& counter);

		[[nodiscard]] U32 GetSize(I64 sceneIndex) const;
		void Serialize(I64 sceneIndex, char* toData, U64& pointerPosition) const;
		void Deserialize(I64 sceneIndex, const char* fromData, U64& pointerPosition);

	private:
		std::map<std::string, std::vector<SAssetReferenceCounter>> Registry;
		const std::string InvalidPath = "INVALID_ASSET_PATH";
	};
}
