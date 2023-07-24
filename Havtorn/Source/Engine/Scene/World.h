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
		HAVTORN_API void OpenDemoScene();
		HAVTORN_API CAssetRegistry* GetAssetRegistry() const;

	private:
		CWorld() = default;
		~CWorld() = default;
		
		bool Init(CRenderManager* renderManager);
		void Update();

		void LoadScene(const std::string& filePath);

	private:
		std::vector<Ptr<CScene>> Scenes;
		std::vector<Ptr<ISystem>> Systems;
		Ptr<CAssetRegistry> AssetRegistry = nullptr;
		CRenderManager* RenderManager = nullptr;
	};
}