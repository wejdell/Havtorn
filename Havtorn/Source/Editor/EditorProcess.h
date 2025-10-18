// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <Core.h>
#include <../Launcher/Application/Process.h>

namespace Havtorn
{
	class CPlatformManager;

	class EDITOR_API CEditorProcess : public IProcess
	{
	public:
		CEditorProcess();
		~CEditorProcess() override;

		bool Init(CPlatformManager* platformManager) override;

		void BeginFrame() override;
		void PostUpdate() override;
		void EndFrame() override;

	private:
		class CEditorManager* EditorManager = nullptr;
	};
}
