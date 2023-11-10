// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once

#include "EditorWindow.h"
#include "EditorManager.h"

#include <Scene/Scene.h>
#include <ECS/Systems/SequencerSystem.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "Core/ImGuizmo/ImGuizmo.h"

#include <set>
#include <map>
#include <string>

using Havtorn::U64;
using Havtorn::U32;

enum SEQUENCER_OPTIONS
{
    SEQUENCER_EDIT_NONE = 0,
    SEQUENCER_EDIT_STARTEND = 1 << 1,
    SEQUENCER_CHANGE_FRAME = 1 << 3,
    SEQUENCER_ADD = 1 << 4,
    SEQUENCER_DEL = 1 << 5,
    SEQUENCER_COPYPASTE = 1 << 6,
    SEQUENCER_EDIT_ALL = SEQUENCER_EDIT_STARTEND | SEQUENCER_CHANGE_FRAME
};

struct SEditorKeyframe
{
    U32 FrameNumber = 0;
    bool ShouldBlendLeft = true;
    bool ShouldBlendRight = true;
};

struct SEditorComponentTrack
{
    Havtorn::EComponentType ComponentType;
    std::vector<SEditorKeyframe> Keyframes = {};
};

struct SEditorEntityTrack
{
    std::string Name;
    std::vector<SEditorComponentTrack> ComponentTracks;
    int FrameStart, FrameEnd;
    bool IsExpanded;
};

struct SEditorKeyframeColorPack
{
    U32 KeyframeBaseColor = 0xFF0080FF; // ARGB, 2 values per channel
    U32 KeyframeHighlightColor = 0xFF80B0FF;
};

namespace ImGui
{
    class CSequencerWindow;
}

namespace Havtorn
{
    class CSequencerSystem;
}

namespace ImGui
{
	class CSequencerWindow : public CWindow
	{
    public:
		CSequencerWindow(const char* displayName, Havtorn::CEditorManager* manager, Havtorn::CSequencerSystem* sequencerSystem);
		~CSequencerWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

        void AddComponentTrack(Havtorn::EComponentType componentType);
        template<typename T>
        void AddKeyframe(Havtorn::EComponentType componentType);
        
        void EditSelectedKeyframe(SEditorKeyframe* selectedKeyframe);

        // TODO.NR: Refactor
        // Draw Functions
        // return true if selection is made
        bool DrawSequencer(int* currentFrame, bool* expanded, int* selectedEntry, int* firstFrame, int sequenceOptions);
        void Scrollbar(bool hasScrollBar, ImVec2& scrollBahvvcrSize, int firstFrameUsed, int frameCount, ImVec2& canvas_size, int legendWidth, ImDrawList* draw_list, ImGuiIO& io, const float& barWidthInPixels, bool& MovingScrollBar, float& framePixelWidthTarget, float& framePixelWidth, int* firstFrame, const int& visibleFrameCount, ImVec2& panningViewSource, int& panningViewFrame, bool MovingCurrentFrame, int movingEntry);
        void CopyPaste(int sequenceOptions, const ImVec2& contentMin, ImVec2& canvas_pos, int ItemHeight, ImGuiIO& io, ImDrawList* draw_list);
        void Moving(int& movingEntry, int cx, int& movingPos, float framePixelWidth, int* selectedEntry, int movingPart, ImGuiIO& io, bool& ret);
        void Selection(bool selected, size_t& customHeight, int* selectedEntry, ImDrawList* draw_list, const ImVec2& contentMin, int ItemHeight, ImVec2& canvas_size);
        void TrackSlotsBackground(int sequenceCount, const ImVec2& contentMin, int legendWidth, int ItemHeight, size_t& customHeight, ImVec2& canvas_size, ImVec2& canvas_pos, bool popupOpened, int cy, int movingEntry, int cx, ImDrawList* draw_list);
        void TrackHeader(int sequenceCount, const ImVec2& contentMin, int ItemHeight, size_t& customHeight, ImDrawList* draw_list, int sequenceOptions, int legendWidth, ImGuiIO& io, int& delEntry, int& dupEntry);
        void ChangeCurrentFrame(bool& MovingCurrentFrame, bool MovingScrollBar, int movingEntry, int sequenceOptions, int* currentFrame, ImRect& topRect, ImGuiIO& io, int frameCount, float framePixelWidth, int firstFrameUsed);
        void Panning(ImGuiIO& io, bool& panningView, ImVec2& panningViewSource, int& panningViewFrame, int* firstFrame, float& framePixelWidth, const int& visibleFrameCount, float& framePixelWidthTarget, int& frameCount);
        void NotExpanded(ImVec2& canvas_size, ImVec2& canvas_pos, int ItemHeight, ImDrawList* draw_list, int frameCount, int sequenceCount);
        void AddEntityTrackButton(int /*sequenceOptions*/, ImDrawList* /*draw_list*/, ImVec2& /*canvas_pos*/, int /*legendWidth*/, int /*ItemHeight*/, ImGuiIO& /*io*/, int* /*selectedEntry*/, bool& /*popupOpened*/);
        // Draw Functions

