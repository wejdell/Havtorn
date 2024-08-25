// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Entity.h"
#include "Scene.h"
#include "Core/EngineException.h"

namespace Havtorn
{
	struct SEntity;
	class ISystem;
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
		HAVTORN_API CAssetRegistry* GetAssetRegistry() const;
		HAVTORN_API void RegisterSystem(Ptr<ISystem> system);
		
		template<typename T>
		void OpenDemoScene(const bool shouldOpen3DDemo = true);

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

	template<typename T>
	inline void CWorld::OpenDemoScene(const bool shouldOpen3DDemo)
	{
		Scenes.clear();
		Scenes.emplace_back(std::make_unique<T>());

		if (shouldOpen3DDemo)
		{
			ENGINE_BOOL_POPUP(Scenes.back()->Init3DDemoScene(RenderManager), "Demo Scene could not be initialized.");
		}
		else
		{
			ENGINE_BOOL_POPUP(Scenes.back()->Init2DDemoScene(RenderManager), "Demo Scene could not be initialized.");
		}
	}

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