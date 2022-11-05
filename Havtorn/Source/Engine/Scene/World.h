// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

namespace Havtorn
{
	struct SEntity;
	class ISystem;
	class CScene;
	class CRenderManager;

	class CWorld
	{
		friend class GEngine;

	public:
		void LoadScene(std::string filePath);
		HAVTORN_API std::vector<Ptr<CScene>>& GetActiveScenes();
		HAVTORN_API std::vector<Ref<SEntity>>& GetEntities() const;

	private:
		CWorld() = default;
		~CWorld() = default;
		
		bool Init(CRenderManager* renderManager);
		void Update();

	private:
		std::vector<Ptr<CScene>> Scenes;
		std::vector<Ptr<ISystem>> Systems;
		CRenderManager* RenderManager = nullptr;
	};
}