        const SEditorKeyframeColorPack GetColorPackFromComponentType(Havtorn::EComponentType componentType) const;

        SEditorKeyframe* GetSelectedKeyframe();
        void SetSelectedKeyframe(Havtorn::U32 entityTrackIndex, Havtorn::U32 componentTrackIndex, Havtorn::U32 keyframeIndex);
        void ResetSelectedKeyframe();

        virtual int GetFrameMin() const
        {
            return FrameMin;
        }

        virtual int GetFrameMax() const
        {
            return FrameMax;
        }

        virtual int GetEntityTrackCount() const { return (int)EntityTracks.size(); }

        virtual void BeginEdit(int /*index*/) {}
        virtual void EndEdit() {}
        virtual int GetComponentTrackCount(int entityTrackIndex) const { return static_cast<int>(EntityTracks[entityTrackIndex].ComponentTracks.size()); }
        virtual int GetKeyframeCount(int entityTrackIndex, int componentTrackIndex) const { return static_cast<int>(EntityTracks[entityTrackIndex].ComponentTracks[componentTrackIndex].Keyframes.size()); }

        virtual std::string GetComponentTrackLabel(int entityTrackIndex, int componentTrackIndex) const { return Havtorn::GetComponentTypeString(EntityTracks[entityTrackIndex].ComponentTracks[componentTrackIndex].ComponentType); }
        virtual const char* GetEntityTrackLabel(int index) const
        {
            static char tmps[512];
            snprintf(tmps, 512, "[%02d] %s", index, EntityTracks[index].Name.c_str());
            return tmps;
        }
        virtual std::vector<SEditorComponentTrack>& GetComponentTracks(int index) { return EntityTracks[index].ComponentTracks; }
        virtual const char* GetCollapseFmt() const { return "%d Frames / %d entries"; }

        virtual void Get(int index, int** start, int** end, int* /*type*/, unsigned int* color)
        {
            SEditorEntityTrack& item = EntityTracks[index];
            if (color)
                *color = 0xFFAA8080; // same color for everyone, return color based on type
            if (start)
                *start = &item.FrameStart;
            if (end)
                *end = &item.FrameEnd;
        }

        virtual void GetBlendRegionInfo(ImGui::CSequencerWindow* window, int entityTrackIndex, int componentTrackIndex, std::vector<std::pair<int, int>>& blendRegions, unsigned int* color);

        virtual void Add(int /*type*/) { EntityTracks.push_back(SEditorEntityTrack{ "Player", {{Havtorn::EComponentType::TransformComponent}}, 10, 20, false }); };
        virtual void Del(int index) { EntityTracks.erase(EntityTracks.begin() + index); }
        virtual void Duplicate(int index) { EntityTracks.push_back(EntityTracks[index]); }

        virtual void Copy() {}
        virtual void Paste() {}

