// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include <set>

#include "EditorWindow.h"
#include "Core/ImGuizmo/ImSequencer.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include "Core/ImGuizmo/ImGuizmo.h"

static const char* SequencerItemTypeNames[] = { "Camera","Music", "ScreenEffect", "FadeIn", "Animation" };

using namespace ImSequencer;
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
    
    virtual ImCurveEdit::ECurveType GetCurveType(U64 /*curveIndex*/) const { return ImCurveEdit::CurveSmooth; }
    
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

struct SSequenceItem
{
    int Type;
    int FrameStart, FrameEnd;
    bool IsExpanded;
};

struct SSequencer : public ImSequencer::SequenceInterface
{
    virtual int GetFrameMin() const 
    {
        return FrameMin;
    }

    virtual int GetFrameMax() const 
    {
        return FrameMax;
    }

    virtual int GetItemCount() const { return (int)Items.size(); }

    virtual int GetItemTypeCount() const { return sizeof(SequencerItemTypeNames) / sizeof(char*); }
    virtual const char* GetItemTypeName(int typeIndex) const { return SequencerItemTypeNames[typeIndex]; }
    virtual const char* GetItemLabel(int index) const
    {
        static char tmps[512];
        snprintf(tmps, 512, "[%02d] %s", index, SequencerItemTypeNames[Items[index].Type]);
        return tmps;
    }

    virtual void Get(int index, int** start, int** end, int* type, unsigned int* color)
    {
        SSequenceItem& item = Items[index];
        if (color)
            *color = 0xFFAA8080; // same color for everyone, return color based on type
        if (start)
            *start = &item.FrameStart;
        if (end)
            *end = &item.FrameEnd;
        if (type)
            *type = item.Type;
    }
    virtual void Add(int type) { Items.push_back(SSequenceItem{ type, 0, 10, false }); };
    virtual void Del(int index) { Items.erase(Items.begin() + index); }
    virtual void Duplicate(int index) { Items.push_back(Items[index]); }

    virtual U64 GetCustomHeight(int index) { return Items[index].IsExpanded ? 300 : 0; }

    // Data
    SSequencer() : FrameMin(0), FrameMax(0) {}
    int FrameMin, FrameMax;
    std::vector<SSequenceItem> Items;
    SRampEdit RampEdit;

    virtual void DoubleClick(int index) 
    {
        if (Items[index].IsExpanded)
        {
            Items[index].IsExpanded = false;
            return;
        }

        for (auto& item : Items)
            item.IsExpanded = false;

        Items[index].IsExpanded = !Items[index].IsExpanded;
    }

    virtual void CustomDraw(int index, ImDrawList* drawList, const ImRect& rect, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect)
    {
        static const char* labels[] = { "Translation", "Rotation" , "Scale" };

        RampEdit.MaxValue = ImVec2(float(FrameMax), 1.f);
        RampEdit.MinValue = ImVec2(float(FrameMin), 0.f);
        drawList->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);

        for (int i = 0; i < 3; i++)
        {
            ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
            ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
            drawList->AddText(pta, RampEdit.IsVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);

            if (ImRect(pta, ptb).Contains(GImGui->IO.MousePos) && ImGui::IsMouseClicked(0))
                RampEdit.IsVisible[i] = !RampEdit.IsVisible[i];
        }

        drawList->PopClipRect();

        ImGui::SetCursorScreenPos(rect.Min);
        ImVec2 max = rect.Max;
        ImVec2 min = rect.Min;
        ImVec2 size = ImVec2(max.x - min.x, max.y - min.y);
        ImCurveEdit::Edit(RampEdit, size, 137 + index, &clippingRect);
    }

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
                if (p < Items[index].FrameStart || p > Items[index].FrameEnd)
                    continue;
                float r = (p - FrameMin) / float(FrameMax - FrameMin);
                float x = ImLerp(rect.Min.x, rect.Max.x, r);
                drawList->AddLine(ImVec2(x, rect.Min.y + 6), ImVec2(x, rect.Max.y - 4), 0xAA000000, 4.f);
            }
        }

        drawList->PopClipRect();
    }
};

namespace ImGui
{
	class CSequencerWindow : public CWindow
	{
	public:
		CSequencerWindow(const char* displayName, Havtorn::CEditorManager* manager);
		~CSequencerWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

	private:
        SSequencer Sequencer;
	};
}
