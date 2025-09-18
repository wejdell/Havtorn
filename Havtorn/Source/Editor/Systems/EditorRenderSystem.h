// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include <ECS/System.h>
#include <Delegate.h>

namespace Havtorn
{
	class CRenderManager;
	class CWorld;
	class CEditorManager;
	struct SEntity;

	class CEditorRenderSystem : public ISystem
	{
	public:
		CEditorRenderSystem(CRenderManager* renderManager, CWorld* world, CEditorManager* editorManager);
		~CEditorRenderSystem() override = default;

		void Update(CScene* scene) override;

		void OnEntityPreDestroy(const SEntity entity);

	private:
		CEditorManager* Manager = nullptr;
		CRenderManager* RenderManager = nullptr;
		CWorld* World = nullptr;
		DelegateHandle Handle = {};
	};
}
