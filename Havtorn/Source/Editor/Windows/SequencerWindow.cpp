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
        FrameMin = 0;
        FrameMax = 100;

        Sequencers.push_back("Intro");
        Sequencers.push_back("BossFight");

        KeyframeColorMap.insert({Havtorn::EComponentType::TransformComponent, { 0xFF95CAA5, 0xFFCAE4D2 }});
        KeyframeColorMap.insert({Havtorn::EComponentType::SpriteComponent, { 0xFFCA94A3, 0xFFE4C9D1 }});
        KeyframeColorMap.insert({Havtorn::EComponentType::SpotLightComponent, { 0xFF03BAFC, 0xFF5FABC7 }});

        SupportedComponentTrackTypes.push_back(Havtorn::EComponentType::TransformComponent);
        SupportedComponentTrackTypes.push_back(Havtorn::EComponentType::SpriteComponent);
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
            IsMenuOpen = false;

            ImGui::InputInt("Current Frame ", &imGuiFrame);
            ImGui::SameLine();
            ImGui::InputInt("Frame Max", &imGuiMaxFrame);
            ImGui::SameLine();
            ImGui::InputInt("Play Rate", &playRate);
            ImGui::SameLine();
            ImGui::Checkbox("Loop", &contextData.IsLooping);

            ContentControls();

            ImGui::PopItemWidth();            

            FillSequencer();

            DrawSequencer(&imGuiFrame, &expanded, &selectedEntry, &firstFrame, SEQUENCER_EDIT_STARTEND | SEQUENCER_ADD | SEQUENCER_DEL | SEQUENCER_COPYPASTE | SEQUENCER_CHANGE_FRAME);

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

    void CSequencerWindow::ContentControls()
    {
        if (ImGui::Button("Add Component Track"))
        {
            // TODO.NR: Could have a bool distinguish between functionality and keep one popup function
            ImGui::OpenPopup(NewComponentTrackPopupName.c_str());
        }

        if (ImGui::BeginPopup(NewComponentTrackPopupName.c_str()))
        {
            IsMenuOpen = true;

            for (const Havtorn::EComponentType componentType : SupportedComponentTrackTypes)
            {
                if (ImGui::MenuItem(Havtorn::GetComponentTypeString(componentType).c_str()))
                {
                    AddComponentTrack(componentType);
                }
            }
            ImGui::EndPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Add Keyframe"))
        {
            ImGui::OpenPopup(NewKeyframePopupName.c_str());
        }

        if (ImGui::BeginPopup(NewKeyframePopupName.c_str()))
        {
            IsMenuOpen = true;

            for (const Havtorn::EComponentType componentType : SupportedComponentTrackTypes)
            {
                if (ImGui::MenuItem(Havtorn::GetComponentTypeString(componentType).c_str()))
                {
                    switch (componentType)
                    {
                    case Havtorn::EComponentType::TransformComponent:
                        AddKeyframe<Havtorn::SSequencerTransformKeyframe>(componentType);
                        break;
                    case Havtorn::EComponentType::SpriteComponent:
                        AddKeyframe<Havtorn::SSequencerSpriteKeyframe>(componentType);
                        break;
                    default:
                        break;
                    }
                }
            }
            ImGui::EndPopup();
        }

        SEditorKeyframe* selectedKeyframe = GetSelectedKeyframe();
        if (selectedKeyframe != nullptr)
        {
            bool keyframeIsEdited = false;
            ImGui::SameLine();
            ImGui::Text(" | ");
            ImGui::SameLine();
            ImGui::Text("Selected Keyframe");
            ImGui::SameLine();

            int frameNumber = static_cast<int>(selectedKeyframe->FrameNumber);
            if (ImGui::InputInt("Frame Number:", &frameNumber))
                keyframeIsEdited = true;

            if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
                float framePixelWidth = 10.f;
                int legendWidth = 200;
                int itemHeight = 20;

                ImGuiIO& io = ImGui::GetIO();
                ImVec2 canvasPosition = ImGui::GetCursorScreenPos();
                ImVec2 canvas_size = ImGui::GetContentRegionAvail();

                ImRect topRect(ImVec2(canvasPosition.x + legendWidth, canvasPosition.y), ImVec2(canvasPosition.x + canvas_size.x, canvasPosition.y + itemHeight));
                frameNumber = (int)((io.MousePos.x - topRect.Min.x) / framePixelWidth) + 0;
                frameNumber = Havtorn::UMath::Clamp(frameNumber, 0, 100);
                keyframeIsEdited = true;
            }

            selectedKeyframe->FrameNumber = frameNumber;
            ImGui::SameLine();
            if (ImGui::Checkbox("Blend Left:", &selectedKeyframe->ShouldBlendLeft))
                keyframeIsEdited = true;

            ImGui::SameLine();
            if (ImGui::Checkbox("Blend Right:", &selectedKeyframe->ShouldBlendRight))
                keyframeIsEdited = true;

            ImGui::SameLine();
            if (ImGui::Button("Set Current Component Value"))
                SetCurrentComponentValueOnKeyframe();

            if (keyframeIsEdited)
                EditSelectedKeyframe(selectedKeyframe);
        }
    }

    void CSequencerWindow::SetCurrentComponentValueOnKeyframe()
    {
        if (!SelectedKeyframeMetaData.IsValid(this))
            return;

        Havtorn::CScene* const scene = Manager->GetCurrentScene();
        if (scene == nullptr)
            return;

        std::vector<Havtorn::SSequencerComponent>& sequencerComponents = scene->GetSequencerComponents();
        for (U64 index = 0, entityTrackIndex = 0; index < sequencerComponents.size(); index++)
        {
            Havtorn::SSequencerComponent& component = sequencerComponents[index];
            if (!component.IsInUse)
                continue;

            if (entityTrackIndex++ != SelectedKeyframeMetaData.EntityTrackIndex)
                continue;

            Havtorn::SSequencerKeyframe* sequencerKeyframe = component.ComponentTracks[SelectedKeyframeMetaData.ComponentTrackIndex].Keyframes[SelectedKeyframeMetaData.KeyframeIndex];

            sequencerKeyframe->SetEntityDataOnKeyframe(scene, index);
        }
    }

    void CSequencerWindow::SetCurrentKeyframeValueOnComponent()
    {
        if (!SelectedKeyframeMetaData.IsValid(this))
            return;

        Havtorn::CScene* const scene = Manager->GetCurrentScene();
        if (scene == nullptr)
            return;

        std::vector<Havtorn::SSequencerComponent>& sequencerComponents = scene->GetSequencerComponents();
        for (U64 index = 0, entityTrackIndex = 0; index < sequencerComponents.size(); index++)
        {
            Havtorn::SSequencerComponent& component = sequencerComponents[index];
            if (!component.IsInUse)
                continue;

            if (entityTrackIndex++ != SelectedKeyframeMetaData.EntityTrackIndex)
                continue;

            Havtorn::SSequencerKeyframe* sequencerKeyframe = component.ComponentTracks[SelectedKeyframeMetaData.ComponentTrackIndex].Keyframes[SelectedKeyframeMetaData.KeyframeIndex];

            sequencerKeyframe->SetKeyframeDataOnEntity(scene, index);
        }
    }

    void CSequencerWindow::FillSequencer()
    {
        Havtorn::CScene* const scene = Manager->GetCurrentScene();
        if (scene == nullptr)
            return;

        std::vector<Havtorn::SSequencerComponent>& sequencerComponents = scene->GetSequencerComponents();
        std::vector<Havtorn::SMetaDataComponent>& metaDataComponents = scene->GetMetaDataComponents();

        EntityTracks.clear();

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
                std::vector<SEditorKeyframe> keyframes;
                for (U64 keyframeIndex = 0; keyframeIndex < componentTrack.Keyframes.size(); keyframeIndex++)
                {
                    // TODO.NR: Make constructor for SEditorKeyframe taking a SSequencerKeyframe
                    const Havtorn::SSequencerKeyframe* keyframe = componentTrack.Keyframes[keyframeIndex];
                    keyframes.push_back({ keyframe->FrameNumber, keyframe->ShouldBlendLeft, keyframe->ShouldBlendRight });
                }

                entityTrack.ComponentTracks.push_back({ componentTrack.ComponentType, keyframes });
            }

            entityTrack.FrameStart = 0;
            entityTrack.FrameEnd = 20;
            entityTrack.IsExpanded = true;

            EntityTracks.push_back(entityTrack);
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
    }

    void CSequencerWindow::EditSelectedKeyframe(SEditorKeyframe* selectedKeyframe)
    {
        if (!SelectedKeyframeMetaData.IsValid(this))
            return;

        Havtorn::CScene* const scene = Manager->GetCurrentScene();
        if (scene == nullptr)
            return;

        std::vector<Havtorn::SSequencerComponent>& sequencerComponents = scene->GetSequencerComponents();
        for (U64 index = 0, entityTrackIndex = 0; index < sequencerComponents.size(); index++)
        {
            Havtorn::SSequencerComponent& component = sequencerComponents[index];
            if (!component.IsInUse)
                continue;

            if (entityTrackIndex++ != SelectedKeyframeMetaData.EntityTrackIndex)
                continue;

            Havtorn::SSequencerKeyframe* sequencerKeyframe = component.ComponentTracks[SelectedKeyframeMetaData.ComponentTrackIndex].Keyframes[SelectedKeyframeMetaData.KeyframeIndex];
            
            sequencerKeyframe->FrameNumber = selectedKeyframe->FrameNumber;
            sequencerKeyframe->ShouldBlendLeft = selectedKeyframe->ShouldBlendLeft;
            sequencerKeyframe->ShouldBlendRight = selectedKeyframe->ShouldBlendRight;
        }

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

    bool CSequencerWindow::DrawSequencer(int* currentFrame, bool* expanded, int* selectedEntry, int* firstFrame, int sequenceOptions)
    {
#pragma region Setup
        int entityTrackCount = GetEntityTrackCount();
        if (!entityTrackCount)
            return false;

        bool returnValue = false;
        ImGuiIO& io = ImGui::GetIO();
        int cx = (int)(io.MousePos.x);
        int cy = (int)(io.MousePos.y);
        static float framePixelWidth = 10.f;
        static float framePixelWidthTarget = 10.f;
        int legendWidth = 200;
        //int legendHeight = 0;

        //for (int i = 0; i < entityTrackCount; i++)
        //    legendHeight += int(sequence->GetCustomHeight(i));

        static int movingEntry = -1;
        static int movingPos = -1;
        static int movingPart = -1;
        int delEntry = -1;
        int dupEntry = -1;
        int ItemHeight = 20;

        bool popupOpened = false;

        ImGui::BeginGroup();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvasPosition = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
        ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
        int firstFrameUsed = firstFrame ? *firstFrame : 0;

        int controlHeight = entityTrackCount * ItemHeight;
        for (int i = 0; i < entityTrackCount; i++)
            controlHeight += int(GetCustomHeight(i));
        int frameCount = ImMax(GetFrameMax() - GetFrameMin(), 1);

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
        std::vector<CustomDraw> customDraws;
        ImVector<CustomDraw> compactCustomDraws;
        // zoom in/out
        const int visibleFrameCount = (int)floorf((canvas_size.x - legendWidth) / framePixelWidth);
        const float barWidthRatio = ImMin(visibleFrameCount / (float)frameCount, 1.f);
        const float barWidthInPixels = barWidthRatio * (canvas_size.x - legendWidth);

        ImRect regionRect(canvasPosition, canvasPosition + canvas_size);
#pragma endregion

        static bool panningView = false;
        static ImVec2 panningViewSource;
        static int panningViewFrame;
        Panning(io, panningView, panningViewSource, panningViewFrame, firstFrame, framePixelWidth, visibleFrameCount, framePixelWidthTarget, frameCount);

        // --
        if (expanded && !*expanded)
        {
            NotExpanded(canvas_size, canvasPosition, ItemHeight, draw_list, frameCount, entityTrackCount);
        }
        else
        {
            bool hasScrollBar(true);

            // test scroll area
            ImVec2 headerSize(canvas_size.x, (float)ItemHeight);
            ImVec2 scrollBarSize(canvas_size.x, 14.f);
            ImGui::InvisibleButton("topBar", headerSize);
            draw_list->AddRectFilled(canvasPosition, canvasPosition + headerSize, 0xFFFF0000, 0);
            ImVec2 childFramePos = ImGui::GetCursorScreenPos();
            ImVec2 childFrameSize(canvas_size.x, canvas_size.y - 8.f - headerSize.y - (hasScrollBar ? scrollBarSize.y : 0));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
            ImGui::BeginChildFrame(889, childFrameSize);
            IsFocused = ImGui::IsWindowFocused();
            ImGui::InvisibleButton("contentBar", ImVec2(canvas_size.x, float(controlHeight)));
            const ImVec2 contentMin = ImGui::GetItemRectMin();
            const ImVec2 contentMax = ImGui::GetItemRectMax();
            //const ImRect contentRect(contentMin, contentMax);
            const float contentHeight = contentMax.y - contentMin.y;

            // full background
            draw_list->AddRectFilled(canvasPosition, canvasPosition + canvas_size, 0xFF242424, 0);

            // current frame top
            ImRect topRect(ImVec2(canvasPosition.x + legendWidth, canvasPosition.y), ImVec2(canvasPosition.x + canvas_size.x, canvasPosition.y + ItemHeight));

            ChangeCurrentFrame(MovingCurrentFrame, MovingScrollBar, movingEntry, sequenceOptions, currentFrame, topRect, io, frameCount, framePixelWidth, firstFrameUsed);

            //header
            draw_list->AddRectFilled(canvasPosition, ImVec2(canvas_size.x + canvasPosition.x, canvasPosition.y + ItemHeight), 0xFF3D3837, 0);

            AddEntityTrackButton(sequenceOptions, draw_list, canvasPosition, legendWidth, ItemHeight, io, selectedEntry, popupOpened);
            
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
                bool baseIndex = ((i % modFrameCount) == 0) || (i == GetFrameMax() || i == GetFrameMin());
                bool halfIndex = (i % halfModFrameCount) == 0;
                int px = (int)canvasPosition.x + int(i * framePixelWidth) + legendWidth - int(firstFrameUsed * framePixelWidth);
                int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
                int tiretEnd = baseIndex ? regionHeight : ItemHeight;

                if (px <= (canvas_size.x + canvasPosition.x) && px >= (canvasPosition.x + legendWidth))
                {
                    draw_list->AddLine(ImVec2((float)px, canvasPosition.y + (float)tiretStart), ImVec2((float)px, canvasPosition.y + (float)tiretEnd - 1), 0xFF606060, 1);

                    draw_list->AddLine(ImVec2((float)px, canvasPosition.y + (float)ItemHeight), ImVec2((float)px, canvasPosition.y + (float)regionHeight - 1), 0x30606060, 1);
                }

                if (baseIndex && px > (canvasPosition.x + legendWidth))
                {
                    char tmps[512];
                    ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", i);
                    draw_list->AddText(ImVec2((float)px + 3.f, canvasPosition.y), 0xFFBBBBBB, tmps);
                }

            };

            auto drawLineContent = [&](int i, int /*regionHeight*/) {
                int px = (int)canvasPosition.x + int(i * framePixelWidth) + legendWidth - int(firstFrameUsed * framePixelWidth);
                int tiretStart = int(contentMin.y);
                int tiretEnd = int(contentMax.y);

                if (px <= (canvas_size.x + canvasPosition.x) && px >= (canvasPosition.x + legendWidth))
                {
                    draw_list->AddLine(ImVec2(float(px), float(tiretStart)), ImVec2(float(px), float(tiretEnd)), 0x30606060, 1);
                }
            };

            for (int i = GetFrameMin(); i <= GetFrameMax(); i += frameStep)
            {
                drawLine(i, ItemHeight);
            }
            drawLine(GetFrameMin(), ItemHeight);
            drawLine(GetFrameMax(), ItemHeight);

            // clip content
            draw_list->PushClipRect(childFramePos, childFramePos + childFrameSize);

            size_t customHeight = 0;
            
            // Draw entity names in the legend rect on the left
            TrackHeader(entityTrackCount, contentMin, ItemHeight, customHeight, draw_list, sequenceOptions, legendWidth, io, delEntry, dupEntry);

            // clipping rect so items bars are not visible in the legend on the left when scrolled
            //

            // Track rect and backgrounds
            customHeight = 0;
            TrackSlotsBackground(entityTrackCount, contentMin, legendWidth, ItemHeight, customHeight, canvas_size, canvasPosition, popupOpened, cy, movingEntry, cx, draw_list);

            draw_list->PushClipRect(childFramePos + ImVec2(float(legendWidth), 0.f), childFramePos + childFrameSize);

            // Vertical frame lines on tracks
            for (int i = GetFrameMin(); i <= GetFrameMax(); i += frameStep)
            {
                drawLineContent(i, int(contentHeight));
            }
            drawLineContent(GetFrameMin(), int(contentHeight));
            drawLineContent(GetFrameMax(), int(contentHeight));

            // selection
            bool selected = selectedEntry && (*selectedEntry >= 0);
            Selection(selected, customHeight, selectedEntry, draw_list, contentMin, ItemHeight, canvas_size);

            // slots
#pragma region Track Content, component tracks
            customHeight = 0;
            for (int entityTrackIndex = 0; entityTrackIndex < entityTrackCount; entityTrackIndex++)
            {
                for (int componentTrackIndex = 0; componentTrackIndex < GetComponentTrackCount(entityTrackIndex); componentTrackIndex++)
                {
                    std::vector<std::pair<int, int>> blendRegions = {};
                    unsigned int color;
                    GetBlendRegionInfo(this, entityTrackIndex, componentTrackIndex, blendRegions, &color);

                    if (blendRegions.empty())
                        continue;

                    for (int blendRegionIndex = 0; blendRegionIndex < blendRegions.size(); blendRegionIndex++)
                    {
                        const std::pair<int, int>& blendRegion = blendRegions[blendRegionIndex];
                        const int start = blendRegion.first, end = blendRegion.second;

                        ImVec2 pos = ImVec2(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth, contentMin.y + ItemHeight * (entityTrackIndex + 1 + componentTrackIndex) - 7.f + customHeight);
                        ImVec2 slotP1(pos.x + Havtorn::UMath::Max(start - 1, 0) * framePixelWidth, pos.y + 2 + 10.f);
                        ImVec2 slotP2(pos.x + end * framePixelWidth, pos.y + ItemHeight - 2);
                        unsigned int slotColor = color - 0x88000000;

                        if (slotP1.x <= (canvas_size.x + contentMin.x) && slotP2.x >= (contentMin.x + legendWidth))
                        {
                            // Blend region rect
                            draw_list->AddRectFilled(slotP1, slotP2, slotColor, 2);
                        }
                        if (ImRect(slotP1, slotP2).Contains(io.MousePos) && io.MouseDoubleClicked[0])
                        {
                            DoubleClick(entityTrackIndex);
                        }
                    }
                }

                int* start, *end;
                unsigned int color;
                Get(entityTrackIndex, &start, &end, NULL, &color);
                size_t localCustomHeight = GetCustomHeight(entityTrackIndex);
                
                // custom draw
                if (localCustomHeight > 0)
                {
                    ImVec2 rp(canvasPosition.x, contentMin.y + ItemHeight * entityTrackIndex + 1 + customHeight);
                    ImRect customRect(rp + ImVec2(legendWidth - (firstFrameUsed - GetFrameMin() - 0.5f) * framePixelWidth, float(ItemHeight)),
                        rp + ImVec2(legendWidth + (GetFrameMax() - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(localCustomHeight + ItemHeight)));
                    ImRect clippingRect(rp + ImVec2(float(legendWidth), float(ItemHeight)), rp + ImVec2(canvas_size.x, float(localCustomHeight + ItemHeight)));

                    ImRect legendRect(rp + ImVec2(0.f, float(ItemHeight)), rp + ImVec2(float(legendWidth), float(localCustomHeight)));

                    //ImRect legendClippingRect(canvas_pos + ImVec2(0.f, float(ItemHeight)), canvas_pos + ImVec2(float(legendWidth), float(localCustomHeight + ItemHeight)));
                    ImRect legendClippingRect(canvasPosition, canvasPosition + ImVec2(float(legendWidth), float(canvas_size.y)));

                    std::vector<SEditorComponentTrack>& componentTracks = GetComponentTracks(entityTrackIndex);
                    customDraws.push_back({ entityTrackIndex, customRect, legendRect, clippingRect, legendClippingRect, componentTracks });
                }
                else
                {
                    ImVec2 rp(canvasPosition.x, contentMin.y + ItemHeight * entityTrackIndex + customHeight);
                    ImRect customRect(rp + ImVec2(legendWidth - (firstFrameUsed - GetFrameMin() - 0.5f) * framePixelWidth, float(0.f)),
                        rp + ImVec2(legendWidth + (GetFrameMax() - firstFrameUsed - 0.5f + 2.f) * framePixelWidth, float(ItemHeight)));
                    ImRect clippingRect(rp + ImVec2(float(legendWidth), float(0.f)), rp + ImVec2(canvas_size.x, float(ItemHeight)));

                    compactCustomDraws.push_back({ entityTrackIndex, customRect, ImRect(), clippingRect, ImRect() });
                }
                customHeight += localCustomHeight; 
            }
#pragma endregion

            // moving
            Moving(movingEntry, cx, movingPos, framePixelWidth, selectedEntry, movingPart, io, returnValue);

#pragma region Playhead
            if (currentFrame && firstFrame && *currentFrame >= *firstFrame && *currentFrame <= GetFrameMax())
            {
                static const float cursorWidth = 4.5f;
                float cursorOffset = contentMin.x + legendWidth + (*currentFrame - firstFrameUsed) * framePixelWidth + framePixelWidth / 2 - cursorWidth * 0.5f;
                draw_list->AddLine(ImVec2(cursorOffset, canvasPosition.y), ImVec2(cursorOffset, contentMax.y), 0xA02A2AFF, cursorWidth);
                //char tmps[512];
                //ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", *currentFrame);
                //draw_list->AddText(ImVec2(cursorOffset + 10, canvasPosition.y + 2), 0xFF2A2AFF, tmps);
            }
#pragma endregion

            draw_list->PopClipRect();
            draw_list->PopClipRect();

            //if (io.MouseClicked[0])
            //    ResetSelectedKeyframe();

            for (CustomDraw& customDraw : customDraws)
                DrawComponentTracks(this, customDraw.index, draw_list, customDraw.customRect, customDraw.legendRect, customDraw.clippingRect, customDraw.legendClippingRect, customDraw.componentTracks);

            // copy paste
            CopyPaste(sequenceOptions, contentMin, canvasPosition, ItemHeight, io, draw_list);
            //

            ImGui::EndChildFrame();
            ImGui::PopStyleColor();
            Scrollbar(hasScrollBar, scrollBarSize, firstFrameUsed, frameCount, canvas_size, legendWidth, draw_list, io, barWidthInPixels, MovingScrollBar, framePixelWidthTarget, framePixelWidth, firstFrame, visibleFrameCount, panningViewSource, panningViewFrame, MovingCurrentFrame, movingEntry);
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
            bool overExpanded = SequencerAddDelButton(draw_list, ImVec2(canvasPosition.x + 2, canvasPosition.y + 2), !*expanded);
            if (overExpanded && io.MouseReleased[0])
                *expanded = !*expanded;
        }

        if (delEntry != -1)
        {
            Del(delEntry);
            if (selectedEntry && (*selectedEntry == delEntry || *selectedEntry >= GetEntityTrackCount()))
                *selectedEntry = -1;
        }

        if (dupEntry != -1)
        {
            Duplicate(dupEntry);
        }
        return returnValue;
    }

    void CSequencerWindow::Scrollbar(bool hasScrollBar, ImVec2& scrollBarSize, int firstFrameUsed, int frameCount, ImVec2& canvas_size, int legendWidth, ImDrawList* draw_list, ImGuiIO& io, const float& barWidthInPixels, bool& MovingScrollBar, float& framePixelWidthTarget, float& framePixelWidth, int* firstFrame, const int& visibleFrameCount, ImVec2& panningViewSource, int& panningViewFrame, bool MovingCurrentFrame, int movingEntry)
    {
        if (hasScrollBar)
        {
            ImGui::InvisibleButton("scrollBar", scrollBarSize);
            ImVec2 scrollBarMin = ImGui::GetItemRectMin();
            ImVec2 scrollBarMax = ImGui::GetItemRectMax();

            // ratio = number of frames visible in control / number to total frames

            float startFrameOffset = ((float)(firstFrameUsed - GetFrameMin()) / (float)frameCount) * (canvas_size.x - legendWidth);
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
                    if (lastFrame > GetFrameMax())
                    {
                        framePixelWidthTarget = framePixelWidth = (canvas_size.x - legendWidth) / float(GetFrameMax() - *firstFrame);
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
                        newFirstFrame = ImClamp(newFirstFrame, GetFrameMin(), ImMax(GetFrameMax() - visibleFrameCount, GetFrameMin()));
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
                        *firstFrame = ImClamp(*firstFrame, GetFrameMin(), ImMax(GetFrameMax() - visibleFrameCount, GetFrameMin()));
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

    void CSequencerWindow::CopyPaste(int sequenceOptions, const ImVec2& contentMin, ImVec2& canvas_pos, int ItemHeight, ImGuiIO& io, ImDrawList* draw_list)
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
                Copy();
            }
            if (inRectPaste && io.MouseReleased[0])
            {
                Paste();
            }
        }
    }

    void CSequencerWindow::Moving(int& movingEntry, int cx, int& movingPos, float framePixelWidth, int* selectedEntry, int movingPart, ImGuiIO& io, bool& ret)
    {
        if (movingEntry >= 0)
        {
            ImGui::CaptureMouseFromApp();
            int diffFrame = int((cx - movingPos) / framePixelWidth);
            if (std::abs(diffFrame) > 0)
            {
                int* start, * end;
                Get(movingEntry, &start, &end, NULL, NULL);
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
                EndEdit();
            }
        }
    }

    void CSequencerWindow::Selection(bool selected, size_t& customHeight, int* selectedEntry, ImDrawList* draw_list, const ImVec2& contentMin, int ItemHeight, ImVec2& canvas_size)
    {
        if (selected)
        {
            customHeight = 0;
            for (int i = 0; i < *selectedEntry; i++)
                customHeight += GetCustomHeight(i);;
            draw_list->AddRectFilled(ImVec2(contentMin.x, contentMin.y + ItemHeight * *selectedEntry + customHeight), ImVec2(contentMin.x + canvas_size.x, contentMin.y + ItemHeight * (*selectedEntry + 1) + customHeight), 0x801080FF, 1.f);
        }
    }

    void CSequencerWindow::TrackSlotsBackground(int numberOfEntityTracks, const ImVec2& contentMin, int legendWidth, int ItemHeight, size_t& customHeight, ImVec2& canvas_size, ImVec2& canvas_pos, bool popupOpened, int cy, int movingEntry, int cx, ImDrawList* draw_list)
    {
        for (int i = 0; i < numberOfEntityTracks; i++)
        {
            unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;

            size_t localCustomHeight = GetCustomHeight(i);
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

    void CSequencerWindow::TrackHeader(int numberOfEntityTracks, const ImVec2& contentMin, int ItemHeight, size_t& customHeight, ImDrawList* draw_list, int sequenceOptions, int legendWidth, ImGuiIO& io, int& delEntry, int& dupEntry)
    {
        for (int i = 0; i < numberOfEntityTracks; i++)
        {
            int type;
            Get(i, NULL, NULL, &type, NULL);
            ImVec2 tpos(contentMin.x + 3, contentMin.y + i * ItemHeight + 2 + customHeight);
            draw_list->AddText(tpos, 0xFFFFFFFF, GetEntityTrackLabel(i));

            if (sequenceOptions & SEQUENCER_DEL)
            {
                bool overDel = SequencerAddDelButton(draw_list, ImVec2(contentMin.x + legendWidth - ItemHeight + 2 - 10, tpos.y + 2), false);
                if (overDel && io.MouseReleased[0])
                    delEntry = i;

                bool overDup = SequencerAddDelButton(draw_list, ImVec2(contentMin.x + legendWidth - ItemHeight - ItemHeight + 2 - 10, tpos.y + 2), true);
                if (overDup && io.MouseReleased[0])
                    dupEntry = i;
            }
            customHeight += GetCustomHeight(i);
        }
    }

    void CSequencerWindow::ChangeCurrentFrame(bool& MovingCurrentFrame, bool MovingScrollBar, int movingEntry, int sequenceOptions, int* currentFrame, ImRect& topRect, ImGuiIO& io, int frameCount, float framePixelWidth, int firstFrameUsed)
    {
        if (!IsMenuOpen && !MovingCurrentFrame && !MovingScrollBar && movingEntry == -1 && sequenceOptions & SEQUENCER_CHANGE_FRAME && currentFrame && *currentFrame >= 0 && topRect.Contains(io.MousePos) && io.MouseDown[0])
        {
            MovingCurrentFrame = true;
        }
        if (MovingCurrentFrame)
        {
            if (frameCount)
            {
                *currentFrame = (int)((io.MousePos.x - topRect.Min.x) / framePixelWidth) + firstFrameUsed;
                if (*currentFrame < GetFrameMin())
                    *currentFrame = GetFrameMin();
                if (*currentFrame >= GetFrameMax())
                    *currentFrame = GetFrameMax();
            }
            if (!io.MouseDown[0])
                MovingCurrentFrame = false;
        }
    }

    void CSequencerWindow::Panning(ImGuiIO& io, bool& panningView, ImVec2& panningViewSource, int& panningViewFrame, int* firstFrame, float& framePixelWidth, const int& visibleFrameCount, float& framePixelWidthTarget, int& frameCount)
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
            *firstFrame = ImClamp(*firstFrame, GetFrameMin(), GetFrameMax() - visibleFrameCount);
        }
        if (panningView && !io.MouseDown[2])
        {
            panningView = false;
        }
        framePixelWidthTarget = ImClamp(framePixelWidthTarget, 0.1f, 50.f);

        framePixelWidth = ImLerp(framePixelWidth, framePixelWidthTarget, 0.33f);

        frameCount = GetFrameMax() - GetFrameMin();
        if (visibleFrameCount >= frameCount && firstFrame)
            *firstFrame = GetFrameMin();
    }

    void CSequencerWindow::NotExpanded(ImVec2& canvas_size, ImVec2& canvas_pos, int ItemHeight, ImDrawList* draw_list, int frameCount, int sequenceCount)
    {
        ImGui::InvisibleButton("canvas", ImVec2(canvas_size.x - canvas_pos.x, (float)ItemHeight));
        draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), 0xFF3D3837, 0);
        char tmps[512];
        ImFormatString(tmps, IM_ARRAYSIZE(tmps), GetCollapseFmt(), frameCount, sequenceCount);
        draw_list->AddText(ImVec2(canvas_pos.x + 26, canvas_pos.y + 2), 0xFFFFFFFF, tmps);
    }

    void CSequencerWindow::AddEntityTrackButton(int sequenceOptions, ImDrawList* draw_list, ImVec2& canvas_pos, int legendWidth, int ItemHeight, ImGuiIO& io, int* selectedEntry, bool& popupOpened)
    {
        if (sequenceOptions & SEQUENCER_ADD)
        {
            // Square with one (minus) or two (plus) lines in it
            if (SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + legendWidth - ItemHeight, canvas_pos.y + 2), true) && io.MouseReleased[0])
                ImGui::OpenPopup("addEntry");

            if (ImGui::BeginPopup("addEntry"))
            {
                if (*selectedEntry > -1 && *selectedEntry < GetEntityTrackCount())
                {
                    for (int i = 0; i < GetComponentTrackCount(*selectedEntry); i++)
                    {
                        if (ImGui::Selectable(GetComponentTrackLabel(*selectedEntry, i).c_str()))
                        {
                            Add(i);
                            //*selectedEntry = sequence->GetEntityTrackCount() - 1;
                        }
                    }
                }

                ImGui::EndPopup();
                popupOpened = true;
            }
        }
    }

    const SEditorKeyframeColorPack CSequencerWindow::GetColorPackFromComponentType(Havtorn::EComponentType componentType) const
    {
        if (!KeyframeColorMap.contains(componentType))
            return SEditorKeyframeColorPack();
        
        return KeyframeColorMap.at(componentType);
    }
    
    SEditorKeyframe* CSequencerWindow::GetSelectedKeyframe()
    {
        if (!SelectedKeyframeMetaData.IsValid(this))
            return nullptr;

        return &GetComponentTracks(SelectedKeyframeMetaData.EntityTrackIndex)[SelectedKeyframeMetaData.ComponentTrackIndex].Keyframes[SelectedKeyframeMetaData.KeyframeIndex];
    }
    
    void CSequencerWindow::SetSelectedKeyframe(Havtorn::U32 entityTrackIndex, Havtorn::U32 componentTrackIndex, Havtorn::U32 keyframeIndex)
    {
        SelectedKeyframeMetaData.EntityTrackIndex = entityTrackIndex;
        SelectedKeyframeMetaData.ComponentTrackIndex = componentTrackIndex;
        SelectedKeyframeMetaData.KeyframeIndex = keyframeIndex;

        // Set playhead at frame index
    }
    
    void CSequencerWindow::ResetSelectedKeyframe()
    {
        SelectedKeyframeMetaData.EntityTrackIndex = -1;
        SelectedKeyframeMetaData.ComponentTrackIndex = -1;
        SelectedKeyframeMetaData.KeyframeIndex = -1;
    }

    void CSequencerWindow::GetBlendRegionInfo(ImGui::CSequencerWindow* window, int entityTrackIndex, int componentTrackIndex, std::vector<std::pair<int, int>>& blendRegions, unsigned int* color)
    {
        SEditorEntityTrack& entityTrack = EntityTracks[entityTrackIndex];
        SEditorComponentTrack& componentTrack = entityTrack.ComponentTracks[componentTrackIndex];

        if (color)
            *color = /*0x00AA8080*/window->GetColorPackFromComponentType(componentTrack.ComponentType).KeyframeBaseColor;

        U64 numberOfKeyframes = componentTrack.Keyframes.size();
        for (Havtorn::U64 keyframeIndex = 0; keyframeIndex < numberOfKeyframes; keyframeIndex++)
        {
            const SEditorKeyframe& keyframe = componentTrack.Keyframes[keyframeIndex];
            if (!keyframe.ShouldBlendRight)
                continue;

            std::pair<int, int> potentialRegion;
            potentialRegion.first = keyframe.FrameNumber;
            for (Havtorn::U64 nextKeyframeIndex = keyframeIndex + 1; nextKeyframeIndex < numberOfKeyframes; nextKeyframeIndex++, keyframeIndex++)
            {           
                const SEditorKeyframe& nextKeyframe = componentTrack.Keyframes[nextKeyframeIndex];
                if (nextKeyframe.ShouldBlendLeft)
                {
                    potentialRegion.second = nextKeyframe.FrameNumber;
                }
                else
                {
                    break;
                }
            }
        
            if (potentialRegion.first < potentialRegion.second)
            {
                blendRegions.push_back(potentialRegion);
            }
        }
    }

    void CSequencerWindow::DrawComponentTracks(class ImGui::CSequencerWindow* sequencerWindow, int index, ImDrawList* drawList, const ImRect& rect, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect, std::vector<SEditorComponentTrack>& componentTracks)
    {
        // ========= LEGEND ========= 
        drawList->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);

        constexpr float componentTrackHeight = 20.0f;
        constexpr float componentTrackIndentation = 30.0f;

        for (int i = 0; i < componentTracks.size(); i++)
        {
            ImVec2 upperLeft(legendRect.Min.x + componentTrackIndentation, legendRect.Min.y + i * componentTrackHeight);
            ImVec2 lowerRight(legendRect.Max.x, legendRect.Min.y + (i + 1) * componentTrackHeight);

            drawList->AddText(upperLeft, 0xFFFFFFFF, Havtorn::GetComponentTypeString(componentTracks[i].ComponentType).c_str());

            // Select component track
            //if (ImRect(upperLeft, lowerRight).Contains(GImGui->IO.MousePos) && ImGui::IsMouseClicked(0))
        }

        drawList->PopClipRect();

        // ========= CONTENT ========= 
        ImGui::SetCursorScreenPos(rect.Min);
        ImVec2 rectMax = rect.Max;
        ImVec2 rectMin = rect.Min;
        ImVec2 size = ImVec2(rectMax.x - rectMin.x, rectMax.y - rectMin.y);
        U32 id = 137 + index;

        // ImGui variables / style setup
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Border, 0);
        ImGui::BeginChildFrame(id, size);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);

        // Temp variables
        // NR: What is -4.5 exactly? horizontal offset from frame line or something. Look at playhead's horizontal offset
        const ImVec2 offset = ImGui::GetCursorScreenPos() + ImVec2(-4.5f, size.y);

        const ImVec2 viewSize(size.x, -size.y);
        ImVec2 min = ImVec2(float(FrameMin), offset.y);
        ImVec2 max = ImVec2(float(FrameMax), offset.y + viewSize.y - 9.0f);
        ImVec2 range = max - min + ImVec2(1.f, 0.f);  // +1 because of inclusive last frame

        //draw_list->AddRectFilled(offset, offset + viewSize, RampEdit.GetBackgroundColor());

        auto pointToRange = [&](ImVec2 pt) { return (pt - min) / range; };
        for (int i2 = 0; i2 < componentTracks.size(); i2++)
        {
            SEditorComponentTrack& componentTrack = componentTracks[i2];
            for (U32 i3 = 0; i3 < componentTrack.Keyframes.size(); i3++)
            {
                U32 keyframe = componentTrack.Keyframes[i3].FrameNumber;
                ImVec2 point = ImVec2(static_cast<Havtorn::F32>(keyframe), legendRect.Min.y + i2 * componentTrackHeight);
                const SEditorKeyframeColorPack colorPack = sequencerWindow->GetColorPackFromComponentType(componentTrack.ComponentType);

                // TODO.NR: Draw selected keyframe highlighted
                // TODO.NR: Don't base selection on DrawKeyframe function
                int returnValue = DrawKeyframe(drawList, pointToRange(point), viewSize, offset, false, colorPack.KeyframeBaseColor, colorPack.KeyframeHighlightColor);
                if (returnValue == 2)
                {
                    sequencerWindow->SetSelectedKeyframe(index, i2, i3);
                }
            }
        }

        draw_list->PopClipRect();

        ImGui::EndChildFrame();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(1);
    }
}
