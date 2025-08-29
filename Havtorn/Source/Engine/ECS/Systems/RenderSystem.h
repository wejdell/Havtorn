// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

namespace Havtorn
{
	class CRenderManager;
	class CWorld;
	struct SComponent;

	class CRenderSystem final : public ISystem
	{
	public:
		CRenderSystem(CRenderManager* renderManager, CWorld* world);
		~CRenderSystem() override = default;

		void Update(CScene* scene) override;

		U32 GetEditorTextureIndex(const SComponent* component);

	private:
		CRenderManager* RenderManager;
		CWorld* World;
	};
}