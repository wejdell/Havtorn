// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/Entity.h"
#include "Assets/FileHeaderDeclarations.h"
#include "HexPhys/HexPhys.h"
#include "HexAudio/HexAudio.h"
#include "HexRune/HexRune.h"

#include "Graphics/GraphicsStructs.h"
#include "Graphics/GraphicsEnums.h"
#include "Scene/Scene.h"
#include "Assets/SequencerAsset.h"
#include "Assets/FileHeaders/PrefabAssetFileHeader.h"

#include <HavtornString.h>
#include <EngineException.h>
#include <HavtornDelegate.h>
#include <FileSystem.h>

#include <queue>
#include <concepts>

namespace Havtorn
{
	struct SEntity;
	struct SPhysics2DComponent;
	struct SPhysics3DComponent;
	struct STransformComponent;
	class ISystem;
	class CPlatformManager;
	class CRenderManager;
	class CGraphicsFramework;
	class CAssetRegistry;
	class CSequencerSystem;
	class CScene;

	template<typename T>
	concept SceneType = std::derived_from<T, CScene>;

	template<typename T>
	concept ScriptType = std::derived_from<T, HexRune::SScript>;

	namespace HexPhys2D
	{
		class CPhysicsWorld2D;
	}

	namespace HexPhys3D
	{
		class CPhysicsWorld3D;
	}

	namespace HexAudio
	{
		class CAudioBackend;
	}

	enum class ENGINE_API ESystemUpdateOrder
	{
		Update,
		EndFrame
	};

	struct SSystemData
	{
		Ptr<ISystem> System = nullptr;
		std::vector<U64> Requesters;
		std::vector<U64> Blockers;
		// TODO.NW: Decide if we want to store systems in different collections to do this, 
		// or whether the runtime checks for update order are ok.
		ESystemUpdateOrder UpdateOrder = ESystemUpdateOrder::Update;

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
		ENGINE_API void SaveActiveScene(const std::string& destinationPath, CScene* scene) const;
		ENGINE_API void RemoveScene(const U64 sceneIndex);
		ENGINE_API void RemoveScene(const CHavtornStaticString<255>& sceneName);
		ENGINE_API void ClearScenes();

		ENGINE_API void SetMainCamera(const SEntity& entity);
		ENGINE_API SEntity GetMainCamera() const;
		ENGINE_API void SetEditorRenderExemptEntity(const SEntity& entity);
		ENGINE_API SEntity GetEditorRenderExemptEntity() const;
		
		ENGINE_API void BindSceneLoader(const std::function<bool(const std::string&)>& loadingFunction);

		ENGINE_API CScene* const CreateNewScene(const std::string& sceneName);
		ENGINE_API Ptr<CScene> CreateMovableScene(const std::string& sceneName);

		ENGINE_API Ptr<HexRune::SScript> CreateMovableScript(const std::string& scriptName);

		template<SceneType CSceneType>
		void CreateScene();

		template<SceneType CSceneType, ScriptType SScriptType>
		void BindGameTypes();

		template<typename T>
		bool AddScene(const std::string& filePath);

		template<typename T>
		void ChangeScene(const std::string& filePath);

		template<typename T>
		void OpenDemoScene(const bool shouldOpen3DDemo = true);

		template<typename T>
		inline T* GetSystem();

		template<typename T>
		inline SSystemData* GetSystemHolder();

		template<typename T>
		inline bool HasSystem();

		template<typename T, typename... Args>
		inline void RequestSystem(void* requester, Args&&... args);

		template<typename T>
		inline void UnrequestSystem(void* requester);

		ENGINE_API inline void UnrequestSystems(void* requester);

		template<typename T>
		inline void BlockSystem(void* requester);

		template<typename T>
		inline void UnblockSystem(void* requester);

		ENGINE_API void RequestPhysicsSystem(void* requester);
		ENGINE_API void BlockPhysicsSystem(void* requester);
		ENGINE_API void UnblockPhysicsSystem(void* requester);

		ENGINE_API void InitializePhysics3D(std::vector<Ptr<CScene>>& scenes);
		ENGINE_API void DeInitializePhysics3D(std::vector<Ptr<CScene>>& scenes);

		ENGINE_API void Initialize2DPhysicsData(const SEntity& entity) const;
		ENGINE_API void Update2DPhysicsData(STransformComponent* transformComponent, SPhysics2DComponent* phys2DComponent) const;

		ENGINE_API void Initialize3DPhysicsData(const SEntity& entity) const;
		ENGINE_API void Update3DPhysicsData(STransformComponent* transformComponent, SPhysics3DComponent* phys2DComponent) const;

		ENGINE_API void RequestAudioSystem(void* requester);
		ENGINE_API U64 RegisterAudioObject(const bool isListener = false);
		ENGINE_API void UnregisterAudioObject(const U64 objectID);

	public:
		// TODO.NW: Maybe unify and have Enum arg instead
		CMulticastDelegate<std::vector<Ptr<CScene>>&> OnBeginPlayDelegate;
		CMulticastDelegate<std::vector<Ptr<CScene>>&> OnPausePlayDelegate;
		CMulticastDelegate<std::vector<Ptr<CScene>>&> OnEndPlayDelegate;

