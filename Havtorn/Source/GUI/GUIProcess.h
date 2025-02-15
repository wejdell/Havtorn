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

		void InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

		void BeginFrame() override;
		void PreUpdate() override {}
		void Update() override {}
		void PostUpdate() override {}
		void EndFrame()override;

		void WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) override;

		//ImGui Wrap
		//static void BeginGUI(const char* aName, bool* open = 0, int flags = 0);
		//static void EndGUI();
		//static void Text(const char* fmt, ...);
		//static bool DragFloat(const char* label, float* value, float min, float max, float speed = 0.1f);
		//static bool DragFloat2(const char* label, float* value, float min, float max, float speed = 0.1f);
		//static bool DragFloat3(const char* label, float* value, float min, float max, float speed = 0.1f);
		//static bool DragFloat4(const char* label, float* value, float min, float max, float speed = 0.1f);
		//static bool CheckBox(const char* label, bool* v);

	private:
		std::unique_ptr<GUI> EditorGUI;
		//class _declspec(dllexport) GUIPimpl;
		//GUIPimpl* Impl;

		//static GUIProcess* Instance;
	};
}