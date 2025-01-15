// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <../Launcher/Application/Process.h>
#include <wtypes.h>
#include <memory>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace GUI
{
	class _declspec(dllexport) GUIProcess : public Havtorn::IProcess
	{
	public:
		GUIProcess();
		~GUIProcess() override;
		bool Init() override;

		void InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

		void BeginFrame() override;
		void PreUpdate() override	{ }
		void Update() override		{ }
		void PostUpdate() override	{ }
		void EndFrame()override;	
		
		void WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;		

		static void BeginGUI(const char* aName, bool* open = 0, int flags = 0);
		static void EndGUI();

	private:
		class _declspec(dllexport) GUIPimpl;
		GUIPimpl* Impl;

		static GUIProcess* Instance;
	};
}