		CMulticastDelegate<CScene*, const SEntity, const SEntity> OnBeginOverlap;
		CMulticastDelegate<CScene*, const SEntity, const SEntity> OnEndOverlap;
		CMulticastDelegate<const SEntity, const SEntity> OnBeginOverlapWorld;
		CMulticastDelegate<const SEntity, const SEntity> OnEndOverlapWorld;

		template<typename T>
		T* GetComponent(const SEntity& fromEntity) const
		{
			for (auto& scene : Scenes)
			{
				if (!scene->HasEntity(fromEntity.GUID))
					continue;

				return scene->GetComponent<T>(fromEntity);
			}

			return nullptr;
		}

	private:
		CWorld() = default;
		~CWorld() = default;
		
		bool Init(CPlatformManager* platformManager, CRenderManager* renderManager);
		void Update();
		void EndFrame();

		ENGINE_API void LoadScene(const std::string& filePath, CScene* outScene) const;

		void OnSceneCreated(CScene* const scene) const;

	private:
		std::vector<Ptr<CScene>> Scenes;
		std::vector<SSystemData> SystemData;

		std::queue<U64> QueuedSystemUnrequests;

		Ptr<HexPhys2D::CPhysicsWorld2D> PhysicsWorld2D = nullptr;
		Ptr<HexPhys3D::CPhysicsWorld3D> PhysicsWorld3D = nullptr;
		Ptr<HexAudio::CAudioBackend> AudioBackend = nullptr;
		
		CRenderManager* RenderManager = nullptr;

		SEntity MainCameraEntity = SEntity::Null;
		SEntity EditorRenderExemptEntity = SEntity::Null;

		CMulticastDelegate<CScene* const> OnSceneCreatedDelegate;

		EWorldPlayState PlayState = EWorldPlayState::Stopped;
		EWorldPlayDimensions PlayDimensions = EWorldPlayDimensions::World3D;

		std::function<CScene* const(const std::string&)> CreateNewSceneFunction;
		std::function<Ptr<CScene>(const std::string&)> CreateMovableSceneFunction;
		
		std::function<Ptr<HexRune::SScript>(const std::string&)> CreateMovableScriptFunction;
	};

	template<SceneType CSceneType>
	inline void CWorld::CreateScene()
	{
		Scenes.emplace_back(std::make_unique<CSceneType>());
		OnSceneCreatedDelegate.Broadcast(Scenes.back().get());
	}

	template<SceneType CSceneType, ScriptType SScriptType>
	inline void CWorld::BindGameTypes()
	{
		CreateNewSceneFunction = [&](const std::string& sceneName) 
			{
				CreateScene<CSceneType>();
				CScene* scene = Scenes.back().get();
				scene->Init(sceneName); 
				if (GetWorldPlayDimensions() == EWorldPlayDimensions::World3D)
					scene->Init3DDefaults();

				return scene;
			};

		CreateMovableSceneFunction = [](const std::string& sceneName) { Ptr<CSceneType> newScene = std::make_unique<CSceneType>(); newScene->Init(sceneName); return std::move(newScene); };
		
		auto loadGameScene = [&](const std::string& filePath) { return AddScene<CSceneType>(filePath); };
		BindSceneLoader(loadGameScene);
		
		CreateMovableScriptFunction = [](const std::string& scriptName) { Ptr<SScriptType> script = std::make_unique<SScriptType>(); script->Initialize(); script->Name = scriptName; return std::move(script); };
	}

	template<typename T>
	inline bool CWorld::AddScene(const std::string& filePath)
	{
		if (!UFileSystem::Exists(filePath))
		{
			HV_LOG_WARN("CWorld::AddScene: Scene file '%s' could not be found, aborting load.", filePath.c_str());
			return false;
		}

		std::string sceneName = UGeneralUtils::ExtractFileBaseNameFromPath(UGeneralUtils::ConvertToPlatformAgnosticPath(filePath));
		for (Ptr<CScene>& scene : Scenes)
		{
			if (scene->GetSceneName() == sceneName)
			{
				HV_LOG_WARN("CWorld::AddScene: Scene '%s' is already loaded!", sceneName.c_str());
				return false;
			}
		}

		CreateScene<T>();
		LoadScene(filePath, Scenes.back().get());
		return true;
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

	template<typename T>
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

	template<typename T>
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

	template<typename T>
	inline bool CWorld::HasSystem()
	{
		return GetSystem<T>() != nullptr;
	}

	template<typename T, typename... Args>
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

	template<typename T>
	void CWorld::UnrequestSystem(void* requester)
	{
		SSystemData* holder = GetSystemHolder<T>();
		if (holder == nullptr)
			return;

		std::erase(holder->Requesters, reinterpret_cast<U64>(requester));
		if (holder->Requesters.empty())
			SystemData.erase(std::ranges::find(SystemData, *holder));
	}

	template<typename T>
	void CWorld::BlockSystem(void* requester)
	{
		SSystemData* holder = GetSystemHolder<T>();
		if (holder == nullptr)
			return;

		holder->Blockers.push_back(reinterpret_cast<U64>(requester));
	}

	template<typename T>
	void CWorld::UnblockSystem(void* requester)
	{
		SSystemData* holder = GetSystemHolder<T>();
		if (holder == nullptr)
			return;
			
		std::erase(holder->Blockers, reinterpret_cast<U64>(requester));
	}
}
