// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "World.h"
#include "ECS/ECSInclude.h"
#include "Scene.h"
#include "Graphics/RenderManager.h"
#include "AssetRegistry.h"

namespace Havtorn
{
	bool CWorld::Init(CRenderManager* renderManager)
	{
		RenderManager = renderManager;
		AssetRegistry = std::make_unique<CAssetRegistry>();

		// Setup systems
		Systems.emplace_back(std::make_unique<CCameraSystem>());
		Systems.emplace_back(std::make_unique<CLightSystem>(RenderManager));
		Systems.emplace_back(std::make_unique<CRenderSystem>(RenderManager));
		//Systems.emplace_back(std::make_unique<Debug::UDebugShapeSystem>(Scenes.back().get(), RenderManager));

		return true;
	}

	void CWorld::Update()
	{
		for (auto& scene : Scenes)
		{
			for (const auto& system : Systems)
			{
				system->Update(scene.get());
			}
		}
	}

	std::vector<Ptr<CScene>>& CWorld::GetActiveScenes()
	{
		return Scenes;
	}

	const std::vector<SEntity>& CWorld::GetEntities() const
	{
		return Scenes.back()->GetEntities();
	}
	
	void CWorld::SaveActiveScene(const std::string& destinationPath)
	{
		const Ptr<CScene>& scene = Scenes.back();
		I64 sceneIndex = Scenes.size() - 1;

		SSceneFileHeader fileHeader;
		fileHeader.SceneName = "TestScene";
		fileHeader.SceneNameLength = static_cast<U32>(fileHeader.SceneName.size());
		fileHeader.NumberOfEntities = static_cast<U32>(scene->GetEntities().size());
		fileHeader.Scene = scene.get();

		const U32 fileSize = fileHeader.GetSize() + AssetRegistry->GetSize(sceneIndex);
		char* data = new char[fileSize];

		U32 pointerPosition = 0;	
		fileHeader.Serialize(data, pointerPosition, AssetRegistry.get(), sceneIndex); //1325
		GEngine::GetFileSystem()->Serialize(destinationPath, data, fileSize);
		
		delete[] data;
	}

	void CWorld::LoadScene(const std::string& filePath)
	{
		Scenes.emplace_back(std::make_unique<CScene>());
		SSceneFileHeader sceneFile;
		//I64 sceneIndex = Scenes.size() - 1;

		const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
		char* data = new char[fileSize];

		// Don't put it here if it does assetregistry work
		Scenes.back()->Init(RenderManager);

		U32 pointerPosition = 0;
		GEngine::GetFileSystem()->Deserialize(filePath, data, static_cast<U32>(fileSize));	
		sceneFile.Deserialize(data, pointerPosition, Scenes.back().get(), AssetRegistry.get()); //1325
		
		delete[] data;

	}

	void CWorld::OpenDemoScene()
	{
		Scenes.emplace_back(std::make_unique<CScene>());
		ENGINE_BOOL_POPUP(Scenes.back()->InitDemoScene(RenderManager), "World could not be initialized.");
	}

	CAssetRegistry* CWorld::GetAssetRegistry() const
	{
		return AssetRegistry.get();
	}
}
