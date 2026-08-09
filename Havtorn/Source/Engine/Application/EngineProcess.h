// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <../Launcher/Application/Process.h>

namespace Havtorn
{
	class ENGINE_API CEngineProcess : public IProcess
	{
	public:
		CEngineProcess();
		~CEngineProcess() override;

		bool Init(CPlatformManager* platformManager) override;

		void BeginFrame() override;
		void Update() override;
		void PostUpdate() override;
		void EndFrame() override;

	private:
		class GEngine* Engine = nullptr;
	};
}
