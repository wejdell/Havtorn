// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS\Entity.h"

namespace Havtorn
{
	struct SEntity;
	class ISystem;
	class CScene;
	class CRenderManager;
	class CAssetRegistry;
	class CSequencerSystem;

	class CWorld
	{
		friend class GEngine;

	public:
		HAVTORN_API std::vector<Ptr<CScene>>& GetActiveScenes();
		HAVTORN_API std::vector<SEntity>& GetEntities() const;
		HAVTORN_API void SaveActiveScene(const std::string& destinationPath);
		HAVTORN_API void AddScene(const std::string& filePath);
		HAVTORN_API void RemoveScene(U64 sceneIndex);
		HAVTORN_API void ChangeScene(const std::string& filePath);
		HAVTORN_API void OpenDemoScene(const bool shouldOpen3DDemo = true);
		HAVTORN_API CAssetRegistry* GetAssetRegistry() const;
		HAVTORN_API void RegisterSystem(Ptr<ISystem> system);

		template<class TSystem>
		inline TSystem* GetSystem();

		CSequencerSystem* GetSequencerSystem();
	private:
		CWorld() = default;
		~CWorld() = default;
		
		bool Init(CRenderManager* renderManager);
		void Update();

		void LoadScene(const std::string& filePath);

	private:
		std::vector<Ptr<CScene>> Scenes;
		std::vector<Ptr<ISystem>> Systems;
		Ptr<CAssetRegistry> AssetRegistry = nullptr;
		CRenderManager* RenderManager = nullptr;
	};

	template<class TSystem>
	inline TSystem* CWorld::GetSystem()
	{
		U64 targetHashCode = typeid(TSystem).hash_code();
		for (U32 i = 0; i < Systems.size(); i++)
		{
			U64 hashCode = typeid(*Systems[i].get()).hash_code();
			if (hashCode == targetHashCode)
				return static_cast<TSystem*>(Systems[i].get());
		}
		return nullptr;
	}
}