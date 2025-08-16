// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "GUI.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
#include <ImGuizmo.h>
#include <imgui_node_editor.h>
#include <utilities/builders.h>
#include <utilities/widgets.h>
#include <utilities/drawing.h>
#include <d3d11.h>
#include <DirectXTex/DirectXTex.h>

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

	namespace NE = ax::NodeEditor;

	class GUI::ImGuiImpl
	{
		NE::EditorContext* NodeEditorContext = nullptr;
		NE::Utilities::BlueprintNodeBuilder NodeBuilder;
		ID3D11ShaderResourceView* BlueprintBackgroundSRV = nullptr;
		ImTextureID BlueprintBackgroundImage = 0;

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

			NE::Config config;
			config.SettingsFile = "Simple.json";
			config.UserPointer = this;
			NodeEditorContext = NE::CreateEditor(&config);

			std::string filePath = "Resources/NodeBackground.dds";
			DirectX::ScratchImage scratchImage;
			DirectX::TexMetadata metaData = {};
			const auto widePath = new wchar_t[filePath.length() + 1];
			std::ranges::copy(filePath, widePath);
			widePath[filePath.length()] = 0;
			GetMetadataFromDDSFile(widePath, DirectX::DDS_FLAGS_NONE, metaData);
			LoadFromDDSFile(widePath, DirectX::DDS_FLAGS_NONE, &metaData, scratchImage);
			delete[] widePath;
			const DirectX::Image* image = scratchImage.GetImage(0, 0, 0);
			DirectX::CreateShaderResourceView(device, image, scratchImage.GetImageCount(), metaData, &BlueprintBackgroundSRV);

			BlueprintBackgroundImage = (ImTextureID)BlueprintBackgroundSRV;
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

		void TextWrapped(const char* fmt, va_list args)
		{
			ImGui::TextWrappedV(fmt, args);
		}

		void TextDisabled(const char* fmt, va_list args)
		{
			ImGui::TextDisabledV(fmt, args);
		}

		void TextUnformatted(const char* text)
		{
			ImGui::TextUnformatted(text);
		}

		bool InputText(const char* label, char* buf, size_t bufSize, ImGuiInputTextCallback callback, void* data)
		{
			return ImGui::InputText(label, buf, bufSize, ImGuiInputTextFlags_CallbackResize, callback, data);
		}

		bool InputText(const char* label, std::string& buffer)
		{
			return ImGui::InputText(label, &buffer);
		}

		void SetTooltip(const char* fmt, va_list args)
		{
			ImGui::SetTooltipV(fmt, args);
		}

		SVector2<F32> CalculateTextSize(const char* text)
		{
			ImVec2 imVec = ImGui::CalcTextSize(text);
			return { imVec.x, imVec.y };
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

		bool InputInt(const char* label, I32& value, I32 step, I32 stepFast)
		{
			return ImGui::InputInt(label, &value, step, stepFast);
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

		bool Selectable(const char* label, const bool selected, const std::vector<ESelectableFlag>& flags, const SVector2<F32>& size)
		{
			int imFlags = 0;
			for (const ESelectableFlag& flag : flags)
				imFlags += int(flag);

			ImVec2 imSize = { size.X, size.Y };

			return ImGui::Selectable(label, selected, imFlags, imSize);
		}

		SGuiMultiSelectIO BeginMultiSelect(const std::vector<EMultiSelectFlag>& flags, I32 selectionSize, I32 itemsCount)
		{
			int imFlags = 0;
			for (const EMultiSelectFlag& flag : flags)
				imFlags += int(flag);

			const ImGuiMultiSelectIO* imGuiSelectIO = ImGui::BeginMultiSelect(imFlags, selectionSize, itemsCount);
			if (imGuiSelectIO == nullptr)
				return {};

			SGuiMultiSelectIO guiSelectIO;
			for (const ImGuiSelectionRequest& imRequest : imGuiSelectIO->Requests)
			{
				SSelectionRequest& guiRequest = guiSelectIO.Requests.emplace_back();
				guiRequest.Type = static_cast<ESelectionRequestType>(imRequest.Type);
				guiRequest.IsSelected = imRequest.Selected;
				guiRequest.RangeDirection = imRequest.RangeDirection;
				guiRequest.RangeFirstItem = imRequest.RangeFirstItem;
				guiRequest.RangeLastItem = imRequest.RangeLastItem;
			}

			guiSelectIO.RangeSourceItem = imGuiSelectIO->RangeSrcItem;
			guiSelectIO.NavIdItem = imGuiSelectIO->NavIdItem;
			guiSelectIO.NavIdSelected = imGuiSelectIO->NavIdSelected;
			guiSelectIO.RangeSourceReset = imGuiSelectIO->RangeSrcReset;
			guiSelectIO.ItemsCount = imGuiSelectIO->ItemsCount;

			return guiSelectIO;
		}

		SGuiMultiSelectIO EndMultiSelect()
		{
			const ImGuiMultiSelectIO* imGuiSelectIO = ImGui::EndMultiSelect();
			if (imGuiSelectIO == nullptr)
				return {};

			SGuiMultiSelectIO guiSelectIO;
			for (const ImGuiSelectionRequest& imRequest : imGuiSelectIO->Requests)
			{
				SSelectionRequest& guiRequest = guiSelectIO.Requests.emplace_back();
				guiRequest.Type = static_cast<ESelectionRequestType>(imRequest.Type);
				guiRequest.IsSelected = imRequest.Selected;
				guiRequest.RangeDirection = imRequest.RangeDirection;
				guiRequest.RangeFirstItem = imRequest.RangeFirstItem;
				guiRequest.RangeLastItem = imRequest.RangeLastItem;
			}

			guiSelectIO.RangeSourceItem = imGuiSelectIO->RangeSrcItem;
			guiSelectIO.NavIdItem = imGuiSelectIO->NavIdItem;
			guiSelectIO.NavIdSelected = imGuiSelectIO->NavIdSelected;
			guiSelectIO.RangeSourceReset = imGuiSelectIO->RangeSrcReset;
			guiSelectIO.ItemsCount = imGuiSelectIO->ItemsCount;

			return guiSelectIO;
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

		bool IsItemClicked(const EGUIMouseButton button)
		{
			return ImGui::IsItemClicked(static_cast<ImGuiMouseButton>(button));
		}

		bool IsMouseReleased(int mouseButton)
		{
			return ImGui::IsMouseReleased(mouseButton);
		}

		bool IsItemHovered()
		{
			return ImGui::IsItemHovered();
		}

		bool IsItemVisible()
		{
			return ImGui::IsItemVisible();
		}

		bool IsWindowFocused()
		{
			return ImGui::IsWindowFocused();
		}

		bool IsWindowHovered()
		{
			return ImGui::IsWindowHovered();
		}

		SVector2<F32> GetCursorPos()
		{
			const ImVec2& imCursorPos = ImGui::GetCursorPos();
			return { imCursorPos.x, imCursorPos.y };
		}

		void SetCursorPos(const SVector2<F32>& cursorPos)
		{
			const ImVec2& imCursorPos = { cursorPos.X, cursorPos.Y };
			ImGui::SetCursorPos(imCursorPos);
		}

		F32 GetCursorPosX()
		{
			return ImGui::GetCursorPosX();
		}

		F32 GetCursorPosY()
		{
			return ImGui::GetCursorPosY();
		}

		void SetCursorPosX(const F32 cursorPosX)
		{
			ImGui::SetCursorPosX(cursorPosX);
		}

		void SetCursorPosY(const F32 cursorPosY)
		{
			ImGui::SetCursorPosY(cursorPosY);
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

		SVector4 GetLastRect()
		{
			const ImRect& imRect = GImGui->LastItemData.Rect;
			return { imRect.Min.x, imRect.Min.y, imRect.Max.x, imRect.Max.y };
		}

		void Spring(const F32 weight, const F32 spacing)
		{
			ImGui::Spring(weight, spacing);
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

		void PushScriptStyleVar(const EScriptStyleVar styleVar, const SVector4& value)
		{
			NE::PushStyleVar((NE::StyleVar)styleVar, { value.X, value.Y, value.Z, value.W });
		}

		void PushScriptStyleVar(const EScriptStyleVar styleVar, const SVector2<F32>& value)
		{
			NE::PushStyleVar((NE::StyleVar)styleVar, { value.X, value.Y });
		}

		void PopScriptStyleVar(int count = 1)
		{
			NE::PopStyleVar(count);
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
				break;
			case ImGuiStyleVar_FramePadding:
				value = ImGui::GetStyle().FramePadding;
				break;
			case ImGuiStyleVar_ItemSpacing:
				value = ImGui::GetStyle().ItemSpacing;
				break;
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

		void PushScriptStyleColor(const EScriptStyleColor styleColor, const SColor& color)
		{
			SVector4 colorFloat = color.AsVector4();
			ImVec4 imValue = { colorFloat.X, colorFloat.Y, colorFloat.Z, colorFloat.W };
			NE::PushStyleColor((NE::StyleColor)styleColor, imValue);
		}

		void PopStyleColor()
		{
			ImGui::PopStyleColor();
		}

		void PopScriptStyleColor()
		{
			NE::PopStyleColor();
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

		bool IsLeftMouseHeld()
		{
			return ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Left);
		}

		bool IsDoubleClick()
		{
			return ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
		}

		bool IsShiftHeld()
		{
			return ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey::ImGuiKey_RightShift);
		}

		bool IsControlHeld()
		{
			return ImGui::IsKeyDown(ImGuiKey::ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey::ImGuiKey_RightCtrl);
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

		SVector2<F32> GetMousePosition()
		{
			ImVec2 imPos = ImGui::GetMousePos();
			return { imPos.x, imPos.y };
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

		void PushItemWidth(const F32 width)
		{
			ImGui::PushItemWidth(width);
		}

		void PopItemWidth()
		{
			ImGui::PopItemWidth();
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

		void BeginGroup()
		{
			ImGui::BeginGroup();
		}

		void EndGroup()
		{
			ImGui::EndGroup();
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

		bool BeginDragDropSource(const std::vector<EDragDropFlag>& flags)
		{
			int imFlags = 0;
			for (const EDragDropFlag& flag : flags)
				imFlags += int(flag);

			return ImGui::BeginDragDropSource(imFlags);
		}

		SGuiPayload GetDragDropPayload()
		{
			const ImGuiPayload* imGuiPayload = ImGui::GetDragDropPayload();
			if (imGuiPayload == nullptr)
				return {};

			SGuiPayload guiPayload;
			guiPayload.Data = imGuiPayload->Data;
			guiPayload.Size = imGuiPayload->DataSize;
			guiPayload.SourceID = imGuiPayload->SourceId;
			guiPayload.SourceParentID = imGuiPayload->SourceParentId;
			guiPayload.DataFrameCount = imGuiPayload->DataFrameCount;
			guiPayload.IDTag = imGuiPayload->DataType;
			guiPayload.IsPreview = imGuiPayload->Preview;
			guiPayload.IsDelivery = imGuiPayload->Delivery;
			return guiPayload;
		}

		bool SetDragDropPayload(const char* type, const void* data, U64 dataSize)
		{
			return ImGui::SetDragDropPayload(type, data, dataSize);
		}

		void EndDragDropSource()
		{
			ImGui::EndDragDropSource();
		}

		bool BeginDragDropTarget()
		{
			return ImGui::BeginDragDropTarget();
		}

		bool IsDragDropPayloadBeingAccepted()
		{
			return ImGui::IsDragDropPayloadBeingAccepted();
		}

		SGuiPayload AcceptDragDropPayload(const char* type, const std::vector<EDragDropFlag>& flags)
		{
			int imFlags = 0;
			for (const EDragDropFlag& flag : flags)
				imFlags += int(flag);

			const ImGuiPayload* imGuiPayload = ImGui::AcceptDragDropPayload(type, imFlags);
			if (imGuiPayload == nullptr)
				return {};

			SGuiPayload guiPayload;
			guiPayload.Data = imGuiPayload->Data;
			guiPayload.Size = imGuiPayload->DataSize;
			guiPayload.SourceID = imGuiPayload->SourceId;
			guiPayload.SourceParentID = imGuiPayload->SourceParentId;
			guiPayload.DataFrameCount = imGuiPayload->DataFrameCount;
			guiPayload.IDTag = imGuiPayload->DataType;
			guiPayload.IsPreview = imGuiPayload->Preview;
			guiPayload.IsDelivery = imGuiPayload->Delivery;
			return guiPayload;
		}

		void EndDragDropTarget()
		{
			ImGui::EndDragDropTarget();
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

		void TableNextRow()
		{
			ImGui::TableNextRow();
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

		bool TreeNodeEx(const char* strID, const std::vector<ETreeNodeFlag>& treeNodeFlags)
		{
			int imTreeNodeFlags = 0;
			for (const ETreeNodeFlag& treeNodeFlag : treeNodeFlags)
				imTreeNodeFlags += int(treeNodeFlag);

			return ImGui::TreeNodeEx(strID, imTreeNodeFlags);
		}

		void TreePop()
		{
			ImGui::TreePop();
		}

		bool BeginCombo(const char* label, const char* selectedLabel)
		{
			return ImGui::BeginCombo(label, selectedLabel);
		}

		void EndCombo()
		{
			ImGui::EndCombo();
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

		void AddRectFilled(const SVector2<F32>& cursorScreenPos, const SVector2<F32>& size, const SColor& color)
		{
			ImVec2 posMin = { cursorScreenPos.X, cursorScreenPos.Y };
			ImVec2 posMax = { cursorScreenPos.X + size.X, cursorScreenPos.Y + size.Y };
			SVector4 colorFloat = color.AsVector4();
			ImVec4 imColor = { colorFloat.X, colorFloat.Y, colorFloat.Z, colorFloat.W };
			ImGui::GetWindowDrawList()->AddRectFilled(posMin, posMax, ImGui::ColorConvertFloat4ToU32(imColor));
			//ImGui::GetBackgroundDrawList()->
		}

		void HighlightPins(const U64* pinIds)
		{
			ImDrawList* drawList = ImGui::GetForegroundDrawList();
			//ImVec2 position = NE::GetNodePosition(nodeId);


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

		void BeginScript(const char* label, const SVector2<F32>& size)
		{
			NE::SetCurrentEditor(NodeEditorContext);
			NE::Begin(label, ImVec2(size.X, size.Y));
		}

		void EndScript()
		{
			NE::End();
			NE::SetCurrentEditor(nullptr);
		}

		void BeginNode(const U64 id)
		{
			NE::BeginNode(id);
		}

		void EndNode()
		{
			NE::EndNode();
		}

		void SetNodePosition(const U64 id, const SVector2<F32>& position)
		{
			NE::SetNodePosition(id, { position.X, position.Y });
		}

		void BeginPin(const U64 id, const EGUIPinDirection direction)
		{
			NE::BeginPin(id, (ax::NodeEditor::PinKind)direction);
		}

		void EndPin()
		{
			NE::EndPin();
		}

		void DrawPinIcon(const SVector2<F32>& size, const EGUIIconType type, const bool isConnected, const SColor& color, const bool highlighted)
		{
			ax::Widgets::Icon(ImVec2(size.X, size.Y), static_cast<ax::Drawing::IconType>(type), isConnected, ImColor{ color.R, color.G, color.B, color.A }, ImColor(32, 32, 32, 255), highlighted);
			ImGui::GetWindowDrawList()->GetClipRectMax();
		}

		void DrawNodeHeader(const U64 nodeID, intptr_t textureID, const SVector2<F32>& posMin, const SVector2<F32>& posMax, const SVector2<F32>& uvMin, const SVector2<F32>& uvMax, const SColor& color, const F32 rounding)
		{
			NE::GetNodeBackgroundDrawList(nodeID)->AddImageRounded((ImTextureID)textureID, { posMin.X, posMin.Y }, { posMax.X, posMax.Y }, { uvMin.X, uvMin.Y }, { uvMax.X, uvMax.Y }, ImColor{ color.R, color.G, color.B, color.A }, rounding, ImDrawFlags_RoundCornersAll);
		}

		void Link(const U64 linkID, const U64 startPinID, const U64 endPinID, const SColor& color, const F32 thickness)
		{
			SVector4 floatColor = color.AsVector4();
			ImVec4 imColor = { floatColor.X, floatColor.Y, floatColor.Z, floatColor.W };
			NE::Link(linkID, startPinID, endPinID, imColor, thickness);
		}

		bool BeginScriptCreate()
		{
			return NE::BeginCreate();
		}

		void EndScriptCreate()
		{
			NE::EndCreate();
		}

		bool BeginScriptDelete()
		{
			return NE::BeginDelete();;
		}

		void EndScriptDelete()
		{
			NE::EndDelete();
		}

		void SuspendScript()
		{
			NE::Suspend();
		}

		void ResumeScript()
		{
			NE::Resume();
		}

		bool QueryNewLink(U64& inputPinID, U64& outputPinID)
		{
			NE::PinId inputPinId, outputPinId;
			const bool returnValue = NE::QueryNewLink(&inputPinId, &outputPinId);
			inputPinID = inputPinId.Get();
			outputPinID = outputPinId.Get();
			return returnValue;
		}

		bool QueryDeletedLink(U64& linkID)
		{
			NE::LinkId linkId;
			const bool returnValue = NE::QueryDeletedLink(&linkId);
			linkID = linkId.Get();
			return returnValue;
		}

		bool QueryDeletedNode(U64& nodeID)
		{
			NE::NodeId nodeId;
			const bool returnValue = NE::QueryDeletedNode(&nodeId);
			nodeID = nodeId.Get();
			return returnValue;
		}

		bool AcceptNewScriptItem()
		{
			return NE::AcceptNewItem();
		}

		bool AcceptDeletedScriptItem()
		{
			return NE::AcceptDeletedItem();
		}

		bool ShowScriptContextMenu()
		{
			return NE::ShowBackgroundContextMenu();
		}

		void BeginVertical(const char* label, const SVector2<F32>& size)
		{
			ImGui::BeginVertical(label, { size.X, size.Y });
		}

		void EndVertical()
		{
			ImGui::EndVertical();
		}

		void BeginHorizontal(const char* label, const SVector2<F32>& size)
		{
			ImGui::BeginHorizontal(label, { size.X, size.Y });
		}

		void EndHorizontal()
		{
			ImGui::EndHorizontal();
		}

		void Indent(const F32 indent)
		{
			ImGui::Indent(indent);
		}

		void Unindent(const F32 indent)
		{
			ImGui::Unindent(indent);
		}

		void LogToClipboard()
		{
			ImGui::LogToClipboard();
		}

		void LogFinish()
		{
			ImGui::LogFinish();
		}

		void CopyToClipboard(const char* text)
		{
			ImGui::SetClipboardText(text);
		}

		std::string CopyFromClipboard()
		{
			return ImGui::GetClipboardText();
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

	void GUI::TextWrapped(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		Instance->Impl->TextWrapped(fmt, args);
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

	bool GUI::InputText(const char* label, CHavtornStaticString<255>* customString)
	{
		return Instance->Impl->InputText(label, customString->Data(), (size_t)customString->Length() + 1, HavtornInputTextResizeCallback, (void*)customString);
	}

	bool GUI::InputText(const char* label, std::string& buffer)
	{
		return Instance->Impl->InputText(label, buffer);
	}

	void GUI::CenterText(const std::string& text, SVector2<F32> dimensions, SVector2<F32> alignment)
	{
		GUI::SetCursorPos(SVector2<F32>(0.0f));
		const SVector2<F32> textWidth = GUI::CalculateTextSize(text.c_str());
		const SVector2<F32> offset = (dimensions - textWidth) * alignment;
		if (alignment.SizeSquared() > 0.0f)
			GUI::OffsetCursorPos(offset);

		return GUI::Text(text.c_str());
	}

	void GUI::SetTooltip(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		Instance->Impl->SetTooltip(fmt, args);
		va_end(args);
	}

	SVector2<F32> GUI::CalculateTextSize(const char* text)
	{
		return Instance->Impl->CalculateTextSize(text);
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

	bool GUI::InputInt(const char* label, I32& value, I32 step, I32 stepFast)
	{
		return Instance->Impl->InputInt(label, value, step, stepFast);
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

	void GUI::PushID(U64 uintID)
	{
		Instance->Impl->PushID(STATIC_I32(uintID));
	}

	void GUI::PopID()
	{
		Instance->Impl->PopID();
	}

	I32 GUI::GetID(const char* label)
	{
		return Instance->Impl->GetID(label);
	}

	void GUI::PushItemWidth(const F32 width)
	{
		Instance->Impl->PushItemWidth(width);
	}

	void GUI::PopItemWidth()
	{
		Instance->Impl->PopItemWidth();
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

	void GUI::BeginGroup()
	{
		Instance->Impl->BeginGroup();
	}

	void GUI::EndGroup()
	{
		Instance->Impl->EndGroup();
	}

	bool GUI::BeginChild(const char* label, const SVector2<F32>& size, const std::vector<EChildFlag>& childFlags, const std::vector<EWindowFlag>& windowFlags)
	{
		return Instance->Impl->BeginChild(label, size, childFlags, windowFlags);
	}

	void GUI::EndChild()
	{
		Instance->Impl->EndChild();
	}

	bool GUI::BeginDragDropSource(const std::vector<EDragDropFlag>& flags)
	{
		return Instance->Impl->BeginDragDropSource(flags);
	}

	SGuiPayload GUI::GetDragDropPayload()
	{
		return Instance->Impl->GetDragDropPayload();
	}

	bool GUI::SetDragDropPayload(const char* type, const void* data, U64 dataSize)
	{
		return Instance->Impl->SetDragDropPayload(type, data, dataSize);
	}

	void GUI::EndDragDropSource()
	{
		Instance->Impl->EndDragDropSource();
	}

	bool GUI::BeginDragDropTarget()
	{
		return Instance->Impl->BeginDragDropTarget();
	}

	bool GUI::IsDragDropPayloadBeingAccepted()
	{
		return Instance->Impl->IsDragDropPayloadBeingAccepted();
	}

	SGuiPayload GUI::AcceptDragDropPayload(const char* type, const std::vector<EDragDropFlag>& flags)
	{
		return Instance->Impl->AcceptDragDropPayload(type, flags);
	}

	void GUI::EndDragDropTarget()
	{
		Instance->Impl->EndDragDropTarget();
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

	void GUI::TableNextRow()
	{
		Instance->Impl->TableNextRow();
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

	bool GUI::TreeNodeEx(const char* label, const std::vector<ETreeNodeFlag>& treeNodeFlags)
	{
		return Instance->Impl->TreeNodeEx(label, treeNodeFlags);
	}

	void GUI::TreePop()
	{
		Instance->Impl->TreePop();
	}

	bool GUI::BeginCombo(const char* label, const char* selectedLabel)
	{
		return Instance->Impl->BeginCombo(label, selectedLabel);
	}

	void GUI::EndCombo()
	{
		return Instance->Impl->EndCombo();
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

	SAssetPickResult GUI::AssetPicker(const char* label, const char* modalLabel, intptr_t image, const std::string& directory, I32 columns, const DirEntryFunc& assetInspector)
	{
		if (GUI::ImageButton("AssetPicker", image, { GUI::TexturePreviewSizeX * 0.75f, GUI::TexturePreviewSizeY * 0.75f }))
		{
			GUI::OpenPopup(modalLabel);
			GUI::SetNextWindowPos(GUI::GetViewportCenter(), EWindowCondition::Appearing, SVector2<F32>(0.5f, 0.5f));
		}

		GUI::Text(label);

		if (!GUI::BeginPopupModal(modalLabel, NULL, { EWindowFlag::AlwaysAutoResize }))
			return SAssetPickResult();

		if (!GUI::BeginTable("AssetPickerTable", columns))
		{
			GUI::EndPopup();
			return SAssetPickResult();
		}



		I32 id = 0;
		for (auto& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			if (entry.is_directory())
				continue;

			SAssetInspectionData data = assetInspector(entry);
			if (!data.IsValid())
				continue;

			GUI::TableNextColumn();
			GUI::PushID(id++);

			if (GUI::ImageButton(data.Name.c_str(), data.TextureRef, { GUI::TexturePreviewSizeX * 0.75f, GUI::TexturePreviewSizeY * 0.75f }))
			{
				GUI::PopID();
				GUI::EndTable();
				GUI::CloseCurrentPopup();
				GUI::EndPopup();
				return SAssetPickResult(entry);
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
		GUI::OffsetCursorPos(SVector2<F32>(off, 0.0f));

		if (GUI::Button("Cancel"))
		{
			GUI::CloseCurrentPopup();
			GUI::EndPopup();
			return SAssetPickResult(EAssetPickerState::Cancelled);
		}

		GUI::EndPopup();
		return SAssetPickResult(EAssetPickerState::Active);
	}


	SAssetPickResult GUI::AssetPickerFilter(const char* label, const char* modalLabel, intptr_t image, const std::string& directory, I32 columns, const DirEntryEAssetTypeFunc& assetInspector, EAssetType filterByAssetType)
	{
		if (GUI::ImageButton("AssetPicker", image, { GUI::TexturePreviewSizeX * 0.75f, GUI::TexturePreviewSizeY * 0.75f }))
		{
			GUI::OpenPopup(modalLabel);
			GUI::SetNextWindowPos(GUI::GetViewportCenter(), EWindowCondition::Appearing, SVector2<F32>(0.5f, 0.5f));
		}

		GUI::Text(label);

		if (!GUI::BeginPopupModal(modalLabel, NULL, { EWindowFlag::AlwaysAutoResize }))
			return SAssetPickResult();

		if (!GUI::BeginTable("AssetPickerTable", columns))
		{
			GUI::EndPopup();
			return SAssetPickResult();
		}



		I32 id = 0;
		for (auto& entry : std::filesystem::recursive_directory_iterator(directory))
		{
			if (entry.is_directory())
				continue;

			SAssetInspectionData data = assetInspector(entry, static_cast<EAssetType>(filterByAssetType));
			if (!data.IsValid())
				continue;

			GUI::TableNextColumn();
			GUI::PushID(id++);

			if (GUI::ImageButton(data.Name.c_str(), data.TextureRef, { GUI::TexturePreviewSizeX * 0.75f, GUI::TexturePreviewSizeY * 0.75f }))
			{
				GUI::PopID();
				GUI::EndTable();
				GUI::CloseCurrentPopup();
				GUI::EndPopup();
				return SAssetPickResult(entry);
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
		GUI::OffsetCursorPos(SVector2<F32>(off, 0.0f));

		if (GUI::Button("Cancel"))
		{
			GUI::CloseCurrentPopup();
			GUI::EndPopup();
			return SAssetPickResult(EAssetPickerState::Cancelled);
		}

		GUI::EndPopup();
		return SAssetPickResult(EAssetPickerState::Active);
	}

	SRenderAssetCardResult GUI::RenderAssetCard(const char* label, const bool isSelected, const intptr_t& thumbnailID, const char* typeName, const SColor& color, void* dragDropPayloadToSet, U64 payLoadSize)
	{
		SRenderAssetCardResult result;

		SVector2<F32> cardStartPos = GUI::GetCursorPos();
		SVector2<F32> framePadding = GUI::GetStyleVar(EStyleVar::FramePadding);

		SVector2<F32> cardSize = { GUI::ThumbnailSizeX + framePadding.X * 0.5f, GUI::ThumbnailSizeY + framePadding.Y * 0.5f };
		cardSize.Y *= 1.6f;
		SVector2<F32> thumbnailSize = { GUI::ThumbnailSizeX + framePadding.X * 0.5f, GUI::ThumbnailSizeY + framePadding.Y * 0.5f + 4.0f };

		// TODO.NW: Can't seem to get the leftmost line to show correctly. Maybe need to start the table as usual and then offset inwards?
		constexpr F32 borderThickness = 1.0f;
		GUI::SetCursorPos(cardStartPos + SVector2<F32>(-1.0f * borderThickness));
		GUI::AddRectFilled(GUI::GetCursorScreenPos(), cardSize + SVector2<F32>(2.0f * borderThickness), SColor(10));
		GUI::SetCursorPos(cardStartPos);
		GUI::AddRectFilled(GUI::GetCursorScreenPos(), cardSize, SColor(65));
		GUI::SetCursorPos(cardStartPos);
		GUI::AddRectFilled(GUI::GetCursorScreenPos(), thumbnailSize, SColor(40));
		GUI::SetCursorPos(cardStartPos);

		if (GUI::Selectable("", isSelected, { ESelectableFlag::AllowDoubleClick, ESelectableFlag::AllowOverlap }, cardSize))
		{
			if (GUI::IsMouseReleased())
				result.IsClicked = true;
			if (GUI::IsDoubleClick())
				result.IsDoubleClicked = true;
		}

		if (GUI::BeginDragDropSource())
		{
			SGuiPayload payload = GUI::GetDragDropPayload();
			if (payload.Data == nullptr)
			{
				GUI::SetDragDropPayload("AssetDrag", dragDropPayloadToSet, payLoadSize);
			}
			GUI::Text(label);

			GUI::EndDragDropSource();
		}

		SVector2<F32> cardEndPos = GUI::GetCursorPos();
		GUI::SetCursorPos(cardStartPos + SVector2<F32>(1.0f, 0.0f));

		SColor imageBorderColor = color;
		imageBorderColor.A = SColor::ToU8Range(0.5f);

		GUI::Image(thumbnailID, { GUI::ThumbnailSizeX, GUI::ThumbnailSizeY }, SVector2<F32>(0.0f), SVector2<F32>(1.0f), SColor::White);

		GUI::AddRectFilled(GUI::GetCursorScreenPos(), SVector2<F32>(cardSize.X, 2.0f), imageBorderColor);

		GUI::OffsetCursorPos(SVector2<F32>(2.0f, 4.0f));

		if (GUI::IsItemHovered())
		{
			result.IsHovered = true;
		}

		GUI::TextWrapped(label);
		if (GUI::IsItemHovered())
			GUI::SetTooltip(label);

		//GUI::TextDisabled(typeName);

		return result;
	}

	bool GUI::Selectable(const char* label, const bool selected, const std::vector<ESelectableFlag>& flags, const SVector2<F32>& size)
	{
		return Instance->Impl->Selectable(label, selected, flags, size);
	}

	SGuiMultiSelectIO GUI::BeginMultiSelect(const std::vector<EMultiSelectFlag>& flags, I32 selectionSize, I32 itemsCount)
	{
		return Instance->Impl->BeginMultiSelect(flags, selectionSize, itemsCount);
	}

	SGuiMultiSelectIO GUI::EndMultiSelect()
	{
		return Instance->Impl->EndMultiSelect();
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

	bool GUI::IsItemClicked(const EGUIMouseButton button)
	{
		return Instance->Impl->IsItemClicked(button);
	}

	bool GUI::IsMouseReleased(I32 mouseButton)
	{
		return Instance->Impl->IsMouseReleased(mouseButton);
	}

	bool GUI::IsItemHovered()
	{
		return Instance->Impl->IsItemHovered();
	}

	bool GUI::IsItemVisible()
	{
		return Instance->Impl->IsItemVisible();
	}

	bool GUI::IsWindowFocused()
	{
		return Instance->Impl->IsWindowFocused();
	}

	bool GUI::IsWindowHovered()
	{
		return Instance->Impl->IsWindowHovered();
	}

	void GUI::BeginVertical(const char* label, const SVector2<F32>& size)
	{
		Instance->Impl->BeginVertical(label, size);
	}

	void GUI::EndVertical()
	{
		Instance->Impl->EndVertical();
	}

	void GUI::BeginHorizontal(const char* label, const SVector2<F32>& size)
	{
		Instance->Impl->BeginHorizontal(label, size);
	}

	void GUI::EndHorizontal()
	{
		Instance->Impl->EndHorizontal();
	}

	void GUI::Indent(const F32 indent)
	{
		Instance->Impl->Indent(indent);
	}

	void GUI::Unindent(const F32 indent)
	{
		Instance->Impl->Unindent(indent);
	}

	SVector2<F32> GUI::GetCursorPos()
	{
		return Instance->Impl->GetCursorPos();;
	}

	void GUI::SetCursorPos(const SVector2<F32>& cursorPos)
	{
		Instance->Impl->SetCursorPos(cursorPos);
	}

	F32 GUI::GetCursorPosX()
	{
		return Instance->Impl->GetCursorPosX();
	}

	F32 GUI::GetCursorPosY()
	{
		return Instance->Impl->GetCursorPosY();
	}

	void GUI::SetCursorPosX(const F32 cursorPosX)
	{
		Instance->Impl->SetCursorPosX(cursorPosX);
	}

	void GUI::SetCursorPosY(const F32 cursorPosY)
	{
		Instance->Impl->SetCursorPosY(cursorPosY);
	}

	void GUI::OffsetCursorPos(const SVector2<F32>& cursorOffset)
	{
		GUI::SetCursorPos(GUI::GetCursorPos() + cursorOffset);
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

	SVector4 GUI::GetLastRect()
	{
		return Instance->Impl->GetLastRect();
	}

	void GUI::Spring(const F32 weight, const F32 spacing)
	{
		return Instance->Impl->Spring(weight, spacing);
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

	void GUI::PushScriptStyleVar(const EScriptStyleVar styleVar, const SVector4& value)
	{
		Instance->Impl->PushScriptStyleVar(styleVar, value);
	}

	void GUI::PushScriptStyleVar(const EScriptStyleVar styleVar, const SVector2<F32>& value)
	{
		Instance->Impl->PushScriptStyleVar(styleVar, value);
	}

	void GUI::PopScriptStyleVar(const I32 count)
	{
		Instance->Impl->PopScriptStyleVar(count);
	}

	std::vector<SColor> GUI::GetStyleColors()
	{
		return Instance->Impl->GetStyleColors();
	}

	void GUI::PushStyleColor(const EStyleColor styleColor, const SColor& color)
	{
		Instance->Impl->PushStyleColor(styleColor, color);
	}

	void GUI::PushScriptStyleColor(const EScriptStyleColor styleColor, const SColor& color)
	{
		Instance->Impl->PushScriptStyleColor(styleColor, color);
	}

	void GUI::PopStyleColor()
	{
		Instance->Impl->PopStyleColor();
	}

	void GUI::PopScriptStyleColor()
	{
		Instance->Impl->PopScriptStyleColor();
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

	bool GUI::IsLeftMouseHeld()
	{
		return Instance->Impl->IsLeftMouseHeld();
	}

	bool GUI::IsDoubleClick()
	{
		return Instance->Impl->IsDoubleClick();
	}

	bool GUI::IsShiftHeld()
	{
		return Instance->Impl->IsShiftHeld();
	}

	bool GUI::IsControlHeld()
	{
		return Instance->Impl->IsControlHeld();
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

	SVector2<F32> GUI::GetMousePosition()
	{
		return Instance->Impl->GetMousePosition();
	}

	void GUI::AddRectFilled(const SVector2<F32>& cursorScreenPos, const SVector2<F32>& size, const SColor& color)
	{
		Instance->Impl->AddRectFilled(cursorScreenPos, size, color);
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

	void GUI::BeginScript(const char* label, const SVector2<F32>& size)
	{
		Instance->Impl->BeginScript(label, size);
	}

	void GUI::EndScript()
	{
		Instance->Impl->EndScript();
	}

	void GUI::BeginNode(const U64 id)
	{
		Instance->Impl->BeginNode(id);
	}

	void GUI::EndNode()
	{
		Instance->Impl->EndNode();
	}

	void GUI::SetNodePosition(const U64 id, const SVector2<F32>& position)
	{
		Instance->Impl->SetNodePosition(id, position);
	}

	void GUI::BeginPin(const U64 id, const EGUIPinDirection direction)
	{
		Instance->Impl->BeginPin(id, direction);
	}

	void GUI::EndPin()
	{
		Instance->Impl->EndPin();
	}

	void GUI::DrawPinIcon(const SVector2<F32>& size, const EGUIIconType type, const bool isConnected, const SColor& color, const bool highlighted)
	{
		Instance->Impl->DrawPinIcon(size, type, isConnected, color, highlighted);
	}

	void GUI::DrawNodeHeader(const U64 nodeID, intptr_t textureID, const SVector2<F32>& posMin, const SVector2<F32>& posMax, const SVector2<F32>& uvMin, const SVector2<F32>& uvMax, const SColor& color, const F32 rounding)
	{
		Instance->Impl->DrawNodeHeader(nodeID, textureID, posMin, posMax, uvMin, uvMax, color, rounding);
	}

	void GUI::Link(const U64 linkID, const U64 startPinID, const U64 endPinID, const SColor& color, const F32 thickness)
	{
		Instance->Impl->Link(linkID, startPinID, endPinID, color, thickness);
	}

	bool GUI::BeginScriptCreate()
	{
		return Instance->Impl->BeginScriptCreate();
	}

	void GUI::EndScriptCreate()
	{
		Instance->Impl->EndScriptCreate();
	}

	bool GUI::BeginScriptDelete()
	{
		return Instance->Impl->BeginScriptDelete();;
	}

	void GUI::EndScriptDelete()
	{
		Instance->Impl->EndScriptDelete();
	}

	void GUI::SuspendScript()
	{
		Instance->Impl->SuspendScript();
	}

	void GUI::ResumeScript()
	{
		Instance->Impl->ResumeScript();
	}

	bool GUI::QueryNewLink(U64& inputPinID, U64& outputPinID)
	{
		return Instance->Impl->QueryNewLink(inputPinID, outputPinID);
	}

	bool GUI::QueryDeletedLink(U64& linkID)
	{
		return Instance->Impl->QueryDeletedLink(linkID);
	}

	bool GUI::QueryDeletedNode(U64& nodeID)
	{
		return Instance->Impl->QueryDeletedNode(nodeID);
	}

	bool GUI::AcceptNewScriptItem()
	{
		return Instance->Impl->AcceptNewScriptItem();
	}

	bool GUI::AcceptDeletedScriptItem()
	{
		return Instance->Impl->AcceptDeletedScriptItem();
	}

	bool GUI::ShowScriptContextMenu()
	{
		return Instance->Impl->ShowScriptContextMenu();;
	}

	void GUI::LogToClipboard()
	{
		Instance->Impl->LogToClipboard();
	}

	void GUI::LogFinish()
	{
		Instance->Impl->LogFinish();
	}

	void GUI::CopyToClipboard(const char* text)
	{
		Instance->Impl->CopyToClipboard(text);
	}

	std::string GUI::CopyFromClipboard()
	{
		return Instance->Impl->CopyFromClipboard();
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
			strncpy_s(dst, count, src, count - 1);
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