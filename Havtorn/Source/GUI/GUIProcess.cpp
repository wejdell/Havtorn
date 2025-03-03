// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "GUIProcess.h"
#include "imgui.h"
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include <ImGuizmo.h>

#include <string>

#include "GUI.h"


IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Havtorn
{
	GUIProcess::GUIProcess()
		: EditorGUI(new GUI())
	{

	}

	GUIProcess::~GUIProcess() = default;

	bool GUIProcess::Init(CPlatformManager* platformManager)
	{
		return true;
	}

	void GUIProcess::InitGUI(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		EditorGUI->InitGUI(hwnd, device, context);
	}

	void GUIProcess::BeginFrame()
	{
		EditorGUI->BeginFrame();
	}

	void GUIProcess::EndFrame()
	{
		EditorGUI->EndFrame();
	}

	//void GUIProcess::WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	//{
	//	EditorGUI->WindowsProc(hWnd, msg, wParam, lParam);
	//}
}