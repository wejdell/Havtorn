// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "GUI.h"
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <ImGuizmo.h>

#include <../Engine/Core/CoreTypes.h>
#include <../Engine/Core/MathTypes/Vector.h>
#include <../Engine/Core/MathTypes/Matrix.h>

#include <string>

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Havtorn
{
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

		void TextDisabled(const char* fmt, va_list args)
		{
			ImGui::TextDisabled(fmt, args);
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

		bool DragInt2(const char* label, int* value, int vSpeed = 1.0f, int min = 0.0f, int max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
		{
			return ImGui::DragInt2(label, value, vSpeed, min, max, format, flags);
		}

		bool SliderInt(const char* label, int* value, int min, int max, const char* format = "%.3", ImGuiSliderFlags flags = 0)
		{
			return ImGui::SliderInt(label, value, min, max, format, flags);
		}

		bool ColorPicker3(const char* label, float* color)
		{
			return ImGui::ColorPicker3(label, color);
		}

		bool ColorPicker4(const char* label, float* color)
		{
			return ImGui::ColorPicker4(label, color);
		}

		bool Checkbox(const char* label, bool* v)
		{
			return ImGui::Checkbox(label, v);
		}

		void Separator()
		{
			ImGui::Separator();
		}

		void DecomposeMatrixToComponents(float* matrix, float* translation, float* rotation, float* scale)
		{
			ImGuizmo::DecomposeMatrixToComponents(matrix, translation, rotation, scale);
		}

		void RecomposeMatrixFromComponents(float* matrix, float* translation, float* rotation, float* scale)
		{
			ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);
		}

		void SetOrthographic(bool enabled)
		{
			return ImGuizmo::SetOrthographic(enabled);
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
			return ImGui::BeginPopup(str_id, flags);
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

		bool Button(const char* str_id)
		{
			return ImGui::Button(str_id);
		}
	};

	GUI* GUI::Instance = nullptr;

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

	const float GUI::SliderSpeed = 0.1f;
	const float GUI::TexturePreviewSizeX = 64.f;
	const float GUI::TexturePreviewSizeY = 64.f;
	const float GUI::DummySizeX = 0.0f;
	const float GUI::DummySizeY = 0.5f;
	const float GUI::ThumbnailPadding = 4.0f;
	const float GUI::PanelWidth = 256.0f;

	const char* GUI::SelectTextureModalName = "Select Texture Asset";

	//const ImVec2 GUI::TexturePreviewSize = { GUI::TexturePreviewSizeX, GUI::TexturePreviewSizeY };
	//const ImVec2 GUI::DummySize = { GUI::DummySizeX, GUI::DummySizeY };
	//const ImVec2 GUI::ModalWindowPivot = { 0.5f, 0.5f };

	bool GUI::TryOpenComponentView(const std::string& componentViewName)
	{
		return ImGui::CollapsingHeader(componentViewName.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
	}

	// NR: Just as with viewing components, components know how to add themselves and how to remove them. 
	// Figure out an abstraction that holds all of this. It should be easy to extend components with specific knowledge of them this way.
	// Just a lot of boilerplate. Try to introduce this in base class? Should probably include sequencer node behavior as well
	//TryAddComponent()

	//TryRemoveComponent()

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

	void GUI::TextDisabled(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		Instance->Impl->TextDisabled(fmt, args);
		va_end(args);
	}

	bool GUI::InputFloat(const char* label, float* value, float step, float stepFast, const char* format)
	{
		return Instance->Impl->InputFloat(label, value, step, stepFast, format);
	}

	bool GUI::DragFloat(const char* label, float* value, float speed, float min, float max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->DragFloat(label, value, speed, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::DragFloat2(const char* label, float* value, float speed, float min, float max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->DragFloat2(label, value, speed, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::DragFloat3(const char* label, SVector& value, float speed, float min, float max, const char* format, EDragMode dragMode)
	{
		F32* valueData;
		valueData[0] = value.X;
		valueData[1] = value.Y;
		valueData[2] = value.Z;
		const bool returnValue = Instance->Impl->DragFloat3(label, valueData, speed, min, max, format, static_cast<int>(dragMode));
		value = { valueData[0], valueData[1], valueData[2] };
		return returnValue;
	}

	bool GUI::DragFloat4(const char* label, float* value, float speed, float min, float max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->DragFloat4(label, value, speed, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::SliderFloat(const char* label, float* value, float min, float max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->SliderFloat(label, value, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::DragInt2(const char* label, int* value, int speed, int min, int max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->DragInt2(label, value, speed, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::SliderInt(const char* label, int* value, int min, int max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->SliderInt(label, value, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::ColorPicker3(const char* label, float* value)
	{
		return Instance->Impl->ColorPicker3(label, value);
	}

	bool GUI::ColorPicker4(const char* label, float* value)
	{
		return Instance->Impl->ColorPicker4(label, value);
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

	bool GUI::Button(const char* str_id)
	{
		return Instance->Impl->Button(str_id);
	}

	bool GUI::Checkbox(const char* str_id, bool* value)
	{
		return Instance->Impl->Checkbox(str_id, value);
	}

	void GUI::Separator()
	{
		return Instance->Impl->Separator();
	}

	void Havtorn::GUI::DecomposeMatrixToComponents(const SMatrix& matrix, SVector& translation, SVector& rotation, SVector& scale)
	{
		SMatrix matrixCopy = matrix;
		F32* matrixData = matrixCopy.data;
		F32 translationData[3], rotationData[3], scaleData[3];
		Instance->Impl->DecomposeMatrixToComponents(matrixData, translationData, rotationData, scaleData);
		translation = { translationData[0], translationData[1], translationData[2] };
		rotation = { rotationData[0], rotationData[1], rotationData[2] };
		scale = { scaleData[0], scaleData[1], scaleData[2] };
	}

	void GUI::RecomposeMatrixFromComponents(SMatrix& matrix, const SVector& translation, const SVector& rotation, const SVector& scale)
	{
		F32 translationData[3], rotationData[3], scaleData[3];
		translationData[0] = translation.X;
		translationData[1] = translation.Y;
		translationData[2] = translation.Z;
		rotationData[0] = rotation.X;
		rotationData[1] = rotation.Y;
		rotationData[2] = rotation.Z;
		scaleData[0] = scale.X;
		scaleData[1] = scale.Y;
		scaleData[2] = scale.Z;
		Instance->Impl->RecomposeMatrixFromComponents(matrix.data, translationData, rotationData, scaleData);
	}

	void GUI::SetOrthographic(const bool enabled)
	{
		return Instance->Impl->SetOrthographic(enabled);
	}
}