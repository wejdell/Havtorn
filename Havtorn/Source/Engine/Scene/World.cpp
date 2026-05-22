// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "World.h"
#include "Engine.h"
#include "ECS/ECSInclude.h"
#include "Scene.h"
#include "Graphics/RenderManager.h"
#include "Assets/AssetRegistry.h"
#include "Graphics/Debug/DebugDrawUtility.h"
#include "Input/InputMapper.h"

#include <PlatformManager.h>

namespace Havtorn
{
	bool CWorld::Init(CPlatformManager* platformManager, CRenderManager* renderManager)
	{
		RenderManager = renderManager;
		PhysicsWorld2D = std::make_unique<HexPhys2D::CPhysicsWorld2D>();
		PhysicsWorld3D = std::make_unique<HexPhys3D::CPhysicsWorld3D>();
		AudioBackend = std::make_unique<HexAudio::CAudioBackend>();

		// Setup systems
		RequestSystem<CCameraSystem>(this);
		RequestSystem<CLightSystem>(this, RenderManager);
		RequestSystem<CSequencerSystem>(this);
		RequestSystem<CAnimatorGraphSystem>(this, RenderManager);
		RequestSystem<CScriptSystem>(this, this);
		RequestSystem<CUISystem>(this, platformManager);
		RequestSystem<CRenderSystem>(this, RenderManager, this);
		
		RequestSystem<CLevelStreamingSystem>(this);
		SystemData.back().UpdateOrder = ESystemUpdateOrder::EndFrame;

		OnSceneCreatedDelegate.AddMember(this, &CWorld::OnSceneCreated);

		//Would like to Setup these Listeners in CPhysicsSystem3D instead.
		OnBeginPlayDelegate.AddMember(this, &CWorld::InitializePhysics3D);
		OnEndPlayDelegate.AddMember(this, &CWorld::DeInitializePhysics3D);

		return true;
	}

	void CWorld::Update()
	{
		for (const auto& data : SystemData)
		{
			if (data.Blockers.empty() && data.UpdateOrder == ESystemUpdateOrder::Update)
				data.System->Update(Scenes);			
		}

		while (!QueuedSystemUnrequests.empty())
		{
			const U64 unrequest = QueuedSystemUnrequests.front();

			for (SSystemData& data : SystemData)
				std::erase(data.Requesters, unrequest);

			std::erase_if(SystemData, [](const SSystemData& data) { return data.Requesters.empty(); });

			QueuedSystemUnrequests.pop();
		}
	}

	void CWorld::EndFrame()
	{
		for (const auto& data : SystemData)
		{
			if (data.UpdateOrder == ESystemUpdateOrder::EndFrame && data.Blockers.empty())
				data.System->Update(Scenes);
		}
	}

	bool CWorld::BeginPlay()
	{
		if (PlayState == EWorldPlayState::Playing)
			return false;

		if (Scenes.empty())
			return false;

		// TODO.NW: Go through mappings and cover all contexts, or even better
		// fix assigning multiple contexts (bitset for combining contexts) so 
		// we can toggle it correctly here
		GEngine::GetInput()->SetInputContext(EInputContext::InGame);

		PlayState = EWorldPlayState::Playing;
		OnBeginPlayDelegate.Broadcast(Scenes);

		return true;
	}

	bool CWorld::PausePlay()
	{
		if (PlayState == EWorldPlayState::Paused || PlayState == EWorldPlayState::Stopped)
			return false;

		if (Scenes.empty())
			return false;

		PlayState = EWorldPlayState::Paused;
		OnPausePlayDelegate.Broadcast(Scenes);

		return true;
	}

	bool CWorld::StopPlay()
	{
		if (PlayState == EWorldPlayState::Stopped)
			return false;

		if (Scenes.empty())
			return false;

#ifdef HV_EDITOR_BUILD
		GEngine::GetInput()->SetInputContext(EInputContext::Editor);
#endif // TODO.NW: Introduce MainMenu context?

		PlayState = EWorldPlayState::Stopped;
		OnEndPlayDelegate.Broadcast(Scenes);

		return true;
	}

	EWorldPlayState CWorld::GetWorldPlayState() const
	{
		return PlayState;
	}

	EWorldPlayDimensions CWorld::GetWorldPlayDimensions() const
	{
		return PlayDimensions;
	}

	void CWorld::ToggleWorldPlayDimensions()
	{
		if (PlayState == EWorldPlayState::Stopped)
			PlayDimensions == EWorldPlayDimensions::World3D ? PlayDimensions = EWorldPlayDimensions::World2D : PlayDimensions = EWorldPlayDimensions::World3D;
	}

	std::vector<Ptr<CScene>>& CWorld::GetActiveScenes()
	{
		return Scenes;
	}
	
	void CWorld::SaveActiveScene(const std::string& destinationPath, CScene* scene) const
	{
		if (scene == nullptr)
		{
			HV_LOG_ERROR("CWorld::SaveActiveScene: Provided scene pointer was nullptr, cannot save scene to %s", destinationPath.c_str());
			return;
		}

		SSceneFileHeader fileHeader;
		fileHeader.Scene = scene;

		const U32 fileSize = fileHeader.GetSize();
		char* data = new char[fileSize];

		U64 pointerPosition = 0;	
		fileHeader.Serialize(data, pointerPosition);
		UFileSystem::Serialize(destinationPath, data, fileSize);
		
		delete[] data;
	}

