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

namespace ImCurveEdit
{
    enum ECurveType
    {
        CurveNone,
        CurveDiscrete,
        CurveLinear,
        CurveSmooth,
        CurveBezier,
    };

    struct SSequencerEditPoint
    {
        U64 CurveIndex;
        U32 PointIndex;
        
        bool operator<(const SSequencerEditPoint& other) const
        {
            if (CurveIndex < other.CurveIndex)
                return true;
            if (CurveIndex > other.CurveIndex)
                return false;

            if (PointIndex < other.PointIndex)
                return true;

            return false;
        }
    };

    struct SSequencerDelegate
    {
        bool IsFocused = false;
        virtual U64 GetCurveCount() = 0;
        virtual bool GetIsVisible(U64 /*curveIndex*/) { return true; }
        virtual ECurveType GetCurveType(U64 /*curveIndex*/) const { return CurveLinear; }
        virtual ImVec2& GetMin() = 0;
        virtual ImVec2& GetMax() = 0;
        virtual ImVec2 PointToRange(const ImVec2& point, const ImVec2& range) = 0;
        virtual ImVec2 RangeToPoint(const ImVec2& point, const ImVec2& range) = 0;
        virtual U64 GetPointCount(U64 curveIndex) = 0;
        virtual U32 GetCurveColor(U64 curveIndex) = 0;
        virtual ImVec2* GetPoints(U64 curveIndex) = 0;
        virtual U32 EditPoint(U64 curveIndex, U32 pointIndex, ImVec2 value) = 0;
        virtual void AddPoint(U64 curveIndex, ImVec2 value) = 0;
        virtual U32 GetBackgroundColor() { return 0xFF202020; }
        
        // TODO.NR: Handle undo/redo through these functions
        virtual void BeginEdit(U32 /*index*/) {}
        virtual void EndEdit() {}
    };

    U32 Edit(SSequencerDelegate& delegate, const ImVec2& size, U32 id, const ImRect* clippingRect = NULL, ImVector<SSequencerEditPoint>* selectedPoints = NULL);

    void CheckIsOverSelectedPoint(bool& overSelectedPoint, ImGuiIO& io, bool& pointsMoved, ImCurveEdit::SSequencerDelegate& delegate);

    void UpdateSelectedPoints(ImVector<ImCurveEdit::SSequencerEditPoint>* selectedPoints, std::set<ImCurveEdit::SSequencerEditPoint>& selection);

    void QuadSelection(bool& selectingQuad, ImVec2& quadSelection, ImGuiIO& io, ImDrawList* draw_list, std::set<ImCurveEdit::SSequencerEditPoint>& selection, const size_t& curveCount, ImCurveEdit::SSequencerDelegate& delegate, ImVec2& range, const ImVec2& viewSize, const ImVec2& offset, bool overCurveOrPoint, int movingCurveIndex, bool overSelectedPoint, const ImRect& container);

    void AddPoint(int overCurveIndex, ImGuiIO& io, ImCurveEdit::SSequencerDelegate& delegate, const ImVec2& offset, const ImVec2& viewSize, ImVec2& range, int& returnValue);

    void MoveCurve(int& movingCurveIndex, ImCurveEdit::SSequencerDelegate& delegate, bool& pointsMoved, ImVec2& mousePosOrigin, ImGuiIO& io, std::vector<ImVec2>& originalPoints, ImVec2& range, const ImVec2& sizeOfPixel, int& returnValue, int overCurveIndex, std::set<ImCurveEdit::SSequencerEditPoint>& selection, bool selectingQuad);

    void MoveSelection(bool overSelectedPoint, ImGuiIO& io, std::set<ImCurveEdit::SSequencerEditPoint>& selection, bool& pointsMoved, ImCurveEdit::SSequencerDelegate& delegate, ImVec2& mousePosOrigin, std::vector<ImVec2>& originalPoints, int& ret, const ImVec2& range, const ImVec2& sizeOfPixel);

    void LoopOverCurves(const size_t& curveCount, int* curvesIndex, ImCurveEdit::SSequencerDelegate& delegate, int highLightedCurveIndex, std::set<ImCurveEdit::SSequencerEditPoint>& selection, bool selectingQuad, int movingCurve, const ImVec2& range, ImVec2& min, const ImVec2& viewSize, const ImVec2& offset, ImGuiIO& io, bool scrollingV, int& localOverCurve, int& overCurve, bool& overCurveOrPoint, ImDrawList* draw_list, bool& overSelectedPoint);

    void HandleZoomAndVScroll(const ImRect& container, const ImGuiIO& io, const ImVec2& offset, const ImVec2& ssize, ImVec2& inOutMin, ImVec2& inOutMax, bool& outScrollingV);
}

