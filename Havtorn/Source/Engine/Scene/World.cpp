// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "World.h"
#include "ECS/ECSInclude.h"
#include "Scene.h"
#include "Graphics/RenderManager.h"

namespace Havtorn
{
	void CWorld::LoadScene(std::string filePath)
	{
		const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
		char* data = new char[fileSize];

		GEngine::GetFileSystem()->Deserialize(filePath, data, static_cast<U32>(fileSize));

		SSceneFileHeader sceneFile;
		sceneFile.Deserialize(data);
		
		//Scenes.emplace_back(std::make_unique<CScene>(std::move(sceneFile.Scene)));
	}

	bool CWorld::Init(CRenderManager* renderManager)
	{
		RenderManager = renderManager;

		//LoadScene("Assets/Scenes/TestScene.hvs");
		Scenes.emplace_back(std::make_unique<CScene>());
		ENGINE_ERROR_BOOL_MESSAGE(Scenes.back()->Init(RenderManager), "World could not be initialized.");

		// Setup systems
		Systems.emplace_back(std::make_unique<CCameraSystem>());
		Systems.emplace_back(std::make_unique<CLightSystem>(RenderManager));
		Systems.emplace_back(std::make_unique<CRenderSystem>(RenderManager));
		Systems.emplace_back(std::make_unique<Debug::UDebugShapeSystem>(Scenes.back().get(), RenderManager));

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

	std::vector<Ref<SEntity>>& CWorld::GetEntities() const
	{
		return Scenes.back()->GetEntities();
	}
	
	void CWorld::SaveActiveScene(const std::string& destinationPath)
	{
		Scenes.back()->SaveScene(destinationPath);
	}
}