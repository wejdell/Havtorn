// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "SequencerWindow.h"
#include "../ImGuizmo/ImGradient.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"

#include <ECS/Systems/SequencerSystem.h>
#include <ECS/Components/MetaDataComponent.h>
#include <SequencerKeyframes/SequencerTransformKeyframe.h>
#include <SequencerKeyframes/SequencerSpriteKeyframe.h>

#include <set>

namespace ImGui
{
	CSequencerWindow::CSequencerWindow(const char* displayName, Havtorn::CEditorManager* manager, Havtorn::CSequencerSystem* sequencerSystem)
		: CWindow(displayName, manager)
        , SequencerSystem(sequencerSystem)
	{
        Sequencer.FrameMin = 0;
        Sequencer.FrameMax = 100;

        //Sequencer.EntityTracks.push_back(SEditorEntityTrack{ std::string("Camera"), {{Havtorn::EComponentType::TransformComponent}, {Havtorn::EComponentType::SpriteComponent}}, 10, 30, false });
        //Sequencer.EntityTracks.push_back(SEditorEntityTrack{ std::string("Player"), {{Havtorn::EComponentType::TransformComponent}, {Havtorn::EComponentType::SpriteComponent}}, 20, 30, true });

        Sequencers.push_back("Intro");
        Sequencers.push_back("BossFight");
	}

	CSequencerWindow::~CSequencerWindow()
	{
	}

	void CSequencerWindow::OnEnable()
	{
	}

