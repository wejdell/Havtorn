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

		void Update(std::vector<Ptr<CScene>>& scenes) override;

		EDITOR_API void PushCommandsForScene(CScene* scene, const U64& renderViewID, const SMatrix& cameraMatrix);

		void AddEditorWidgetPass(STextureAsset* textureAsset, const std::vector<SMatrix>& transforms, const std::vector<SVector4>& uvRects, const std::vector<SVector4>& colors, const std::vector<SEntity>& entities);

		void OnEntityPreDestroy(const SEntity entity);

	private:
		CEditorManager* Manager = nullptr;
		CRenderManager* RenderManager = nullptr;
		CWorld* World = nullptr;
		DelegateHandle Handle = {};
	};
}
