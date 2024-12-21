// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

namespace Havtorn
{
	class CRenderManager;
	class CWorld;

	class CRenderSystem final : public ISystem
	{
	public:
		CRenderSystem(CRenderManager* renderManager, CWorld* world);
		~CRenderSystem() override = default;

		void Update(CScene* scene) override;
	private:
		CRenderManager* RenderManager;
		CWorld* World;
	};
}