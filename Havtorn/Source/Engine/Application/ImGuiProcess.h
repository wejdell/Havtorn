// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <../Launcher/Application/Process.h>

namespace Havtorn
{
	class GImGuiManager;

	class HAVTORN_API CImGuiProcess : public IProcess
	{
	public:
		CImGuiProcess();
		~CImGuiProcess() override;

		bool Init() override;

		void BeginFrame() override;
		void PreUpdate() override;
		void Update() override;
		void PostUpdate() override;
		void EndFrame()override;

	private:
		GImGuiManager* ImGuiManager = nullptr;
	};
}