	void CSequencerWindow::OnInspectorGUI()
	{
        if (ImGui::Begin(Name(), nullptr))
        {
            ImGui::SameLine();
            if (ImGui::BeginCombo("Sequencer", Sequencers[CurrentSequencerIndex].c_str()))
            {
                for (Havtorn::U16 index = 0; index < Sequencers.size(); index++)
                {
                    const std::string& sequencerName = Sequencers[index];
                    
                    if (ImGui::Selectable(sequencerName.c_str()))
                    {
                        // Load Sequencer function
                        CurrentSequencerIndex = index;
                        break;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            ImGui::Text("Scene: ");

            Havtorn::SSequencerContextData contextData = SequencerSystem->GetSequencerContextData();

            ImGui::PushItemWidth(140);

            FlowControls(contextData);

            // let's create the sequencer
            static int selectedEntry = -1;
            static int firstFrame = 0;
            static bool expanded = true;
            int imGuiFrame = contextData.CurrentFrame;
            int imGuiMaxFrame = contextData.MaxFrames;
            int playRate = static_cast<int>(contextData.PlayRate);

            ImGui::InputInt("Current Frame ", &imGuiFrame);
            ImGui::SameLine();
            ImGui::InputInt("Frame Max", &imGuiMaxFrame);
            ImGui::SameLine();
            ImGui::InputInt("Play Rate", &playRate);
            ImGui::SameLine();
            ImGui::Checkbox("Loop", &contextData.IsLooping);

            if (ImGui::Button("Add New Transform Keyframe"))
            {
                AddKeyframe<Havtorn::SSequencerTransformKeyframe>(Havtorn::EComponentType::TransformComponent);
            }

            if (ImGui::Button("Add New Sprite Keyframe"))
            {
                AddKeyframe<Havtorn::SSequencerSpriteKeyframe>(Havtorn::EComponentType::SpriteComponent);
            }

            if (ImGui::Button("Add Sprite Track"))
            {
                AddComponentTrack(Havtorn::EComponentType::SpriteComponent);
            }

            ImGui::PopItemWidth();            

            FillSequencer();

            DrawSequencer(&Sequencer, &imGuiFrame, &expanded, &selectedEntry, &firstFrame, SEQUENCER_EDIT_STARTEND | SEQUENCER_ADD | SEQUENCER_DEL | SEQUENCER_COPYPASTE | SEQUENCER_CHANGE_FRAME);

            contextData.CurrentFrame = imGuiFrame;
            contextData.MaxFrames = imGuiMaxFrame;
            contextData.PlayRate = static_cast<Havtorn::U16>(playRate);

            SequencerSystem->SetSequencerContextData(contextData);
        }
        ImGui::End();
	}

    void CSequencerWindow::FlowControls(Havtorn::SSequencerContextData& contextData)
    {
        ImTextureID playButtonID = Manager->GetResourceManager()->GetEditorTexture(Havtorn::EEditorTexture::PlayIcon);
        if (ImGui::ImageButton(playButtonID, { 16.0f, 16.0f }))
        {
            contextData.IsPlayingSequence = true;
        }
        ImGui::SameLine();

        ImTextureID pauseButtonID = Manager->GetResourceManager()->GetEditorTexture(Havtorn::EEditorTexture::PauseIcon);
        if (ImGui::ImageButton(pauseButtonID, { 16.0f, 16.0f }))
        {
            contextData.IsPlayingSequence = false;
        }
        ImGui::SameLine();

        ImTextureID stopButtonID = Manager->GetResourceManager()->GetEditorTexture(Havtorn::EEditorTexture::StopIcon);
        if (ImGui::ImageButton(stopButtonID, { 16.0f, 16.0f }))
        {
            contextData.IsPlayingSequence = false;
            contextData.CurrentFrame = 0;
        }
        ImGui::SameLine();
    }

    void CSequencerWindow::FillSequencer()
    {
        Havtorn::CScene* const scene = Manager->GetCurrentScene();
        if (scene == nullptr)
            return;

        std::vector<Havtorn::SSequencerComponent>& sequencerComponents = scene->GetSequencerComponents();
        std::vector<Havtorn::SMetaDataComponent>& metaDataComponents = scene->GetMetaDataComponents();

        Sequencer.EntityTracks.clear();

        for (U64 index = 0; index < sequencerComponents.size(); index++)
        {
            const Havtorn::SSequencerComponent& component = sequencerComponents[index];
            if (!component.IsInUse)
                continue;

            const Havtorn::SMetaDataComponent& metaData = metaDataComponents[index];

            SEditorEntityTrack entityTrack;
            entityTrack.Name = metaData.Name.AsString();
            
            for (U64 componentTrackIndex = 0; componentTrackIndex < component.ComponentTracks.size(); componentTrackIndex++)
            {
                // TODO.NR: Algo library?

                const Havtorn::SSequencerComponentTrack& componentTrack = component.ComponentTracks[componentTrackIndex];
                std::vector<U32> keyframes;
                for (U64 keyframeIndex = 0; keyframeIndex < componentTrack.Keyframes.size(); keyframeIndex++)
                {
                    keyframes.push_back(componentTrack.Keyframes[keyframeIndex]->FrameNumber);
                }

                entityTrack.ComponentTracks.push_back({ componentTrack.ComponentType, keyframes });
            }

            entityTrack.FrameStart = 10;
            entityTrack.FrameEnd = 20;
            entityTrack.IsExpanded = true;

            Sequencer.EntityTracks.push_back(entityTrack);
        }
    }

	void CSequencerWindow::OnDisable()
	{
	}

    void CSequencerWindow::AddComponentTrack(Havtorn::EComponentType componentType)
    {
        Havtorn::SEntity* const entity = Manager->GetSelectedEntity();
        Havtorn::CScene* const scene = Manager->GetCurrentScene();

        if (entity == nullptr || scene == nullptr)
            return;

        U64 sceneIndex = scene->GetSceneIndex(entity->GUID);
        std::vector<Havtorn::SSequencerComponent>& sequencerComponents = scene->GetSequencerComponents();

        if (sceneIndex >= sequencerComponents.size())
            return;

        SequencerSystem->AddComponentTrackToComponent(sequencerComponents[sceneIndex], componentType);

        //AddSequencerItem(SEditorEntityTrack{ std::string("Sprite"), { {componentType} }, 10, 30, false });
    }

    void CSequencerWindow::AddSequencerItem(SEditorEntityTrack item)
    {
        Sequencer.EntityTracks.push_back(item);
    }

    // Draw Functions

#ifndef IMGUI_DEFINE_MATH_OPERATORS
    static ImVec2 operator+(const ImVec2& a, const ImVec2& b)
    {
        return ImVec2(a.x + b.x, a.y + b.y);
    }
#endif

    static bool SequencerAddDelButton(ImDrawList* draw_list, ImVec2 pos, bool add = true)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImRect delRect(pos, ImVec2(pos.x + 16, pos.y + 16));
        bool overDel = delRect.Contains(io.MousePos);
        int delColor = overDel ? 0xFFAAAAAA : 0x77A3B2AA;
        float midy = pos.y + 16 / 2 - 0.5f;
        float midx = pos.x + 16 / 2 - 0.5f;
        draw_list->AddRect(delRect.Min, delRect.Max, delColor, 4);
        draw_list->AddLine(ImVec2(delRect.Min.x + 3, midy), ImVec2(delRect.Max.x - 3, midy), delColor, 2);
        if (add)
            draw_list->AddLine(ImVec2(midx, delRect.Min.y + 3), ImVec2(midx, delRect.Max.y - 3), delColor, 2);
        return overDel;
    }

    bool CSequencerWindow::DrawSequencer(SSequencer* sequence, int* currentFrame, bool* expanded, int* selectedEntry, int* firstFrame, int sequenceOptions)
    {
#pragma region Setup
        bool ret = false;
        ImGuiIO& io = ImGui::GetIO();
        int cx = (int)(io.MousePos.x);
        int cy = (int)(io.MousePos.y);
        static float framePixelWidth = 10.f;
        static float framePixelWidthTarget = 10.f;
        int legendWidth = 200;

        static int movingEntry = -1;
        static int movingPos = -1;
        static int movingPart = -1;
        int delEntry = -1;
        int dupEntry = -1;
        int ItemHeight = 20;

        bool popupOpened = false;
        int sequenceCount = sequence->GetEntityTrackCount();
        if (!sequenceCount)
            return false;
        ImGui::BeginGroup();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
        int firstFrameUsed = firstFrame ? *firstFrame : 0;

        int controlHeight = sequenceCount * ItemHeight;
        for (int i = 0; i < sequenceCount; i++)
            controlHeight += int(sequence->GetCustomHeight(i));
        int frameCount = ImMax(sequence->GetFrameMax() - sequence->GetFrameMin(), 1);

        static bool MovingScrollBar = false;
        static bool MovingCurrentFrame = false;
        struct CustomDraw
        {
            int index;
            ImRect customRect;
            ImRect legendRect;
            ImRect clippingRect;
            ImRect legendClippingRect;
            std::vector<SEditorComponentTrack> componentTracks;
        };
        ImVector<CustomDraw> customDraws;
        ImVector<CustomDraw> compactCustomDraws;
        // zoom in/out
        const int visibleFrameCount = (int)floorf((canvas_size.x - legendWidth) / framePixelWidth);
        const float barWidthRatio = ImMin(visibleFrameCount / (float)frameCount, 1.f);
        const float barWidthInPixels = barWidthRatio * (canvas_size.x - legendWidth);

        ImRect regionRect(canvas_pos, canvas_pos + canvas_size);
#pragma endregion

        static bool panningView = false;
        static ImVec2 panningViewSource;
        static int panningViewFrame;
        Panning(io, panningView, panningViewSource, panningViewFrame, firstFrame, framePixelWidth, sequence, visibleFrameCount, framePixelWidthTarget, frameCount);

        // --
        if (expanded && !*expanded)
        {
            NotExpanded(canvas_size, canvas_pos, ItemHeight, draw_list, sequence, frameCount, sequenceCount);
        }
        else
        {
            bool hasScrollBar(true);

            // test scroll area
            ImVec2 headerSize(canvas_size.x, (float)ItemHeight);
            ImVec2 scrollBarSize(canvas_size.x, 14.f);
            ImGui::InvisibleButton("topBar", headerSize);
            draw_list->AddRectFilled(canvas_pos, canvas_pos + headerSize, 0xFFFF0000, 0);
            ImVec2 childFramePos = ImGui::GetCursorScreenPos();
            ImVec2 childFrameSize(canvas_size.x, canvas_size.y - 8.f - headerSize.y - (hasScrollBar ? scrollBarSize.y : 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
            ImGui::BeginChildFrame(889, childFrameSize);
            sequence->IsFocused = ImGui::IsWindowFocused();
            ImGui::InvisibleButton("contentBar", ImVec2(canvas_size.x, float(controlHeight)));
            const ImVec2 contentMin = ImGui::GetItemRectMin();
            const ImVec2 contentMax = ImGui::GetItemRectMax();
            //const ImRect contentRect(contentMin, contentMax);
            const float contentHeight = contentMax.y - contentMin.y;

            // full background
            draw_list->AddRectFilled(canvas_pos, canvas_pos + canvas_size, 0xFF242424, 0);

            // current frame top
            ImRect topRect(ImVec2(canvas_pos.x + legendWidth, canvas_pos.y), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + ItemHeight));

            ChangeCurrentFrame(MovingCurrentFrame, MovingScrollBar, movingEntry, sequenceOptions, currentFrame, topRect, io, frameCount, framePixelWidth, firstFrameUsed, sequence);

            //header
            draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), 0xFF3D3837, 0);

            AddTrackButton(sequenceOptions, draw_list, canvas_pos, legendWidth, ItemHeight, io, sequence, selectedEntry, popupOpened);
            
            //header frame number and lines
            int modFrameCount = 10;
            int frameStep = 1;
            while ((modFrameCount * framePixelWidth) < 150)
            {
                modFrameCount *= 2;
                frameStep *= 2;
            };
            int halfModFrameCount = modFrameCount / 2;

            auto drawLine = [&](int i, int regionHeight) {
                bool baseIndex = ((i % modFrameCount) == 0) || (i == sequence->GetFrameMax() || i == sequence->GetFrameMin());
                bool halfIndex = (i % halfModFrameCount) == 0;
                int px = (int)canvas_pos.x + int(i * framePixelWidth) + legendWidth - int(firstFrameUsed * framePixelWidth);
                int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
                int tiretEnd = baseIndex ? regionHeight : ItemHeight;

                if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
                {
                    draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

                    draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)ItemHeight), ImVec2((float)px, canvas_pos.y + (float)regionHeight - 1), 0x30606060, 1);
                }

                if (baseIndex && px > (canvas_pos.x + legendWidth))
                {
                    char tmps[512];
                    ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", i);
                    draw_list->AddText(ImVec2((float)px + 3.f, canvas_pos.y), 0xFFBBBBBB, tmps);
                }

            };

            auto drawLineContent = [&](int i, int /*regionHeight*/) {
                int px = (int)canvas_pos.x + int(i * framePixelWidth) + legendWidth - int(firstFrameUsed * framePixelWidth);
                int tiretStart = int(contentMin.y);
                int tiretEnd = int(contentMax.y);

                if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
                {
                    //draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

                    draw_list->AddLine(ImVec2(float(px), float(tiretStart)), ImVec2(float(px), float(tiretEnd)), 0x30606060, 1);
                }
            };

            for (int i = sequence->GetFrameMin(); i <= sequence->GetFrameMax(); i += frameStep)
            {
                drawLine(i, ItemHeight);
            }
            drawLine(sequence->GetFrameMin(), ItemHeight);
            drawLine(sequence->GetFrameMax(), ItemHeight);

            // clip content
            draw_list->PushClipRect(childFramePos, childFramePos + childFrameSize);

            size_t customHeight = 0;
            // draw item names in the legend rect on the left
            TrackHeader(sequenceCount, sequence, contentMin, ItemHeight, customHeight, draw_list, sequenceOptions, legendWidth, io, delEntry, dupEntry);

            // clipping rect so items bars are not visible in the legend on the left when scrolled
            //

            // slots background
            customHeight = 0;
            TrackSlotsBackground(sequenceCount, sequence, contentMin, legendWidth, ItemHeight, customHeight, canvas_size, canvas_pos, popupOpened, cy, movingEntry, cx, draw_list);

