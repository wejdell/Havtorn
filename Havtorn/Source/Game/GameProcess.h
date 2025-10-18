// Copyright 2023 Team Havtorn. All Rights Reserved.

#pragma once

#include <../Launcher/Application/Process.h>

namespace Havtorn
{
	class GAME_API CGameProcess : public IProcess
	{
	public:
		CGameProcess();
		~CGameProcess() override;

		bool Init(CPlatformManager* platformManager) override;
		void OnApplicationReady(const std::string& commandLine) override;

		void BeginFrame() override;
		void PreUpdate() override;
		void Update() override;
		void PostUpdate() override;
		void EndFrame()override;

	private:
		class CGameManager* GameManager = nullptr;
	};
}