struct SRampEdit : public ImCurveEdit::SSequencerDelegate
{
    SRampEdit()
    {
        Points[0][0] = ImVec2(-10.f, 0);
        Points[0][1] = ImVec2(20.f, 0.6f);
        Points[0][2] = ImVec2(25.f, 0.2f);
        Points[0][3] = ImVec2(70.f, 0.4f);
        Points[0][4] = ImVec2(120.f, 1.f);
        PointCount[0] = 5;

        Points[1][0] = ImVec2(-50.f, 0.2f);
        Points[1][1] = ImVec2(33.f, 0.7f);
        Points[1][2] = ImVec2(80.f, 0.2f);
        Points[1][3] = ImVec2(82.f, 0.8f);
        PointCount[1] = 4;

        Points[2][0] = ImVec2(40.f, 0);
        Points[2][1] = ImVec2(60.f, 0.1f);
        Points[2][2] = ImVec2(90.f, 0.82f);
        Points[2][3] = ImVec2(150.f, 0.24f);
        Points[2][4] = ImVec2(200.f, 0.34f);
        Points[2][5] = ImVec2(250.f, 0.12f);
        PointCount[2] = 6;

        IsVisible[0] = IsVisible[1] = IsVisible[2] = true;
        MaxValue = ImVec2(1.f, 1.f);
        MinValue = ImVec2(0.f, 0.f);
    }

    U64 GetCurveCount()
    {
        return 3;
    }

    bool GetIsVisible(U64 curveIndex)
    {
        return IsVisible[curveIndex];
    }

    U64 GetPointCount(U64 curveIndex)
    {
        return PointCount[curveIndex];
    }

    U32 GetCurveColor(U64 curveIndex)
    {
        U32 cols[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 };
        return cols[curveIndex];
    }

    ImVec2* GetPoints(U64 curveIndex)
    {
        return Points[curveIndex];
    }
    
    virtual ImCurveEdit::ECurveType GetCurveType(U64 /*curveIndex*/) const { return ImCurveEdit::CurveDiscrete; }
    
    virtual U32 EditPoint(U64 curveIndex, U32 pointIndex, ImVec2 value)
    {
        Points[curveIndex][pointIndex] = ImVec2(value.x, value.y);

        SortValues(curveIndex);

        for (U64 i = 0; i < GetPointCount(curveIndex); i++)
        {
            if (Points[curveIndex][i].x == value.x)
                return static_cast<U32>(i);
        }

        return pointIndex;
    }

    virtual void AddPoint(U64 curveIndex, ImVec2 value)
    {
        if (PointCount[curveIndex] >= 8)
            return;

        Points[curveIndex][PointCount[curveIndex]++] = value;
        SortValues(curveIndex);
    }

    virtual ImVec2& GetMax() { return MaxValue; }
    virtual ImVec2& GetMin() { return MinValue; }

    virtual ImVec2 PointToRange(const ImVec2& point, const ImVec2& range) { return (point - MinValue) / range; }
    virtual ImVec2 RangeToPoint(const ImVec2& point, const ImVec2& range) { return (point * range) + MinValue; }

    virtual U32 GetBackgroundColor() { return 0; }

    ImVec2 Points[3][8];
    U64 PointCount[3];
    bool IsVisible[3];
    ImVec2 MinValue;
    ImVec2 MaxValue;

private:

    void SortValues(U64 curveIndex)
    {
        auto b = std::begin(Points[curveIndex]);
        auto e = std::begin(Points[curveIndex]) + GetPointCount(curveIndex);
        std::sort(b, e, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
    }
};


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

struct SEditorComponentTrack
{
    Havtorn::EComponentType ComponentType;
    std::vector<U32> Keyframes = {};
};

struct SEditorEntityTrack
{
    std::string Name;
    std::vector<SEditorComponentTrack> ComponentTracks;
    int FrameStart, FrameEnd;
    bool IsExpanded;
};

struct SSequencer
{
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
    virtual std::string GetComponentTrackLabel(int entityTrackIndex, int componentTrackIndex) const { return Havtorn::GetComponentTypeString(EntityTracks[entityTrackIndex].ComponentTracks[componentTrackIndex].ComponentType); }
    virtual const char* GetEntityTrackLabel(int index) const
    {
        static char tmps[512];
        snprintf(tmps, 512, "[%02d] %s", index, EntityTracks[index].Name.c_str());
        return tmps;
    }
    virtual const std::vector<SEditorComponentTrack>& GetComponentTracks(int index) { return EntityTracks[index].ComponentTracks; }
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
    virtual void Add(int /*type*/) { EntityTracks.push_back(SEditorEntityTrack{ "Player", {{Havtorn::EComponentType::TransformComponent}}, 10, 20, false }); };
    virtual void Del(int index) { EntityTracks.erase(EntityTracks.begin() + index); }
    virtual void Duplicate(int index) { EntityTracks.push_back(EntityTracks[index]); }

    virtual void Copy() {}
    virtual void Paste() {}

    virtual U64 GetCustomHeight(int index) { return EntityTracks[index].IsExpanded ? 300 : 0; }

    // Data
    SSequencer() : FrameMin(0), FrameMax(0) {}
    int FrameMin, FrameMax;
    bool IsFocused = false;
    std::vector<SEditorEntityTrack> EntityTracks;
    SRampEdit RampEdit;

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