            draw_list->PushClipRect(childFramePos + ImVec2(float(legendWidth), 0.f), childFramePos + childFrameSize);

            // vertical frame lines in content area
            for (int i = sequence->GetFrameMin(); i <= sequence->GetFrameMax(); i += frameStep)
            {
                drawLineContent(i, int(contentHeight));
            }
            drawLineContent(sequence->GetFrameMin(), int(contentHeight));
            drawLineContent(sequence->GetFrameMax(), int(contentHeight));

            // selection
            bool selected = selectedEntry && (*selectedEntry >= 0);
            Selection(selected, customHeight, selectedEntry, sequence, draw_list, contentMin, ItemHeight, canvas_size);

            // slots
#pragma region Slots
            customHeight = 0;
            for (int i = 0; i < sequenceCount; i++)
            {
                int* start, * end;
                unsigned int color;
                sequence->Get(i, &start, &end, NULL, &color);
                size_t localCustomHeight = sequence->GetCustomHeight(i);

                ImVec2 pos = ImVec2(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth, contentMin.y + ItemHeight * i + 1 + customHeight);
                ImVec2 slotP1(pos.x + *start * framePixelWidth, pos.y + 2);
                ImVec2 slotP2(pos.x + *end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);
                ImVec2 slotP3(pos.x + *end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2 + localCustomHeight);
                unsigned int slotColor = color | 0xFF000000;
                unsigned int slotColorHalf = (color & 0xFFFFFF) | 0x40000000;

                if (slotP1.x <= (canvas_size.x + contentMin.x) && slotP2.x >= (contentMin.x + legendWidth))
                {
                    draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
                    draw_list->AddRectFilled(slotP1, slotP2, slotColor, 2);
                }
                if (ImRect(slotP1, slotP2).Contains(io.MousePos) && io.MouseDoubleClicked[0])
                {
                    sequence->DoubleClick(i);
                }
                // Ensure grabbable handles
                const float max_handle_width = slotP2.x - slotP1.x / 3.0f;
                const float min_handle_width = ImMin(10.0f, max_handle_width);
                const float handle_width = ImClamp(framePixelWidth / 2.0f, min_handle_width, max_handle_width);
                ImRect rects[3] = { ImRect(slotP1, ImVec2(slotP1.x + handle_width, slotP2.y))
                    , ImRect(ImVec2(slotP2.x - handle_width, slotP1.y), slotP2)
                    , ImRect(slotP1, slotP2) };

                const unsigned int quadColor[] = { 0xFFFFFFFF, 0xFFFFFFFF, slotColor + (selected ? 0 : 0x202020) };
                if (movingEntry == -1 && (sequenceOptions & SEQUENCER_EDIT_STARTEND))// TODOFOCUS && backgroundRect.Contains(io.MousePos))
                {
                    for (int j = 2; j >= 0; j--)
                    {
                        ImRect& rc = rects[j];
                        if (!rc.Contains(io.MousePos))
                            continue;
                        draw_list->AddRectFilled(rc.Min, rc.Max, quadColor[j], 2);
                    }

                    for (int j = 0; j < 3; j++)
                    {
                        ImRect& rc = rects[j];
                        if (!rc.Contains(io.MousePos))
                            continue;
                        if (!ImRect(childFramePos, childFramePos + childFrameSize).Contains(io.MousePos))
                            continue;
                        if (ImGui::IsMouseClicked(0) && !MovingScrollBar && !MovingCurrentFrame)
                        {
                            movingEntry = i;
                            movingPos = cx;
                            movingPart = j + 1;
                            sequence->BeginEdit(movingEntry);
                            break;
                        }
                    }
                }

                // custom draw
                if (localCustomHeight > 0)
                {
                    ImVec2 rp(canvas_pos.x, contentMin.y + ItemHeight * i + 1 + customHeight);
                    ImRect customRect(rp + ImVec2(legendWidth - (firstFrameUsed - sequence->GetFrameMin() - 0.5f) * framePixelWidth, float(ItemHeight)),
                        rp + ImVec2(legendWidth + (sequence->GetFrameMax() - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(localCustomHeight + ItemHeight)));
                    ImRect clippingRect(rp + ImVec2(float(legendWidth), float(ItemHeight)), rp + ImVec2(canvas_size.x, float(localCustomHeight + ItemHeight)));

                    ImRect legendRect(rp + ImVec2(0.f, float(ItemHeight)), rp + ImVec2(float(legendWidth), float(localCustomHeight)));
                    ImRect legendClippingRect(canvas_pos + ImVec2(0.f, float(ItemHeight)), canvas_pos + ImVec2(float(legendWidth), float(localCustomHeight + ItemHeight)));
                    std::vector<SEditorComponentTrack> componentTracks = sequence->GetComponentTracks(i);
                    customDraws.push_back({ i, customRect, legendRect, clippingRect, legendClippingRect, componentTracks });
                }
                else
                {
                    ImVec2 rp(canvas_pos.x, contentMin.y + ItemHeight * i + customHeight);
                    ImRect customRect(rp + ImVec2(legendWidth - (firstFrameUsed - sequence->GetFrameMin() - 0.5f) * framePixelWidth, float(0.f)),
                        rp + ImVec2(legendWidth + (sequence->GetFrameMax() - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(ItemHeight)));
                    ImRect clippingRect(rp + ImVec2(float(legendWidth), float(0.f)), rp + ImVec2(canvas_size.x, float(ItemHeight)));

                    compactCustomDraws.push_back({ i, customRect, ImRect(), clippingRect, ImRect() });
                }
                customHeight += localCustomHeight;
            }
#pragma endregion

            // moving
            Moving(movingEntry, cx, movingPos, framePixelWidth, sequence, selectedEntry, movingPart, io, ret);

            // cursor
            if (currentFrame && firstFrame && *currentFrame >= *firstFrame && *currentFrame <= sequence->GetFrameMax())
            {
                static const float cursorWidth = 8.f;
                float cursorOffset = contentMin.x + legendWidth + (*currentFrame - firstFrameUsed) * framePixelWidth + framePixelWidth / 2 - cursorWidth * 0.5f;
                draw_list->AddLine(ImVec2(cursorOffset, canvas_pos.y), ImVec2(cursorOffset, contentMax.y), 0xA02A2AFF, cursorWidth);
                char tmps[512];
                ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", *currentFrame);
                draw_list->AddText(ImVec2(cursorOffset + 10, canvas_pos.y + 2), 0xFF2A2AFF, tmps);
            }

            draw_list->PopClipRect();
            draw_list->PopClipRect();

            for (auto& customDraw : customDraws)
                sequence->CustomDraw(customDraw.index, draw_list, customDraw.customRect, customDraw.legendRect, customDraw.clippingRect, customDraw.legendClippingRect, /*customDraw.labels*/sequence->EntityTracks[0].ComponentTracks);
            for (auto& customDraw : compactCustomDraws)
                sequence->CustomDrawCompact(customDraw.index, draw_list, customDraw.customRect, customDraw.clippingRect);

            // copy paste
            CopyPaste(sequenceOptions, contentMin, canvas_pos, ItemHeight, io, draw_list, sequence);
            //

            ImGui::EndChildFrame();
            ImGui::PopStyleColor();
            Scrollbar(hasScrollBar, scrollBarSize, firstFrameUsed, sequence, frameCount, canvas_size, legendWidth, draw_list, io, barWidthInPixels, MovingScrollBar, framePixelWidthTarget, framePixelWidth, firstFrame, visibleFrameCount, panningViewSource, panningViewFrame, MovingCurrentFrame, movingEntry);
        }

        ImGui::EndGroup();

        if (regionRect.Contains(io.MousePos))
        {
            bool overCustomDraw = false;
            for (auto& custom : customDraws)
            {
                if (custom.customRect.Contains(io.MousePos))
                {
                    overCustomDraw = true;
                }
            }
            if (overCustomDraw)
            {
            }
            else
            {
#if 0
                frameOverCursor = *firstFrame + (int)(visibleFrameCount * ((io.MousePos.x - (float)legendWidth - canvas_pos.x) / (canvas_size.x - legendWidth)));
                //frameOverCursor = max(min(*firstFrame - visibleFrameCount / 2, frameCount - visibleFrameCount), 0);

                /**firstFrame -= frameOverCursor;
                *firstFrame *= framePixelWidthTarget / framePixelWidth;
                *firstFrame += frameOverCursor;*/
                if (io.MouseWheel < -FLT_EPSILON)
                {
                    *firstFrame -= frameOverCursor;
                    *firstFrame = int(*firstFrame * 1.1f);
                    framePixelWidthTarget *= 0.9f;
                    *firstFrame += frameOverCursor;
                }

                if (io.MouseWheel > FLT_EPSILON)
                {
                    *firstFrame -= frameOverCursor;
                    *firstFrame = int(*firstFrame * 0.9f);
                    framePixelWidthTarget *= 1.1f;
                    *firstFrame += frameOverCursor;
                }
#endif
            }
        }

        if (expanded)
        {
            bool overExpanded = SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + 2, canvas_pos.y + 2), !*expanded);
            if (overExpanded && io.MouseReleased[0])
                *expanded = !*expanded;
        }

        if (delEntry != -1)
        {
            sequence->Del(delEntry);
            if (selectedEntry && (*selectedEntry == delEntry || *selectedEntry >= sequence->GetEntityTrackCount()))
                *selectedEntry = -1;
        }

        if (dupEntry != -1)
        {
            sequence->Duplicate(dupEntry);
        }
        return ret;
    }

    void CSequencerWindow::Scrollbar(bool hasScrollBar, ImVec2& scrollBarSize, int firstFrameUsed, SSequencer* sequence, int frameCount, ImVec2& canvas_size, int legendWidth, ImDrawList* draw_list, ImGuiIO& io, const float& barWidthInPixels, bool& MovingScrollBar, float& framePixelWidthTarget, float& framePixelWidth, int* firstFrame, const int& visibleFrameCount, ImVec2& panningViewSource, int& panningViewFrame, bool MovingCurrentFrame, int movingEntry)
    {
        if (hasScrollBar)
        {
            ImGui::InvisibleButton("scrollBar", scrollBarSize);
            ImVec2 scrollBarMin = ImGui::GetItemRectMin();
            ImVec2 scrollBarMax = ImGui::GetItemRectMax();

            // ratio = number of frames visible in control / number to total frames

            float startFrameOffset = ((float)(firstFrameUsed - sequence->GetFrameMin()) / (float)frameCount) * (canvas_size.x - legendWidth);
            ImVec2 scrollBarA(scrollBarMin.x + legendWidth, scrollBarMin.y - 2);
            ImVec2 scrollBarB(scrollBarMin.x + canvas_size.x, scrollBarMax.y - 1);
            draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);

            ImRect scrollBarRect(scrollBarA, scrollBarB);
            bool inScrollBar = scrollBarRect.Contains(io.MousePos);

            draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF101010, 8);


            ImVec2 scrollBarC(scrollBarMin.x + legendWidth + startFrameOffset, scrollBarMin.y);
            ImVec2 scrollBarD(scrollBarMin.x + legendWidth + barWidthInPixels + startFrameOffset, scrollBarMax.y - 2);
            draw_list->AddRectFilled(scrollBarC, scrollBarD, (inScrollBar || MovingScrollBar) ? 0xFF606060 : 0xFF505050, 6);

            ImRect barHandleLeft(scrollBarC, ImVec2(scrollBarC.x + 14, scrollBarD.y));
            ImRect barHandleRight(ImVec2(scrollBarD.x - 14, scrollBarC.y), scrollBarD);

            bool onLeft = barHandleLeft.Contains(io.MousePos);
            bool onRight = barHandleRight.Contains(io.MousePos);

            static bool sizingRBar = false;
            static bool sizingLBar = false;

            draw_list->AddRectFilled(barHandleLeft.Min, barHandleLeft.Max, (onLeft || sizingLBar) ? 0xFFAAAAAA : 0xFF666666, 6);
            draw_list->AddRectFilled(barHandleRight.Min, barHandleRight.Max, (onRight || sizingRBar) ? 0xFFAAAAAA : 0xFF666666, 6);

            ImRect scrollBarThumb(scrollBarC, scrollBarD);
            static const float MinBarWidth = 44.f;
            if (sizingRBar)
            {
                if (!io.MouseDown[0])
                {
                    sizingRBar = false;
                }
                else
                {
                    float barNewWidth = ImMax(barWidthInPixels + io.MouseDelta.x, MinBarWidth);
                    float barRatio = barNewWidth / barWidthInPixels;
                    framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
                    int newVisibleFrameCount = int((canvas_size.x - legendWidth) / framePixelWidthTarget);
                    int lastFrame = *firstFrame + newVisibleFrameCount;
                    if (lastFrame > sequence->GetFrameMax())
                    {
                        framePixelWidthTarget = framePixelWidth = (canvas_size.x - legendWidth) / float(sequence->GetFrameMax() - *firstFrame);
                    }
                }
            }
            else if (sizingLBar)
            {
                if (!io.MouseDown[0])
                {
                    sizingLBar = false;
                }
                else
                {
                    if (fabsf(io.MouseDelta.x) > FLT_EPSILON)
                    {
                        float barNewWidth = ImMax(barWidthInPixels - io.MouseDelta.x, MinBarWidth);
                        float barRatio = barNewWidth / barWidthInPixels;
                        float previousFramePixelWidthTarget = framePixelWidthTarget;
                        framePixelWidthTarget = framePixelWidth = framePixelWidth / barRatio;
                        int newVisibleFrameCount = int(visibleFrameCount / barRatio);
                        int newFirstFrame = *firstFrame + newVisibleFrameCount - visibleFrameCount;
                        newFirstFrame = ImClamp(newFirstFrame, sequence->GetFrameMin(), ImMax(sequence->GetFrameMax() - visibleFrameCount, sequence->GetFrameMin()));
                        if (newFirstFrame == *firstFrame)
                        {
                            framePixelWidth = framePixelWidthTarget = previousFramePixelWidthTarget;
                        }
                        else
                        {
                            *firstFrame = newFirstFrame;
                        }
                    }
                }
            }
            else
            {
                if (MovingScrollBar)
                {
                    if (!io.MouseDown[0])
                    {
                        MovingScrollBar = false;
                    }
                    else
                    {
                        float framesPerPixelInBar = barWidthInPixels / (float)visibleFrameCount;
                        *firstFrame = int((io.MousePos.x - panningViewSource.x) / framesPerPixelInBar) - panningViewFrame;
                        *firstFrame = ImClamp(*firstFrame, sequence->GetFrameMin(), ImMax(sequence->GetFrameMax() - visibleFrameCount, sequence->GetFrameMin()));
                    }
                }
                else
                {
                    if (scrollBarThumb.Contains(io.MousePos) && ImGui::IsMouseClicked(0) && firstFrame && !MovingCurrentFrame && movingEntry == -1)
                    {
                        MovingScrollBar = true;
                        panningViewSource = io.MousePos;
                        panningViewFrame = -*firstFrame;
                    }
                    if (!sizingRBar && onRight && ImGui::IsMouseClicked(0))
                        sizingRBar = true;
                    if (!sizingLBar && onLeft && ImGui::IsMouseClicked(0))
                        sizingLBar = true;

                }
            }
        }
    }

    void CSequencerWindow::CopyPaste(int sequenceOptions, const ImVec2& contentMin, ImVec2& canvas_pos, int ItemHeight, ImGuiIO& io, ImDrawList* draw_list, SSequencer* sequence)
    {
        if (sequenceOptions & SEQUENCER_COPYPASTE)
        {
            ImRect rectCopy(ImVec2(contentMin.x + 100, canvas_pos.y + 2)
                , ImVec2(contentMin.x + 100 + 30, canvas_pos.y + ItemHeight - 2));
            bool inRectCopy = rectCopy.Contains(io.MousePos);
            unsigned int copyColor = inRectCopy ? 0xFF1080FF : 0xFF000000;
            draw_list->AddText(rectCopy.Min, copyColor, "Copy");

            ImRect rectPaste(ImVec2(contentMin.x + 140, canvas_pos.y + 2)
                , ImVec2(contentMin.x + 140 + 30, canvas_pos.y + ItemHeight - 2));
            bool inRectPaste = rectPaste.Contains(io.MousePos);
            unsigned int pasteColor = inRectPaste ? 0xFF1080FF : 0xFF000000;
            draw_list->AddText(rectPaste.Min, pasteColor, "Paste");

            if (inRectCopy && io.MouseReleased[0])
            {
                sequence->Copy();
            }
            if (inRectPaste && io.MouseReleased[0])
            {
                sequence->Paste();
            }
        }
    }

    void CSequencerWindow::Moving(int& movingEntry, int cx, int& movingPos, float framePixelWidth, SSequencer* sequence, int* selectedEntry, int movingPart, ImGuiIO& io, bool& ret)
    {
        if (movingEntry >= 0)
        {
            ImGui::CaptureMouseFromApp();
            int diffFrame = int((cx - movingPos) / framePixelWidth);
            if (std::abs(diffFrame) > 0)
            {
                int* start, * end;
                sequence->Get(movingEntry, &start, &end, NULL, NULL);
                if (selectedEntry)
                    *selectedEntry = movingEntry;
                int& l = *start;
                int& r = *end;
                if (movingPart & 1)
                    l += diffFrame;
                if (movingPart & 2)
                    r += diffFrame;
                if (l < 0)
                {
                    if (movingPart & 2)
                        r -= l;
                    l = 0;
                }
                if (movingPart & 1 && l > r)
                    l = r;
                if (movingPart & 2 && r < l)
                    r = l;
                movingPos += int(diffFrame * framePixelWidth);
            }
            if (!io.MouseDown[0])
            {
                // single select
                if (!diffFrame && movingPart && selectedEntry)
                {
                    *selectedEntry = movingEntry;
                    ret = true;
                }

                movingEntry = -1;
                sequence->EndEdit();
            }
        }
    }

    void CSequencerWindow::Selection(bool selected, size_t& customHeight, int* selectedEntry, SSequencer* sequence, ImDrawList* draw_list, const ImVec2& contentMin, int ItemHeight, ImVec2& canvas_size)
    {
        if (selected)
        {
            customHeight = 0;
            for (int i = 0; i < *selectedEntry; i++)
                customHeight += sequence->GetCustomHeight(i);;
            draw_list->AddRectFilled(ImVec2(contentMin.x, contentMin.y + ItemHeight * *selectedEntry + customHeight), ImVec2(contentMin.x + canvas_size.x, contentMin.y + ItemHeight * (*selectedEntry + 1) + customHeight), 0x801080FF, 1.f);
        }
    }

    void CSequencerWindow::TrackSlotsBackground(int sequenceCount, SSequencer* sequence, const ImVec2& contentMin, int legendWidth, int ItemHeight, size_t& customHeight, ImVec2& canvas_size, ImVec2& canvas_pos, bool popupOpened, int cy, int movingEntry, int cx, ImDrawList* draw_list)
    {
        for (int i = 0; i < sequenceCount; i++)
        {
            unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;

            size_t localCustomHeight = sequence->GetCustomHeight(i);
            ImVec2 pos = ImVec2(contentMin.x + legendWidth, contentMin.y + ItemHeight * i + 1 + customHeight);
            ImVec2 sz = ImVec2(canvas_size.x + canvas_pos.x, pos.y + ItemHeight - 1 + localCustomHeight);
            if (!popupOpened && cy >= pos.y && cy < pos.y + (ItemHeight + localCustomHeight) && movingEntry == -1 && cx>contentMin.x && cx < contentMin.x + canvas_size.x)
            {
                col += 0x80201008;
                pos.x -= legendWidth;
            }
            draw_list->AddRectFilled(pos, sz, col, 0);
            customHeight += localCustomHeight;
        }
    }

    void CSequencerWindow::TrackHeader(int sequenceCount, SSequencer* sequence, const ImVec2& contentMin, int ItemHeight, size_t& customHeight, ImDrawList* draw_list, int sequenceOptions, int legendWidth, ImGuiIO& io, int& delEntry, int& dupEntry)
    {
        for (int i = 0; i < sequenceCount; i++)
        {
            int type;
            sequence->Get(i, NULL, NULL, &type, NULL);
            ImVec2 tpos(contentMin.x + 3, contentMin.y + i * ItemHeight + 2 + customHeight);
            draw_list->AddText(tpos, 0xFFFFFFFF, sequence->GetEntityTrackLabel(i));

            if (sequenceOptions & SEQUENCER_DEL)
            {
                bool overDel = SequencerAddDelButton(draw_list, ImVec2(contentMin.x + legendWidth - ItemHeight + 2 - 10, tpos.y + 2), false);
                if (overDel && io.MouseReleased[0])
                    delEntry = i;

                bool overDup = SequencerAddDelButton(draw_list, ImVec2(contentMin.x + legendWidth - ItemHeight - ItemHeight + 2 - 10, tpos.y + 2), true);
                if (overDup && io.MouseReleased[0])
                    dupEntry = i;
            }
            customHeight += sequence->GetCustomHeight(i);
        }
    }

    void CSequencerWindow::ChangeCurrentFrame(bool& MovingCurrentFrame, bool MovingScrollBar, int movingEntry, int sequenceOptions, int* currentFrame, ImRect& topRect, ImGuiIO& io, int frameCount, float framePixelWidth, int firstFrameUsed, SSequencer* sequence)
    {
        if (!MovingCurrentFrame && !MovingScrollBar && movingEntry == -1 && sequenceOptions & SEQUENCER_CHANGE_FRAME && currentFrame && *currentFrame >= 0 && topRect.Contains(io.MousePos) && io.MouseDown[0])
        {
            MovingCurrentFrame = true;
        }
        if (MovingCurrentFrame)
        {
            if (frameCount)
            {
                *currentFrame = (int)((io.MousePos.x - topRect.Min.x) / framePixelWidth) + firstFrameUsed;
                if (*currentFrame < sequence->GetFrameMin())
                    *currentFrame = sequence->GetFrameMin();
                if (*currentFrame >= sequence->GetFrameMax())
                    *currentFrame = sequence->GetFrameMax();
            }
            if (!io.MouseDown[0])
                MovingCurrentFrame = false;
        }
    }

    void CSequencerWindow::Panning(ImGuiIO& io, bool& panningView, ImVec2& panningViewSource, int& panningViewFrame, int* firstFrame, float& framePixelWidth, SSequencer* sequence, const int& visibleFrameCount, float& framePixelWidthTarget, int& frameCount)
    {
        if (ImGui::IsWindowFocused() && io.KeyAlt && io.MouseDown[2])
        {
            if (!panningView)
            {
                panningViewSource = io.MousePos;
                panningView = true;
                panningViewFrame = *firstFrame;
            }
            *firstFrame = panningViewFrame - int((io.MousePos.x - panningViewSource.x) / framePixelWidth);
            *firstFrame = ImClamp(*firstFrame, sequence->GetFrameMin(), sequence->GetFrameMax() - visibleFrameCount);
        }
        if (panningView && !io.MouseDown[2])
        {
            panningView = false;
        }
        framePixelWidthTarget = ImClamp(framePixelWidthTarget, 0.1f, 50.f);

        framePixelWidth = ImLerp(framePixelWidth, framePixelWidthTarget, 0.33f);

        frameCount = sequence->GetFrameMax() - sequence->GetFrameMin();
        if (visibleFrameCount >= frameCount && firstFrame)
            *firstFrame = sequence->GetFrameMin();
    }

    void CSequencerWindow::NotExpanded(ImVec2& canvas_size, ImVec2& canvas_pos, int ItemHeight, ImDrawList* draw_list, SSequencer* sequence, int frameCount, int sequenceCount)
    {
        ImGui::InvisibleButton("canvas", ImVec2(canvas_size.x - canvas_pos.x, (float)ItemHeight));
        draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), 0xFF3D3837, 0);
        char tmps[512];
        ImFormatString(tmps, IM_ARRAYSIZE(tmps), sequence->GetCollapseFmt(), frameCount, sequenceCount);
        draw_list->AddText(ImVec2(canvas_pos.x + 26, canvas_pos.y + 2), 0xFFFFFFFF, tmps);
    }

    void CSequencerWindow::AddTrackButton(int sequenceOptions, ImDrawList* draw_list, ImVec2& canvas_pos, int legendWidth, int ItemHeight, ImGuiIO& io, SSequencer* sequence, int* selectedEntry, bool& popupOpened)
    {
        if (sequenceOptions & SEQUENCER_ADD)
        {
            // Square with one (minus) or two (plus) lines in it
            if (SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + legendWidth - ItemHeight, canvas_pos.y + 2), true) && io.MouseReleased[0])
                ImGui::OpenPopup("addEntry");

            if (ImGui::BeginPopup("addEntry"))
            {
                if (*selectedEntry > -1 && *selectedEntry < sequence->GetEntityTrackCount())
                {
                    for (int i = 0; i < sequence->GetComponentTrackCount(*selectedEntry); i++)
                    {
                        if (ImGui::Selectable(sequence->GetComponentTrackLabel(*selectedEntry, i).c_str()))
                        {
                            sequence->Add(i);
                            //*selectedEntry = sequence->GetEntityTrackCount() - 1;
                        }
                    }
                }

                ImGui::EndPopup();
                popupOpened = true;
            }
        }
    }
}

