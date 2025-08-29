// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "FileSystem/FileHeaderDeclarations.h"
#include "Core/RuntimeAssetDeclarations.h"
#include <map>

namespace Havtorn
{
	class CGraphicsFramework;
	class CRenderManager;

	class CAssetRegistry
	{
	public:
		CAssetRegistry();
		~CAssetRegistry();

		bool Init(CRenderManager* renderManager);

		template<typename T>
		T* RequestAssetData(const SAssetReference& assetRef, const U64 requesterID);

		template<typename T>
		std::vector<T*> RequestAssetData(const std::vector<SAssetReference>& assetRefs, const U64 requesterID);

		template<typename T>
		T* RequestAssetData(const U32 assetUID, const U64 requesterID);

		template<typename T>
		std::vector<T*> RequestAssetData(const std::vector<U32>& assetUIDs, const U64 requesterID);

		ENGINE_API SAsset* RequestAsset(const SAssetReference& assetRef, const U64 requesterID);
		ENGINE_API void UnrequestAsset(const SAssetReference& assetRef, const U64 requesterID);

		ENGINE_API SAsset* RequestAsset(const U32 assetUID, const U64 requesterID);
		ENGINE_API void UnrequestAsset(const U32 assetUID, const U64 requesterID);

		// TODO.NW: If we extend our own filePath struct, could be nice to separate full paths from folders
		ENGINE_API std::string ImportAsset(const std::string& filePath, const std::string& destinationPath, const SSourceAssetData& sourceData);
		ENGINE_API std::string SaveAsset(const std::string& destinationPath, const SAssetFileHeader& fileHeader);

	private:
		// TODO.NW: Catch asset location changes! Both source and asset itself, as part of file watching? 
		// At the very least we shouldn't crash if we try to load an asset with an invalid path

		// Load asset synchronously
		SAsset* LoadAsset(const SAssetReference& assetRef, const U64 requesterID);
		void UnloadAsset(const SAssetReference& assetRef);	

		CRenderManager* RenderManager = nullptr;
		std::map<U32, SAsset> Registry;
	};

	template<typename T>
	inline T* CAssetRegistry::RequestAssetData(const SAssetReference& assetRef, const U64 requesterID)
	{
		SAsset* asset = RequestAsset(assetRef, requesterID);
		if (!std::holds_alternative<T>(asset->Data))
		{
			HV_LOG_WARN("CAssetRegistry::RequestAssetData could not provide the requested asset data in %s", assetRef.FilePath.c_str());
			return nullptr;
		}

		return &std::get<T>(asset->Data);
	}

	template<typename T>
	inline std::vector<T*> CAssetRegistry::RequestAssetData(const std::vector<SAssetReference>& assetRefs, const U64 requesterID)
	{
		std::vector<T*> assets;
		
		for (const SAssetReference& ref : assetRefs)
			assets.emplace_back(RequestAssetData<T>(ref, requesterID));
		
		return assets;
	}

	template<typename T>
	inline T* CAssetRegistry::RequestAssetData(const U32 assetUID, const U64 requesterID)
	{
		if (!Registry.contains(assetUID))
		{
			HV_LOG_WARN("CAssetRegistry::RequestAssetData could not provide the requested asset data with ID: %i. No path was provided so it could not be loaded.", assetUID);
			return nullptr;
		}

		SAsset* asset = &Registry[assetUID];
		if (!std::holds_alternative<T>(asset->Data))
		{
			HV_LOG_WARN("CAssetRegistry::RequestAssetData could not provide the requested asset data with ID: %i. The data is not of the requested type.", assetUID);
			return nullptr;
		}
		
		asset->Requesters.insert(requesterID);
		
		return &std::get<T>(asset->Data);
	}

	template<typename T>
	inline std::vector<T*> CAssetRegistry::RequestAssetData(const std::vector<U32>& assetUIDs, const U64 requesterID)
	{
		std::vector<T*> assets;

		for (const U32 uid : assetUIDs)
			assets.emplace_back(RequestAssetData<T>(uid, requesterID));

		return assets;
	}
}