	void CWorld::LoadScene(const std::string& filePath, CScene* outScene) const
	{
		SSceneFileHeader sceneFile;

		const U64 fileSize = UFileSystem::GetFileSize(filePath);
		char* data = new char[fileSize];

		outScene->Init(UGeneralUtils::ExtractFileBaseNameFromPath(filePath));

		U64 pointerPosition = 0;
		UFileSystem::Deserialize(filePath, data, STATIC_U32(fileSize));
		sceneFile.Deserialize(data, pointerPosition, outScene);

		delete[] data;
	}

	void CWorld::OnSceneCreated(CScene* const/*scene*/) const
	{
		//PhysicsWorld3D->CreateScene(scene);
	}

	void CWorld::RemoveScene(const U64 sceneIndex)
	{
		if (sceneIndex >= Scenes.size())
			return;

		std::swap(Scenes.back(), Scenes[sceneIndex]);
		Scenes.pop_back();
	}

	void CWorld::RemoveScene(const CHavtornStaticString<255>& sceneName)
	{
		auto it = std::ranges::find(Scenes, sceneName, &CScene::SceneName);
		if (it != Scenes.end())
			Scenes.erase(it);
	}

	void CWorld::ClearScenes()
	{
		// TODO.NW: Figure out mechanism for checking if scenes may close?
		Scenes.clear();
	}

	void CWorld::SetMainCamera(const SEntity& entity)
	{
		if (MainCameraEntity.IsValid())
			RenderManager->UnrequestRenderView(MainCameraEntity.GUID);

		MainCameraEntity = entity;
	}

	SEntity CWorld::GetMainCamera() const
	{
		return MainCameraEntity;
	}

	void CWorld::SetEditorRenderExemptEntity(const SEntity& entity)
	{
		EditorRenderExemptEntity = entity;
	}

	SEntity CWorld::GetEditorRenderExemptEntity() const
	{
		return EditorRenderExemptEntity;
	}

	void CWorld::BindSceneLoader(const std::function<bool(const std::string&)>& loadingFunction)
	{
		GetSystem<CLevelStreamingSystem>()->BindSceneLoader(loadingFunction);
	}

	CScene* const CWorld::CreateNewScene(const std::string& sceneName)
	{
		return CreateNewSceneFunction(sceneName);
	}

	Ptr<CScene> CWorld::CreateMovableScene(const std::string& sceneName)
	{
		return CreateMovableSceneFunction(sceneName);
	}

	Ptr<HexRune::SScript> CWorld::CreateMovableScript(const std::string& scriptName)
	{
		return CreateMovableScriptFunction(scriptName);
	}

	void CWorld::UnrequestSystems(void* requester)
	{
		QueuedSystemUnrequests.push(reinterpret_cast<U64>(requester));
	}

	void CWorld::RequestPhysicsSystem(void* requester)
	{
		if (PlayDimensions == EWorldPlayDimensions::World3D)
		{
			RequestSystem<HexPhys3D::CPhysics3DSystem>(requester, PhysicsWorld3D.get());
			return;
		}

		RequestSystem<HexPhys2D::CPhysics2DSystem>(requester, PhysicsWorld2D.get());
	}

	void CWorld::BlockPhysicsSystem(void* requester)
	{
		PlayDimensions == EWorldPlayDimensions::World3D ? BlockSystem<HexPhys3D::CPhysics3DSystem>(requester) : BlockSystem<HexPhys2D::CPhysics2DSystem>(requester);
	}

	void CWorld::UnblockPhysicsSystem(void* requester)
	{
		PlayDimensions == EWorldPlayDimensions::World3D ? UnblockSystem<HexPhys3D::CPhysics3DSystem>(requester) : UnblockSystem<HexPhys2D::CPhysics2DSystem>(requester);
	}

	void CWorld::InitializePhysics3D(std::vector<Ptr<CScene>>& scenes)
	{
		PhysicsWorld3D->InitializeScene(scenes);
	}

	void CWorld::DeInitializePhysics3D(std::vector<Ptr<CScene>>& scenes)
	{
		PhysicsWorld3D->DeInitializeScene(scenes);
	}

	void CWorld::Initialize2DPhysicsData(const SEntity& entity) const
	{
		if (Scenes.empty())
			return;

		// TODO.NR: Make find-scene-from-entity util? Could be a thing to just take component pointers as params instead, 
		// but wouldn't be as clean an interface when this is extended to 3D physics. Probably fine with multiple overloads.
		if (SPhysics2DComponent* phys2DComponent = Scenes.back()->GetComponent<SPhysics2DComponent>(entity))
			if (STransformComponent* transformComponent = Scenes.back()->GetComponent<STransformComponent>(entity))
				PhysicsWorld2D->InitializePhysicsData(transformComponent, phys2DComponent);
	}

	void CWorld::Update2DPhysicsData(STransformComponent* transformComponent, SPhysics2DComponent* phys2DComponent) const
	{
		if (PhysicsWorld2D != nullptr)
			PhysicsWorld2D->UpdatePhysicsData(transformComponent, phys2DComponent);
	}

	void CWorld::RequestAudioSystem(void* requester)
	{
		RequestSystem<HexAudio::CAudioSystem>(requester, AudioBackend.get());
	}

	U64 CWorld::RegisterAudioObject(const bool isListener, const std::vector<SAssetReference>& assetReferences)
	{
		return AudioBackend->RegisterAudioObject(isListener, assetReferences);
	}

	void CWorld::UnregisterAudioObject(const U64 objectID, const std::vector<SAssetReference>& assetReferences)
	{
		AudioBackend->UnregisterAudioObject(objectID, assetReferences);
	}
}