    virtual void CustomDraw(int index, ImDrawList* drawList, const ImRect& rect, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect, const std::vector<SEditorComponentTrack>& componentTracks);

    virtual void CustomDrawCompact(int index, ImDrawList* drawList, const ImRect& rect, const ImRect& clippingRect)
    {
        RampEdit.MaxValue = ImVec2(float(FrameMax), 1.f);
        RampEdit.MinValue = ImVec2(float(FrameMin), 0.f);
        drawList->PushClipRect(clippingRect.Min, clippingRect.Max, true);

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < RampEdit.PointCount[i]; j++)
            {
                float p = RampEdit.Points[i][j].x;
                if (p < EntityTracks[index].FrameStart || p > EntityTracks[index].FrameEnd)
                    continue;
                float r = (p - FrameMin) / float(FrameMax - FrameMin);
                float x = ImLerp(rect.Min.x, rect.Max.x, r);
                drawList->AddLine(ImVec2(x, rect.Min.y + 6), ImVec2(x, rect.Max.y - 4), 0xAA000000, 4.f);
            }
        }

        drawList->PopClipRect();
    }

    static int DrawKeyframe(ImDrawList* draw_list, ImVec2 pos, const ImVec2 size, const ImVec2 offset, bool edited)
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

};

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
        void FlowControls(Havtorn::SSequencerContextData& contextData);
        void FillSequencer();
		void OnDisable() override;

        void AddComponentTrack(Havtorn::EComponentType componentType);
        template<typename T>
        void AddKeyframe(Havtorn::EComponentType componentType);

        void AddSequencerItem(SEditorEntityTrack item);

        // Draw Functions

        // return true if selection is made
        bool DrawSequencer(SSequencer* sequence, int* currentFrame, bool* expanded, int* selectedEntry, int* firstFrame, int sequenceOptions);

        void Scrollbar(bool hasScrollBar, ImVec2& scrollBarSize, int firstFrameUsed, SSequencer* sequence, int frameCount, ImVec2& canvas_size, int legendWidth, ImDrawList* draw_list, ImGuiIO& io, const float& barWidthInPixels, bool& MovingScrollBar, float& framePixelWidthTarget, float& framePixelWidth, int* firstFrame, const int& visibleFrameCount, ImVec2& panningViewSource, int& panningViewFrame, bool MovingCurrentFrame, int movingEntry);

        void CopyPaste(int sequenceOptions, const ImVec2& contentMin, ImVec2& canvas_pos, int ItemHeight, ImGuiIO& io, ImDrawList* draw_list, SSequencer* sequence);

        void Moving(int& movingEntry, int cx, int& movingPos, float framePixelWidth, SSequencer* sequence, int* selectedEntry, int movingPart, ImGuiIO& io, bool& ret);

        void Selection(bool selected, size_t& customHeight, int* selectedEntry, SSequencer* sequence, ImDrawList* draw_list, const ImVec2& contentMin, int ItemHeight, ImVec2& canvas_size);

        void TrackSlotsBackground(int sequenceCount, SSequencer* sequence, const ImVec2& contentMin, int legendWidth, int ItemHeight, size_t& customHeight, ImVec2& canvas_size, ImVec2& canvas_pos, bool popupOpened, int cy, int movingEntry, int cx, ImDrawList* draw_list);

        void TrackHeader(int sequenceCount, SSequencer* sequence, const ImVec2& contentMin, int ItemHeight, size_t& customHeight, ImDrawList* draw_list, int sequenceOptions, int legendWidth, ImGuiIO& io, int& delEntry, int& dupEntry);

        void ChangeCurrentFrame(bool& MovingCurrentFrame, bool MovingScrollBar, int movingEntry, int sequenceOptions, int* currentFrame, ImRect& topRect, ImGuiIO& io, int frameCount, float framePixelWidth, int firstFrameUsed, SSequencer* sequence);

        void Panning(ImGuiIO& io, bool& panningView, ImVec2& panningViewSource, int& panningViewFrame, int* firstFrame, float& framePixelWidth, SSequencer* sequence, const int& visibleFrameCount, float& framePixelWidthTarget, int& frameCount);

        void NotExpanded(ImVec2& canvas_size, ImVec2& canvas_pos, int ItemHeight, ImDrawList* draw_list, SSequencer* sequence, int frameCount, int sequenceCount);

        void AddTrackButton(int /*sequenceOptions*/, ImDrawList* /*draw_list*/, ImVec2& /*canvas_pos*/, int /*legendWidth*/, int /*ItemHeight*/, ImGuiIO& /*io*/, SSequencer* /*sequence*/, int* /*selectedEntry*/, bool& /*popupOpened*/);
        // Draw Functions

	private:
        Havtorn::CSequencerSystem* SequencerSystem = nullptr;
        SSequencer Sequencer;
        std::vector<std::string> Sequencers;
        Havtorn::U16 CurrentSequencerIndex = 0;
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
