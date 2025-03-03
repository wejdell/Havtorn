// Copyright 2024 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <Log.h>

namespace Havtorn
{
	class SColor;

	class COutputLogWindow : public CWindow, public ILogContext
	{
	public:
		COutputLogWindow(const char* displayName, CEditorManager* manager);
		~COutputLogWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		void Log(const ELogCategory category, const std::string& message) override;

		// Portable helpers
		static int   Stricmp(const char* s1, const char* s2) { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
		static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
		static char* Strdup(const char* s) { /*HV_ASSERT(s != nullptr); size_t len = strlen(s) + 1; void* buf = ImGui::MemAlloc(len); HV_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len);*/ }
		static void  Strtrim(char* s) { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

	private:
		void OnDragDropFiles(std::vector<std::string> filePaths);
        void AddLog(SColor color, const char* format, ...);
        void ClearLog();
        void ExecCommand(const char* commandLine);
		I32 TextEditCallback(SGuiInputTextCallbackData* data);

	private:
		struct SLogItem
		{
			char* Text;
			SColor Color = SColor(0.0f, 0.0f, 0.0f, 0.0f);
		};

		char                  InputBuffer[256] = "";
		std::vector<SLogItem>    Items;
		std::vector<const char*> Commands;
		std::vector<char*>       History;
		I32 HistoryPos = -1;    // -1: new line, 0..History.Size-1 browsing history.
		SGuiTextFilter			Filter = ImGuiTextFilter();
		bool					ShouldAutoScroll = true;
		bool					ShouldScrollToBottom = false;
	};
}