namespace ImCurveEdit
{
#ifndef IMGUI_DEFINE_MATH_OPERATORS
    static ImVec2 operator+(const ImVec2& a, const ImVec2& b) {
        return ImVec2(a.x + b.x, a.y + b.y);
    }

    static ImVec2 operator-(const ImVec2& a, const ImVec2& b) {
        return ImVec2(a.x - b.x, a.y - b.y);
    }

    static ImVec2 operator*(const ImVec2& a, const ImVec2& b) {
        return ImVec2(a.x * b.x, a.y * b.y);
    }

    static ImVec2 operator/(const ImVec2& a, const ImVec2& b) {
        return ImVec2(a.x / b.x, a.y / b.y);
    }

    static ImVec2 operator*(const ImVec2& a, const float b) {
        return ImVec2(a.x * b, a.y * b);
    }
#endif

    static float smoothstep(float edge0, float edge1, float x)
    {
        x = ImClamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return x * x * (3 - 2 * x);
    }

    static float distance(float x, float y, float x1, float y1, float x2, float y2)
    {
        float A = x - x1;
        float B = y - y1;
        float C = x2 - x1;
        float D = y2 - y1;

        float dot = A * C + B * D;
        float len_sq = C * C + D * D;
        float param = -1.f;
        if (len_sq > FLT_EPSILON)
            param = dot / len_sq;

        float xx, yy;

        if (param < 0.f) {
            xx = x1;
            yy = y1;
        }
        else if (param > 1.f) {
            xx = x2;
            yy = y2;
        }
        else {
            xx = x1 + param * C;
            yy = y1 + param * D;
        }

        float dx = x - xx;
        float dy = y - yy;
        return sqrtf(dx * dx + dy * dy);
    }

