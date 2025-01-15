#include "GUIProcess.h"
#include "imgui.h"
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>

#include <imgui_internal.h>

#include <ImGuizmo.h>

#include <string>


IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace GUI
{

	class _declspec(dllexport) GUIProcess::GUIPimpl
	{
	public: // Essentials to get things running
		void Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
		{
			ImGuiContext = ImGui::CreateContext();
			ImGui_ImplWin32_Init(hwnd);
			ImGui_ImplDX11_Init(device, context);
		}

		void BeginFrame()
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
			ImGuizmo::BeginFrame();
		}

		LRESULT WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
		}

		void EndFrame()
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

	public: // ImGui Begin / End / Implementation / Wrapper
		void Begin(const std::string& aName, bool* open, int flags) const
		{
			ImGui::Begin(aName.c_str(), open, flags);
		}

		void End() const
		{
			ImGui::End();
		}

		GUIPimpl()
		{

		}

		~GUIPimpl()
		{

		}

	private:
		ImGuiContext* ImGuiContext;

	};

	GUIProcess* GUIProcess::Instance = nullptr;
	GUIProcess::GUIProcess()
		: Impl(new GUIPimpl())
			//std::make_unique<GUIProcess::GUIPimpl>())
	{
		Instance = this;
		//Impl = std::make_unique<GUIProcess::GUIPimpl>();
	}
	GUIProcess::~GUIProcess() = default;

	bool GUIProcess::Init() { return true; }

	void GUIProcess::InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		Impl->Init(hwnd, device, context);
		//ImGui_ImplWin32_Init(hwnd);
		//ImGui_ImplDX11_Init(device, context);
	}

	void GUIProcess::BeginFrame()
	{
		Impl->BeginFrame();
		//ImGui_ImplDX11_NewFrame();
		//ImGui_ImplWin32_NewFrame();
		//ImGui::NewFrame();
		//ImGuizmo::BeginFrame();
	}

	void GUIProcess::EndFrame()
	{
		Impl->EndFrame();
		//ImGui::Render();
		//ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	void GUIProcess::WindowsWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		/*if (ImGui::GetCurrentContext() == nullptr)
			return;*/

		Impl->WindowProc(hWnd, msg, wParam, lParam);
	}

	void GUIProcess::BeginGUI(const char* aName, bool* open, int flags)
	{
		Instance->Impl->Begin(aName, open, flags);

		/*if (Instance == nullptr)
			return;

		ImGui::SetNextWindowSize(ImVec2(300, 100));
		ImGui::Begin(name);*/
	}

	void GUIProcess::EndGUI()
	{
		Instance->Impl->End();

		//if (Instance == nullptr)
		//	return;
		//
		//ImGui::End();
	}
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