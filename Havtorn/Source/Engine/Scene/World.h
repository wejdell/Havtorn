// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Entity.h"
#include "FileSystem/FileHeaderDeclarations.h"
#include "Core/EngineException.h"
#include "HexPhys/HexPhys.h"

namespace Havtorn
{
	struct SEntity;
	struct SPhysics2DComponent;
	struct SPhysics3DComponent;
	struct STransformComponent;
	class ISystem;
	class CRenderManager;
	class CAssetRegistry;
	class CSequencerSystem;

	namespace HexPhys2D
	{
		class CPhysicsWorld2D;
	}

	namespace HexPhys3D
	{
		class CPhysicsWorld3D;
	}

	class CWorld
	{
		friend class GEngine;

	public:
		HAVTORN_API std::vector<Ptr<CScene>>& GetActiveScenes();
		HAVTORN_API std::vector<SEntity>& GetEntities() const;
		HAVTORN_API void SaveActiveScene(const std::string& destinationPath);
		HAVTORN_API void RemoveScene(U64 sceneIndex);
		HAVTORN_API CAssetRegistry* GetAssetRegistry() const;
		
		template<typename T>
		void AddScene(const std::string& filePath);

		template<typename T>
		void ChangeScene(const std::string& filePath);

		template<typename T>
		void OpenDemoScene(const bool shouldOpen3DDemo = true);

		HAVTORN_API void RegisterSystem(Ptr<ISystem> system);

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

		void HAVTORN_API Initialize2DPhysicsData(const SEntity& entity) const;
		void HAVTORN_API Update2DPhysicsData(STransformComponent* transformComponent, SPhysics2DComponent* phys2DComponent) const;

		void HAVTORN_API Initialize3DPhysicsData(const SEntity& entity) const;
		void HAVTORN_API Update3DPhysicsData(STransformComponent* transformComponent, SPhysics3DComponent* phys2DComponent) const;

	private:
		CWorld() = default;
		~CWorld() = default;
		
		bool Init(CRenderManager* renderManager);
		void Update();
		void AddPendingSystems();
		void RemovePendingSystems();

		void RemoveSystem(U16 index);
		void RemoveSystemRespectOrder(U16 index);

		HAVTORN_API void LoadScene(const std::string& filePath, CScene* outScene);

		void OnSceneCreated(CScene* scene) const;

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
		Ptr<HexPhys2D::CPhysicsWorld2D> PhysicsWorld2D = nullptr;
		Ptr<HexPhys3D::CPhysicsWorld3D> PhysicsWorld3D = nullptr;
		CRenderManager* RenderManager = nullptr;

		CMulticastDelegate<CScene*> OnSceneCreatedDelegate;
	};

	template<typename T>
	inline void CWorld::AddScene(const std::string& filePath)
	{
		Scenes.emplace_back(std::make_unique<T>());
		OnSceneCreatedDelegate.Broadcast(Scenes.back().get());
		LoadScene(filePath, Scenes.back().get());
	}

	template<typename T>
	inline void CWorld::ChangeScene(const std::string& filePath)
	{
		Scenes.clear();
		AddScene<T>(filePath);
	}

	template<typename T>
	inline void CWorld::OpenDemoScene(const bool shouldOpen3DDemo)
	{
		Scenes.clear();
		Scenes.emplace_back(std::make_unique<T>());
		OnSceneCreatedDelegate.Broadcast(Scenes.back().get());

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
