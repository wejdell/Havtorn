// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <Core/HavtornString.h>
//#include <imgui.h>

namespace ImGui
{
	static int HavtornInputTextResizeCallback(struct ImGuiInputTextCallbackData* data)
	{
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			Havtorn::CHavtornStaticString<255>* customString = reinterpret_cast<Havtorn::CHavtornStaticString<255>*>(data->UserData);
			data->Buf = customString->Data();
			customString->SetLength(static_cast<Havtorn::U8>(data->BufSize) - 1);
		}
		return 0;
	}

	// Note: Because ImGui:: is a namespace you would typically add your own function into the namespace.
	// For example, you code may declare a function 'ImGui::InputText(const char* label, MyString* my_str)'
	static bool HavtornInputText(const char* label, Havtorn::CHavtornStaticString<255>* customString, ImGuiInputTextFlags flags = 0)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		return InputText(label, customString->Data(), (size_t)customString->Length() + 1, flags | ImGuiInputTextFlags_CallbackResize, HavtornInputTextResizeCallback, (void*)customString);
	}
}