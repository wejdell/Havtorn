// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "Assets/FileHeaderDeclarations.h"
#include "Assets/RuntimeAssetDeclarations.h"

#include <map>
#include <shared_mutex>

namespace Havtorn
{
	class CGraphicsFramework;
	class CRenderManager;

	class CAssetRegistry
	{
	public:
		static constexpr U64 AssetRegistryRequestID = 100;
		static constexpr U64 RenderManagerRequestID = 200;
		static constexpr U64 EditorManagerRequestID = 300;

		CMulticastDelegate<const std::string&> OnAssetReloaded;

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

		ENGINE_API std::vector<SAsset*> RequestAssets(const std::vector<SAssetReference>& assetRefs, const U64 requesterID);
		ENGINE_API void UnrequestAssets(const std::vector<SAssetReference>& assetRefs, const U64 requesterID);

		ENGINE_API std::string GetAssetDatabaseEntry(const U32 uid);

		// TODO.NW: If we extend our own filePath struct, could be nice to separate full paths from folders
		ENGINE_API std::string ImportAsset(const std::string& filePath, const std::string& destinationPath, const SSourceAssetData& sourceData);
		ENGINE_API std::string SaveAsset(const std::string& destinationPath, const SAssetFileHeader& fileHeader);

		ENGINE_API void RefreshDatabase();
		ENGINE_API void FixUpAssetRedirectors();

		ENGINE_API std::set<U64> GetReferencers(const SAssetReference& assetRef);

		ENGINE_API void StartSourceFileWatch(const SAssetReference& assetRef);
		ENGINE_API void StopSourceFileWatch(const SAssetReference& assetRef);

		ENGINE_API std::string GetDebugString(const bool shouldExpand);

	private:
		// TODO.NW: Catch asset location changes! Both source and asset itself, as part of file watching? 
		// At the very least we shouldn't crash if we try to load an asset with an invalid path

		void RequestDependencies(const U32 assetUID, const U64 requesterID);
		void UnrequestDependencies(const U32 assetUID, const U64 requesterID);

		// Load asset synchronously
		bool LoadAsset(const SAssetReference& assetRef);
		bool UnloadAsset(const SAssetReference& assetRef);

		void OnSourceFileChanged(const std::string& sourceFilePath);

		inline SAsset* GetAsset(const U32 assetUID);
		inline void AddAsset(const U32 assetUID, SAsset& asset);
		inline void RemoveAsset(const U32 assetUID);

		CRenderManager* RenderManager = nullptr;
		std::map<U32, std::string> AssetDatabase;
		std::map<U32, SAsset> LoadedAssets;

		std::map<std::string, SAsset*> WatchedAssets;
		std::shared_mutex RegistryMutex;
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
		std::shared_lock lock{ RegistryMutex };

		if (!LoadedAssets.contains(assetUID))
		{
			if (AssetDatabase.contains(assetUID))
			{
				RequestAsset(SAssetReference(AssetDatabase[assetUID]), requesterID);
			}
			else
			{
				HV_LOG_WARN("CAssetRegistry::RequestAssetData could not provide the requested asset data with ID: %u. It is not loaded and is not found in the database.", assetUID);
				return nullptr;
			}
		}

		SAsset* asset = GetAsset(assetUID);
		if (!std::holds_alternative<T>(asset->Data))
		{
			HV_LOG_WARN("CAssetRegistry::RequestAssetData could not provide the requested asset data with ID: %u. The data is not of the requested type.", assetUID);
			return nullptr;
		}
		
		asset->Requesters.insert(requesterID);
		RequestDependencies(assetUID, requesterID);
		
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
