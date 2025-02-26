// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "OutputLogWindow.h"

#include "EditorManager.h"
#include "DockSpaceWindow.h"

#include <imgui.h>

namespace ImGui
{
	using Havtorn::F32;

	COutputLogWindow::COutputLogWindow(const char* displayName, Havtorn::CEditorManager* manager)
		: CWindow(displayName, manager)
	{
		Havtorn::GEngine::GetWindowHandler()->OnDragDropAccepted.AddMember(this, &COutputLogWindow::OnDragDropFiles);
        Havtorn::ULog::AddLogContext(this);

        ClearLog();
        memset(InputBuffer, 0, sizeof(InputBuffer));

        // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
        Commands.push_back("HELP");
        Commands.push_back("HISTORY");
        Commands.push_back("CLEAR");
        Commands.push_back("CLASSIFY");
	}

	COutputLogWindow::~COutputLogWindow()
	{
        ClearLog();
        for (Havtorn::I32 i = 0; i < static_cast<Havtorn::I32>(History.Size); i++)
            ImGui::MemFree(History[i]);
        History.clear();
	}

	void COutputLogWindow::OnEnable()
	{
	}

	void COutputLogWindow::OnInspectorGUI()
	{
		if (!ImGui::Begin(Name(), nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus))
        {
            ImGui::End();
            return;
        }

        // TODO: display items starting from the bottom

        ImGui::SameLine();
        Filter.Draw("Search", 180);
        
        ImGui::SameLine();

        bool shouldCopyToClipboard = ImGui::SmallButton("Copy");

        ImGui::SameLine();

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &ShouldAutoScroll);
            ImGui::EndPopup();
        }

        ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_Tooltip);
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");

        ImGui::Separator();

        // Reserve enough left-over height for 1 separator + 1 input text
        const Havtorn::F32 footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeightToReserve), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::Selectable("Clear")) 
                    ClearLog();

                ImGui::EndPopup();
            }

            // Display every line as a separate entry so we can change their color or add custom widgets.
            // If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
            // NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
            // to only process visible items. The clipper will automatically measure the height of your first item and then
            // "seek" to display only items in the visible area.
            // To use the clipper we can replace your standard loop:
            //      for (int i = 0; i < Items.Size; i++)
            //   With:
            //      ImGuiListClipper clipper;
            //      clipper.Begin(Items.Size);
            //      while (clipper.Step())
            //         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            // - That your items are evenly spaced (same height)
            // - That you have cheap random access to your elements (you can access them given their index,
            //   without processing all the ones before)
            // You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
            // We would need random-access on the post-filtered list.
            // A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
            // or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
            // and appending newly elements as they are inserted. This is left as a task to the user until we can manage
            // to improve this example code!
            // If your items are of variable height:
            // - Split them into same height items would be simpler and facilitate random-seeking into your list.
            // - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
            if (shouldCopyToClipboard)
                ImGui::LogToClipboard();
            
            for (const SLogItem& item : Items)
            //ImGuiListClipper clipper;
            //clipper.Begin(Items.Size);
            //while (clipper.Step())
                //for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                {
                    //const char* item = Items[i].Text;

                    if (!Filter.PassFilter(item.Text))
                        continue;

                    bool hasColor = item.Color != ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
                    if (hasColor)
                        ImGui::PushStyleColor(ImGuiCol_Text, item.Color);

                    ImGui::TextUnformatted(item.Text);

                    if (hasColor)
                        ImGui::PopStyleColor();
                }
            if (shouldCopyToClipboard)
                ImGui::LogFinish();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (ShouldScrollToBottom || (ShouldAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);
            ShouldScrollToBottom = false;

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line
        bool shouldReclaimFocus = false;
        ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
        if (ImGui::InputText("Input", InputBuffer, IM_ARRAYSIZE(InputBuffer), inputTextFlags, [](ImGuiInputTextCallbackData* data)
            {
                COutputLogWindow* window = (COutputLogWindow*)data->UserData;
                return window->TextEditCallback(data);
            }, (void*)this))
        {
            char* s = InputBuffer;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy_s(s, sizeof(char), "");
            shouldReclaimFocus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (shouldReclaimFocus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

        ImGui::End();
	}

	void COutputLogWindow::OnDisable()
	{
	}

    void COutputLogWindow::Log(const Havtorn::ELogCategory category, const std::string& message)
    {
        ImVec4 color = ImVec4();
        switch (category)
        {
        case Havtorn::ELogCategory::Trace:
            color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
            break;
        case Havtorn::ELogCategory::Debug:
            color = ImVec4(0.4f, 1.0f, 0.4f, 1.0f);
            break;
        case Havtorn::ELogCategory::Info:
            color = ImVec4(0.4f, 0.4f, 1.0f, 1.0f);
            break;
        case Havtorn::ELogCategory::Warning:
            color = ImVec4(0.4f, 1.0f, 1.0f, 1.0f);
            break;
        case Havtorn::ELogCategory::Error:
            color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            break;
        case Havtorn::ELogCategory::Fatal:
        default:
            color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            break;
        }

        AddLog(color, message.c_str());
    }

	void COutputLogWindow::OnDragDropFiles(const std::vector<std::string> filePaths)
	{
	}

    void COutputLogWindow::AddLog(ImVec4 color, const char* format, ...)
    {
        // FIXME-OPT
        char buf[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, IM_ARRAYSIZE(buf), format, args);
        buf[IM_ARRAYSIZE(buf) - 1] = 0;
        va_end(args);
        Items.push_back({ Strdup(buf), color });
    }

    void COutputLogWindow::ClearLog()
    {
        for (Havtorn::I32 i = 0; i < static_cast<Havtorn::I32>(Items.Size); i++)
            ImGui::MemFree(Items[i].Text);
        Items.clear();
    }

    void COutputLogWindow::ExecCommand(const char* commandLine)
    {
        HV_LOG_TRACE("# %s\n", commandLine);

        // Insert into history. First find match and delete it so it can be pushed to the back.
        // This isn't trying to be smart or optimal.
        HistoryPos = -1;
        for (Havtorn::I32 i = History.Size - 1; i >= 0; i--)
            if (Stricmp(History[i], commandLine) == 0)
            {
                ImGui::MemFree(History[i]);
                History.erase(History.begin() + i);
                break;
            }
        History.push_back(Strdup(commandLine));

        // Process command
        if (Stricmp(commandLine, "CLEAR") == 0)
        {
            ClearLog();
        }
        else if (Stricmp(commandLine, "HELP") == 0)
        {
            HV_LOG_TRACE("Commands:");
            for (Havtorn::I32 i = 0; i < Commands.Size; i++)
                HV_LOG_TRACE("- %s", Commands[i]);
        }
        else if (Stricmp(commandLine, "HISTORY") == 0)
        {
            Havtorn::I32 first = History.Size - 10;
            for (Havtorn::I32 i = first > 0 ? first : 0; i < History.Size; i++)
                HV_LOG_TRACE("%3d: %s\n", i, History[i]);
        }
        else
        {
            HV_LOG_TRACE("Unknown command: '%s'\n", commandLine);
        }

        // On command input, we scroll to bottom even if AutoScroll==false
        ShouldScrollToBottom = true;
    }

    Havtorn::I32 COutputLogWindow::TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        switch (data->EventFlag)
        {
        case ImGuiInputTextFlags_CallbackCompletion:
        {
            // Example of TEXT COMPLETION

            // Locate beginning of current word
            const char* wordEnd = data->Buf + data->CursorPos;
            const char* wordStart = wordEnd;
            while (wordStart > data->Buf)
            {
                const char c = wordStart[-1];
                if (c == ' ' || c == '\t' || c == ',' || c == ';')
                    break;
                wordStart--;
            }

            // Build a list of candidates
            ImVector<const char*> candidates;
            for (Havtorn::I32 i = 0; i < Commands.Size; i++)
                if (Strnicmp(Commands[i], wordStart, (Havtorn::I32)(wordEnd - wordStart)) == 0)
                    candidates.push_back(Commands[i]);

            if (candidates.Size == 0)
            {
                // No match
                HV_LOG_TRACE("No match for \"%.*s\"!\n", (Havtorn::I32)(wordEnd - wordStart), wordStart);
            }
            else if (candidates.Size == 1)
            {
                // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                data->DeleteChars((Havtorn::I32)(wordStart - data->Buf), (Havtorn::I32)(wordEnd - wordStart));
                data->InsertChars(data->CursorPos, candidates[0]);
                data->InsertChars(data->CursorPos, " ");
            }
            else
            {
                // Multiple matches. Complete as much as we can..
                // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                Havtorn::I32 matchLength = (Havtorn::I32)(wordEnd - wordStart);
                for (;;)
                {
                    Havtorn::I32 c = 0;
                    bool allCandidatesMatch = true;
                    for (Havtorn::I32 i = 0; i < candidates.Size && allCandidatesMatch; i++)
                        if (i == 0)
                            c = toupper(candidates[i][matchLength]);
                        else if (c == 0 || c != toupper(candidates[i][matchLength]))
                            allCandidatesMatch = false;
                    if (!allCandidatesMatch)
                        break;
                    matchLength++;
                }

                if (matchLength > 0)
                {
                    data->DeleteChars((Havtorn::I32)(wordStart - data->Buf), (Havtorn::I32)(wordEnd - wordStart));
                    data->InsertChars(data->CursorPos, candidates[0], candidates[0] + matchLength);
                }

                // List matches
                HV_LOG_TRACE("Possible matches:\n");
                for (Havtorn::I32 i = 0; i < candidates.Size; i++)
                    HV_LOG_TRACE("- %s\n", candidates[i]);
            }

            break;
        }
        case ImGuiInputTextFlags_CallbackHistory:
        {
            // Example of HISTORY
            const Havtorn::I32 previousHistoryPosition = HistoryPos;
            if (data->EventKey == ImGuiKey_UpArrow)
            {
                if (HistoryPos == -1)
                    HistoryPos = History.Size - 1;
                else if (HistoryPos > 0)
                    HistoryPos--;
            }
            else if (data->EventKey == ImGuiKey_DownArrow)
            {
                if (HistoryPos != -1)
                    if (++HistoryPos >= History.Size)
                        HistoryPos = -1;
            }

            // A better implementation would preserve the data on the current input line along with cursor position.
            if (previousHistoryPosition != HistoryPos)
            {
                const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, history_str);
            }
        }
        }
        return 0;
    }
}
