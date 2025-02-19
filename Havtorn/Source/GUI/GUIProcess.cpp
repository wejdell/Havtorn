#include "GUIProcess.h"
#include "imgui.h"
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

//#include <imgui_internal.h>

#include <ImGuizmo.h>

#include <string>

#include "GUI.h"


IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Havtorn
{

	GUIProcess::GUIProcess()
		: EditorGUI(std::make_unique<GUI>())
	{

	}

	GUIProcess::~GUIProcess() = default;

	bool GUIProcess::Init()
	{
		return true;
	}

	void GUIProcess::InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		EditorGUI->InitImGui(hwnd, device, context);
	}

	void GUIProcess::BeginFrame()
	{
		EditorGUI->BeginFrame();
	}

	void GUIProcess::EndFrame()
	{
		EditorGUI->EndFrame();
	}

	void GUIProcess::WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		EditorGUI->WindowsProc(hWnd, msg, wParam, lParam);
	}
}