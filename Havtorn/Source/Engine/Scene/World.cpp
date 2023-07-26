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

	void CWorld::LoadScene(const std::string& filePath)
	{
		Scenes.emplace_back(std::make_unique<CScene>());
		SSceneFileHeader sceneFile;
		//I64 sceneIndex = Scenes.size() - 1;

		const U64 fileSize = GEngine::GetFileSystem()->GetFileSize(filePath);
		char* data = new char[fileSize];

		Scenes.back()->Init(RenderManager);

		U64 pointerPosition = 0;
		GEngine::GetFileSystem()->Deserialize(filePath, data, static_cast<U32>(fileSize));
		sceneFile.Deserialize(data, pointerPosition, Scenes.back().get(), AssetRegistry.get());

		delete[] data;
	}

	std::vector<Ptr<CScene>>& CWorld::GetActiveScenes()
	{
		return Scenes;
	}

	std::vector<SEntity>& CWorld::GetEntities() const
	{
		return Scenes.back()->GetEntities();
	}
	
	void CWorld::SaveActiveScene(const std::string& destinationPath)
	{
		if (Scenes.empty())
		{
			HV_LOG_ERROR("Tried to save empty Scene.");
			return;
		}

		const Ptr<CScene>& scene = Scenes.back();
		I64 sceneIndex = Scenes.size() - 1;

		SSceneFileHeader fileHeader;
		fileHeader.SceneName = "TestScene";
		fileHeader.SceneNameLength = static_cast<U32>(fileHeader.SceneName.size());
		fileHeader.NumberOfEntities = static_cast<U32>(scene->GetEntities().size());
		fileHeader.Scene = scene.get();

		const U32 fileSize = fileHeader.GetSize() + AssetRegistry->GetSize(sceneIndex);
		char* data = new char[fileSize];

		U64 pointerPosition = 0;	
		fileHeader.Serialize(data, pointerPosition, AssetRegistry.get(), sceneIndex);
		GEngine::GetFileSystem()->Serialize(destinationPath, data, fileSize);
		
		delete[] data;
	}

	void CWorld::AddScene(const std::string& filePath)
	{
		LoadScene(filePath);
	}

	void CWorld::RemoveScene(U64 sceneIndex)
	{
		if (sceneIndex >= Scenes.size())
			return;

		std::swap(Scenes.back(), Scenes[sceneIndex]);
		Scenes.pop_back();
	}

	void CWorld::ChangeScene(const std::string& filePath)
	{
		Scenes.clear();
		LoadScene(filePath);
	}

	void CWorld::OpenDemoScene(const bool shouldOpen3DDemo)
	{
		Scenes.clear();
		Scenes.emplace_back(std::make_unique<CScene>());
		
		if (shouldOpen3DDemo)
		{
			ENGINE_BOOL_POPUP(Scenes.back()->Init3DDemoScene(RenderManager), "Demo Scene could not be initialized.");
		}
		else
		{
			ENGINE_BOOL_POPUP(Scenes.back()->Init2DDemoScene(RenderManager), "Demo Scene could not be initialized.");
		}
	}

	CAssetRegistry* CWorld::GetAssetRegistry() const
	{
		return AssetRegistry.get();
	}
}
