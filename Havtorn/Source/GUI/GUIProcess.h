// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include <Core.h>
#include <../Launcher/Application/Process.h>
#include <wtypes.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Havtorn
{
	class GUI;

	class GUIProcess : public IProcess
	{
	public:
		GUI_API GUIProcess();
		GUI_API ~GUIProcess() override;
		bool Init(CPlatformManager* platformManager) override;

		GUI_API void InitGUI(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

		void BeginFrame() override;
		void PreUpdate() override {}
		void Update() override {}
		void PostUpdate() override {}
		void EndFrame()override;

	private:
		GUI* EditorGUI;
	};
}