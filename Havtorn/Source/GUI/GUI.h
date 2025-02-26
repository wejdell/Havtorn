// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include <string>
#include <memory>
#include <wtypes.h>

#define EXPORT _declspec(dllexport)

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Havtorn
{
	struct SMatrix;
	struct SVector;

	enum class EXPORT EDragMode
	{
		None = 1 << 0,
		Logarithmic = 1 << 5,
	};

	class EXPORT GUI
	{
	public:
		GUI();
		~GUI();
		void InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

		void BeginFrame();
		void EndFrame();
		void WindowsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	public:
		static const float SliderSpeed;
		static const float TexturePreviewSizeX;
		static const float TexturePreviewSizeY;
		static const float DummySizeX;
		static const float DummySizeY;
		static const float ThumbnailPadding;
		static const float PanelWidth;

		static const char* SelectTextureModalName;

		//static const ImVec2 TexturePreviewSize;
		//static const ImVec2 DummySize;
		//static const ImVec2 ModalWindowPivot;

		static bool TryOpenComponentView(const std::string& componentViewName);

	public:
		static void Begin(const char* name, bool* open = 0);
		static void End();

		static void Text(const char* fmt, ...);
		static void TextDisabled(const char* fmt, ...);

		static bool InputFloat(const char* label, float* value, float step = 0.0f, float stepFast = 0.0f, const char* format = "%.3");
		static bool DragFloat(const char* label, float* value, float speed = 0.1f, float min = 0.0f, float max = 1.0f, const char* format = "%.3", EDragMode dragMode = EDragMode::None);
		static bool DragFloat2(const char* label, float* value, float speed = 0.1f, float min = 0.0f, float max = 1.0f, const char* format = "%.3", EDragMode dragMode = EDragMode::None);
		static bool DragFloat3(const char* label, SVector& value, float speed = 0.1f, float min = 0.0f, float max = 1.0f, const char* format = "%.3", EDragMode dragMode = EDragMode::None);
		static bool DragFloat4(const char* label, float* value, float speed = 0.1f, float min = 0.0f, float max = 1.0f, const char* format = "%.3", EDragMode dragMode = EDragMode::None);
		static bool SliderFloat(const char* label, float* value, float min = 0.0f, float max = 1.0f, const char* format = "%.3", EDragMode dragMode = EDragMode::None);

		static bool DragInt2(const char* label, int* value, int speed = 0.1f, int min = 0.0f, int max = 1.0f, const char* format = "%.3", EDragMode dragMode = EDragMode::None);
		static bool SliderInt(const char* label, int* value, int min = 0.0f, int max = 1.0f, const char* format = "%.3", EDragMode dragMode = EDragMode::None);

		static bool ColorPicker3(const char* label, float* value);
		static bool ColorPicker4(const char* label, float* value);

		static void PushID(const char* str_id);
		static void PopID();

		static bool BeginMainMenuBar();
		static void EndMainMenuBar();

		static bool BeginMenu(const char* label, bool enabled);
		static void EndMenu();

		static bool BeginPopup(const char* str_id);
		static void EndPopup();

		static bool Button(const char* str_id);
		static bool Checkbox(const char* str_id, bool* value);

		static void Separator();

		static void DecomposeMatrixToComponents(const SMatrix& matrix, SVector& translation, SVector& rotation, SVector& scale);
		static void RecomposeMatrixFromComponents(SMatrix& matrix, const SVector& translation, const SVector& rotation, const SVector& scale);
		static void SetOrthographic(const bool enabled);

		//static bool BeginPopupModal()
		//static void CloseCurrentPopup();

		//static bool BeginChild()
		//static void EndChild()

		//static bool BeginTable()
		//static void TableNextColumn()
		//static void EndTable()

		//static void TreeNode()
		//static void TreePop()

	private:
		class ImGuiImpl;
		ImGuiImpl* Impl;
		static GUI* Instance;
	};
}