// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <../Launcher/Application/Process.h>

namespace Havtorn
{
	class HAVTORN_API CEngineProcess : public IProcess
	{
	public:
		CEngineProcess(U16 windowPosX, U16 windowPosY, U16 windowWidth, U16 windowHeight);
		~CEngineProcess() override;

		bool Init() override;

		void BeginFrame() override;
		void PreUpdate() override;
		void Update() override;
		void PostUpdate() override;
		void EndFrame()override;

	private:
		class CEngine* Engine = nullptr;

		U16 WindowPositionX = 100;
		U16 WindowPositionY = 100;
		U16 WindowWidth = 1280;
		U16 WindowHeight = 720;
	};
}
