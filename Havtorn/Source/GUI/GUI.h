#pragma once
#include <memory>
#include <wtypes.h>

#define EXPORT _declspec(dllexport)

struct ID3D11Device;
struct ID3D11DeviceContext;

class GUI
{
public:
	EXPORT GUI();
	EXPORT ~GUI();
	EXPORT void InitImGui(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context);

	void BeginFrame();
	void EndFrame();
	void WindowsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	static void Begin(const char* name, bool* open = 0);
	static void End();

	static void Text(const char* fmt, ...);
	
	static bool DragFloat(const char* label, float* value, float min, float max, float speed = 0.1f);
	static bool DragFloat2(const char* label, float* value, float min, float max, float speed = 0.1f);
	static bool DragFloat3(const char* label, float* value, float min, float max, float speed = 0.1f);
	static bool DragFloat4(const char* label, float* value, float min, float max, float speed = 0.1f);
	static bool InputFloat(const char* label, float* value, float step = 0.0f, float stepFast = 0.0f, const char* format = "%.3");
	static bool SliderFloat(const char* label, float* value, float min, float max, const char* format);

	static void PushID(const char* str_id);
	static void PopID();

	static bool BeginMainMenuBar();
	static void EndMainMenuBar();

	static bool BeginMenu(const char* label, bool enabled);
	static void EndMenu();

	//static bool BeginPopup()
	//static void EndPopup()

	//static bool BeginPopupModal()
	//static void CloseCurrentPopup();

	//static bool BeginChild()
	//static void EndChild()

	//static bool BeginTable()
	//static void TableNextColumn()
	//static void EndTable()

	//static void TreeNode()
	//static void TreePop()

	static bool CheckBox(const char* label, bool* v);

private:
	class EXPORT ImGuiImpl;
	std::unique_ptr<ImGuiImpl> Impl;
	static GUI* Instance;
};