        virtual U64 GetCustomHeight(int index)
        {
            constexpr int buffer = 30;
            constexpr int componentTrackLabelHeight = 20;
            return EntityTracks[index].IsExpanded ? (EntityTracks[index].ComponentTracks.size() * componentTrackLabelHeight) + buffer : 0;
        }

        virtual void DoubleClick(int index)
        {
            if (EntityTracks[index].IsExpanded)
            {
                EntityTracks[index].IsExpanded = false;
                return;
            }

            for (auto& item : EntityTracks)
                item.IsExpanded = false;

            EntityTracks[index].IsExpanded = !EntityTracks[index].IsExpanded;
        }

        virtual void DrawComponentTracks(ImGui::CSequencerWindow* sequencerWindow, int index, ImDrawList* drawList, const ImRect& rect, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect, std::vector<SEditorComponentTrack>& componentTracks);

        static int DrawKeyframe(ImDrawList* draw_list, ImVec2 pos, const ImVec2 size, const ImVec2 offset, bool edited, U32 baseColor, U32 highlightColor)
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
            // ABGR // base 0xFF0080FF // highlight 0xFF80B0FF
            if (edited)
                draw_list->AddPolyline(offsets, 4, 0xFFFFFFFF, true, 3.0f);
            else if (ret)
                draw_list->AddPolyline(offsets, 4, highlightColor, true, 2.0f);
            else
                draw_list->AddPolyline(offsets, 4, baseColor, true, 2.0f);

            return ret;
        }

    private:
        void FlowControls(Havtorn::SSequencerContextData& contextData);
        void ContentControls();
        void SetCurrentComponentValueOnKeyframe();
        void SetCurrentKeyframeValueOnComponent();
        void FillSequencer();

	private:
        Havtorn::CSequencerSystem* SequencerSystem = nullptr;
        std::vector<std::string> Sequencers;
        Havtorn::U16 CurrentSequencerIndex = 0;
        std::map<Havtorn::EComponentType, SEditorKeyframeColorPack> KeyframeColorMap;
        std::vector<Havtorn::EComponentType> SupportedComponentTrackTypes;

        int FrameMin = 0, FrameMax = 0;
        bool IsFocused = false;
        std::vector<SEditorEntityTrack> EntityTracks;

        bool IsMenuOpen = false;

        struct SEditorKeyframeMetaData
        {
            Havtorn::I32 EntityTrackIndex = -1;
            Havtorn::I32 ComponentTrackIndex = -1;
            Havtorn::I32 KeyframeIndex = -1;

            bool IsValid(const CSequencerWindow* sequencerWindow) const
            { 
                using Math = Havtorn::UMath;
                if (!Math::IsWithin(EntityTrackIndex, 0, sequencerWindow->GetEntityTrackCount()))
                    return false;

                if (!Math::IsWithin(ComponentTrackIndex, 0, sequencerWindow->GetComponentTrackCount(EntityTrackIndex)))
                    return false;

                if (!Math::IsWithin(KeyframeIndex, 0, sequencerWindow->GetKeyframeCount(EntityTrackIndex, ComponentTrackIndex)))
                    return false;

                return true;
            }
        } SelectedKeyframeMetaData;

        std::string NewComponentTrackPopupName = "AddNewComponentTrackPopup";
        std::string NewKeyframePopupName = "AddNewKeyframePopup";
	};
    
    template<typename T>
    inline void CSequencerWindow::AddKeyframe(Havtorn::EComponentType componentType)
    {
        Havtorn::SEntity* entity = Manager->GetSelectedEntity();
        Havtorn::CScene* scene = Manager->GetCurrentScene();

        if (entity == nullptr || scene == nullptr)
            return;

        U64 sceneIndex = scene->GetSceneIndex(entity->GUID);
        std::vector<Havtorn::SSequencerComponent>& sequencerComponents = scene->GetSequencerComponents();

        if (sceneIndex >= sequencerComponents.size())
            return;

        SequencerSystem->AddEmptyKeyframeToComponent<T>(sequencerComponents[sceneIndex], componentType);
    }
}
