// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <map>
// TODO.NW: Unify these asset files under the same directory
#include "FileSystem/FileHeaderDeclarations.h"
#include "Assets/Asset.h"

namespace Havtorn
{
	class CGraphicsFramework;
	class CRenderManager;

	class CAssetRegistry
	{
		struct SAssetReference
		{
			// TODO.NW: Essentially a hashed string. Can we use that instead?
			U64 UID = 0;
			std::string FilePath = "";

			bool operator==(const SAssetReference& other)
			{
				return UID == other.UID && FilePath == other.FilePath;
			}
		};

	public:
		CAssetRegistry();
		~CAssetRegistry();

		bool Init(CGraphicsFramework* framework, CRenderManager* renderManager);

		//[[nodiscard]] U32 GetSize() const;
		//void Serialize(char* toData, U64& pointerPosition) const;
		//void Deserialize(const char* fromData, U64& pointerPosition);

		template<typename T>
		T& RequestAssetData(const SAssetReference& reference, const U64 requesterID);

		SAsset* RequestAsset(const SAssetReference& reference, const U64 requesterID);
		void UnrequestAsset(const SAssetReference& reference, const U64 requesterID);

		// TODO.NW: If we extend our own filePath struct, could be nice to separate full paths from folders
		std::string ImportAsset(const std::string& filePath, const std::string& destinationPath, const SSourceAssetData& sourceData);
		std::string SaveAsset(const std::string& destinationPath, const SAssetFileHeader& fileHeader);
	
	private:
		// Load asset synchronously
		SAsset* LoadAsset(const SAssetReference& reference, const U64 requesterID);
		void UnloadAsset(const SAssetReference& reference);	

		CRenderManager* RenderManager = nullptr;
		std::map<SAssetReference, SAsset> Registry;
	};

	template<typename T>
	inline T& CAssetRegistry::RequestAssetData(const SAssetReference& reference, const U64 requesterID)
	{
		SAsset* asset = RequestAsset(reference, requesterID);
		if (!std::holds_alternative<T>(asset->Data))
		{
			HV_LOG_WARN("CAssetRegistry::RequestAssetData could not provide the requested asset data in %s", reference.FilePath.c_str());
			return T();
		}

		return std::get<T>(asset->Data);
	}
}
