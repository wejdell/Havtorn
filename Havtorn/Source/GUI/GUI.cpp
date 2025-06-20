// Copyright 2025 Team Havtorn. All Rights Reserved.

#include "GUI.h"
#include <imgui.h>
#include <imgui_internal.h>
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

			//NE::SetCurrentEditor(NodeEditorContext);
			//NE::Begin("My Editor", ImVec2(0.0, 0.0f));
			//int uniqueId = 1;
			//// Start drawing nodes.
			//	NE::BeginNode(uniqueId++);
			//		ImGui::Text("Node A");
			//			NE::BeginPin(uniqueId++, NE::PinKind::Input);
			//				ImGui::Text("-> In");
			//			NE::EndPin();
			//		ImGui::SameLine();
			//			NE::BeginPin(uniqueId++, NE::PinKind::Output);
			//				ImGui::Text("Out ->");
			//			NE::EndPin();
			//	NE::EndNode();
			//NE::End();
			//NE::SetCurrentEditor(nullptr);
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

		void InputText(const char* label, char* buf, size_t bufSize, ImGuiInputTextCallback callback, void* data)
		{
			ImGui::InputText(label, buf, bufSize, 0, callback, data);
		}

		void SetTooltip(const char* fmt, va_list args)
		{
			ImGui::SetTooltipV(fmt, args);	
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

		bool IsItemClicked()
		{
			return ImGui::IsItemClicked();
		}

		bool IsItemHovered()
		{
			return ImGui::IsItemHovered();
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

		SVector2<F32> CalculateTextSize(const char* text)
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

		// TODO.NW: Move to script asset editor
		ImColor GetPinTypeColor(EGUIPinType type)
		{
			switch (type)
			{
			default:
			case EGUIPinType::Flow:     return ImColor(255, 255, 255);
			case EGUIPinType::Bool:     return ImColor(220, 48, 48);
			case EGUIPinType::Int:      return ImColor(68, 201, 156);
			case EGUIPinType::Float:    return ImColor(147, 226, 74);
			case EGUIPinType::String:   return ImColor(124, 21, 153);
			case EGUIPinType::Vector:   return ImColor(255, 206, 27);
			case EGUIPinType::Object:   return ImColor(51, 150, 215);
			case EGUIPinType::ObjectArray:   return ImColor(51, 150, 215);
			case EGUIPinType::Function: return ImColor(218, 0, 183);
			case EGUIPinType::Delegate: return ImColor(255, 48, 48);
			}
		};

		// TODO.NW: Make context action for splitting objects? e.g. in vector
		void DrawPinIcon(const SGUIPin& pin, bool connected, int alpha)
		{
			EGUIIconType iconType;
			ImColor color = GetPinTypeColor(pin.Type);
			color.Value.w = alpha / 255.0f;
			switch (pin.Type)
			{
			case EGUIPinType::Flow:     iconType = EGUIIconType::Flow;   break;
			case EGUIPinType::Bool:     iconType = EGUIIconType::Circle; break;
			case EGUIPinType::Int:      iconType = EGUIIconType::Circle; break;
			case EGUIPinType::Float:    iconType = EGUIIconType::Circle; break;
			case EGUIPinType::String:   iconType = EGUIIconType::Circle; break;
			case EGUIPinType::Vector:   iconType = EGUIIconType::Circle; break;
			case EGUIPinType::Object:   iconType = EGUIIconType::Circle; break;
			case EGUIPinType::ObjectArray:   iconType = EGUIIconType::Grid; break;
			case EGUIPinType::Function: iconType = EGUIIconType::Circle; break;
			case EGUIPinType::Delegate: iconType = EGUIIconType::Square; break;
			default:
				return;
			}

			ax::Widgets::Icon(ImVec2(static_cast<float>(24.0f), static_cast<float>(24.0f)), static_cast<ax::Drawing::IconType>(iconType), connected, color, ImColor(32, 32, 32, alpha));
		};

		bool CanCreateLink(SGUIPin* a, SGUIPin* b)
		{
			if (!a || !b || a == b || a->Direction == b->Direction || a->Type != b->Type || a->Node == b->Node)
				return false;

			return true;
		}

		bool IsPinLinked(U64 id, const std::vector<SGUILink>& links)
		{
			if (!id)
				return false;

			for (auto& link : links)
				if (link.StartPinID == id || link.EndPinID == id)
					return true;

			return false;
		}

		void ShowStyleEditor(bool* show = nullptr)
		{
			if (!ImGui::Begin("Style", show))
			{
				ImGui::End();
				return;
			}

			auto paneWidth = ImGui::GetContentRegionAvail().x;

			auto& editorStyle = NE::GetStyle();
			ImGui::BeginHorizontal("Style buttons", ImVec2(paneWidth, 0), 1.0f);
			ImGui::TextUnformatted("Values");
			ImGui::Spring();
			if (ImGui::Button("Reset to defaults"))
				editorStyle = NE::Style();
			ImGui::EndHorizontal();
			ImGui::Spacing();
			ImGui::DragFloat4("Node Padding", &editorStyle.NodePadding.x, 0.1f, 0.0f, 40.0f);
			ImGui::DragFloat("Node Rounding", &editorStyle.NodeRounding, 0.1f, 0.0f, 40.0f);
			ImGui::DragFloat("Node Border Width", &editorStyle.NodeBorderWidth, 0.1f, 0.0f, 15.0f);
			ImGui::DragFloat("Hovered Node Border Width", &editorStyle.HoveredNodeBorderWidth, 0.1f, 0.0f, 15.0f);
			ImGui::DragFloat("Hovered Node Border Offset", &editorStyle.HoverNodeBorderOffset, 0.1f, -40.0f, 40.0f);
			ImGui::DragFloat("Selected Node Border Width", &editorStyle.SelectedNodeBorderWidth, 0.1f, 0.0f, 15.0f);
			ImGui::DragFloat("Selected Node Border Offset", &editorStyle.SelectedNodeBorderOffset, 0.1f, -40.0f, 40.0f);
			ImGui::DragFloat("Pin Rounding", &editorStyle.PinRounding, 0.1f, 0.0f, 40.0f);
			ImGui::DragFloat("Pin Border Width", &editorStyle.PinBorderWidth, 0.1f, 0.0f, 15.0f);
			ImGui::DragFloat("Link Strength", &editorStyle.LinkStrength, 1.0f, 0.0f, 500.0f);
			//ImVec2  SourceDirection;
			//ImVec2  TargetDirection;
			ImGui::DragFloat("Scroll Duration", &editorStyle.ScrollDuration, 0.001f, 0.0f, 2.0f);
			ImGui::DragFloat("Flow Marker Distance", &editorStyle.FlowMarkerDistance, 1.0f, 1.0f, 200.0f);
			ImGui::DragFloat("Flow Speed", &editorStyle.FlowSpeed, 1.0f, 1.0f, 2000.0f);
			ImGui::DragFloat("Flow Duration", &editorStyle.FlowDuration, 0.001f, 0.0f, 5.0f);
			//ImVec2  PivotAlignment;
			//ImVec2  PivotSize;
			//ImVec2  PivotScale;
			//float   PinCorners;
			//float   PinRadius;
			//float   PinArrowSize;
			//float   PinArrowWidth;
			ImGui::DragFloat("Group Rounding", &editorStyle.GroupRounding, 0.1f, 0.0f, 40.0f);
			ImGui::DragFloat("Group Border Width", &editorStyle.GroupBorderWidth, 0.1f, 0.0f, 15.0f);

			ImGui::Separator();

			static ImGuiColorEditFlags edit_mode = ImGuiColorEditFlags_DisplayRGB;
			ImGui::BeginHorizontal("Color Mode", ImVec2(paneWidth, 0), 1.0f);
			ImGui::TextUnformatted("Filter Colors");
			ImGui::Spring();
			ImGui::RadioButton("RGB", &edit_mode, ImGuiColorEditFlags_DisplayRGB);
			ImGui::Spring(0);
			ImGui::RadioButton("HSV", &edit_mode, ImGuiColorEditFlags_DisplayHSV);
			ImGui::Spring(0);
			ImGui::RadioButton("HEX", &edit_mode, ImGuiColorEditFlags_DisplayHex);
			ImGui::EndHorizontal();

			static ImGuiTextFilter filter;
			filter.Draw("##filter", paneWidth);

			ImGui::Spacing();

			ImGui::PushItemWidth(-160);
			for (int i = 0; i < NE::StyleColor_Count; ++i)
			{
				auto name = NE::GetStyleColorName((NE::StyleColor)i);
				if (!filter.PassFilter(name))
					continue;

				ImGui::ColorEdit4(name, &editorStyle.Colors[i].x, edit_mode);
			}
			ImGui::PopItemWidth();

			ImGui::End();
		}

		void ShowLeftPane(float paneWidth)
		{
			auto& io = ImGui::GetIO();

			ImGui::BeginChild("Selection", ImVec2(paneWidth, 0));

			static bool showStyleEditor = true;
			ShowStyleEditor(&showStyleEditor);

			ImGui::EndChild();
		}

		static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
		{
			using namespace ImGui;
			ImGuiContext& g = *GImGui;
			ImGuiWindow* window = g.CurrentWindow;
			ImGuiID id = window->GetID("##Splitter");
			ImRect bb;
			bb.Min = ImVec2(window->DC.CursorPos.x + (split_vertically ? ImVec2(*size1, 0.0f).x : ImVec2(0.0f, *size1).x), window->DC.CursorPos.y + (split_vertically ? ImVec2(*size1, 0.0f).y : ImVec2(0.0f, *size1).y));
			bb.Max = ImVec2(bb.Min.x + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f).x, bb.Min.y + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f).y);
			return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
		}

		bool IsPinTypeLiteral(SGUIPin& pin)
		{
			return pin.Type == EGUIPinType::String || pin.Type == EGUIPinType::Bool || pin.Type == EGUIPinType::Int || pin.Type == EGUIPinType::Float;
		}

		bool DrawLiteralTypePin(SGUIPin& pin)
		{
			bool wasPinValueModified = false;
			constexpr float emptyItemWidth = 50.0f;

			float cursorPosY = ImGui::GetCursorPosY();
			ImGui::SetCursorPosY(cursorPosY + 2.0f);

			switch (pin.Type)
			{
			case EGUIPinType::String:
			{
				if (pin.IsDataUnset())
					pin.Data = "";

				ImGui::PushID(pin.UID);
				ImGui::PushItemWidth(emptyItemWidth);
				wasPinValueModified = ImGui::InputText("##edit", (char*)std::get<std::string>(pin.Data).c_str(), 127);
				ImGui::PopItemWidth();
				ImGui::PopID();
				break;
			}
			case EGUIPinType::Int:
			{
				if (pin.IsDataUnset())
					pin.Data = 0;

				ImGui::PushID(pin.UID);
				ImGui::PushItemWidth(emptyItemWidth);
				wasPinValueModified = ImGui::InputInt("##edit", &std::get<I32>(pin.Data));
				ImGui::PopItemWidth();
				ImGui::PopID();
				break;
			}
			case EGUIPinType::Bool:
			{
				if (pin.IsDataUnset())
					pin.Data = false;

				ImGui::PushID(pin.UID);
				ImGui::PushItemWidth(emptyItemWidth);
				wasPinValueModified = ImGui::Checkbox("##edit", &std::get<bool>(pin.Data));
				ImGui::PopItemWidth();
				ImGui::PopID();
				break;
			}
			case EGUIPinType::Float:
			{
				if (pin.IsDataUnset())
					pin.Data = 0.0f;

				ImGui::PushID(pin.UID);
				ImGui::PushItemWidth(emptyItemWidth);
				wasPinValueModified = ImGui::InputFloat("##edit", &std::get<F32>(pin.Data));
				ImGui::PopItemWidth();
				ImGui::PopID();
				break;
			}
			default:
				assert(0);
			}

			return wasPinValueModified;
		}
			
		SGUINode* GetNodeFromPinID(U64 id, std::vector<SGUINode>& nodes)
		{
			for (auto& node : nodes)
			{
				for (auto& pin : node.Inputs)
				{
					if (pin.UID == id)
					{
						return &node;
					}
				}

				for (auto& pin : node.Outputs)
				{
					if (pin.UID == id)
					{
						return &node;
					}
				}

			}
			return nullptr;
		}

		SGUIPin* GetPinFromID(U64 id, SGUINode& node)
		{
			for (auto& pin : node.Inputs)
			{
				if (pin.UID == id)
					return &pin;
			}
			for (auto& pin : node.Outputs)
			{
				if (pin.UID == id)
					return &pin;
			}
			return nullptr;
		}

		SGUIPin* GetPinFromID(U64 id, std::vector<SGUINode>& nodes)
		{
			for (auto& node : nodes)
			{
				for (auto& pin : node.Inputs)
				{
					if (pin.UID == id)
						return &pin;
				}
				for (auto& pin : node.Outputs)
				{
					if (pin.UID == id)
						return &pin;
				}
			}
			return nullptr;
		}

		ImVec2 CalculateRequiredSize(const SGUINode& node)
		{
			constexpr float iconSize = 24.0f;
			constexpr float iconNamePadding = 6.0f;
			constexpr float iconPadding = iconSize + iconNamePadding * 1.5f;
			constexpr float headerHeight = 12.0f;

			int maxPinColumnLength = UMath::Max(node.Inputs.size(), node.Outputs.size());
			float inputMaxRequired = 0.0f;
			float outputMaxRequired = 0.0f;
			for (auto& pin : node.Inputs)
			{
				float nameWidth = ImGui::CalcTextSize(pin.Name.c_str()).x;
				if (nameWidth > inputMaxRequired)
					inputMaxRequired = nameWidth + iconPadding;
			}
			for (auto& pin : node.Outputs)
			{
				float nameWidth = ImGui::CalcTextSize(pin.Name.c_str()).x;
				if (nameWidth > outputMaxRequired)
					outputMaxRequired = nameWidth + iconPadding;
			}
			float requiredWidth = UMath::Max(ImGui::CalcTextSize(node.Name.c_str()).x, inputMaxRequired + outputMaxRequired);
			if (node.Type == EGUINodeType::Simple)
				requiredWidth += 50.0f;
			requiredWidth = UMath::Max(requiredWidth, 100.0f);
			return ImVec2(requiredWidth, headerHeight + 1.5f * iconNamePadding + iconPadding * float(maxPinColumnLength));
		}

		void OpenScript(const std::vector<SGUINode>& nodes, const std::vector<SGUILink>& links)
		{
			auto toImVec = [](const SVector2<F32>& position) { return ImVec2(position.X, position.Y); };

			for (const SGUINode& node : nodes)
				NE::SetNodePosition(node.UID, toImVec(node.Position));
		}

		void CloseScript(std::vector<SGUINode>& nodes, std::vector<SGUILink>& links)
		{
			auto fromImVec = [](const ImVec2& position) { return SVector2<F32>(position.x, position.y); };

			for (SGUILink& link : links)
				NE::DeleteLink(link.UID);

			for (SGUINode& node : nodes)
			{
				node.Position = fromImVec(NE::GetNodePosition(node.UID));
				NE::DeleteNode(node.UID);
			}
		}

		SNodeOperation RenderScript(std::vector<SGUINode>& nodes, std::vector<SGUILink>& links, const std::vector<SGUINodeContext>& registeredContexts)
		{
			NE::SetCurrentEditor(NodeEditorContext);
			//static float leftPaneWidth = 400.0f;
			//static float rightPaneWidth = 800.0f;
			//Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

			//ShowLeftPane(leftPaneWidth - 4.0f);

			//ImGui::SameLine(0.0f, 12.0f);

			NE::Begin("ScriptEditor", ImVec2(0.0f, 0.0f));
			auto toImColor = [](const SColor& color) { return ImColor(color.R, color.G, color.B, color.A); };
			auto toImVec = [](const SVector2<F32>& position) { return ImVec2(position.X, position.Y); };

			SNodeOperation result;

			constexpr float headerHeight = 12.0f;
			constexpr float nodeNameIndent = 6.0f;
			constexpr float pinNameOffset = 4.0f;
			constexpr float outputIndent = 0.0f;

			//NE::PushStyleColor(NE::StyleColor_Bg, toImColor(SColor(20)));
			for (SGUINode& node : nodes)
			{	
				ImVec2 requiredSize = CalculateRequiredSize(node);

				// Start drawing nodes.
				NE::PushStyleVar(NE::StyleVar_NodePadding, ImVec4(8, 4, 8, 8));
				NE::BeginNode(node.UID);
				
				if (!node.HasBeenInitialized)
					NE::SetNodePosition(node.UID, toImVec(node.Position));

				ImGui::PushID(node.UID);
				ImGui::BeginVertical("node", requiredSize);

				ImRect headerRect = ImRect();

				ImGui::BeginHorizontal("header", ImVec2(requiredSize.x, headerHeight));
				ImVec2 nodeNameCursorStart = ImGui::GetCursorPos();
				nodeNameCursorStart.y += 2.0f;
				ImGui::SetCursorPos(nodeNameCursorStart);
				ImGui::TextUnformatted(node.Name.c_str());
				ImGui::EndHorizontal();
				headerRect = GImGui->LastItemData.Rect;
				ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.y * 3.0f);

				int maxPinColumnLength = UMath::Max(node.Inputs.size(), node.Outputs.size());
				for (int i = 0; i < maxPinColumnLength; i++)
				{
					SGUIPin* inputPin = node.Inputs.size() > i ? &node.Inputs[i] : nullptr;
					SGUIPin* outputPin = node.Outputs.size() > i ? &node.Outputs[i] : nullptr;

					if (inputPin != nullptr && inputPin->Direction == EGUIPinDirection::Input)
					{
						NE::PushStyleVar(NE::StyleVar_PivotAlignment, ImVec2(0.1f, 0.5f));
						NE::BeginPin(inputPin->UID, NE::PinKind::Input);

						const bool isPinLinked = IsPinLinked(inputPin->UID, links);

						if (IsPinTypeLiteral(*inputPin) && !isPinLinked)
						{
							bool wasPinValueModified = DrawLiteralTypePin(*inputPin);
							if (wasPinValueModified)
								result.ModifiedLiteralValuePin = *inputPin;
						}
						else
						{
							DrawPinIcon(*inputPin, isPinLinked, 255);
						}

						ImGui::SameLine(0, 0);
						float cursorY = ImGui::GetCursorPosY();
						ImGui::SetCursorPosY(cursorY + pinNameOffset);
						ImGui::Text(inputPin->Name.c_str());
						ImGui::SetCursorPosY(cursorY - pinNameOffset);

						NE::EndPin();
						NE::PopStyleVar();
					}
					
					if (outputPin != nullptr && outputPin->Direction == EGUIPinDirection::Output)
					{
						if (inputPin != nullptr)
							ImGui::SameLine();

						float nameWidth = ImGui::CalcTextSize(outputPin->Name.c_str()).x;
						constexpr float iconSize = 24.0f;
						float indent = requiredSize.x - nameWidth - iconSize;
						ImGui::Indent(indent);

						NE::PushStyleVar(NE::StyleVar_PivotAlignment, ImVec2(0.9f, 0.5));
						NE::BeginPin(outputPin->UID, NE::PinKind::Output);

						float cursorX = ImGui::GetCursorPosX();
						float cursorY = ImGui::GetCursorPosY();
						ImGui::SetCursorPosY(cursorY + pinNameOffset);
						ImGui::Text(outputPin->Name.c_str());
						ImGui::SetCursorPos(ImVec2(cursorX + nameWidth, cursorY));

						DrawPinIcon(*outputPin, IsPinLinked(outputPin->UID, links), 255);
						NE::EndPin();
						NE::PopStyleVar();
						ImGui::Unindent(indent);
					}
				}

				ImGui::EndVertical();
				ImRect contentRect = GImGui->LastItemData.Rect;
				NE::EndNode();

				if (ImGui::IsItemVisible())
				{
					auto drawList = NE::GetNodeBackgroundDrawList(node.UID);

					const auto halfBorderWidth = NE::GetStyle().NodeBorderWidth * 0.5f;
					const auto uv = ImVec2(
						headerRect.GetWidth() / (float)(4.0f * 64.0f),
						headerRect.GetHeight() / (float)(4.0f * 64.0f));

					ImVec2 imagePadding = ImVec2(8 - halfBorderWidth, 4 - halfBorderWidth);
					ImVec2 imagePaddingMax = ImVec2(8 - halfBorderWidth, 10 - halfBorderWidth);
					ImVec2 imageMin = ImVec2(headerRect.Min.x - imagePadding.x, headerRect.Min.y - imagePadding.y);
					ImVec2 imageMax = ImVec2(headerRect.Max.x + imagePaddingMax.x, headerRect.Max.y + imagePaddingMax.y);
					drawList->AddImageRounded(BlueprintBackgroundImage, imageMin, imageMax, ImVec2(0.0f, 0.0f), uv, toImColor(node.Color), NE::GetStyle().NodeRounding, ImDrawFlags_RoundCornersAll);
				}
				ImGui::PopID();
				NE::PopStyleVar();
			}

			for (auto& linkInfo : links)
			{
				SGUIPin* startPin = GetPinFromID(linkInfo.StartPinID, nodes);
				NE::Link(linkInfo.UID, linkInfo.StartPinID, linkInfo.EndPinID, startPin != nullptr ? GetPinTypeColor(startPin->Type).Value : ImVec4(1,1,1,1));
			}

			// Handle creation action, returns true if editor want to create new object (node or link)
			if (NE::BeginCreate())
			{
				NE::PinId inputPinId, outputPinId;
				if (NE::QueryNewLink(&inputPinId, &outputPinId))
				{
					if (inputPinId && outputPinId)
					{
						if (NE::AcceptNewItem())
						{
							SGUINode* firstNode = GetNodeFromPinID(inputPinId.Get(), nodes);
							SGUINode* secondNode = GetNodeFromPinID(outputPinId.Get(), nodes);
							assert(firstNode);
							assert(secondNode);

							if (firstNode != secondNode)
							{						
								SGUIPin* firstPin = GetPinFromID(inputPinId.Get(), *firstNode);
								SGUIPin* secondPin = GetPinFromID(outputPinId.Get(), *secondNode);

								bool canAddlink = true;
								if (firstPin && secondPin)
								{
									if (firstPin->Direction == EGUIPinDirection::Input && secondPin->Direction == EGUIPinDirection::Input)
									{
										canAddlink = false;
									}
								}

								if (firstPin->Type != secondPin->Type)
								{
									canAddlink = false;
								}

								// TODO.NW: Think about these, certain rules apply to flows vs nonflows/inputs vs outputs
								//if (!firstNode->CanAddLink(inputPinId.Get()))
								//{
								//	canAddlink = false;
								//}
								//if (!secondNode->CanAddLink(outputPinId.Get()))
								//{
								//	canAddlink = false;
								//}

								//if (firstNode->HasLinkBetween(inputPinId.Get(), outputPinId.Get()))
								//{
								//	canAddlink = false;
								//}

								if (canAddlink)
								{
									// TODO.NW: Add functions to populate this with function call
									static U64 linkID = 99;
									result.NewLink.UID = linkID++;
									result.NewLink.StartPinID = firstPin->UID;
									result.NewLink.EndPinID = secondPin->UID;

									//if (secondPin->Type == EGUIPinType::Unknown)
									//{
									//	secondNode->ChangPinTypes(firstPin->Type);
									//}
									//int linkId = myNextLinkIdCounter++;
									//firstNode->AddLinkToVia(secondNode, inputPinId.Get(), outputPinId.Get(), linkId);
									//secondNode->AddLinkToVia(firstNode, outputPinId.Get(), inputPinId.Get(), linkId);

									//bool aIsCyclic = false;
									//WillBeCyclic(firstNode, secondNode, aIsCyclic, firstNode);
									//if (aIsCyclic || !canAddlink)
									//{
									//	firstNode->RemoveLinkToVia(secondNode, inputPinId.Get());
									//	secondNode->RemoveLinkToVia(firstNode, outputPinId.Get());
									//}
									//else
									//{
									//	// Depending on if you drew the new link from the output to the input we need to create the link as the flow FROM->TO to visualize the correct flow
									//	if (firstPin->Direction == EGUIPinDirection::Input)
									//	{
									//		myLinks.push_back({ NE::LinkId(linkId), outputPinId, inputPinId });
									//	}
									//	else
									//	{
									//		myLinks.push_back({ NE::LinkId(linkId), inputPinId, outputPinId });
									//	}		

									//	std::cout << "push add link command!" << std::endl;
									//	myUndoCommands.push({ CommandAction::AddLink, firstNode, secondNode, myLinks.back(), 0});
									//
									//	ReTriggerTree();
									//}
								}
								
							}
						}
					}
				}
			}
			NE::EndCreate(); 

			if (NE::BeginDelete())
			{
				NE::LinkId deletedLinkId = 0;
				while (NE::QueryDeletedLink(&deletedLinkId))
				{
					if (NE::AcceptDeletedItem())
					{
						for (SGUILink& link : links)
						{
							if (link.UID == deletedLinkId.Get())
							{
								result.RemovedLinks.emplace_back(link);

								//if (myShouldPushCommand)
								//{
								//	std::cout << "push remove link action!" << std::endl;
								//	myUndoCommands.push({ CommandAction::RemoveLink, firstNode, secondNode, link, 0/*static_cast<unsigned int>(link.Id.Get())*//*, static_cast<unsigned int>(link.UID.Get()), static_cast<unsigned int>(link.OutputId.Get())*/ });
								//}
							}
						}
					}
				}
				NE::NodeId nodeId = 0;
				while (NE::QueryDeletedNode(&nodeId))
				{
					if (NE::AcceptDeletedItem())
					{
						for (SGUINode& node : nodes)
						{
							if (node.UID == nodeId.Get())
							{
								result.RemovedNodes.emplace_back(node);

								//if (myShouldPushCommand) 
								//{
								//	std::cout << "Push delete command!" << std::endl;
								//	myUndoCommands.push({ CommandAction::Delete, (*it), nullptr,  {0,0,0}, (*it)->UID });
								//}
							}
						}
					}
				}
			}
			NE::EndDelete(); 
	
			auto openPopupPosition = ImGui::GetMousePos();
			NE::Suspend();

			if (NE::ShowBackgroundContextMenu())
			{
				ImGui::OpenPopup("Create New Node");
			}
			NE::Resume();

			NE::Suspend();
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
	
			if (ImGui::BeginPopup("Create New Node"))
			{
				auto newNodePostion = openPopupPosition;
				//CNodeType** types = CNodeTypeCollector::GetAllNodeTypes();
				//unsigned short noOfTypes = CNodeTypeCollector::GetNodeTypeCount();

				//std::map<std::string, std::vector<CNodeType*>> cats;

				//for (int i = 0; i < noOfTypes; i++)
				//{
				//	cats[types[i]->GetNodeTypeCategory()].push_back(types[i]);
				//}

				//ImGui::PushItemWidth(100.0f);
				//ImGui::InputText("##edit", (char*)myMenuSeachField, 127);
				//if (mySetSearchFokus)
				//{
				//	ImGui::SetKeyboardFocusHere(0);
				//}
				//mySetSearchFokus = false;
				//ImGui::PopItemWidth();

				//if (myMenuSeachField[0] != '\0')
				//{

				//	std::vector<SDistBestResult> distanceResults;
				//	for (int i = 0; i < noOfTypes; i++)
				//	{
				//		distanceResults.push_back(SDistBestResult());
				//		SDistBestResult& dist = distanceResults.back();
				//		dist.ourInstance = types[i];
				//		dist.myScore = uiLevenshteinDistance<std::string>(types[i]->Name, myMenuSeachField);
				//	}

				//	std::sort(distanceResults.begin(), distanceResults.end(), less_than_key());

				//	int firstCost = distanceResults[0].myScore;
				//	for (int i = 0; i < distanceResults.size(); i++)
				//	{
				//		CNodeInstance* node = nullptr;
				//		if (ImGui::MenuItem(distanceResults[i].ourInstance->Name.c_str()))
				//		{
				//			node = new CNodeInstance();

				//			int nodeType = i;
				//			node.myNodeType = distanceResults[i].ourInstance;
				//			node.ConstructUniquePins();
				//			NE::SetNodePosition(node.UID, newNodePostion);
				//			node.myHasSetEditorPos = true;

				//			myNodeInstancesInGraph.push_back(node);

				//			if (myShouldPushCommand)
				//			{
				//				std::cout << "Push create command!" << std::endl;
				//				myUndoCommands.push({ CommandAction::Create, node, nullptr, {0,0,0}, node.UID});
				//			}
				//		}
				//		int distance = distanceResults[i].myScore - firstCost;
				//		if (distance > 3)
				//		{
				//			break;
				//		}
				//	}

				//}
				//else
				//{
					//for (auto& category : cats)
					//{
					//	std::string theCatName = category.first;
					//	if (theCatName.empty())
					//	{
					//		theCatName = "General";
					//	}

					//	if (ImGui::BeginMenu(theCatName.c_str()))
					//	{
					//		CNodeInstance* node = nullptr;
					//		for (int i = 0; i < category.second.size(); i++)
					//		{
					//			CNodeType* type = category.second[i];
					//			if (ImGui::MenuItem(type->Name.c_str()))
					//			{
					//				node = new CNodeInstance();

					//				int nodeType = i;
					//				node.myNodeType = type;
					//				node.ConstructUniquePins();
					//				NE::SetNodePosition(node.UID, newNodePostion);
					//				node.myHasSetEditorPos = true;

					//				myNodeInstancesInGraph.push_back(node);
					//		
					//				if (myShouldPushCommand)
					//				{
					//					std::cout << "Push create command!" << std::endl;
					//					myUndoCommands.push({CommandAction::Create, node, nullptr, {0,0,0}, node.UID});
					//				}
					//			}
					//		}
					//		ImGui::EndMenu();
					//	}
					//}
					
					for (auto& context : registeredContexts)
					{
						if (ImGui::BeginMenu(context.Category.c_str()))
						{
							if (ImGui::MenuItem(context.Name.c_str()))
							{
								result.NewNodeContext = context;
								result.NewNodePosition = { openPopupPosition.x, openPopupPosition.y	};
							}
							ImGui::EndMenu();
						}
					}
				//}		
				ImGui::EndPopup();
			}
			else
			{
				//mySetSearchFokus = true;
				//memset(&myMenuSeachField[0], 0, sizeof(myMenuSeachField));
			}

			ImGui::PopStyleVar();
			NE::Resume();

			//myShouldPushCommand = true;
	
			if (NE::BeginShortcut())
			{
				/*if (NE::AcceptCopy())
				{
					SaveNodesToClipboard();
				}

				if (NE::AcceptPaste())
				{
					LoadNodesFromClipboard();
				}

				if (NE::AcceptUndo())
				{
					if (!myUndoCommands.empty()) 
					{
						myShouldPushCommand = false;
						NE::ResetShortCutAction();
						auto& command = myUndoCommands.top();
						EditorCommand inverseCommand = command;
						CPin* firstPin;
						CPin* secondPin;

						switch (command.myAction)
						{
						case CGraphManager::CommandAction::Create:
							inverseCommand.myAction = CommandAction::Delete;
							NE::DeleteNode(command.myResourceUID);
							break;
						case CGraphManager::CommandAction::Delete:
							inverseCommand.myAction = CommandAction::Create;
							myNodeInstancesInGraph.push_back(command.myNodeInstance);
							break;
						case CGraphManager::CommandAction::AddLink:
							inverseCommand.myAction = CommandAction::RemoveLink;
							NE::DeleteLink(command.myEditorLinkInfo.Id);
							break;
						case CGraphManager::CommandAction::RemoveLink:
							inverseCommand.myAction = CommandAction::AddLink;
							command.myNodeInstance->AddLinkToVia(command.mySecondNodeInstance, command.myEditorLinkInfo.InputId.Get(), command.myEditorLinkInfo.OutputId.Get(), command.myResourceUID);
							command.mySecondNodeInstance->AddLinkToVia(command.myNodeInstance, command.myEditorLinkInfo.OutputId.Get(), command.myEditorLinkInfo.InputId.Get(), command.myResourceUID);

							firstPin = command.myNodeInstance->GetPinFromID(command.myEditorLinkInfo.InputId.Get());
							secondPin = command.mySecondNodeInstance->GetPinFromID(command.myEditorLinkInfo.OutputId.Get());

							if (firstPin->Direction == EGUIPinDirection::Input)
								myLinks.push_back({ command.myEditorLinkInfo.Id, command.myEditorLinkInfo.InputId, command.myEditorLinkInfo.OutputId });
							else
								myLinks.push_back({ command.myEditorLinkInfo.Id, command.myEditorLinkInfo.OutputId, command.myEditorLinkInfo.InputId });
							ReTriggerTree();
							break;
						default:
							break;
						}
						std::cout << "undo!" << std::endl;
						myUndoCommands.pop();
						std::cout << "Push redo command!" << std::endl;
						myRedoCommands.push(inverseCommand);
					}
				}

				if (NE::AcceptRedo())
				{
					if (!myRedoCommands.empty())
					{
						myShouldPushCommand = false;
						NE::ResetShortCutAction();
						auto& command = myRedoCommands.top();
						EditorCommand inverseCommand = command;
						CPin* firstPin;
						CPin* secondPin;

						switch (command.myAction)
						{
						case CGraphManager::CommandAction::Create:
							inverseCommand.myAction = CommandAction::Delete;
							NE::DeleteNode(command.myResourceUID);
							break;
						case CGraphManager::CommandAction::Delete:
							inverseCommand.myAction = CommandAction::Create;
							myNodeInstancesInGraph.push_back(command.myNodeInstance);
							break;
						case CGraphManager::CommandAction::AddLink:
							inverseCommand.myAction = CommandAction::RemoveLink;
							NE::DeleteLink(command.myEditorLinkInfo.Id);
							break;
						case CGraphManager::CommandAction::RemoveLink:
							inverseCommand.myAction = CommandAction::AddLink;
							command.myNodeInstance->AddLinkToVia(command.mySecondNodeInstance, command.myEditorLinkInfo.InputId.Get(), command.myEditorLinkInfo.OutputId.Get(), command.myResourceUID);
							command.mySecondNodeInstance->AddLinkToVia(command.myNodeInstance, command.myEditorLinkInfo.OutputId.Get(), command.myEditorLinkInfo.InputId.Get(), command.myResourceUID);

							firstPin = command.myNodeInstance->GetPinFromID(command.myEditorLinkInfo.InputId.Get());
							secondPin = command.mySecondNodeInstance->GetPinFromID(command.myEditorLinkInfo.OutputId.Get());

							if (firstPin->Direction == EGUIPinDirection::Input)
								myLinks.push_back({ command.myEditorLinkInfo.Id, command.myEditorLinkInfo.InputId, command.myEditorLinkInfo.OutputId });
							else
								myLinks.push_back({ command.myEditorLinkInfo.Id, command.myEditorLinkInfo.OutputId, command.myEditorLinkInfo.InputId });
							ReTriggerTree();
							break;
						default:
							break;
						}
						std::cout << "redo!" << std::endl;
						myRedoCommands.pop();
						std::cout << "Push undo command!" << std::endl;
						myUndoCommands.push(inverseCommand);
					}
				}*/
			}

			//NE::PopStyleColor(NE::StyleColor_Bg);
			NE::End();
			NE::SetCurrentEditor(nullptr);

			return result;
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

			GUI::TableNextColumn();
			GUI::PushID(id++);

			auto data = assetInspector(entry);
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

	SRenderAssetCardResult GUI::RenderAssetCard(const char* label, const intptr_t& thumbnailID, const char* typeName, const SColor& color, void* dragDropPayloadToSet, U64 payLoadSize)
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

		if (GUI::Selectable("", false, { ESelectableFlag::AllowDoubleClick, ESelectableFlag::AllowOverlap }, cardSize))
		{
			if (GUI::IsDoubleClick())
			{
				result.IsDoubleClicked = true;
			}
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

	bool GUI::IsItemClicked()
	{
		return Instance->Impl->IsItemClicked();
	}

	bool GUI::IsItemHovered()
	{
		return Instance->Impl->IsItemHovered();
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

	void GUI::SetCursorPosX(const F32 cursorPosX)
	{
		Instance->Impl->SetCursorPosX(cursorPosX);
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

	SVector2<F32> GUI::CalculateTextSize(const char* text)
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

	void GUI::OpenScript(const std::vector<SGUINode>& nodes, const std::vector<SGUILink>& links)
	{
		Instance->Impl->OpenScript(nodes, links);
	}

	SNodeOperation GUI::RenderScript(std::vector<SGUINode>& nodes, std::vector<SGUILink>& links, const std::vector<SGUINodeContext>& registeredContexts)
	{
		return Instance->Impl->RenderScript(nodes, links, registeredContexts);
	}

	void GUI::CloseScript(std::vector<SGUINode>& nodes, std::vector<SGUILink>& links)
	{
		Instance->Impl->CloseScript(nodes, links);
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