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

		template<class TSystem>
		inline TSystem* GetSystem();

		template<class TSystem>
		inline bool HasSystem();

		template<class TSystem>
		inline bool TryGetSystem(TSystem* outSystem);

		template<class TSystem>
		inline void AddSystem();

		template<class TSystem, typename... Args>
		inline void AddSystem(Args&&... args);

		template<class TSystem>
		inline void QueueAddSystem();

		template<class TSystem, typename... Args>
		inline void QueueAddSystem(Args&&... args);

		template<class TSystem>
		inline void QueueRemoveSystem();

	private:
		CWorld() = default;
		~CWorld() = default;
		
		bool Init(CRenderManager* renderManager);
		void Update();
		void AddPendingSystems();
		void RemovePendingSystems();

		void RemoveSystem(U16 index);
		void RemoveSystemRespectOrder(U16 index);

		void LoadScene(const std::string& filePath);

	private:
		struct SystemTypeCode
		{
			const U64 HashCode = 0;

			SystemTypeCode(U64 hashCode) 
				: HashCode(hashCode)
			{}
		};

		std::vector<Ptr<CScene>> Scenes;
		std::vector<Ptr<ISystem>> Systems;

		std::vector<SystemTypeCode> SystemsToRemove;
		std::vector<Ptr<ISystem>> SystemsToAdd;

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

	template<class TSystem>
	inline bool CWorld::HasSystem()
	{
		return GetSystem<TSystem>() != nullptr;
	}

	template<class TSystem>
	inline bool CWorld::TryGetSystem(TSystem* outSystem)
	{
		outSystem = GetSystem<TSystem>();
		return outSystem != nullptr;
	}

	template<class TSystem>
	inline void CWorld::AddSystem()
	{
		Systems.push_back(std::make_unique<TSystem>());
	}

	template<class TSystem, typename... Args>
	inline void CWorld::AddSystem(Args&&... args)
	{
		Systems.push_back(std::make_unique<TSystem>(std::forward<Args>(args)...));
	}

	template<class TSystem>
	inline void CWorld::QueueAddSystem()
	{
		SystemsToAdd.push_back(std::make_unique<TSystem>());
	}

	template<class TSystem, typename... Args>
	inline void CWorld::QueueAddSystem(Args&&... args)
	{
		SystemsToAdd.push_back(std::make_unique<TSystem>(std::forward<Args>(args)...));
	}

	template<class TSystem>
	inline void CWorld::QueueRemoveSystem()
	{
		SystemsToRemove.push_back(SystemTypeCode(typeid(TSystem).hash_code()));
	}
}