    static int DrawPoint(ImDrawList* draw_list, ImVec2 pos, const ImVec2 size, const ImVec2 offset, bool edited)
    {
        int ret = 0;
        ImGuiIO& io = ImGui::GetIO();

        static const ImVec2 localOffsets[4] = { ImVec2(1,0), ImVec2(0,1), ImVec2(-1,0), ImVec2(0,-1) };
        ImVec2 offsets[4];
        for (int i = 0; i < 4; i++)
        {
            offsets[i] = pos * size + localOffsets[i] * 4.5f + offset;
        }

        const ImVec2 center = pos * size + offset;
        const ImRect anchor(center - ImVec2(5, 5), center + ImVec2(5, 5));
        draw_list->AddConvexPolyFilled(offsets, 4, 0xFF000000);
        if (anchor.Contains(io.MousePos))
        {
            ret = 1;
            if (io.MouseDown[0])
                ret = 2;
        }
        if (edited)
            draw_list->AddPolyline(offsets, 4, 0xFFFFFFFF, true, 3.0f);
        else if (ret)
            draw_list->AddPolyline(offsets, 4, 0xFF80B0FF, true, 2.0f);
        else
            draw_list->AddPolyline(offsets, 4, 0xFF0080FF, true, 2.0f);

        return ret;
    }

