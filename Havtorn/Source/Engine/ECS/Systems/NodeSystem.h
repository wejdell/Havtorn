// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "ECS/System.h"

namespace Havtorn 
{
	// Temp used for testing!
	class CNodeSystem final : public ISystem {
	public:
		CNodeSystem(CScene* scene, class CRenderManager* renderManager);
		~CNodeSystem() override;

		void Update(CScene* scene) override;

	private:
	};
}
