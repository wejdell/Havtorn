// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "ECS/System.h"

namespace Havtorn
{
	class CLevelStreamingSystem : public ISystem
	{
	public:
		CLevelStreamingSystem();
		~CLevelStreamingSystem() override = default;
		ENGINE_API void Update(std::vector<Ptr<CScene>>& scenes) override;

		void BindSceneLoader(const std::function<bool(const std::string&)>& loadingFunction);

	private:
		std::function<bool(const std::string&)> SceneLoader;
	};
}
