// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "GUI.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <ImGuizmo.h>
#include <d3d11.h>

#include <CoreTypes.h>
#include <MathTypes/Vector.h>
#include <MathTypes/Matrix.h>
#include <Color.h>

#include <string>
#include <Log.h>

#include <PlatformManager.h>

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Havtorn
{
	static Havtorn::I32 HavtornInputTextResizeCallback(struct ImGuiInputTextCallbackData* data)
	{
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			Havtorn::CHavtornStaticString<255>* customString = reinterpret_cast<Havtorn::CHavtornStaticString<255>*>(data->UserData);
			data->Buf = customString->Data();
			customString->SetLength(static_cast<Havtorn::U8>(data->BufSize) - 1);
		}
		return 0;
	}

	class GUI::ImGuiImpl
	{
	public:
		ImGuiImpl() = default;
		~ImGuiImpl() = default;

		void Init(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* context)
		{
			const char* Version = IMGUI_VERSION;
			const char* DefaultFont = "../External/imgui/misc/fonts/Roboto-Medium.ttf";
			const F32 DefaultFontSize = 15.0f;

			ImGui::CreateContext();
			ImGui::DebugCheckVersionAndDataLayout(Version, sizeof(ImGuiIO), sizeof(ImGuiStyle), sizeof(ImVec2), sizeof(ImVec4), sizeof(ImDrawVert), sizeof(ImDrawIdx));
			ImGui::GetIO().Fonts->AddFontFromFileTTF(DefaultFont, DefaultFontSize);
			ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());

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

		void SetGUIContext()
		{
			ImGuiMemAllocFunc memAlloc;														
			ImGuiMemFreeFunc memFree;														
			void* userData = nullptr;														
			ImGui::GetAllocatorFunctions(&memAlloc, &memFree, &userData);	
			ImGui::SetAllocatorFunctions(memAlloc, memFree, userData);						
			ImGui::SetCurrentContext(ImGui::GetCurrentContext());					
			ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
		}

		void EndFrame()
		{
			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}

	public:
		bool Begin(const char* name, bool* open, const std::vector<EWindowFlag>& flags)
		{
			int imFlags = 0;
			for (const EWindowFlag& flag : flags)
				imFlags += int(flag);

			return ImGui::Begin(name, open, imFlags);
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

		void TextUnformatted(const char* text)
		{
			ImGui::TextUnformatted(text);
		}

		void InputText(const char* label, char* buf, size_t bufSize, ImGuiInputTextCallback callback, void* data)
		{
			ImGui::InputText(label, buf, bufSize, 0, callback, data);
		}

		void SetTooltip(const char* fmt, ...)
		{
			va_list args;
			va_start(args, fmt);
			ImGui::SetTooltip(fmt, args);
			va_end(args);
		}

		bool DragFloat(const char* label, F32& value, F32 vSpeed = 1.0f, F32 min = 0.0f, F32 max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
		{
			return ImGui::DragFloat(label, &value, vSpeed, min, max, format, flags);
		}

		bool DragFloat2(const char* label, SVector2<F32>& value, F32 vSpeed = 1.0f, F32 min = 0.0f, F32 max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
		{
			F32 valueData[2] = { value.X, value.Y };
			const bool returnValue = ImGui::DragFloat2(label, valueData, vSpeed, min, max, format, flags);
			value = { valueData[0], valueData[1] };
			return returnValue;
		}

		bool DragFloat3(const char* label, SVector& value, F32 vSpeed = 1.0f, F32 min = 0.0f, F32 max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
		{
			F32 valueData[3] = { value.X, value.Y, value.Z };
			const bool returnValue = ImGui::DragFloat3(label, valueData, vSpeed, min, max, format, flags);
			value = { valueData[0], valueData[1], valueData[2] };
			return returnValue;
		}

		bool DragFloat4(const char* label, SVector4& value, F32 vSpeed = 1.0f, F32 min = 0.0f, F32 max = 1.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
		{
			F32 valueData[4] = { value.X, value.Y, value.Z, value.W };
			const bool returnValue = ImGui::DragFloat4(label, valueData, vSpeed, min, max, format, flags);
			value = { valueData[0], valueData[1], valueData[2], valueData[3] };
			return returnValue;
		}

		bool InputFloat(const char* label, F32& value, F32 step = 0.0f, F32 stepFast = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
		{
			return ImGui::InputFloat(label, &value, step, stepFast, format, flags);
		}

		bool SliderFloat(const char* label, F32& value, F32 min, F32 max, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
		{
			return ImGui::SliderFloat(label, &value, min, max, format, flags);
		}

		bool DragInt2(const char* label, SVector2<I32>& value, F32 vSpeed = 1.0f, int min = 0, int max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0)
		{
			I32 valueData[2] = { value.X, value.Y };
			const bool returnValue = ImGui::DragInt2(label, valueData, vSpeed, min, max, format, flags);
			value = { valueData[0], valueData[1] };
			return returnValue;
		}

		bool SliderInt(const char* label, I32& value, int min, int max, const char* format = "%d", ImGuiSliderFlags flags = 0)
		{
			return ImGui::SliderInt(label, &value, min, max, format, flags);
		}

		bool ColorPicker3(const char* label, SColor& color)
		{
			SVector colorFloat = color.AsVector();
			F32 valueData[3] = { colorFloat.X, colorFloat.Y, colorFloat.Z };
			const bool returnValue = ImGui::ColorPicker3(label, valueData);
			color = SColor(valueData[0], valueData[1], valueData[2], SColor::ToFloatRange(color.A));
			return returnValue;
		}

		bool ColorPicker4(const char* label, SColor& color)
		{
			SVector4 colorFloat = color.AsVector4();
			F32 valueData[4] = { colorFloat.X, colorFloat.Y, colorFloat.Z, colorFloat.W };
			const bool returnValue = ImGui::ColorPicker4(label, valueData);
			color = SColor(valueData[0], valueData[1], valueData[2], valueData[3]);
			return returnValue;
		}

		bool Checkbox(const char* label, bool& v)
		{
			return ImGui::Checkbox(label, &v);
		}

		bool Selectable(const char* label, const bool selected)
		{
			return ImGui::Selectable(label, selected);
		}

		void Image(intptr_t textureID, const SVector2<F32>& size, const SVector2<F32>& uv0, const SVector2<F32>& uv1, const SColor& tintColor, const SColor& borderColor)
		{
			ImVec2 imSize = { size.X, size.Y };
			ImVec2 imUV0 = { uv0.X, uv0.Y };
			ImVec2 imUV1 = { uv1.X, uv1.Y };
			SVector4 tintColorFloat = tintColor.AsVector4();
			ImVec4 imColorTint = { tintColorFloat.X, tintColorFloat.Y, tintColorFloat.Z, tintColorFloat.W };
			SVector4 borderColorFloat = borderColor.AsVector4();
			ImVec4 imColorBorder = { borderColorFloat.X, borderColorFloat.Y, borderColorFloat.Z, borderColorFloat.W };
			ImGui::Image((ImTextureID)textureID, imSize, imUV0, imUV1, imColorTint, imColorBorder);
		}

		void Separator()
		{
			ImGui::Separator();
		}
		
		void Dummy(const SVector2<F32>& size)
		{
			ImVec2 imSize = { size.X, size.Y };
			ImGui::Dummy(imSize);
		}

		void SameLine(F32 offsetFromStart = 0.0f, F32 spacing = -1.0f)
		{
			ImGui::SameLine(offsetFromStart, spacing);
		}

		bool IsItemHovered()
		{
			return ImGui::IsItemHovered();
		}

		F32 GetCursorPosX()
		{
			return ImGui::GetCursorPosX();
		}

		void SetCursorPosX(const F32 cursorPosX)
		{
			ImGui::SetCursorPosX(cursorPosX);
		}

		F32 GetScrollY()
		{
			return ImGui::GetScrollY();
		}

		F32 GetScrollMaxY()
		{
			return ImGui::GetScrollMaxY();
		}

		void SetScrollHereY(const F32 centerYRation)
		{
			ImGui::SetScrollHereY(centerYRation);
		}

		const SVector2<F32>& CalculateTextSize(const char* text)
		{
			const ImVec2& imTextSize = ImGui::CalcTextSize(text);
			return { imTextSize.x, imTextSize.y };
		}

		void SetItemDefaultFocus()
		{
			ImGui::SetItemDefaultFocus();
		}

		void SetKeyboardFocusHere(const I32 offset)
		{
			ImGui::SetKeyboardFocusHere(offset);
		}

		void PushStyleVar(const EStyleVar styleVar, const SVector2<F32>& value)
		{
			int imVar = 0;
			switch (styleVar)
			{
			case EStyleVar::WindowPadding:
				imVar += ImGuiStyleVar_WindowPadding;
				break;
			case EStyleVar::FramePadding:
				imVar += ImGuiStyleVar_FramePadding;
				break;
			case EStyleVar::ItemSpacing:
				imVar += ImGuiStyleVar_ItemSpacing;
				break;
			default:
				HV_ASSERT(false, "Unhandled case for EStyleVar, or using wrong value type");
				break;
			}

			ImVec2 imValue = { value.X, value.Y };
			ImGui::PushStyleVar(imVar, imValue);
		}

		void PushStyleVar(const EStyleVar styleVar, const F32 value)
		{
			int imVar = 0;
			switch (styleVar)
			{
			case EStyleVar::WindowRounding:
				imVar += ImGuiStyleVar_WindowRounding;
				break;
			case EStyleVar::WindowBorderSize:
				imVar += ImGuiStyleVar_WindowBorderSize;
				break;
			default:
				HV_ASSERT(false, "Unhandled case for EStyleVar, or using wrong value type");
				break;
			}

			ImGui::PushStyleVar(imVar, value);
		}

		void PopStyleVar(int count = 1)
		{
			ImGui::PopStyleVar(count);
		}

		SVector2<F32> GetStyleVar(const EStyleVar styleVar)
		{
			int imVar = static_cast<int>(styleVar);
			ImGuiStyleVar_ imStyleVar = static_cast<ImGuiStyleVar_>(imVar);
			ImVec2 value{};

			switch (imStyleVar)
			{
			case ImGuiStyleVar_WindowPadding:
				value = ImGui::GetStyle().WindowPadding;
			case ImGuiStyleVar_FramePadding:
				value = ImGui::GetStyle().FramePadding;
			case ImGuiStyleVar_ItemSpacing:
				value = ImGui::GetStyle().ItemSpacing;
			}
			
			return SVector2<F32>(value.x, value.y);
		}

		std::vector<SColor> GetStyleColors()
		{
			std::vector<SColor> colors;
			ImVec4* imColors = ImGui::GetStyle().Colors;
			for (U64 i = 0; i < 58; i++)
			{
				ImVec4 imColor = imColors[i];
				colors.emplace_back(SColor(imColor.x, imColor.y, imColor.z, imColor.w));
			}
			return colors;
		}

		void PushStyleColor(const EStyleColor styleColor, const SColor& color)
		{
			int imVar = static_cast<int>(styleColor);
			SVector4 colorFloat = color.AsVector4();
			ImVec4 imValue = { colorFloat.X, colorFloat.Y, colorFloat.Z, colorFloat.W };
			ImGui::PushStyleColor(imVar, imValue);
		}

		void PopStyleColor()
		{
			ImGui::PopStyleColor();
		}

		void DecomposeMatrixToComponents(F32* matrix, F32* translation, F32* rotation, F32* scale)
		{
			ImGuizmo::DecomposeMatrixToComponents(matrix, translation, rotation, scale);
		}

		void RecomposeMatrixFromComponents(F32* matrix, F32* translation, F32* rotation, F32* scale)
		{
			ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, matrix);
		}

		void SetOrthographic(bool enabled)
		{
			return ImGuizmo::SetOrthographic(enabled);
		}

		bool IsOverGizmo()
		{
			return ImGuizmo::IsOver();
		}

		F32 GetTextLineHeight()
		{
			return ImGui::GetTextLineHeight();
		}

		SVector2<F32> GetCursorScreenPos()
		{
			auto pos = ImGui::GetCursorScreenPos();
			return SVector2<F32>(pos.x, pos.y);
		}

		SVector2<F32> GetViewportWorkPos()
		{
			auto pos = ImGui::GetMainViewport()->WorkPos;
			return SVector2<F32>(pos.x, pos.y);
		}

		SVector2<F32> GetViewportCenter()
		{
			auto pos = ImGui::GetMainViewport()->GetCenter();
			return SVector2<F32>(pos.x, pos.y);
		}

		SVector2<F32> GetWindowContentRegionMin()
		{
			auto min = ImGui::GetWindowContentRegionMin();
			return SVector2<F32>(min.x, min.y);
		}

		SVector2<F32> GetWindowContentRegionMax()
		{
			auto max = ImGui::GetWindowContentRegionMax();
			return SVector2<F32>(max.x, max.y);
		}

		SVector2<F32> GetContentRegionAvail()
		{
			auto avail = ImGui::GetContentRegionAvail();
			return SVector2<F32>(avail.x, avail.y);
		}

		F32 GetFrameHeightWithSpacing()
		{
			return ImGui::GetFrameHeightWithSpacing();
		}

		void SetNextWindowPos(const SVector2<F32>& pos, const EWindowCondition condition, const SVector2<F32>& pivot)
		{
			ImVec2 imPos = { pos.X, pos.Y };
			ImGuiCond imCondition = ImGuiCond(condition);
			ImVec2 imPivot = { pivot.X, pivot.Y };
			ImGui::SetNextWindowPos(imPos, imCondition, imPivot);
		}

		void SetNextWindowSize(const SVector2<F32>& size)
		{
			ImVec2 imSize = { size.X, size.Y };
			ImGui::SetNextWindowSize(imSize);
		}

		void SetRect(const SVector2<F32>& position, const SVector2<F32>& dimensions)
		{
			ImGuizmo::SetRect(position.X, position.Y, dimensions.X, dimensions.Y);
		}

		void SetGizmoDrawList()
		{
			ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
		}

		SVector2<F32> GetCurrentWindowSize()
		{
			ImVec2 imWindowSize = ImGui::GetWindowSize();
			return { imWindowSize.x, imWindowSize.y };
		}

		void PushID(const char* label)
		{
			ImGui::PushID(label);
		}

		void PushID(int int_id)
		{
			ImGui::PushID(int_id);
		}

		void PopID()
		{
			ImGui::PopID();
		}

		int GetID(const char* label)
		{
			return ImGui::GetID(label);
		}

		bool BeginMenu(const char* label, bool enabled)
		{
			return ImGui::BeginMenu(label, enabled);
		}

		void EndMenu()
		{
			ImGui::EndMenu();
		}

		bool MenuItem(const char* label, const char* shortcut = (const char*)0, const bool selected = true, const bool enabled = true)
		{
			return ImGui::MenuItem(label, shortcut, selected, enabled);
		}

		bool BeginPopup(const char* label, ImGuiSliderFlags flags = 0)
		{
			return ImGui::BeginPopup(label, flags);
		}

		void EndPopup()
		{
			ImGui::EndPopup();
		}

		bool BeginPopupModal(const char* label, bool* open = 0, const std::vector<EWindowFlag>& flags = {})
		{
			ImGuiWindowFlags windowFlags = 0;
			for (const EWindowFlag& flag : flags)
				windowFlags += int(flag);

			return ImGui::BeginPopupModal(label, open, windowFlags);
		}

		void CloseCurrentPopup()
		{
			ImGui::CloseCurrentPopup();
		}

		bool BeginChild(const char* label, const SVector2<F32>& size, const std::vector<EChildFlag>& childFlags, const std::vector<EWindowFlag>& windowFlags)
		{
			int imChildFlags = 0;
			for (const EChildFlag& childFlag : childFlags)
				imChildFlags += int(childFlag);

			int imWindowFlags = 0;
			for (const EWindowFlag& windowFlag : windowFlags)
				imWindowFlags += int(windowFlag);

			ImVec2 imSize = { size.X, size.Y };

			return ImGui::BeginChild(label, imSize, imChildFlags, imWindowFlags);
		}

		void EndChild()
		{
			ImGui::EndChild();
		}

		bool BeginPopupContextWindow()
		{
			return ImGui::BeginPopupContextWindow();
		}

		void OpenPopup(const char* label)
		{
			ImGui::OpenPopup(label);
		}

		bool BeginTable(const char* label, const I32 columns)
		{
			return ImGui::BeginTable(label, columns);
		}

		void TableNextColumn()
		{
			ImGui::TableNextColumn();
		}

		void EndTable()
		{
			return ImGui::EndTable();
		}

		bool TreeNode(const char* strID)
		{
			return ImGui::TreeNode(strID);
		}

		void TreePop()
		{
			ImGui::TreePop();
		}

		bool BeginMainMenuBar()
		{
			return ImGui::BeginMainMenuBar();
		}

		void EndMainMenuBar()
		{
			ImGui::EndMainMenuBar();
		}

		bool ArrowButton(const char* label, const EGUIDirection direction)
		{
			const ImGuiDir imDirection = (ImGuiDir)direction;
			return ImGui::ArrowButton(label, imDirection);
		}

		bool Button(const char* label, const SVector2<F32>& size)
		{
			ImVec2 imSize = { size.X, size.Y };
			return ImGui::Button(label, imSize);
		}

		bool SmallButton(const char* label)
		{
			return ImGui::SmallButton(label);
		}

		bool RadioButton(const char* label, bool active)
		{
			return ImGui::RadioButton(label, active);
		}

		bool ImageButton(const char* label, intptr_t textureID, const SVector2<F32>& size, const SVector2<F32>& uv0, const SVector2<F32>& uv1, const SColor& backgroundColor, const SColor& tintColor)
		{
			ImVec2 imSize = { size.X, size.Y };
			ImVec2 imUV0 = { uv0.X, uv0.Y };
			ImVec2 imUV1 = { uv1.X, uv1.Y };
			SVector4 backgroundColorFloat = backgroundColor.AsVector4();
			ImVec4 imColorBackground = { backgroundColorFloat.X, backgroundColorFloat.Y, backgroundColorFloat.Z, backgroundColorFloat.W };
			SVector4 tintColorFloat = tintColor.AsVector4();
			ImVec4 imColorTint = { tintColorFloat.X, tintColorFloat.Y, tintColorFloat.Z, tintColorFloat.W };
			return ImGui::ImageButton(label, (ImTextureID)(textureID), imSize, imUV0, imUV1, imColorBackground, imColorTint);
		}

		void AddRectFilled(const SVector2<F32>& cursorPos, const SVector2<F32>& size, const SColor& color)
		{
			ImVec2 posMin = { cursorPos.X, cursorPos.Y };
			ImVec2 posMax = { cursorPos.X + size.X, cursorPos.Y + size.Y };
			SVector4 colorFloat = color.AsVector4();
			ImVec4 imColor = { colorFloat.X, colorFloat.Y, colorFloat.Z, colorFloat.W };
			ImGui::GetWindowDrawList()->AddRectFilled(posMin, posMax, ImGui::ColorConvertFloat4ToU32(imColor));
		}

		void SetGuiColorProfile(const SGuiColorProfile& colorProfile)
		{
			ImVec4* colors = (&ImGui::GetStyle())->Colors;

			auto convert = [](const SColor& color) { SVector4 colorFloat = color.AsVector4(); return ImVec4(colorFloat.X, colorFloat.Y, colorFloat.Z, colorFloat.W); };

			colors[ImGuiCol_Text] = convert(colorProfile.Text);
			colors[ImGuiCol_TextDisabled] = convert(colorProfile.TextDisabled);
			colors[ImGuiCol_WindowBg] = convert(colorProfile.WindowBg);
			colors[ImGuiCol_ChildBg] = convert(colorProfile.ChildBg);
			colors[ImGuiCol_PopupBg] = convert(colorProfile.PopupBg);
			colors[ImGuiCol_Border] = convert(colorProfile.Border);
			colors[ImGuiCol_BorderShadow] = convert(colorProfile.BorderShadow);
			colors[ImGuiCol_FrameBg] = convert(colorProfile.FrameBg);
			colors[ImGuiCol_FrameBgHovered] = convert(colorProfile.FrameBgHovered);
			colors[ImGuiCol_FrameBgActive] = convert(colorProfile.FrameBgActive);
			colors[ImGuiCol_TitleBg] = convert(colorProfile.TitleBg);
			colors[ImGuiCol_TitleBgActive] = convert(colorProfile.TitleBgActive);
			colors[ImGuiCol_TitleBgCollapsed] = convert(colorProfile.TitleBgCollapsed);
			colors[ImGuiCol_MenuBarBg] = convert(colorProfile.MenuBarBg);
			colors[ImGuiCol_ScrollbarBg] = convert(colorProfile.ScrollbarBg);
			colors[ImGuiCol_ScrollbarGrab] = convert(colorProfile.ScrollbarGrab);
			colors[ImGuiCol_ScrollbarGrabHovered] = convert(colorProfile.ScrollbarGrabHovered);
			colors[ImGuiCol_ScrollbarGrabActive] = convert(colorProfile.ScrollbarGrabActive);
			colors[ImGuiCol_CheckMark] = convert(colorProfile.CheckMark);
			colors[ImGuiCol_SliderGrab] = convert(colorProfile.SliderGrab);
			colors[ImGuiCol_SliderGrabActive] = convert(colorProfile.SliderGrabActive);
			colors[ImGuiCol_Button] = convert(colorProfile.Button);
			colors[ImGuiCol_ButtonHovered] = convert(colorProfile.ButtonHovered);
			colors[ImGuiCol_ButtonActive] = convert(colorProfile.ButtonActive);
			colors[ImGuiCol_Header] = convert(colorProfile.Header);
			colors[ImGuiCol_HeaderHovered] = convert(colorProfile.HeaderHovered);
			colors[ImGuiCol_HeaderActive] = convert(colorProfile.HeaderActive);
			colors[ImGuiCol_Separator] = convert(colorProfile.Separator);
			colors[ImGuiCol_SeparatorHovered] = convert(colorProfile.SeparatorHovered);
			colors[ImGuiCol_SeparatorActive] = convert(colorProfile.ScrollbarGrabActive);
			colors[ImGuiCol_ResizeGrip] = convert(colorProfile.ResizeGrip);
			colors[ImGuiCol_ResizeGripHovered] = convert(colorProfile.ResizeGripHovered);
			colors[ImGuiCol_ResizeGripActive] = convert(colorProfile.ResizeGripActive);
			colors[ImGuiCol_Tab] = convert(colorProfile.Tab);
			colors[ImGuiCol_TabHovered] = convert(colorProfile.TabHovered);
			colors[ImGuiCol_TabSelected] = convert(colorProfile.TabSelected);
			colors[ImGuiCol_TabSelectedOverline] = convert(colorProfile.TabSelected);
			colors[ImGuiCol_TabDimmed] = convert(colorProfile.TabDimmed);
			colors[ImGuiCol_TabDimmedSelected] = convert(colorProfile.TabDimmedSelected);
			colors[ImGuiCol_TabDimmedSelectedOverline] = convert(colorProfile.TabDimmedSelected);
			colors[ImGuiCol_DockingPreview] = convert(colorProfile.TitleBgActive);
			colors[ImGuiCol_PlotLines] = convert(colorProfile.PlotLines);
			colors[ImGuiCol_PlotLinesHovered] = convert(colorProfile.PlotLinesHovered);
			colors[ImGuiCol_PlotHistogram] = convert(colorProfile.PlotHistogram);
			colors[ImGuiCol_PlotHistogramHovered] = convert(colorProfile.PlotHistogramHovered);
			colors[ImGuiCol_TextSelectedBg] = convert(colorProfile.TextSelectedBg);
			colors[ImGuiCol_DragDropTarget] = convert(colorProfile.DragDropTarget);
			colors[ImGuiCol_NavHighlight] = convert(colorProfile.NavHighlight);
			colors[ImGuiCol_NavWindowingHighlight] = convert(colorProfile.NavWindowHighlight);
			colors[ImGuiCol_NavWindowingDimBg] = convert(colorProfile.NavWindowDimBg);
			colors[ImGuiCol_ModalWindowDimBg] = convert(colorProfile.ModalWindowDimBg);
		}

		void SetImGuiStyleProfile(const SGuiStyleProfile& styleProfile)
		{
			ImGuiStyle* style = &ImGui::GetStyle();

			auto convert = [](const SVector2<F32>& value) { return ImVec2(value.X, value.Y); };

			style->WindowPadding = convert(styleProfile.WindowPadding);
			style->FramePadding = convert(styleProfile.FramePadding);
			style->CellPadding = convert(styleProfile.CellPadding);
			style->ItemSpacing = convert(styleProfile.ItemSpacing);
			style->ItemInnerSpacing = convert(styleProfile.ItemInnerSpacing);
			style->TouchExtraPadding = convert(styleProfile.TouchExtraPadding);
			style->IndentSpacing = styleProfile.IndentSpacing;
			style->ScrollbarSize = styleProfile.ScrollbarSize;
			style->GrabMinSize = styleProfile.GrabMinSize;
			style->WindowBorderSize = styleProfile.WindowBorderSize;
			style->ChildBorderSize = styleProfile.ChildBorderSize;
			style->PopupBorderSize = styleProfile.PopupBorderSize;
			style->FrameBorderSize = styleProfile.FrameBorderSize;
			style->TabBorderSize = styleProfile.TabBorderSize;
			style->WindowRounding = styleProfile.WindowRounding;
			style->ChildRounding = styleProfile.ChildRounding;
			style->FrameRounding = styleProfile.FrameRounding;
			style->PopupRounding = styleProfile.PopupRounding;
			style->ScrollbarRounding = styleProfile.ScrollbarRounding;
			style->GrabRounding = styleProfile.GrabRounding;
			style->LogSliderDeadzone = styleProfile.LogSliderDeadzone;
			style->TabRounding = styleProfile.TabRounding;
		}

		void GizmoManipulate(const F32* view, const F32* projection, ImGuizmo::OPERATION operation, ImGuizmo::MODE mode, F32* matrix, F32* deltaMatrix, const F32* snap, const F32* localBounds, const F32* boundsSnap)
		{
			ImGuizmo::Manipulate(view, projection, operation, mode, matrix, deltaMatrix, snap, localBounds, boundsSnap);
		}

		void ViewManipulate(F32* view, F32 length, const SVector2<F32>& position, const SVector2<F32>& size, const SColor& color)
		{
			ImVec2 imPos = { position.X, position.Y };
			ImVec2 imSize = { size.X, size.Y };
			SVector4 colorFloat = color.AsVector4();
			ImU32 imColor = ImGui::ColorConvertFloat4ToU32(ImVec4{ colorFloat.X, colorFloat.Y, colorFloat.Z, colorFloat.W });
			ImGuizmo::ViewManipulate(view, length, imPos, imSize, imColor);
		}

		bool IsDockingEnabled()
		{
			const ImGuiIO& io = ImGui::GetIO();
			return io.ConfigFlags & ImGuiConfigFlags_DockingEnable;
		}

		void DockSpace(const U32 id, const SVector2<F32>& size, const EDockNodeFlag dockNodeFlag)
		{
			ImVec2 imSize = { size.X, size.Y };
			int flags = (ImGuiDockNodeFlags_)dockNodeFlag;
			ImGui::DockSpace(id, imSize, flags);
		}

		void DockBuilderAddNode(U32 id, const std::vector<EDockNodeFlag>& flags)
		{
			I32 imFlags = 0;
			for (EDockNodeFlag flag : flags)
				imFlags += STATIC_U32(flag);

			ImGui::DockBuilderAddNode(id, imFlags);
		}

		void DockBuilderRemoveNode(U32 id)
		{
			ImGui::DockBuilderRemoveNode(id);
		}

		void DockBuilderSetNodeSize(U32 id, const SVector2<F32>& size)
		{
			ImVec2 imSize = { size.X, size.Y };
			ImGui::DockBuilderSetNodeSize(id, imSize);
		}

		void DockBuilderDockWindow(const char* label, U32 id)
		{
			ImGui::DockBuilderDockWindow(label, id);
		}

		void DockBuilderFinish(U32 id)
		{
			ImGui::DockBuilderFinish(id);
		}

		void LogToClipboard()
		{
			ImGui::LogToClipboard();
		}

		void LogFinish()
		{
			ImGui::LogFinish();
		}

		int ColorConvertFloat4ToU32(const ImVec4& color)
		{
			return ImGui::ColorConvertFloat4ToU32(color);
		}

		void MemFree(void* ptr)
		{
			ImGui::MemFree(ptr);
		}

		void ShowDemoWindow(bool* open)
		{
			ImGui::ShowDemoWindow(open);
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

	void GUI::InitGUI(CPlatformManager* platformManager, ID3D11Device* device, ID3D11DeviceContext* context)
	{
		Impl->Init(platformManager->GetWindowHandle(), device, context);
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		platformManager->OnMessageHandled.AddMember(this, &GUI::WindowsProc);
	}

	const F32 GUI::SliderSpeed = 0.1f;
	const F32 GUI::TexturePreviewSizeX = 64.f;
	const F32 GUI::TexturePreviewSizeY = 64.f;
	const F32 GUI::DummySizeX = 0.0f;
	const F32 GUI::DummySizeY = 0.5f;
	const F32 GUI::ThumbnailSizeX = 64.0f;
	const F32 GUI::ThumbnailSizeY = 64.0f;
	const F32 GUI::ThumbnailPadding = 4.0f;
	const F32 GUI::PanelWidth = 256.0f;

	const char* GUI::SelectTextureModalName = "Select Texture Asset";

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

	void GUI::SetGUIContext()
	{
		Instance->Impl->SetGUIContext();
	}

	bool GUI::Begin(const char* name, bool* open, const std::vector<EWindowFlag>& flags)
	{
		return Instance->Impl->Begin(name, open, flags);
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

	void GUI::TextUnformatted(const char* text)
	{
		Instance->Impl->TextUnformatted(text);
	}

	void GUI::InputText(const char* label, CHavtornStaticString<255>* customString)
	{
		return Instance->Impl->InputText(label, customString->Data(), (size_t)customString->Length() + 1, HavtornInputTextResizeCallback, (void*)customString);
	}

	void GUI::SetTooltip(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		Instance->Impl->SetTooltip(fmt, args);
		va_end(args);
	}

	bool GUI::InputFloat(const char* label, F32& value, F32 step, F32 stepFast, const char* format)
	{
		return Instance->Impl->InputFloat(label, value, step, stepFast, format);
	}

	bool GUI::DragFloat(const char* label, F32& value, F32 speed, F32 min, F32 max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->DragFloat(label, value, speed, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::DragFloat2(const char* label, SVector2<F32>& value, F32 speed, F32 min, F32 max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->DragFloat2(label, value, speed, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::DragFloat3(const char* label, SVector& value, F32 speed, F32 min, F32 max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->DragFloat3(label, value, speed, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::DragFloat4(const char* label, SVector4& value, F32 speed, F32 min, F32 max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->DragFloat4(label, value, speed, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::SliderFloat(const char* label, F32& value, F32 min, F32 max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->SliderFloat(label, value, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::DragInt2(const char* label, SVector2<I32>& value, F32 speed, int min, int max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->DragInt2(label, value, speed, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::SliderInt(const char* label, I32& value, int min, int max, const char* format, EDragMode dragMode)
	{
		return Instance->Impl->SliderInt(label, value, min, max, format, static_cast<int>(dragMode));
	}

	bool GUI::ColorPicker3(const char* label, SColor& value)
	{
		return Instance->Impl->ColorPicker3(label, value);
	}

	bool GUI::ColorPicker4(const char* label, SColor& value)
	{
		return Instance->Impl->ColorPicker4(label, value);
	}

	void GUI::PushID(const char* label)
	{
		Instance->Impl->PushID(label);
	}

	void GUI::PushID(I32 intID)
	{
		Instance->Impl->PushID(intID);
	}

	void GUI::PopID()
	{
		Instance->Impl->PopID();
	}

	I32 GUI::GetID(const char* label)
	{
		return Instance->Impl->GetID(label);
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

	bool GUI::MenuItem(const char* label, const char* shortcut, const bool selected, const bool enabled)
	{
		return Instance->Impl->MenuItem(label, shortcut, selected, enabled);
	}

	bool GUI::BeginPopup(const char* label)
	{
		return Instance->Impl->BeginPopup(label);
	}

	void GUI::EndPopup()
	{
		Instance->Impl->EndPopup();
	}

	bool GUI::BeginPopupModal(const char* label, bool* open, const std::vector<EWindowFlag>& flags)
	{
		return Instance->Impl->BeginPopupModal(label, open, flags);
	}

	void GUI::CloseCurrentPopup()
	{
		Instance->Impl->CloseCurrentPopup();
	}

	bool GUI::BeginChild(const char* label, const SVector2<F32>& size, const std::vector<EChildFlag>& childFlags, const std::vector<EWindowFlag>& windowFlags)
	{
		return Instance->Impl->BeginChild(label, size, childFlags, windowFlags);
	}

	void GUI::EndChild()
	{
		Instance->Impl->EndChild();
	}

	bool GUI::BeginPopupContextWindow()
	{
		return Instance->Impl->BeginPopupContextWindow();
	}

	void GUI::OpenPopup(const char* label)
	{
		Instance->Impl->OpenPopup(label);
	}

	bool GUI::BeginTable(const char* label, const I32 columns)
	{
		return Instance->Impl->BeginTable(label, columns);
	}

	void GUI::TableNextColumn()
	{
		Instance->Impl->TableNextColumn();
	}

	void GUI::EndTable()
	{
		return Instance->Impl->EndTable();
	}

	bool GUI::TreeNode(const char* label)
	{
		return Instance->Impl->TreeNode(label);
	}

	void GUI::TreePop()
	{
		Instance->Impl->TreePop();
	}

	bool GUI::ArrowButton(const char* label, const EGUIDirection direction)
	{
		return Instance->Impl->ArrowButton(label, direction);
	}

	bool GUI::Button(const char* label, const SVector2<F32>& size)
	{
		return Instance->Impl->Button(label, size);
	}

	bool GUI::SmallButton(const char* label)
	{
		return Instance->Impl->SmallButton(label);
	}

	bool GUI::RadioButton(const char* label, bool active)
	{
		return Instance->Impl->RadioButton(label, active);
	}

	bool GUI::ImageButton(const char* label, intptr_t imageRef, const SVector2<F32>& size, const SVector2<F32>& uv0, const SVector2<F32>& uv1, const SColor& backgroundColor, const SColor& tintColor)
	{
		return Instance->Impl->ImageButton(label, imageRef, size, uv0, uv1, backgroundColor, tintColor);
	}

	bool GUI::Checkbox(const char* label, bool& value)
	{
		return Instance->Impl->Checkbox(label, value);
	}

	SAssetPickResult GUI::AssetPicker(const char* label, const char* modalLabel, intptr_t image, const std::string& directory, I32 columns, const std::function<SAssetInspectionData(std::filesystem::directory_entry)>& assetInspector)
	{
		if (GUI::ImageButton("AssetPicker", image, { GUI::TexturePreviewSizeX * 0.75f, GUI::TexturePreviewSizeY * 0.75f }))
		{
			GUI::OpenPopup(modalLabel);
			GUI::SetNextWindowPos(GUI::GetViewportCenter(), EWindowCondition::Appearing, SVector2<F32>(0.5f, 0.5f));
		}

		GUI::Text(label);

		if (!GUI::BeginPopupModal(modalLabel, NULL, { EWindowFlag::AlwaysAutoResize }))
			return SAssetPickResult(false);

		if (!GUI::BeginTable("AssetPickerTable", columns))
		{
			GUI::EndPopup();
			return SAssetPickResult(false);
		}

		I32 id = 0;
		for (auto& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			if (entry.is_directory())
				continue;

			GUI::TableNextColumn();
			GUI::PushID(id++);

			auto data = assetInspector(entry);
			if (GUI::ImageButton(data.Name.c_str(), data.TextureRef, { GUI::TexturePreviewSizeX * 0.75f, GUI::TexturePreviewSizeY * 0.75f }))
			{
				GUI::PopID();
				GUI::EndTable();
				GUI::EndPopup();
				GUI::CloseCurrentPopup();
				return SAssetPickResult(false, true, entry);
			}

			GUI::Text(data.Name.c_str());
			GUI::PopID();
		}

		GUI::EndTable();

		// TODO.NW: Make util for centering elements. Look at AssetBrowserWindow for full implementation
		// Center buttons
		F32 width = 0.0f;
		width += GUI::CalculateTextSize("Cancel").X + GUI::ThumbnailPadding;
		F32 avail = GUI::GetContentRegionAvail().X;
		F32 off = (avail - width) * 0.5f;
		GUI::SetCursorPosX(GUI::GetCursorPosX() + off);

		if (GUI::Button("Cancel"))	
			GUI::CloseCurrentPopup();

		GUI::EndPopup();
		return SAssetPickResult(true);
	}

	bool GUI::Selectable(const char* label, const bool selected)
	{
		return Instance->Impl->Selectable(label, selected);
	}

	void GUI::Image(intptr_t image, const SVector2<F32>& size, const SVector2<F32>& uv0, const SVector2<F32>& uv1, const SColor& tintColor, const SColor& borderColor)
	{
		Instance->Impl->Image(image, size, uv0, uv1, tintColor, borderColor);
	}

	void GUI::Separator()
	{
		return Instance->Impl->Separator();
	}

	void GUI::Dummy(const SVector2<F32>& size)
	{
		Instance->Impl->Dummy(size);
	}

	void GUI::SameLine(const F32 offsetFromX, const F32 spacing)
	{
		Instance->Impl->SameLine(offsetFromX, spacing);
	}

	bool GUI::IsItemHovered()
	{
		return Instance->Impl->IsItemHovered();
	}

	F32 GUI::GetCursorPosX()
	{
		return Instance->Impl->GetCursorPosX();
	}

	void GUI::SetCursorPosX(const F32 cursorPosX)
	{
		Instance->Impl->SetCursorPosX(cursorPosX);
	}

	F32 GUI::GetScrollY()
	{
		return Instance->Impl->GetScrollY();
	}

	F32 GUI::GetScrollMaxY()
	{
		return Instance->Impl->GetScrollMaxY();
	}

	void GUI::SetScrollHereY(const F32 scroll)
	{
		Instance->Impl->SetScrollHereY(scroll);
	}

	const SVector2<F32>& GUI::CalculateTextSize(const char* text)
	{
		return Instance->Impl->CalculateTextSize(text);
	}

	void GUI::SetItemDefaultFocus()
	{
		Instance->Impl->SetItemDefaultFocus();
	}

	void GUI::SetKeyboardFocusHere(const I32 offset)
	{
		Instance->Impl->SetKeyboardFocusHere(offset);
	}

	void GUI::PushStyleVar(const EStyleVar styleVar, const SVector2<F32>& value)
	{
		Instance->Impl->PushStyleVar(styleVar, value);
	}

	void GUI::PushStyleVar(const EStyleVar styleVar, const F32 value)
	{
		Instance->Impl->PushStyleVar(styleVar, value);
	}

	void GUI::PopStyleVar(const I32 count)
	{
		Instance->Impl->PopStyleVar(count);
	}

	SVector2<F32> GUI::GetStyleVar(const EStyleVar styleVar)
	{
		return Instance->Impl->GetStyleVar(styleVar);
	}

	std::vector<SColor> GUI::GetStyleColors()
	{
		return Instance->Impl->GetStyleColors();
	}

	void GUI::PushStyleColor(const EStyleColor styleColor, const SColor& color)
	{
		Instance->Impl->PushStyleColor(styleColor, color);
	}

	void GUI::PopStyleColor()
	{
		Instance->Impl->PopStyleColor();
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

	bool GUI::IsOverGizmo()
	{
		return Instance->Impl->IsOverGizmo();
	}

	F32 GUI::GetTextLineHeight()
	{
		return Instance->Impl->GetTextLineHeight();
	}

	SVector2<F32> GUI::GetCursorScreenPos()
	{
		return Instance->Impl->GetCursorScreenPos();
	}

	SVector2<F32> GUI::GetViewportWorkPos()
	{
		return Instance->Impl->GetViewportWorkPos();
	}

	SVector2<F32> GUI::GetViewportCenter()
	{
		return Instance->Impl->GetViewportCenter();
	}

	SVector2<F32> GUI::GetWindowContentRegionMin()
	{
		return Instance->Impl->GetWindowContentRegionMin();
	}

	SVector2<F32> GUI::GetWindowContentRegionMax()
	{
		return Instance->Impl->GetWindowContentRegionMax();
	}

	SVector2<F32> GUI::GetContentRegionAvail()
	{
		return Instance->Impl->GetContentRegionAvail();
	}

	F32 GUI::GetFrameHeightWithSpacing()
	{
		return Instance->Impl->GetFrameHeightWithSpacing();
	}

	void GUI::SetNextWindowPos(const SVector2<F32>& pos, const EWindowCondition condition, const SVector2<F32>& pivot)
	{
		Instance->Impl->SetNextWindowPos(pos, condition, pivot);
	}

	void GUI::SetNextWindowSize(const SVector2<F32>& size)
	{
		Instance->Impl->SetNextWindowSize(size);
	}

	void GUI::SetRect(const SVector2<F32>& position, const SVector2<F32>& dimensions)
	{
		Instance->Impl->SetRect(position, dimensions);
	}

	void GUI::SetGizmoDrawList()
	{
		Instance->Impl->SetGizmoDrawList();
	}

	SVector2<F32> GUI::GetCurrentWindowSize()
	{
		return Instance->Impl->GetCurrentWindowSize();
	}

	void GUI::AddRectFilled(const SVector2<F32>& cursorPos, const SVector2<F32>& size, const SColor& color)
	{
		Instance->Impl->AddRectFilled(cursorPos, size, color);
	}
	
	void GUI::SetGuiColorProfile(const SGuiColorProfile& profile)
	{
		Instance->Impl->SetGuiColorProfile(profile);
	}

	void GUI::SetGuiStyleProfile(const SGuiStyleProfile& profile)
	{
		Instance->Impl->SetImGuiStyleProfile(profile);
	}

	void GUI::GizmoManipulate(const F32* view, const F32* projection, ETransformGizmo operation, ETransformGizmoSpace mode, F32* matrix, F32* deltaMatrix, const F32* snap, const F32* localBounds, const F32* boundsSnap)
	{
		Instance->Impl->GizmoManipulate(view, projection, (ImGuizmo::OPERATION)operation, (ImGuizmo::MODE)mode, matrix, deltaMatrix, snap, localBounds, boundsSnap);
	}

	void GUI::ViewManipulate(F32* view, const F32 length, const SVector2<F32>& position, const SVector2<F32>& size, const SColor& color)
	{
		Instance->Impl->ViewManipulate(view, length, position, size, color);
	}

	bool GUI::IsDockingEnabled()
	{
		return Instance->Impl->IsDockingEnabled();
	}

	void GUI::DockSpace(const U32 id, const SVector2<F32>& size, const EDockNodeFlag dockNodeFlag)
	{
		return Instance->Impl->DockSpace(id, size, dockNodeFlag);
	}

	void GUI::DockBuilderAddNode(U32 id, const std::vector<EDockNodeFlag>& flags)
	{
		Instance->Impl->DockBuilderAddNode(id, flags);
	}

	void GUI::DockBuilderRemoveNode(U32 id)
	{
		Instance->Impl->DockBuilderRemoveNode(id);
	}

	void GUI::DockBuilderSetNodeSize(U32 id, const SVector2<F32>& size)
	{
		Instance->Impl->DockBuilderSetNodeSize(id, size);
	}

	void GUI::DockBuilderDockWindow(const char* label, U32 id)
	{
		Instance->Impl->DockBuilderDockWindow(label, id);
	}

	void GUI::DockBuilderFinish(U32 id)
	{
		Instance->Impl->DockBuilderFinish(id);
	}

	void GUI::LogToClipboard()
	{
		Instance->Impl->LogToClipboard();
	}

	void GUI::LogFinish()
	{
		Instance->Impl->LogFinish();
	}

	void GUI::MemFree(void* ptr)
	{
		Instance->Impl->MemFree(ptr);
	}

	void GUI::ShowDemoWindow(bool* open)
	{
		Instance->Impl->ShowDemoWindow(open);
	}

	SGuiTextFilter::SGuiTextFilter(const char* default_filter)
	{
		InputBuf[0] = 0;
		CountGrep = 0;
		if (default_filter)
		{
			Strncpy(InputBuf, default_filter, ARRAY_SIZE(InputBuf));
			Build();
		}
	}
	
	bool SGuiTextFilter::Draw(const char* label, F32 width)
	{
		if (width != 0.0f)
			ImGui::SetNextItemWidth(width);
		bool valueChanged = ImGui::InputText(label, InputBuf, ARRAY_SIZE(InputBuf));
		if (valueChanged)
			Build();
		return valueChanged;
	}

	bool SGuiTextFilter::PassFilter(const char* text, const char* text_end) const
	{
		if (Filters.empty())
			return true;

		if (text == NULL)
			text = text_end = "";

		for (const SGuiTextRange& f : Filters)
		{
			if (f.b == f.e)
				continue;
			if (f.b[0] == '-')
			{
				// Subtract
				if (Stristr(text, text_end, f.b + 1, f.e) != NULL)
					return false;
			}
			else
			{
				// Grep
				if (Stristr(text, text_end, f.b, f.e) != NULL)
					return true;
			}
		}

		// Implicit * grep
		if (CountGrep == 0)
			return true;

		return false;
	}

	void SGuiTextFilter::Build()
	{
		Filters.resize(0);
		SGuiTextRange inputRange(InputBuf, InputBuf + strlen(InputBuf));
		inputRange.split(',', &Filters);

		CountGrep = 0;
		for (SGuiTextRange& f : Filters)
		{
			while (f.b < f.e && CharIsBlankA(f.b[0]))
				f.b++;
			while (f.e > f.b && CharIsBlankA(f.e[-1]))
				f.e--;
			if (f.empty())
				continue;
			if (f.b[0] != '-')
				CountGrep += 1;
		}
	}

	const char* SGuiTextFilter::Stristr(const char* haystack, const char* haystack_end, const char* needle, const char* needle_end) const
	{
		if (!needle_end)
			needle_end = needle + strlen(needle);

		const char un0 = (char)toupper(*needle);
		while ((!haystack_end && *haystack) || (haystack_end && haystack < haystack_end))
		{
			if (toupper(*haystack) == un0)
			{
				const char* b = needle + 1;
				for (const char* a = haystack + 1; b < needle_end; a++, b++)
					if (toupper(*a) != toupper(*b))
						break;
				if (b == needle_end)
					return haystack;
			}
			haystack++;
		}
		return NULL;
	}

	void SGuiTextFilter::Strncpy(char* dst, const char* src, size_t count) const
	{
		if (count < 1)
			return;
		if (count > 1)
			strncpy(dst, src, count - 1);
		dst[count - 1] = 0;
	}

	void SGuiTextFilter::SGuiTextRange::split(char separator, std::vector<SGuiTextRange>* out) const
	{
		out->resize(0);
		const char* wb = b;
		const char* we = wb;
		while (we < e)
		{
			if (*we == separator)
			{
				out->push_back(SGuiTextRange(wb, we));
				wb = we + 1;
			}
			we++;
		}
		if (wb != we)
			out->push_back(SGuiTextRange(wb, we));
	}
}