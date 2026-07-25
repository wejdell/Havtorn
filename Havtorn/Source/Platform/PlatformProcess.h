// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <CoreTypes.h>
#include <../Launcher/Application/Process.h>

namespace Havtorn
{
	class CPlatformManager;

	class PLATFORM_API CPlatformProcess : public IProcess
	{
	public:
		CPlatformProcess();
		~CPlatformProcess() override;

		bool Init(CPlatformManager* platformManager) override;
		void OnApplicationReady() override;
		bool ShouldRun() override;

		void BeginFrame() override;
		void EndFrame() override;

		class CPlatformManager* PlatformManager = nullptr;
	};
}