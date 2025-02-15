// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include <../Launcher/Application/Process.h>


struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Havtorn
{

	struct HAVTORN_API SEngineRenderBackend
	{
		HWND hwnd;
		ID3D11Device* device;
		ID3D11DeviceContext* context;
	};

	class HAVTORN_API CEngineProcess : public IProcess
	{
	public:
		CEngineProcess(U16 windowPosX, U16 windowPosY, U16 windowWidth, U16 windowHeight);
		~CEngineProcess() override;

		bool Init() override;

		void HavtornWindowProc(IProcess* otherProcess);

		void BeginFrame() override;
		void PreUpdate() override;
		void Update() override;
		void PostUpdate() override;
		void EndFrame() override;

		SEngineRenderBackend GetRenderBackend();
		void WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;
		//static WNDPROC GetWindowsWindowProc() { return WinProc; }

	private:
		static WNDPROC WinProc;
		class GEngine* Engine = nullptr;
		U16 WindowPositionX = 100;
		U16 WindowPositionY = 100;
		U16 WindowWidth = 1280;
		U16 WindowHeight = 720;
		IProcess* OtherProcess = nullptr;
	};
}
