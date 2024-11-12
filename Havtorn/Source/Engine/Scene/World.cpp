// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "World.h"
#include "ECS/ECSInclude.h"
#include "Scene.h"
#include "Graphics/RenderManager.h"
#include "AssetRegistry.h"
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
		AddSystem<CCameraSystem>();
		AddSystem<CLightSystem>(RenderManager);
		AddSystem<CSpriteAnimatorGraphSystem>();
		AddSystem<CSequencerSystem>();
		AddSystem<CRenderSystem>(RenderManager);
		AddSystem<HexPhys2D::CPhysics2DSystem>(PhysicsWorld2D.get());
		AddSystem<HexPhys3D::CPhysics3DSystem>(PhysicsWorld3D.get());

		OnSceneCreatedDelegate.AddMember(this, &CWorld::OnSceneCreated);

		return true;
	}

	void CWorld::Update()
	{
		RemovePendingSystems();
		AddPendingSystems();

		for (auto& scene : Scenes)
		{
			for (const auto& system : Systems)
			{
				system->Update(scene.get());
			}
		}
	}

	void CWorld::AddPendingSystems()
	{
		for (auto& system : SystemsToAdd)
			Systems.push_back(std::move(system));

		SystemsToAdd.clear();
	}

	void CWorld::RemovePendingSystems()
	{
		for (U16 toRemoveIndex = 0; toRemoveIndex < SystemsToRemove.size(); toRemoveIndex++)
		{
			for (U16 systemsIndex = 0; systemsIndex < Systems.size(); systemsIndex++)
			{
				if (typeid(*Systems[systemsIndex].get()).hash_code() != SystemsToRemove[toRemoveIndex].HashCode)
					continue;

				RemoveSystemRespectOrder(systemsIndex);
			}
		}

		SystemsToRemove.clear();
	}

	void  CWorld::RemoveSystem(U16 index)
	{
		std::swap(Systems[index], Systems.back());
		Systems.pop_back();
	}

	void  CWorld::RemoveSystemRespectOrder(U16 index)
	{
		for (U16 i = index; i < Systems.size(); i++)
		{
			U16 next = i + 1;
			if (next == Systems.size())
				break;

			std::swap(Systems[i], Systems[next]);
		}

		Systems.pop_back();
	}

	std::vector<Ptr<CScene>>& CWorld::GetActiveScenes()
	{
		return Scenes;
	}

	std::vector<SEntity>& CWorld::GetEntities() const
	{
		return Scenes.back()->Entities;
	}
	
	void CWorld::SaveActiveScene(const std::string& destinationPath)
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

	void CWorld::LoadScene(const std::string& filePath, CScene* outScene)
	{
		SSceneFileHeader sceneFile;

		const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
		char* data = new char[fileSize];

		const U64 lastSlashIndex = filePath.find_last_of("/");
		const U64 lastDotIndex = filePath.find_last_of(".");
		std::string sceneNameSubstring = filePath.substr(lastSlashIndex, lastDotIndex - lastSlashIndex);
		outScene->Init(RenderManager, sceneNameSubstring);

		U64 pointerPosition = 0;
		GEngine::GetFileSystem()->Deserialize(filePath, data, static_cast<U32>(fileSize));
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

	CAssetRegistry* CWorld::GetAssetRegistry() const
	{
		return AssetRegistry.get();
	}
}
