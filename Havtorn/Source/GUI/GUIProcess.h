// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <../Launcher/Application/Process.h>
#include <wtypes.h>
#include <memory>

#define EXPORT _declspec(dllexport)

struct ID3D11Device;
struct ID3D11DeviceContext;
class GUI;

namespace Havtorn
{
	class GUIProcess : public Havtorn::IProcess
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
		std::unique_ptr<GUI> EditorGUI;
	};
}