    U32 Edit(SSequencerDelegate& delegate, const ImVec2& size, U32 id, const ImRect* clippingRect, ImVector<SSequencerEditPoint>* selectedPoints)
    {
        // Declare variables
        static bool selectingQuad = false;
        static ImVec2 quadSelection;
        static int overCurveIndex = -1;
        static int movingCurveIndex = -1;
        static bool scrollingV = false;
        static std::set<SSequencerEditPoint> selection;
        static bool overSelectedPoint = false;

        int returnValue = 0;

        // ImGui variables / style setup
        ImGuiIO& io = ImGui::GetIO();
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Border, 0);
        ImGui::BeginChildFrame(id, size);
        delegate.IsFocused = ImGui::IsWindowFocused();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        if (clippingRect)
            draw_list->PushClipRect(clippingRect->Min, clippingRect->Max, true);

        // Temp variables
        const ImVec2 offset = ImGui::GetCursorScreenPos() + ImVec2(0.f, size.y);
        const ImVec2 ssize(size.x, -size.y);
        const ImRect container(offset + ImVec2(0.f, ssize.y), offset + ImVec2(ssize.x, 0.f));
        ImVec2& min = delegate.GetMin();
        ImVec2& max = delegate.GetMax();

        HandleZoomAndVScroll(container, io, offset, ssize, min, max, scrollingV);
        ImVec2 range = max - min + ImVec2(1.f, 0.f);  // +1 because of inclusive last frame

        const ImVec2 viewSize(size.x, -size.y);
        const ImVec2 sizeOfPixel = ImVec2(1.f, 1.f) / viewSize;
        const size_t curveCount = delegate.GetCurveCount();

        if (scrollingV)
        {
            float deltaH = io.MouseDelta.y * range.y * sizeOfPixel.y;
            min.y -= deltaH;
            max.y -= deltaH;
            if (!ImGui::IsMouseDown(2))
                scrollingV = false;
        }

        draw_list->AddRectFilled(offset, offset + ssize, delegate.GetBackgroundColor());

        draw_list->AddLine(ImVec2(-1.f, -min.y / range.y) * viewSize + offset, ImVec2(1.f, -min.y / range.y) * viewSize + offset, 0xFF000000, 1.5f);
        bool overCurveOrPoint = false;

        int localOverCurveIndex = -1;
        // make sure highlighted curve is rendered last
        int* curvesIndex = (int*)_malloca(sizeof(int) * curveCount);
        for (size_t c = 0; c < curveCount; c++)
            curvesIndex[c] = int(c);
        int highLightedCurveIndex = -1;
        if (overCurveIndex != -1 && curveCount)
        {
            ImSwap(curvesIndex[overCurveIndex], curvesIndex[curveCount - 1]);
            highLightedCurveIndex = overCurveIndex;
        }

        LoopOverCurves(curveCount, curvesIndex, delegate, highLightedCurveIndex, selection, selectingQuad, movingCurveIndex, range, min, viewSize, offset, io, scrollingV, localOverCurveIndex, overCurveIndex, overCurveOrPoint, draw_list, overSelectedPoint);

        if (localOverCurveIndex == -1)
            overCurveIndex = -1;

        static bool pointsMoved = false;
        static ImVec2 mousePosOrigin;
        static std::vector<ImVec2> originalPoints;

