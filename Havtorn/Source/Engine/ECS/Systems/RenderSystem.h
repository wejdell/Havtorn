// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

namespace Havtorn
{
	class CRenderManager;
	class CWorld;
	struct SEntity;
	struct SComponent;

	class CRenderSystem final : public ISystem
	{
	public:
		CRenderSystem(CRenderManager* renderManager, CWorld* world);
		~CRenderSystem() override = default;

		void Update(std::vector<Ptr<CScene>>& scenes) override;

	private:
		CRenderManager* RenderManager = nullptr;
		CWorld* World = nullptr;
		DelegateHandle Handle = {};
	};
}
