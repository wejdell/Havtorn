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

struct SEntityTrackDrawInfo
{
    int Index;
    ImRect CustomRect;
    ImRect LegendRect;
    ImRect ClippingRect;
    ImRect LegendClippingRect;
    std::vector<SEditorComponentTrack> ComponentTracks;
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

    private:
        // Master Flow
        void FlowControls(Havtorn::SSequencerContextData& contextData);
        void ContentControls();
        void ResolveSelection();
        void SetCurrentComponentValueOnKeyframe();
        void SetCurrentKeyframeValueOnComponent();
        void FillSequencer();
        // !Master Flow

        // Sequencer Window data
        SEditorKeyframe* GetSelectedKeyframe();
        void SetSelectedKeyframe(Havtorn::U32 entityTrackIndex, Havtorn::U32 componentTrackIndex, Havtorn::U32 keyframeIndex);
        void ResetSelectedKeyframe();
        int GetFrameMin() const;
        int GetFrameMax() const;
        int GetEntityTrackCount() const;
        int GetComponentTrackCount(int entityTrackIndex) const;
        int GetKeyframeCount(int entityTrackIndex, int componentTrackIndex) const;
        std::string GetComponentTrackLabel(int entityTrackIndex, int componentTrackIndex) const;
        const char* GetEntityTrackLabel(int index) const;
        const SEditorKeyframeColorPack GetColorPackFromComponentType(Havtorn::EComponentType componentType) const;
        std::vector<SEditorComponentTrack>& GetComponentTracks(int index);
        const char* GetCollapseFmt() const;
        void Get(int index, int** start, int** end, int* /*type*/, unsigned int* color);
        void GetBlendRegionInfo(ImGui::CSequencerWindow* window, int entityTrackIndex, int componentTrackIndex, std::vector<std::pair<int, int>>& blendRegions, unsigned int* color);
        U64 GetCustomHeight(int index);
        void DoubleClick(int index);
        // !Sequencer Window data

        // Draw Functions
        void DrawSequencer(int* currentFrame, bool* expanded, int* selectedEntry, int* firstFrame, int sequenceOptions);
        void Scrollbar(bool hasScrollBar, ImVec2& scrollBarSize, int firstFrameUsed, int frameCount, ImVec2& canvasSize, ImDrawList* drawList, const float& barWidthInPixels, int* firstFrame, const int& visibleFrameCount, ImVec2& panningViewSource, int& panningViewFrame);
        void CopyPaste(int sequenceOptions, const ImVec2& contentMin, ImVec2& canvasPos, ImDrawList* drawList);
        void Moving(int cx, int* selectedEntry);
        void Selection(bool selected, size_t& customHeight, int* selectedEntry, ImDrawList* drawList, const ImVec2& contentMin, ImVec2& canvasSize);
        void TrackSlotsBackground(int sequenceCount, const ImVec2& contentMin, size_t& customHeight, ImVec2& canvasSize, ImVec2& canvasPos, int cy, int cx, ImDrawList* drawList);
        void TrackHeader(int sequenceCount, const ImVec2& contentMin, size_t& customHeight, ImDrawList* drawList);
        void ScrubPlayhead(int sequenceOptions, int* currentFrame, ImRect& topRect, int frameCount, int firstFrameUsed);
        void AddEntityTrackButton(int sequenceOptions, ImDrawList* drawList, ImVec2& canvasPos, int* selectedEntry);
        void Add(int type);
        void Del(int index);
        void Duplicate(int index);
        void Copy() {}
        void Paste() {}
        void DrawComponentTracks(ImDrawList* drawList, const SEntityTrackDrawInfo& entityTrackDrawInfo);
        int DrawKeyframe(ImDrawList* drawList, ImVec2 pos, const ImVec2 size, const ImVec2 offset, bool edited, U32 baseColor, U32 highlightColor);
        // !Draw Functions

        // SequencerSystem interfacing
        void AddComponentTrack(Havtorn::EComponentType componentType);
        template<typename T>
        void AddKeyframe(Havtorn::EComponentType componentType);
        void EditSelectedKeyframe(SEditorKeyframe* selectedKeyframe);
        // !SequencerSystem interfacing

	private:
        Havtorn::CSequencerSystem* SequencerSystem = nullptr;
        std::vector<SEditorEntityTrack> EntityTracks;
        std::vector<std::string> Sequencers;
        Havtorn::U16 CurrentSequencerIndex = 0;
        std::map<Havtorn::EComponentType, SEditorKeyframeColorPack> KeyframeColorMap;
        std::vector<Havtorn::EComponentType> SupportedComponentTrackTypes;

        struct SSequencerState
        {
            float FramePixelWidth = 10.f; 
            float FramePixelWidthTarget = 10.f;
            float PlayHeadWidth = 4.5;
        
            int FrameMin = 0;
            int FrameMax = 100;
            
            int LegendWidth = 200;
            int ItemHeight = 20;
            
            int MovingEntry = -1;
            int MovingPos = -1;
            int MovingPart = -1;

            int DeleteEntry = -1;
            int DuplicateEntry = -1;

            bool IsFocused = false;
            bool IsMenuOpen = false;
            bool IsPopupOpen = false;
            bool IsMovingScrollBar = false;
            bool IsMovingCurrentFrame = false;

            std::map<std::string, ImRect> ClippingRects;
        } SequencerState;

        struct SEditorKeyframeMetaData
        {
            Havtorn::I32 EntityTrackIndex = -1;
            Havtorn::I32 ComponentTrackIndex = -1;
            Havtorn::I32 KeyframeIndex = -1;
            
            bool IsValid(const CSequencerWindow* sequencerWindow) const;
        } SelectedKeyframeMetaData;

        SEditorKeyframeMetaData CandidateKeyframeMetaData;

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
        SequencerSystem->RecordNewKeyframes(scene, sequencerComponents);
    }
}
