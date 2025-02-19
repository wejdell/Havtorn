#include "GUI.h"
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <ImGuizmo.h>
#include <string>

#pragma region ImGui Implementation
GUI* GUI::Instance = nullptr;

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
class EXPORT GUI::ImGuiImpl
{
public:
	ImGuiImpl() = default;
	~ImGuiImpl() = default;

	void Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		ImGui::CreateContext();
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

public: //ImGui Functions
	void Begin(const char* name, bool* open)
	{
		ImGui::Begin(name, open);
	}

	void End()
	{
		ImGui::End();
	}

	void Text(const char* fmt, va_list args)
	{
		ImGui::TextV(fmt, args);
	}

	bool DragFloat(const char* label, float* value, float vSpeed = 1.0f, float min = 0.0f, float max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	{
		return ImGui::DragFloat(label, value, vSpeed, min, max, format, flags);
	}

	bool DragFloat2(const char* label, float* value, float vSpeed = 1.0f, float min = 0.0f, float max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	{
		return ImGui::DragFloat2(label, value, vSpeed, min, max, format, flags);
	}

	bool DragFloat3(const char* label, float* value, float vSpeed = 1.0f, float min = 0.0f, float max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	{
		return ImGui::DragFloat3(label, value, vSpeed, min, max, format, flags);
	}

	bool DragFloat4(const char* label, float* value, float vSpeed = 1.0f, float min = 0.0f, float max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
	{
		return ImGui::DragFloat4(label, value, vSpeed, min, max, format, flags);
	}

	bool InputFloat(const char* label, float* value, float step = 0.0f, float stepFast = 0.0f, const char* format = "%.3", ImGuiSliderFlags flags = 0)
	{
		return ImGui::InputFloat(label, value, step, stepFast, format, flags);
	}

	bool SliderFloat(const char* label, float* value, float min, float max, const char* format = "%.3", ImGuiSliderFlags flags = 0)
	{
		return ImGui::SliderFloat(label, value, min, max, format, flags);
	}

	bool CheckBox(const char* label, bool* v)
	{
		return ImGui::Checkbox(label, v);
	}

	void PushID(const char* str_id)
	{
		ImGui::PushID(str_id);
	}

	void PopID()
	{
		ImGui::PopID();
	}

	bool BeginMenu(const char* label, bool enabled)
	{
		return ImGui::BeginMenu(label, enabled);
	}

	void EndMenu()
	{
		ImGui::EndMenu();
	}

	bool BeginPopup(const char* str_id, ImGuiSliderFlags flags = 0)
	{
		ImGui::BeginPopup(str_id, flags);
	}

	void EndPopup()
	{
		ImGui::EndPopup();
	}

	bool BeginMainMenuBar()
	{
		return ImGui::BeginMainMenuBar();
	}

	void EndMainMenuBar()
	{
		ImGui::EndMainMenuBar();
	}
};
#pragma endregion

#pragma region GUI


GUI::GUI()
	: Impl(new ImGuiImpl())
{
	Instance = this;
}

GUI::~GUI()
{
	Impl = nullptr;
	Instance = nullptr;
}

void GUI::InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
{
	Impl->Init(hwnd, device, context);
}

void GUI::BeginFrame()
{
	Instance->Impl->BeginFrame();
}

void GUI::EndFrame()
{
	Instance->Impl->EndFrame();
}

void GUI::WindowsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	Instance->Impl->WindowProc(hwnd, msg, wParam, lParam);
}


void GUI::Begin(const char* name, bool* open)
{
	Instance->Impl->Begin(name, open);
}

void GUI::End()
{
	Instance->Impl->End();
}


void GUI::Text(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	Instance->Impl->Text(fmt, args);
	va_end(args);
}


bool GUI::DragFloat(const char* label, float* value, float min, float max, float speed)
{
	return Instance->Impl->DragFloat(label, value, speed, min, max);
}

bool GUI::DragFloat2(const char* label, float* value, float min, float max, float speed)
{
	return Instance->Impl->DragFloat2(label, value, speed, min, max);
}

bool GUI::DragFloat3(const char* label, float* value, float min, float max, float speed)
{
	return Instance->Impl->DragFloat3(label, value, speed, min, max);
}

bool GUI::DragFloat4(const char* label, float* value, float min, float max, float speed)
{
	return Instance->Impl->DragFloat4(label, value, speed, min, max);
}

bool GUI::InputFloat(const char* label, float* value, float step, float stepFast, const char* format)
{
	
	return Instance->Impl->InputFloat(label, value, step, stepFast, format);
}

bool GUI::SliderFloat(const char* label, float* value, float min, float max, const char* format)
{
	return Instance->Impl->SliderFloat(label, value, min, max, format);
}

void GUI::PushID(const char* str_id)
{
	Instance->Impl->PushID(str_id);
}

void GUI::PopID()
{
	Instance->Impl->PopID();
}

bool GUI::BeginMainMenuBar()
{
	return Instance->Impl->BeginMainMenuBar();
}

void GUI::EndMainMenuBar()
{
	Instance->Impl->EndMainMenuBar();
}

bool GUI::BeginMenu(const char* label, bool enabled)
{
	return Instance->Impl->BeginMenu(label, enabled);
}

void GUI::EndMenu()
{
	Instance->Impl->EndMenu();
}

bool GUI::BeginPopup(const char* str_id)
{
	return Instance->Impl->BeginPopup(str_id);
}

void GUI::EndPopup()
{
	Instance->Impl->EndPopup();
}

bool GUI::CheckBox(const char* label, bool* value)
{
	return Instance->Impl->CheckBox(label, value);
}

#pragma endregion