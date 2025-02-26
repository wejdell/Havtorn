// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include <../Launcher/Application/Process.h>
#include <wtypes.h>

#define EXPORT _declspec(dllexport)

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Havtorn
{
	class GUI;

	class GUIProcess : public IProcess
	{
	public:
		EXPORT GUIProcess();
		EXPORT ~GUIProcess() override;
		bool Init() override;

		EXPORT void InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

		void BeginFrame() override;
		void PreUpdate() override {}
		void Update() override {}
		void PostUpdate() override {}
		void EndFrame()override;

		void WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

	private:
		GUI* EditorGUI;
	};
}