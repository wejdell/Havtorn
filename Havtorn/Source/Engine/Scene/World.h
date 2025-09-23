// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Entity.h"
#include "Assets/FileHeaderDeclarations.h"
#include "HexPhys/HexPhys.h"
#include <EngineException.h>
#include <HavtornDelegate.h>
#include <FileSystem.h>

namespace Havtorn
{
	struct SEntity;
	struct SPhysics2DComponent;
	struct SPhysics3DComponent;
	struct STransformComponent;
	class ISystem;
	class CRenderManager;
	class CGraphicsFramework;
	class CAssetRegistry;
	class CSequencerSystem;
	class CScene;

	namespace HexPhys2D
	{
		class CPhysicsWorld2D;
	}

	namespace HexPhys3D
	{
		class CPhysicsWorld3D;
	}

	struct SSystemData
	{
		Ptr<ISystem> System = nullptr;
		std::vector<U64> Requesters;
		std::vector<U64> Blockers;

		std::partial_ordering operator<=>(const SSystemData&) const = default;
	};

	enum class ENGINE_API EWorldPlayState
	{
		Stopped,
		Paused,
		Playing
	};

	enum class ENGINE_API EWorldPlayDimensions
	{
		World2D,
		World3D
	};

	class CWorld
	{
		friend class GEngine;

	public:
		ENGINE_API bool BeginPlay();
		ENGINE_API bool PausePlay();
		ENGINE_API bool StopPlay();

		ENGINE_API EWorldPlayState GetWorldPlayState() const;
		ENGINE_API EWorldPlayDimensions GetWorldPlayDimensions() const;
		ENGINE_API void ToggleWorldPlayDimensions();

		ENGINE_API std::vector<Ptr<CScene>>& GetActiveScenes();
		ENGINE_API std::vector<SEntity>& GetEntities() const;
		ENGINE_API void SaveActiveScene(const std::string& destinationPath) const;
		ENGINE_API void RemoveScene(U64 sceneIndex);
		ENGINE_API void ClearScenes();
		
		template<typename T>
		void CreateScene();

		template<typename T>
		void AddScene(const std::string& filePath);

		template<typename T>
		void ChangeScene(const std::string& filePath);

		template<typename T>
		void OpenDemoScene(const bool shouldOpen3DDemo = true);

		template<class T>
		inline T* GetSystem();

		template<class T>
		inline SSystemData* GetSystemHolder();

		template<class T>
		inline bool HasSystem();

		template<class T, typename... Args>
		inline void RequestSystem(void* requester, Args&&... args);

		template<class T>
		inline void UnrequestSystem(void* requester);

		ENGINE_API inline void UnrequestSystems(void* requester);

		template<class T>
		inline void BlockSystem(void* requester);

		template<class T>
		inline void UnblockSystem(void* requester);

		ENGINE_API void RequestPhysicsSystem(void* requester);
		ENGINE_API void BlockPhysicsSystem(void* requester);
		ENGINE_API void UnblockPhysicsSystem(void* requester);

		ENGINE_API void Initialize2DPhysicsData(const SEntity& entity) const;
		ENGINE_API void Update2DPhysicsData(STransformComponent* transformComponent, SPhysics2DComponent* phys2DComponent) const;

		ENGINE_API void Initialize3DPhysicsData(const SEntity& entity) const;
		ENGINE_API void Update3DPhysicsData(STransformComponent* transformComponent, SPhysics3DComponent* phys2DComponent) const;

	public:
		// TODO.NW: Maybe unify and have Enum arg instead
		CMulticastDelegate<CScene*> OnBeginPlayDelegate;
		CMulticastDelegate<CScene*> OnPausePlayDelegate;
		CMulticastDelegate<CScene*> OnEndPlayDelegate;

		CMulticastDelegate<CScene*, const SEntity, const SEntity> OnBeginOverlap;
		CMulticastDelegate<CScene*, const SEntity, const SEntity> OnEndOverlap;

	private:
		CWorld() = default;
		~CWorld() = default;
		
		bool Init(CRenderManager* renderManager);
		void Update() const;

		ENGINE_API void LoadScene(const std::string& filePath, CScene* outScene) const;

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
		std::vector<SSystemData> SystemData;

		std::vector<SystemTypeCode> SystemsToRemove;
		std::vector<Ptr<ISystem>> SystemsToAdd;

		Ptr<HexPhys2D::CPhysicsWorld2D> PhysicsWorld2D = nullptr;
		Ptr<HexPhys3D::CPhysicsWorld3D> PhysicsWorld3D = nullptr;
		
		CRenderManager* RenderManager = nullptr;

		CMulticastDelegate<CScene*> OnSceneCreatedDelegate;

		EWorldPlayState PlayState = EWorldPlayState::Stopped;
		EWorldPlayDimensions PlayDimensions = EWorldPlayDimensions::World3D;
	};

	template<typename T>
	inline void CWorld::CreateScene()
	{
		Scenes.emplace_back(std::make_unique<T>());
		OnSceneCreatedDelegate.Broadcast(Scenes.back().get());
	}

	template<typename T>
	inline void CWorld::AddScene(const std::string& filePath)
	{
		CreateScene<T>();
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
			ENGINE_BOOL_POPUP(Scenes.back()->Init3DDemoScene(), "Demo Scene could not be initialized.");
		}
		else
		{
			ENGINE_BOOL_POPUP(Scenes.back()->Init2DDemoScene(), "Demo Scene could not be initialized.");
		}
	}

	template<class T>
	inline T* CWorld::GetSystem()
	{
		U64 targetHashCode = typeid(T).hash_code();
		for (const SSystemData& holder : SystemData)
		{
			U64 hashCode = typeid(*holder.System.get()).hash_code();
			if (hashCode == targetHashCode)
				return static_cast<T*>(holder.System.get());
		}
		return nullptr;
	}

	template <class T>
	SSystemData* CWorld::GetSystemHolder()
	{
		U64 targetHashCode = typeid(T).hash_code();
		for (SSystemData& holder : SystemData)
		{
			U64 hashCode = typeid(*holder.System.get()).hash_code();
			if (hashCode == targetHashCode)
				return &holder;
		}
		return nullptr;
	}

	template<class T>
	inline bool CWorld::HasSystem()
	{
		return GetSystem<T>() != nullptr;
	}

	template <class T, typename... Args>
	void CWorld::RequestSystem(void* requester, Args&&... args)
	{
		SSystemData* holder = GetSystemHolder<T>();
		if (holder == nullptr)
		{
			SystemData.push_back({std::make_unique<T>(std::forward<Args>(args)...), {}, {}});
			holder = &SystemData.back();
		}

		holder->Requesters.push_back(reinterpret_cast<U64>(requester));
	}

	template <class T>
	void CWorld::UnrequestSystem(void* requester)
	{
		SSystemData* holder = GetSystemHolder<T>();
		if (holder == nullptr)
			return;

		std::erase(holder->Requesters, reinterpret_cast<U64>(requester));
		if (holder->Requesters.empty())
			SystemData.erase(std::ranges::find(SystemData, *holder));
	}

	template <class T>
	void CWorld::BlockSystem(void* requester)
	{
		SSystemData* holder = GetSystemHolder<T>();
		if (holder == nullptr)
			return;

		holder->Blockers.push_back(reinterpret_cast<U64>(requester));
	}

	template <class T>
	void CWorld::UnblockSystem(void* requester)
	{
		SSystemData* holder = GetSystemHolder<T>();
		if (holder == nullptr)
			return;
			
		std::erase(holder->Blockers, reinterpret_cast<U64>(requester));
	}
}
