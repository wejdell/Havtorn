// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "World.h"
#include "ECS/ECSInclude.h"
#include "Scene.h"
#include "Graphics/RenderManager.h"
#include "AssetRegistry.h"
#include "Graphics/Debug/DebugDrawUtility.h"
#include "HexPhys/HexPhys.h"

namespace Havtorn
{
	bool CWorld::Init(CRenderManager* renderManager)
	{
		RenderManager = renderManager;
		AssetRegistry = std::make_unique<CAssetRegistry>();
		PhysicsWorld2D = std::make_unique<HexPhys2D::CPhysicsWorld2D>();
		PhysicsWorld3D = std::make_unique<HexPhys3D::CPhysicsWorld3D>();

		// Setup systems
		RequestSystem<CCameraSystem>(this);
		RequestSystem<CLightSystem>(this, RenderManager);
		RequestSystem<CSequencerSystem>(this);
		RequestSystem<CAnimatorGraphSystem>(this, RenderManager);
		RequestSystem<CScriptSystem>(this, this);
		RequestSystem<CRenderSystem>(this, RenderManager, this);

		OnSceneCreatedDelegate.AddMember(this, &CWorld::OnSceneCreated);

		return true;
	}

	void CWorld::Update() const
	{
		for (auto& scene : Scenes)
		{
			for (const auto& data : SystemData)
			{
				if (data.Blockers.empty())
					data.System->Update(scene.get());

			}
		}

		//GDebugDraw::TestAllShapes();
	}

	bool CWorld::BeginPlay()
	{
		if (PlayState == EWorldPlayState::Playing)
			return false;

		if (Scenes.empty())
			return false;

		PlayState = EWorldPlayState::Playing;
		OnBeginPlayDelegate.Broadcast(Scenes.back().get());

		return true;
	}

	bool CWorld::PausePlay()
	{
		if (PlayState == EWorldPlayState::Paused || PlayState == EWorldPlayState::Stopped)
			return false;

		if (Scenes.empty())
			return false;

		PlayState = EWorldPlayState::Paused;
		OnPausePlayDelegate.Broadcast(Scenes.back().get());

		return true;
	}

	bool CWorld::StopPlay()
	{
		if (PlayState == EWorldPlayState::Stopped)
			return false;

		if (Scenes.empty())
			return false;

		PlayState = EWorldPlayState::Stopped;
		OnEndPlayDelegate.Broadcast(Scenes.back().get());

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

	std::vector<SEntity>& CWorld::GetEntities() const
	{
		return Scenes.back()->Entities;
	}
	
	void CWorld::SaveActiveScene(const std::string& destinationPath) const
	{
		if (Scenes.empty())
		{
			HV_LOG_ERROR("Tried to save empty Scene.");
			return;
		}

		const Ptr<CScene>& scene = Scenes.back();

		SSceneFileHeader fileHeader;
		fileHeader.Scene = scene.get();

		const U32 fileSize = GetDataSize(fileHeader.AssetType) + AssetRegistry->GetSize() + fileHeader.GetSize();
		char* data = new char[fileSize];

		U64 pointerPosition = 0;	
		fileHeader.Serialize(data, pointerPosition, AssetRegistry.get());
		GEngine::GetFileSystem()->Serialize(destinationPath, data, fileSize);
		
		delete[] data;
	}

	void CWorld::LoadScene(const std::string& filePath, CScene* outScene) const
	{
		SSceneFileHeader sceneFile;

		const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
		char* data = new char[fileSize];

		outScene->Init(RenderManager, UGeneralUtils::ExtractFileBaseNameFromPath(filePath));

		U64 pointerPosition = 0;
		GEngine::GetFileSystem()->Deserialize(filePath, data, STATIC_U32(fileSize));
		sceneFile.Deserialize(data, pointerPosition, outScene, AssetRegistry.get());

		delete[] data;
	}

	void CWorld::OnSceneCreated(CScene* scene) const
	{
		PhysicsWorld3D->CreateScene(scene);
	}

	void CWorld::RemoveScene(U64 sceneIndex)
	{
		if (sceneIndex >= Scenes.size())
			return;

		std::swap(Scenes.back(), Scenes[sceneIndex]);
		Scenes.pop_back();
	}

	void CWorld::UnrequestSystems(void* requester)
	{
		for (SSystemData& data : SystemData)
			std::erase(data.Requesters, reinterpret_cast<U64>(requester));

		std::erase_if(SystemData, [](const SSystemData& data) { return data.Requesters.empty(); });
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

	void CWorld::Initialize3DPhysicsData(const SEntity& entity) const
	{
		if (Scenes.empty())
			return;

		// TODO.NR: Make find-scene-from-entity util? Could be a thing to just take component pointers as params instead, 
		// but wouldn't be as clean an interface when this is extended to 3D physics. Probably fine with multiple overloads.
		if (SPhysics3DComponent* phys3DComponent = Scenes.back()->GetComponent<SPhysics3DComponent>(entity))
			if (STransformComponent* transformComponent = Scenes.back()->GetComponent<STransformComponent>(entity))
				PhysicsWorld3D->InitializePhysicsData(transformComponent, phys3DComponent);

		if (SPhysics3DControllerComponent* phys3DControllerComponent = Scenes.back()->GetComponent<SPhysics3DControllerComponent>(entity))
			if (STransformComponent* transformComponent = Scenes.back()->GetComponent<STransformComponent>(entity))
				PhysicsWorld3D->InitializePhysicsData(transformComponent, phys3DControllerComponent);
	}

	void CWorld::Update3DPhysicsData(STransformComponent* transformComponent, SPhysics3DComponent* phys3DComponent) const
	{
		if (PhysicsWorld3D != nullptr)
			PhysicsWorld3D->UpdatePhysicsData(transformComponent, phys3DComponent);
	}

	void CWorld::SaveScript(const std::string& filePath)
	{
		if (!LoadedScripts.contains(filePath))
			return;

		HexRune::SScript* script = LoadedScripts.at(filePath).get();

		SScriptFileHeader fileHeader;
		fileHeader.Script = script;

		const U32 fileSize = fileHeader.GetSize();
		char* data = new char[fileSize];

		fileHeader.Serialize(data);
		GEngine::GetFileSystem()->Serialize(filePath, data, fileSize);

		delete[] data;
	}

	HexRune::SScript* CWorld::LoadScript(const std::string& filePath)
	{
		if (LoadedScripts.contains(filePath))
			return LoadedScripts.at(filePath).get();

		const U32 fileSize = STATIC_U32(GEngine::GetFileSystem()->GetFileSize(filePath));
		char* data = new char[fileSize];

		GEngine::GetFileSystem()->Deserialize(filePath, data, fileSize);

		SScriptFileHeader assetFile;
		LoadedScripts.emplace(filePath, std::make_unique<HexRune::SScript>());
		assetFile.Deserialize(data, LoadedScripts.at(filePath).get());

		// TODO.NW: When unifying asset loading, should have an abstraction for an asset, maybe only key and file path, and make sure
		// they are always fully initialized if they exist.
		LoadedScripts.at(filePath).get()->FileName = filePath;

		delete[] data;
		return LoadedScripts.at(filePath).get();
	}

	void CWorld::UnloadScript(const std::string& filePath)
	{
		// NW: Call only when last asset registry key has been unregistered
		
		if (!LoadedScripts.contains(filePath))
			return;

		LoadedScripts.erase(filePath);
	}

	CAssetRegistry* CWorld::GetAssetRegistry() const
	{
		return AssetRegistry.get();
	}
}
