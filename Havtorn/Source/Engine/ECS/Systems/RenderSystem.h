// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/System.h"
#include "Graphics/GraphicsStructs.h"

namespace Havtorn
{
	class CRenderManager;
	class CWorld;
	struct SEntity;
	struct SComponent;
	struct SSphere;
	struct SFrustum;

	class CRenderSystem final : public ISystem
	{
	public:
		CRenderSystem(CRenderManager* renderManager, CWorld* world);
		~CRenderSystem() override = default;

		void Update(std::vector<Ptr<CScene>>& scenes) override;
		
		// Camera Entity may be null
		ENGINE_API void PushCommandsForScene(CScene* scene, const U64& renderViewID, const SEntity& cameraEntity, const bool runEditorDataPasses, const bool doCulling) const;
		ENGINE_API void PushUniqueCommands(const U64& renderViewID) const;

		// TODO.NW: Light Culling
		// TODO.NW: Technically missing non-ui screen space sprite here, before tonemapping. Haven't really used that but supported it before the rework
		void AddPasses(SRenderView* renderView, const bool runEditorDataPasses);
		
		bool IsCulled(CScene* scene, const SSphere& boundingSphere, const SFrustum& cameraFrustum) const;

	private:
		CRenderManager* RenderManager = nullptr;
		CWorld* World = nullptr;
		DelegateHandle Handle = {};
	};
}
