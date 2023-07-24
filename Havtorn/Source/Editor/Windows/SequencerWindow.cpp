// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "SequencerWindow.h"
#include "../ImGuizmo/ImGradient.h"
#include <set>

namespace ImGui
{
	CSequencerWindow::CSequencerWindow(const char* name, Havtorn::CEditorManager* manager)
		: CWindow(name, manager)
	{
        Sequencer.FrameMin = -100;
        Sequencer.FrameMax = 100;
        Sequencer.Items.push_back(SSequenceItem{ 0, 10, 30, false });
        Sequencer.Items.push_back(SSequenceItem{ 1, 20, 30, true });
        Sequencer.Items.push_back(SSequenceItem{ 3, 12, 60, false });
        Sequencer.Items.push_back(SSequenceItem{ 2, 61, 90, false });
        Sequencer.Items.push_back(SSequenceItem{ 4, 90, 99, false });
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
            // let's create the sequencer
            static int selectedEntry = -1;
            static int firstFrame = 0;
            static bool expanded = true;
            static int currentFrame = 100;

            ImGui::PushItemWidth(130);
            ImGui::InputInt("Frame Min", &Sequencer.FrameMin);
            ImGui::SameLine();
            ImGui::InputInt("Frame ", &currentFrame);
            ImGui::SameLine();
            ImGui::InputInt("Frame Max", &Sequencer.FrameMax);
            ImGui::PopItemWidth();
            ImSequencer::Sequencer(&Sequencer, &currentFrame, &expanded, &selectedEntry, &firstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
            // add a UI to edit that particular item
            if (selectedEntry != -1)
            {
                const SSequenceItem& item = Sequencer.Items[selectedEntry];
                ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.Type]);
                // switch (type) ....
            }
        }
        ImGui::End();
	}

	void CSequencerWindow::OnDisable()
	{
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
