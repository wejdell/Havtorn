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
	//class _declspec(dllexport) GUIProcess::GUIPimpl
	//{
	//public: // Essentials to get things running
	//	GUIPimpl()
	//		: ImGuiContext(nullptr)
	//	{
	//	}
	//	~GUIPimpl()
	//	{
	//		ImGuiContext = nullptr;
	//	}

	//	void Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
	//	{
	//		ImGuiContext = ImGui::CreateContext();
	//		ImGui_ImplWin32_Init(hwnd);
	//		ImGui_ImplDX11_Init(device, context);
	//	}
	//	void BeginFrame()
	//	{
	//		ImGui_ImplDX11_NewFrame();
	//		ImGui_ImplWin32_NewFrame();
	//		ImGui::NewFrame();
	//		ImGuizmo::BeginFrame();
	//	}
	//	LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	//	{
	//		return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	//	}
	//	void EndFrame()
	//	{
	//		ImGui::Render();
	//		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	//	}

	//public: // ImGui Begin / End / Implementation / Wrapper

	//	void Begin(const std::string& name, bool* open, int flags) const
	//	{
	//		ImGui::Begin(name.c_str(), open, flags);
	//	}

	//	void End() const
	//	{
	//		ImGui::End();
	//	}

	//	void Text(const char* fmt, va_list args)
	//	{
	//		ImGui::TextV(fmt, args);
	//	}

	//	bool DragFloat(const std::string& label, float* value, float vSpeed = 1.0f, float min = 0.0f, float max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	//	{
	//		return ImGui::DragFloat(label.c_str(), value, vSpeed, min, max, format, flags);
	//	}

	//	bool DragFloat2(const std::string& label, float* value, float vSpeed = 1.0f, float min = 0.0f, float max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	//	{
	//		return ImGui::DragFloat2(label.c_str(), value, vSpeed, min, max, format, flags);
	//	}

	//	bool DragFloat3(const std::string& label, float* value, float vSpeed = 1.0f, float min = 0.0f, float max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	//	{
	//		return ImGui::DragFloat3(label.c_str(), value, vSpeed, min, max, format, flags);
	//	}

	//	bool DragFloat4(const std::string& label, float* value, float vSpeed = 1.0f, float min = 0.0f, float max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	//	{
	//		return ImGui::DragFloat4(label.c_str(), value, vSpeed, min, max, format, flags);
	//	}

	//	bool CheckBox(const char* label, bool* v)
	//	{
	//		return ImGui::Checkbox(label, v);
	//	}


	//private:
	//	ImGuiContext* ImGuiContext;

	//};

	//GUIProcess* GUIProcess::Instance = nullptr;

	GUIProcess::GUIProcess()
		//: Impl(new GUIPimpl())
		: EditorGUI(std::make_unique<GUI>())
	{
		//Instance = this;
	}

	GUIProcess::~GUIProcess() = default;

	bool GUIProcess::Init()
	{
		return true;
	}

	void GUIProcess::InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		EditorGUI->InitImGui(hwnd, device, context);
		//Impl->Init(hwnd, device, context);
	}

	void GUIProcess::BeginFrame()
	{
		EditorGUI->BeginFrame();
		//Impl->BeginFrame();
	}

	void GUIProcess::EndFrame()
	{
		EditorGUI->EndFrame();
		//Impl->EndFrame();
	}

	void GUIProcess::WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		EditorGUI->WindowsProc(hWnd, msg, wParam, lParam);
		//Impl->WindowProc(hWnd, msg, wParam, lParam);
	}

	//void GUIProcess::BeginGUI(const char* aName, bool* open, int flags)
	//{
	//	Instance->Impl->Begin(aName, open, flags);
	//}

	//void GUIProcess::EndGUI()
	//{
	//	Instance->Impl->End();
	//}

	//void GUIProcess::Text(const char* fmt, ...)
	//{
	//	va_list args;
	//	va_start(args, fmt);
	//	Instance->Impl->Text(fmt, args);
	//	va_end(args);
	//}

	//bool GUIProcess::DragFloat(const char* label, float* value, float min, float max, float speed)
	//{
	//	return Instance->Impl->DragFloat(label, value, speed, min, max);
	//}

	//bool GUIProcess::DragFloat2(const char* label, float* value, float min, float max, float speed)
	//{
	//	return Instance->Impl->DragFloat2(label, value, speed, min, max);
	//}

	//bool GUIProcess::DragFloat3(const char* label, float* value, float min, float max, float speed)
	//{
	//	return Instance->Impl->DragFloat3(label, value, speed, min, max);
	//}

	//bool GUIProcess::DragFloat4(const char* label, float* value, float min, float max, float speed)
	//{
	//	return Instance->Impl->DragFloat4(label, value, speed, min, max);
	//}
	//bool GUIProcess::CheckBox(const char* label, bool* v)
	//{
	//	return Instance->Impl->CheckBox(label, v);
	//}
}









//IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


//struct GUIPimpl::Impl
	//{
	//	ImGui::ImGuiContext* Context;
	//};

//	GUIProcess::GUIProcess()
	//	: name("HelloName")
	//	, impl(nullptr)
//	{

	//}

	//void GUIProcess::InitImGui()
	//{
	//	ImGui_ImplWin32_Init(nullptr);
	//	ImGui_ImplDX11_Init(nullptr, nullptr);
	//}

	//void GUIPimpl::Test()
	//{
		//ImGui::CreateContext();
		//ImGui::DebugCheckVersionAndDataLayout(IMGUI_VERSION,
		//									  sizeof(ImGuiIO),
		//									  sizeof(ImGuiStyle),
		//									  sizeof(ImVec2),
		//									  sizeof(ImVec4),
		//									  sizeof(ImDrawVert),
		//									  sizeof(ImDrawIdx));
	//}

	/*void GUIPimpl::NewFrame()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();
	}

	void GUIPimpl::EndFrame()
	{
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void GUIPimpl::WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	}

	bool GUIPimpl::WindowsInitDX11(const HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX11_Init(device, context);
		return true;
	}*/