        MoveSelection(overSelectedPoint, io, selection, pointsMoved, delegate, mousePosOrigin, originalPoints, returnValue, range, sizeOfPixel);

        CheckIsOverSelectedPoint(overSelectedPoint, io, pointsMoved, delegate);

        AddPoint(overCurveIndex, io, delegate, offset, viewSize, range, returnValue);

        MoveCurve(movingCurveIndex, delegate, pointsMoved, mousePosOrigin, io, originalPoints, range, sizeOfPixel, returnValue, overCurveIndex, selection, selectingQuad);

        QuadSelection(selectingQuad, quadSelection, io, draw_list, selection, curveCount, delegate, range, viewSize, offset, overCurveOrPoint, movingCurveIndex, overSelectedPoint, container);

        if (clippingRect)
            draw_list->PopClipRect();

        ImGui::EndChildFrame();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(1);

        UpdateSelectedPoints(selectedPoints, selection);
        return returnValue;
    }

    void CheckIsOverSelectedPoint(bool& overSelectedPoint, ImGuiIO& io, bool& pointsMoved, ImCurveEdit::SSequencerDelegate& delegate)
    {
        if (overSelectedPoint && !io.MouseDown[0])
        {
            overSelectedPoint = false;
            if (pointsMoved)
            {
                pointsMoved = false;
                delegate.EndEdit();
            }
        }
    }

    void UpdateSelectedPoints(ImVector<ImCurveEdit::SSequencerEditPoint>* selectedPoints, std::set<ImCurveEdit::SSequencerEditPoint>& selection)
    {
        if (selectedPoints)
        {
            selectedPoints->resize(int(selection.size()));
            int index = 0;
            for (auto& point : selection)
                (*selectedPoints)[index++] = point;
        }
    }

    void QuadSelection(bool& selectingQuad, ImVec2& quadSelection, ImGuiIO& io, ImDrawList* draw_list, std::set<ImCurveEdit::SSequencerEditPoint>& selection, const size_t& curveCount, ImCurveEdit::SSequencerDelegate& delegate, ImVec2& range, const ImVec2& viewSize, const ImVec2& offset, bool overCurveOrPoint, int movingCurveIndex, bool overSelectedPoint, const ImRect& container)
    {
        if (selectingQuad)
        {
            const ImVec2 bmin = ImMin(quadSelection, io.MousePos);
            const ImVec2 bmax = ImMax(quadSelection, io.MousePos);
            draw_list->AddRectFilled(bmin, bmax, 0x40FF0000, 1.f);
            draw_list->AddRect(bmin, bmax, 0xFFFF0000, 1.f);
            const ImRect selectionQuad(bmin, bmax);
            if (!io.MouseDown[0])
            {
                if (!io.KeyShift)
                    selection.clear();
                // select everythnig is quad
                for (size_t c = 0; c < curveCount; c++)
                {
                    if (!delegate.GetIsVisible(c))
                        continue;

                    const size_t ptCount = delegate.GetPointCount(c);
                    if (ptCount < 1)
                        continue;

                    const ImVec2* pts = delegate.GetPoints(c);
                    for (size_t p = 0; p < ptCount; p++)
                    {
                        const ImVec2 center = delegate.PointToRange(pts[p], range) * viewSize + offset;
                        if (selectionQuad.Contains(center))
                            selection.insert({ U64(c), U32(p) });
                    }
                }
                // done
                selectingQuad = false;
            }
        }
        if (!overCurveOrPoint && ImGui::IsMouseClicked(0) && !selectingQuad && movingCurveIndex == -1 && !overSelectedPoint && container.Contains(io.MousePos))
        {
            selectingQuad = true;
            quadSelection = io.MousePos;
        }
    }

    void AddPoint(int overCurveIndex, ImGuiIO& io, ImCurveEdit::SSequencerDelegate& delegate, const ImVec2& offset, const ImVec2& viewSize, ImVec2& range, int& returnValue)
    {
        if (overCurveIndex != -1 && io.MouseDoubleClicked[0])
        {
            const ImVec2 np = delegate.RangeToPoint((io.MousePos - offset) / viewSize, range);
            delegate.BeginEdit(overCurveIndex);
            delegate.AddPoint(overCurveIndex, np);
            delegate.EndEdit();
            returnValue = 1;
        }
    }

    void MoveCurve(int& movingCurveIndex, ImCurveEdit::SSequencerDelegate& delegate, bool& pointsMoved, ImVec2& mousePosOrigin, ImGuiIO& io, std::vector<ImVec2>& originalPoints, ImVec2& range, const ImVec2& sizeOfPixel, int& returnValue, int overCurveIndex, std::set<ImCurveEdit::SSequencerEditPoint>& selection, bool selectingQuad)
    {
        if (movingCurveIndex != -1)
        {
            const size_t ptCount = delegate.GetPointCount(movingCurveIndex);
            const ImVec2* pts = delegate.GetPoints(movingCurveIndex);
            if (!pointsMoved)
            {
                mousePosOrigin = io.MousePos;
                pointsMoved = true;
                originalPoints.resize(ptCount);
                for (size_t index = 0; index < ptCount; index++)
                {
                    originalPoints[index] = pts[index];
                }
            }
            if (ptCount >= 1)
            {
                for (size_t p = 0; p < ptCount; p++)
                {
                    delegate.EditPoint(movingCurveIndex, int(p), delegate.RangeToPoint(delegate.PointToRange(originalPoints[p], range) + (io.MousePos - mousePosOrigin) * sizeOfPixel, range));
                }
                returnValue = 1;
            }
            if (!io.MouseDown[0])
            {
                movingCurveIndex = -1;
                pointsMoved = false;
                delegate.EndEdit();
            }
        }
        if (movingCurveIndex == -1 && overCurveIndex != -1 && ImGui::IsMouseClicked(0) && selection.empty() && !selectingQuad)
        {
            movingCurveIndex = overCurveIndex;
            delegate.BeginEdit(overCurveIndex);
        }
    }

    void MoveSelection(bool overSelectedPoint, ImGuiIO& io, std::set<ImCurveEdit::SSequencerEditPoint>& selection, bool& pointsMoved, ImCurveEdit::SSequencerDelegate& delegate, ImVec2& mousePosOrigin, std::vector<ImVec2>& originalPoints, int& ret, const ImVec2& range, const ImVec2& sizeOfPixel)
    {
        if (overSelectedPoint && io.MouseDown[0])
        {
            if ((fabsf(io.MouseDelta.x) > 0.f || fabsf(io.MouseDelta.y) > 0.f) && !selection.empty())
            {
                if (!pointsMoved)
                {
                    delegate.BeginEdit(0);
                    mousePosOrigin = io.MousePos;
                    originalPoints.resize(selection.size());
                    int index = 0;
                    for (auto& sel : selection)
                    {
                        const ImVec2* pts = delegate.GetPoints(sel.CurveIndex);
                        originalPoints[index++] = pts[sel.PointIndex];
                    }
                }
                pointsMoved = true;
                ret = 1;
                auto prevSelection = selection;
                int originalIndex = 0;
                for (auto& sel : prevSelection)
                {
                    const ImVec2 p = delegate.RangeToPoint(delegate.PointToRange(originalPoints[originalIndex], range) + (io.MousePos - mousePosOrigin) * sizeOfPixel, range);
                    const U32 newIndex = delegate.EditPoint(sel.CurveIndex, sel.PointIndex, p);
                    if (newIndex != sel.PointIndex)
                    {
                        selection.erase(sel);
                        selection.insert({ sel.CurveIndex, newIndex });
                    }
                    originalIndex++;
                }
            }
        }
    }

    void LoopOverCurves(const size_t& curveCount, int* curvesIndex, ImCurveEdit::SSequencerDelegate& delegate, int highLightedCurveIndex, std::set<ImCurveEdit::SSequencerEditPoint>& selection, bool selectingQuad, int movingCurve, const ImVec2& range, ImVec2& min, const ImVec2& viewSize, const ImVec2& offset, ImGuiIO& io, bool scrollingV, int& localOverCurve, int& overCurve, bool& overCurveOrPoint, ImDrawList* draw_list, bool& overSelectedPoint)
    {
        auto pointToRange = [&](ImVec2 pt) { return (pt - min) / range; };

        for (size_t cur = 0; cur < curveCount; cur++)
        {
            int c = curvesIndex[cur];
            if (!delegate.GetIsVisible(c))
                continue;
            const size_t ptCount = delegate.GetPointCount(c);
            if (ptCount < 1)
                continue;
            ECurveType curveType = delegate.GetCurveType(c);
            if (curveType == CurveNone)
                continue;
            const ImVec2* pts = delegate.GetPoints(c);
            uint32_t curveColor = delegate.GetCurveColor(c);
            if ((c == highLightedCurveIndex && selection.empty() && !selectingQuad) || movingCurve == c)
                curveColor = 0xFFFFFFFF;

            for (size_t p = 0; p < ptCount - 1; p++)
            {
                const ImVec2 p1 = pointToRange(pts[p]);
                const ImVec2 p2 = pointToRange(pts[p + 1]);

                if (curveType == CurveSmooth || curveType == CurveLinear)
                {
                    size_t subStepCount = (curveType == CurveSmooth) ? 20 : 2;
                    float step = 1.f / float(subStepCount - 1);
                    for (size_t substep = 0; substep < subStepCount - 1; substep++)
                    {
                        float t = float(substep) * step;

                        const ImVec2 sp1 = ImLerp(p1, p2, t);
                        const ImVec2 sp2 = ImLerp(p1, p2, t + step);

                        const float rt1 = smoothstep(p1.x, p2.x, sp1.x);
                        const float rt2 = smoothstep(p1.x, p2.x, sp2.x);

                        const ImVec2 pos1 = ImVec2(sp1.x, ImLerp(p1.y, p2.y, rt1)) * viewSize + offset;
                        const ImVec2 pos2 = ImVec2(sp2.x, ImLerp(p1.y, p2.y, rt2)) * viewSize + offset;

                        if (distance(io.MousePos.x, io.MousePos.y, pos1.x, pos1.y, pos2.x, pos2.y) < 8.f && !scrollingV)
                        {
                            localOverCurve = int(c);
                            overCurve = int(c);
                            overCurveOrPoint = true;
                        }

                        draw_list->AddLine(pos1, pos2, curveColor, 1.3f);
                    } // substep
                }
                else if (curveType == CurveDiscrete)
                {
                    ImVec2 dp1 = p1 * viewSize + offset;
                    ImVec2 dp2 = ImVec2(p2.x, p1.y) * viewSize + offset;
                    ImVec2 dp3 = p2 * viewSize + offset;
                    draw_list->AddLine(dp1, dp2, curveColor, 1.3f);
                    draw_list->AddLine(dp2, dp3, curveColor, 1.3f);

                    if ((distance(io.MousePos.x, io.MousePos.y, dp1.x, dp1.y, dp3.x, dp1.y) < 8.f ||
                        distance(io.MousePos.x, io.MousePos.y, dp3.x, dp1.y, dp3.x, dp3.y) < 8.f)
                        /*&& localOverCurve == -1*/)
                    {
                        localOverCurve = int(c);
                        overCurve = int(c);
                        overCurveOrPoint = true;
                    }
                }
            } // point loop

            for (size_t p = 0; p < ptCount; p++)
            {
                const int drawState = DrawPoint(draw_list, pointToRange(pts[p]), viewSize, offset, (selection.find({ U64(c), U32(p) }) != selection.end() && movingCurve == -1 && !scrollingV));
                if (drawState && movingCurve == -1 && !selectingQuad)
                {
                    overCurveOrPoint = true;
                    overSelectedPoint = true;
                    overCurve = -1;
                    if (drawState == 2)
                    {
                        if (!io.KeyShift && selection.find({ U64(c), U32(p) }) == selection.end())
                            selection.clear();
                        selection.insert({ U64(c), U32(p) });
                    }
                }
            }
        } // curves loop
    }

    void HandleZoomAndVScroll(const ImRect& container, const ImGuiIO& io, const ImVec2& offset, const ImVec2& ssize, ImVec2& inOutMin, ImVec2& inOutMax, bool& outScrollingV)
    {
        if (container.Contains(io.MousePos))
        {
            if (fabsf(io.MouseWheel) > FLT_EPSILON)
            {
                const float r = (io.MousePos.y - offset.y) / ssize.y;
                float ratioY = ImLerp(inOutMin.y, inOutMax.y, r);
                auto scaleValue = [&](float v) {
                    v -= ratioY;
                    v *= (1.f - io.MouseWheel * 0.05f);
                    v += ratioY;
                    return v;
                };
                inOutMin.y = scaleValue(inOutMin.y);
                inOutMax.y = scaleValue(inOutMax.y);
            }
            if (!outScrollingV && ImGui::IsMouseDown(2))
            {
                outScrollingV = true;
            }
        }
    }
}

