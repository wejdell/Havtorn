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
		CPlatformProcess(U16 windowPosX, U16 windowPosY, U16 windowWidth, U16 windowHeight);
		~CPlatformProcess() override;

		bool Init(CPlatformManager* platformManager) override;

		void BeginFrame() override;
		void PreUpdate() override;
		void Update() override;
		void PostUpdate() override;
		void EndFrame()override;

		class CPlatformManager* PlatformManager = nullptr;
	private:
		U16 WindowPositionX = 100;
		U16 WindowPositionY = 100;
		U16 WindowWidth = 1280;
		U16 WindowHeight = 720;
	};
}