void SSequencer::CustomDraw(int index, ImDrawList* drawList, const ImRect& rect, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect, const std::vector<SEditorComponentTrack>& componentTracks)
{
    RampEdit.MaxValue = ImVec2(float(FrameMax), 1.f);
    RampEdit.MinValue = ImVec2(float(FrameMin), 0.f);
    drawList->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);

    for (int i = 0; i < componentTracks.size(); i++)
    {
        ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
        ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);

        drawList->AddText(pta, RampEdit.IsVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, Havtorn::GetComponentTypeString(componentTracks[i].ComponentType).c_str());

        if (ImRect(pta, ptb).Contains(GImGui->IO.MousePos) && ImGui::IsMouseClicked(0))
            RampEdit.IsVisible[i] = !RampEdit.IsVisible[i];
    }

    drawList->PopClipRect();

    ImGui::SetCursorScreenPos(rect.Min);
    ImVec2 rectMax = rect.Max;
    ImVec2 rectMin = rect.Min;
    ImVec2 size = ImVec2(rectMax.x - rectMin.x, rectMax.y - rectMin.y);
    U32 id = 137 + index;

    // ImGui variables / style setup
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, 0);
    ImGui::BeginChildFrame(id, size);
    RampEdit.IsFocused = ImGui::IsWindowFocused();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);

    // Temp variables
    // NR: What is -4.5 exactly? horizontal offset from frame line or something. Look at playhead's horizontal offset
    const ImVec2 offset = ImGui::GetCursorScreenPos() + ImVec2(-4.5f, size.y);
    const ImVec2 ssize(size.x, -size.y);
    ImVec2 min = ImVec2(float(FrameMin), offset.y);
    ImVec2 max = ImVec2(float(FrameMax), offset.y + ssize.y - 9.0f);
    ImVec2 range = max - min + ImVec2(1.f, 0.f);  // +1 because of inclusive last frame
    const ImVec2 viewSize(size.x, -size.y);

    draw_list->AddRectFilled(offset, offset + ssize, RampEdit.GetBackgroundColor());

    auto pointToRange = [&](ImVec2 pt) { return (pt - min) / range; };
    for (int i2 = 0; i2 < componentTracks.size(); i2++)
    {
        const SEditorComponentTrack& componentTrack = componentTracks[i2];
        for (U32 keyframe : componentTrack.Keyframes)
        {
            ImVec2 point = ImVec2(static_cast<Havtorn::F32>(keyframe), legendRect.Min.y + i2 * 14.f);
            DrawKeyframe(drawList, pointToRange(point), viewSize, offset, false);
        }
    }

    draw_list->PopClipRect();

    ImGui::EndChildFrame();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(1);

    //ImGui::SetCursorScreenPos(rect.Min);
    //ImVec2 max = rect.Max;
    //ImVec2 min = rect.Min;
    //ImVec2 size = ImVec2(max.x - min.x, max.y - min.y);
    //ImCurveEdit::Edit(RampEdit, size, 137 + index, &clippingRect);
}
