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
#include <Windows/InspectorWindow.h>

namespace ImGui
{
	CSequencerWindow::CSequencerWindow(const char* displayName, Havtorn::CEditorManager* manager, Havtorn::CSequencerSystem* sequencerSystem)
		: CWindow(displayName, manager)
		, SequencerSystem(sequencerSystem)
	{
		Sequencers.push_back("Intro");
		Sequencers.push_back("BossFight");

		KeyframeColorMap.insert({ Havtorn::EComponentType::TransformComponent, { 0xFF95CAA5, 0xFFCAE4D2 } });
		KeyframeColorMap.insert({ Havtorn::EComponentType::SpriteComponent, { 0xFFCA94A3, 0xFFE4C9D1 } });
		KeyframeColorMap.insert({ Havtorn::EComponentType::SpotLightComponent, { 0xFF03BAFC, 0xFF5FABC7 } });

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
		if (ImGui::Begin(Name(), &IsEnabled))
		{
			FileControls();

			//if (ImGui::BeginCombo("Sequencer", Sequencers[CurrentSequencerIndex].c_str()))
			//{
			//	for (Havtorn::U16 index = 0; index < Sequencers.size(); index++)
			//	{
			//		const std::string& sequencerName = Sequencers[index];

			//		if (ImGui::Selectable(sequencerName.c_str()))
			//		{
			//			// Load Sequencer function
			//			CurrentSequencerIndex = index;
			//			break;
			//		}
			//	}
			//	ImGui::EndCombo();
			//}
			//ImGui::SameLine();
			//ImGui::Text("Scene: ");

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
			SequencerState.IsMenuOpen = false;

			ImGui::InputInt("Current Frame ", &imGuiFrame);
			ImGui::SameLine();
			ImGui::InputInt("Frame Max", &imGuiMaxFrame);
			ImGui::SameLine();
			ImGui::InputInt("Play Rate", &playRate);
			ImGui::SameLine();
			ImGui::Checkbox("Loop", &contextData.IsLooping);

			ContentControls();

			// NR: ClippingRects are used for logic in ContentControls, there's a little bit of a mismatch here with resetting this data and other SequencerState data
			SequencerState.ClippingRects.clear();

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

	void CSequencerWindow::OnDisable()
	{
	}

	void CSequencerWindow::FileControls()
	{
		const char* currentSequencerName = SequencerSystem->GetCurrentSequencerName();

		if (ImGui::Button("Save"))
		{
			// TODO.NR: Save and load from system
			SequencerSystem->SaveCurrentSequencer(std::string("Assets/Sequencers/") + currentSequencerName + std::string(".hva"));
		}
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
		}
		ImGui::SameLine();

		ImGui::Text(currentSequencerName ? currentSequencerName : "");
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
			SequencerState.IsMenuOpen = true;

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
			SequencerState.IsMenuOpen = true;

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

		ResolveSelection();
		CandidateKeyframeMetaData = {};

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

			ImRect clippingRect = SequencerState.ClippingRects["TrackRect"];
			if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::IsMouseHoveringRect(clippingRect.Min, clippingRect.Max))
			{
				float framePixelWidth = 10.f;
				int legendWidth = 200;
				int itemHeight = 20;
				
				ImGuiIO& io = ImGui::GetIO();
				ImVec2 canvasPosition = ImGui::GetCursorScreenPos();
				ImVec2 canvasSize = ImGui::GetContentRegionAvail();

				ImRect topRect(ImVec2(canvasPosition.x + legendWidth, canvasPosition.y), ImVec2(canvasPosition.x + canvasSize.x, canvasPosition.y + itemHeight));
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

			InspectKeyframe();
		}
	}

	void CSequencerWindow::ResolveSelection()
	{
		//if (!SequencerState.IsFocused) 
		//	return;

		if (SequencerState.IsMovingCurrentFrame)
		{
			ResetSelectedKeyframe();
			return;
		}

		//Am i actually dragging a transform position X Slider..?

		ImRect clippingRect = SequencerState.ClippingRects["TrackRect"];
		if (CandidateKeyframeMetaData.IsValid(this) && ImGui::IsMouseClicked(0))
		{
			SetSelectedKeyframe(CandidateKeyframeMetaData.EntityTrackIndex, CandidateKeyframeMetaData.ComponentTrackIndex, CandidateKeyframeMetaData.KeyframeIndex);
			//SetCurrentKeyframeValueOnComponent();
		}
		// Explicit deselect
		else if (ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(clippingRect.Min, clippingRect.Max))
		{
			ResetSelectedKeyframe();
		}
	}

	void CSequencerWindow::SetCurrentComponentValueOnKeyframe()
	{
		if (!SelectedKeyframeMetaData.IsValid(this))
			return;

		Havtorn::CScene* const scene = Manager->GetCurrentScene();
		if (scene == nullptr)
			return;

		const Havtorn::SEntity* entity = Manager->GetSelectedEntity();
		if (entity == nullptr)
			return;

		Havtorn::SSequencerKeyframe* sequencerKeyframe = SequencerSystem->GetKeyframeFromEntityReference(entity->GUID, SelectedKeyframeMetaData.ComponentTrackIndex, SelectedKeyframeMetaData.KeyframeIndex);
		if (sequencerKeyframe == nullptr)
			return;

		sequencerKeyframe->SetEntityDataOnKeyframe(scene, scene->GetSceneIndex(entity->GUID));
	}

	void CSequencerWindow::SetCurrentKeyframeValueOnComponent()
	{
		if (!SelectedKeyframeMetaData.IsValid(this))
			return;

		Havtorn::CScene* const scene = Manager->GetCurrentScene();
		if (scene == nullptr)
			return;

		const Havtorn::SEntity* entity = Manager->GetSelectedEntity();
		if (entity == nullptr)
			return;

		Havtorn::SSequencerKeyframe* sequencerKeyframe = SequencerSystem->GetKeyframeFromEntityReference(entity->GUID, SelectedKeyframeMetaData.ComponentTrackIndex, SelectedKeyframeMetaData.KeyframeIndex);
		if (sequencerKeyframe == nullptr)
			return;

		sequencerKeyframe->SetKeyframeDataOnEntity(scene, scene->GetSceneIndex(entity->GUID));
	}

	void CSequencerWindow::InspectKeyframe()
	{
		if (!SelectedKeyframeMetaData.IsValid(this))
			return;

		Havtorn::CScene* const scene = Manager->GetCurrentScene();
		if (scene == nullptr)
			return;

		Havtorn::SEntity* const selectedEntity = Manager->GetSelectedEntity();
		if (selectedEntity == nullptr)
			return;

		Havtorn::EComponentType componentType = SequencerSystem->GetComponentTrackTypeFromEntityReference(selectedEntity->GUID, SelectedKeyframeMetaData.ComponentTrackIndex);
		if (componentType == Havtorn::EComponentType::Count)
			return;

		Manager->GetEditorWindow<CInspectorWindow>()->TryInspectComponent(componentType);
	}

	void CSequencerWindow::FillSequencer()
	{
		Havtorn::CScene* const scene = Manager->GetCurrentScene();
		if (scene == nullptr)
			return;

		std::vector<Havtorn::SMetaDataComponent>& metaDataComponents = scene->GetMetaDataComponents();

		EntityTracks.clear();

		const std::vector<Havtorn::SSequencerEntityReference>* entityReferences = SequencerSystem->GetCurrentEntityReferences();
		if (entityReferences == nullptr)
			return;

		for (const Havtorn::SSequencerEntityReference& entityReference : *entityReferences)
		{
			const Havtorn::SMetaDataComponent& metaData = metaDataComponents[scene->GetSceneIndex(entityReference.GUID)];

			SEditorEntityTrack entityTrack;
			entityTrack.Name = metaData.Name.AsString();

			for (U64 componentTrackIndex = 0; componentTrackIndex < entityReference.ComponentTracks.size(); componentTrackIndex++)
			{
				// TODO.NR: Algo library?

				const Havtorn::SSequencerComponentTrack& componentTrack = entityReference.ComponentTracks[componentTrackIndex];
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

	int CSequencerWindow::GetFrameMin() const
	{
		return SequencerState.FrameMin;
	}

	int CSequencerWindow::GetFrameMax() const
	{
		return SequencerState.FrameMax;
	}

	int CSequencerWindow::GetEntityTrackCount() const
	{
		return (int)EntityTracks.size();
	}

	int CSequencerWindow::GetComponentTrackCount(int entityTrackIndex) const
	{
		return static_cast<int>(EntityTracks[entityTrackIndex].ComponentTracks.size());
	}

	int CSequencerWindow::GetKeyframeCount(int entityTrackIndex, int componentTrackIndex) const
	{
		return static_cast<int>(EntityTracks[entityTrackIndex].ComponentTracks[componentTrackIndex].Keyframes.size());
	}

	std::string CSequencerWindow::GetComponentTrackLabel(int entityTrackIndex, int componentTrackIndex) const
	{
		return Havtorn::GetComponentTypeString(EntityTracks[entityTrackIndex].ComponentTracks[componentTrackIndex].ComponentType);
	}

	const char* CSequencerWindow::GetEntityTrackLabel(int index) const
	{
		static char tmps[512];
		snprintf(tmps, 512, "[%02d] %s", index, EntityTracks[index].Name.c_str());
		return tmps;
	}

	std::vector<SEditorComponentTrack>& CSequencerWindow::GetComponentTracks(int index)
	{
		return EntityTracks[index].ComponentTracks;
	}

	const char* CSequencerWindow::GetCollapseFmt() const
	{
		return "%d Frames / %d entries";
	}

	void CSequencerWindow::Get(int index, int** start, int** end, int*, unsigned int* color)
	{
		SEditorEntityTrack& item = EntityTracks[index];
		if (color)
			*color = 0xFFAA8080; // same color for everyone, return color based on type
		if (start)
			*start = &item.FrameStart;
		if (end)
			*end = &item.FrameEnd;
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

	void CSequencerWindow::Add(int)
	{
		EntityTracks.push_back(SEditorEntityTrack{ "Player", {{Havtorn::EComponentType::TransformComponent}}, 10, 20, false });
	}

	void CSequencerWindow::Del(int index)
	{
		EntityTracks.erase(EntityTracks.begin() + index);
	}

	void CSequencerWindow::Duplicate(int /*index*/)
	{
	}

	U64 CSequencerWindow::GetCustomHeight(int index)
	{
		constexpr int buffer = 30;
		constexpr int componentTrackLabelHeight = 20;
		return EntityTracks[index].IsExpanded ? (EntityTracks[index].ComponentTracks.size() * componentTrackLabelHeight) + buffer : 0;
	}

	void CSequencerWindow::DoubleClick(int index)
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

	void CSequencerWindow::DrawSequencer(int* currentFrame, bool* expanded, int* selectedEntry, int* firstFrame, int sequenceOptions)
	{
#pragma region Setup
		SequencerState = SSequencerState();

		int entityTrackCount = GetEntityTrackCount();
		int cx = (int)(ImGui::GetIO().MousePos.x);
		int cy = (int)(ImGui::GetIO().MousePos.y);

		ImGui::BeginGroup();

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 canvasPosition = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
		ImVec2 canvasSize = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
		int firstFrameUsed = firstFrame ? *firstFrame : 0;

		int controlHeight = entityTrackCount * SequencerState.ItemHeight;
		for (int i = 0; i < entityTrackCount; i++)
			controlHeight += int(GetCustomHeight(i));
		int frameCount = ImMax(GetFrameMax() - GetFrameMin(), 1);

		std::vector<SEntityTrackDrawInfo> entityTrackDrawInfos;
		// zoom in/out
		const int visibleFrameCount = (int)floorf((canvasSize.x - SequencerState.LegendWidth) / SequencerState.FramePixelWidth);
		const float barWidthRatio = ImMin(visibleFrameCount / (float)frameCount, 1.f);
		const float barWidthInPixels = barWidthRatio * (canvasSize.x - SequencerState.LegendWidth);

		ImRect regionRect(canvasPosition, canvasPosition + canvasSize);
#pragma endregion

		static bool panningView = false;
		static ImVec2 panningViewSource;
		static int panningViewFrame;

		if (expanded && *expanded)
		{
			bool hasScrollBar(true);

			// test scroll area
			ImVec2 headerSize(canvasSize.x, (float)SequencerState.ItemHeight);
			ImVec2 scrollBarSize(canvasSize.x, 14.f);
			ImGui::InvisibleButton("topBar", headerSize);
			drawList->AddRectFilled(canvasPosition, canvasPosition + headerSize, 0xFFFF0000, 0);
			ImVec2 childFramePos = ImGui::GetCursorScreenPos();
			ImVec2 childFrameSize(canvasSize.x, canvasSize.y - 8.f - headerSize.y - (hasScrollBar ? scrollBarSize.y : 0));
			ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
			ImGui::BeginChildFrame(889, childFrameSize);
			SequencerState.IsFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow);
			ImGui::InvisibleButton("contentBar", controlHeight != 0.f ? ImVec2(canvasSize.x, float(controlHeight)) : canvasSize);
			const ImVec2 contentMin = ImGui::GetItemRectMin();
			const ImVec2 contentMax = ImGui::GetItemRectMax();
			const float contentHeight = contentMax.y - contentMin.y;

			// full background
			drawList->AddRectFilled(canvasPosition, canvasPosition + canvasSize, 0xFF242424, 0);

			// current frame top
			ImRect topRect(ImVec2(canvasPosition.x + SequencerState.LegendWidth, canvasPosition.y), ImVec2(canvasPosition.x + canvasSize.x, canvasPosition.y + SequencerState.ItemHeight));

			ScrubPlayhead(sequenceOptions, currentFrame, topRect, frameCount, firstFrameUsed);

			//header
			drawList->AddRectFilled(canvasPosition, ImVec2(canvasSize.x + canvasPosition.x, canvasPosition.y + SequencerState.ItemHeight), 0xFF3D3837, 0);

			//AddEntityTrackButton(sequenceOptions, drawList, canvasPosition, selectedEntry);  

			// ADD DEL BUTTONS KEEP FOR NOW
			//if (sequenceOptions & SEQUENCER_DEL)
			//{
			//    const ImGuiIO& io = ImGui::GetIO();
			//    bool overDel = SequencerAddDelButton(drawList, ImVec2(contentMin.x + SequencerState.LegendWidth - SequencerState.ItemHeight + 2 - 10, tpos.y + 2), false);
			//    if (overDel && io.MouseReleased[0])
			//        SequencerState.DeleteEntry = i;

			//    bool overDup = SequencerAddDelButton(drawList, ImVec2(contentMin.x + SequencerState.LegendWidth - SequencerState.ItemHeight - SequencerState.ItemHeight + 2 - 10, tpos.y + 2), true);
			//    if (overDup && io.MouseReleased[0])
			//        SequencerState.DuplicateEntry = i;
			//}

			//header frame number and lines
			int modFrameCount = 10;
			int frameStep = 1;
			while ((modFrameCount * SequencerState.FramePixelWidth) < 150)
			{
				modFrameCount *= 2;
				frameStep *= 2;
			};
			int halfModFrameCount = modFrameCount / 2;

			auto drawLine = [&](int i, int regionHeight) {
				bool baseIndex = ((i % modFrameCount) == 0) || (i == GetFrameMax() || i == GetFrameMin());
				bool halfIndex = (i % halfModFrameCount) == 0;
				int px = (int)canvasPosition.x + int(i * SequencerState.FramePixelWidth) + SequencerState.LegendWidth - int(firstFrameUsed * SequencerState.FramePixelWidth);
				int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
				int tiretEnd = baseIndex ? regionHeight : SequencerState.ItemHeight;

				if (px <= (canvasSize.x + canvasPosition.x) && px >= (canvasPosition.x + SequencerState.LegendWidth))
				{
					drawList->AddLine(ImVec2((float)px, canvasPosition.y + (float)tiretStart), ImVec2((float)px, canvasPosition.y + (float)tiretEnd - 1), 0xFF606060, 1);

					drawList->AddLine(ImVec2((float)px, canvasPosition.y + (float)SequencerState.ItemHeight), ImVec2((float)px, canvasPosition.y + (float)regionHeight - 1), 0x30606060, 1);
				}

				if (baseIndex && px > (canvasPosition.x + SequencerState.LegendWidth))
				{
					char tmps[512];
					ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", i);
					drawList->AddText(ImVec2((float)px + 3.f, canvasPosition.y), 0xFFBBBBBB, tmps);
				}
			};

			for (int i = GetFrameMin(); i <= GetFrameMax(); i += frameStep)
			{
				drawLine(i, SequencerState.ItemHeight);
			}
			drawLine(GetFrameMin(), SequencerState.ItemHeight);
			drawLine(GetFrameMax(), SequencerState.ItemHeight);

			// clip content
			ImRect mainRect = ImRect(childFramePos, childFramePos + childFrameSize);
			drawList->PushClipRect(childFramePos, childFramePos + childFrameSize);
			SequencerState.ClippingRects.emplace("MainRect", mainRect);

			size_t customHeight = 0;

			// Draw entity names in the legend rect on the left
			TrackHeader(entityTrackCount, contentMin, customHeight, drawList);

			// clipping rect so items bars are not visible in the legend on the left when scrolled
			//

			// Track rect and backgrounds
			customHeight = 0;
			TrackSlotsBackground(entityTrackCount, contentMin, customHeight, canvasSize, canvasPosition, cy, cx, drawList);

			ImRect trackRect = ImRect(childFramePos + ImVec2(float(SequencerState.LegendWidth), 0.f), childFramePos + childFrameSize);
			drawList->PushClipRect(childFramePos + ImVec2(float(SequencerState.LegendWidth), 0.f), childFramePos + childFrameSize);
			SequencerState.ClippingRects.emplace("TrackRect", trackRect);

			auto drawLineContent = [&](int i, int /*regionHeight*/) {
				int px = (int)canvasPosition.x + int(i * SequencerState.FramePixelWidth) + SequencerState.LegendWidth - int(firstFrameUsed * SequencerState.FramePixelWidth);
				int tiretStart = int(contentMin.y);
				int tiretEnd = int(contentMax.y);

				if (px <= (canvasSize.x + canvasPosition.x) && px >= (canvasPosition.x + SequencerState.LegendWidth))
				{
					drawList->AddLine(ImVec2(float(px), float(tiretStart)), ImVec2(float(px), float(tiretEnd)), 0x30606060, 1);
				}
			};

			// Vertical frame lines on tracks
			for (int i = GetFrameMin(); i <= GetFrameMax(); i += frameStep)
			{
				drawLineContent(i, int(contentHeight));
			}
			drawLineContent(GetFrameMin(), int(contentHeight));
			drawLineContent(GetFrameMax(), int(contentHeight));

			// selection
			bool selected = selectedEntry && (*selectedEntry >= 0);
			Selection(selected, customHeight, selectedEntry, drawList, contentMin, canvasSize);

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

						ImVec2 pos = ImVec2(contentMin.x + SequencerState.LegendWidth - firstFrameUsed * SequencerState.FramePixelWidth, contentMin.y + SequencerState.ItemHeight * (entityTrackIndex + 1 + componentTrackIndex) - 7.f + customHeight);
						ImVec2 slotP1(pos.x + Havtorn::UMath::Max(start - 1, 0) * SequencerState.FramePixelWidth, pos.y + 2 + 10.f);
						ImVec2 slotP2(pos.x + end * SequencerState.FramePixelWidth, pos.y + SequencerState.ItemHeight - 2);
						unsigned int slotColor = color - 0x88000000;

						if (slotP1.x <= (canvasSize.x + contentMin.x) && slotP2.x >= (contentMin.x + SequencerState.LegendWidth))
						{
							// Blend region rect
							drawList->AddRectFilled(slotP1, slotP2, slotColor, 2);
						}
						if (ImRect(slotP1, slotP2).Contains(ImGui::GetIO().MousePos) && ImGui::GetIO().MouseDoubleClicked[0])
						{
							DoubleClick(entityTrackIndex);
						}
					}
				}

				int* start, * end;
				unsigned int color;
				Get(entityTrackIndex, &start, &end, NULL, &color);
				size_t localCustomHeight = GetCustomHeight(entityTrackIndex);

				// custom draw
				if (localCustomHeight > 0)
				{
					ImVec2 rp(canvasPosition.x, contentMin.y + SequencerState.ItemHeight * entityTrackIndex + 1 + customHeight);
					ImRect customRect(rp + ImVec2(SequencerState.LegendWidth - (firstFrameUsed - GetFrameMin() - 0.5f) * SequencerState.FramePixelWidth, float(SequencerState.ItemHeight)),
						rp + ImVec2(SequencerState.LegendWidth + (GetFrameMax() - firstFrameUsed - 0.5f + 2.f) * SequencerState.FramePixelWidth, float(localCustomHeight + SequencerState.ItemHeight)));
					ImRect clippingRect(rp + ImVec2(float(SequencerState.LegendWidth), float(SequencerState.ItemHeight)), rp + ImVec2(canvasSize.x, float(localCustomHeight + SequencerState.ItemHeight)));

					ImRect legendRect(rp + ImVec2(0.f, float(SequencerState.ItemHeight)), rp + ImVec2(float(SequencerState.LegendWidth), float(localCustomHeight)));
					ImRect legendClippingRect(canvasPosition, canvasPosition + ImVec2(float(SequencerState.LegendWidth), float(canvasSize.y)));

					std::vector<SEditorComponentTrack>& componentTracks = GetComponentTracks(entityTrackIndex);
					entityTrackDrawInfos.push_back({ entityTrackIndex, customRect, legendRect, clippingRect, legendClippingRect, componentTracks });
				}
				customHeight += localCustomHeight;
			}
#pragma endregion

			// moving
			Moving(cx, selectedEntry);

#pragma region Draw Playhead
			if (currentFrame && firstFrame && *currentFrame >= *firstFrame && *currentFrame <= GetFrameMax())
			{
				float cursorOffset = contentMin.x + SequencerState.LegendWidth + (*currentFrame - firstFrameUsed) * SequencerState.FramePixelWidth + SequencerState.FramePixelWidth * 0.5f - SequencerState.PlayHeadWidth * 0.5f;
				drawList->AddLine(ImVec2(cursorOffset, canvasPosition.y), ImVec2(cursorOffset, contentMax.y), 0xA02A2AFF, SequencerState.PlayHeadWidth);
			}
#pragma endregion

			drawList->PopClipRect();
			drawList->PopClipRect();

			//if (io.MouseClicked[0])
			//    ResetSelectedKeyframe();

			for (const SEntityTrackDrawInfo& entityTrackDrawInfo : entityTrackDrawInfos)
				DrawComponentTracks(drawList, entityTrackDrawInfo);

			ImGui::EndChildFrame();
			ImGui::PopStyleColor();
			Scrollbar(hasScrollBar, scrollBarSize, firstFrameUsed, frameCount, canvasSize, drawList, barWidthInPixels, firstFrame, visibleFrameCount, panningViewSource, panningViewFrame);
		}

		ImGui::EndGroup();

		if (regionRect.Contains(ImGui::GetIO().MousePos))
		{
			bool overCustomDraw = false;
			for (auto& custom : entityTrackDrawInfos)
			{
				if (custom.CustomRect.Contains(ImGui::GetIO().MousePos))
				{
					overCustomDraw = true;
				}
			}
		}

		if (SequencerState.DeleteEntry != -1)
		{
			Del(SequencerState.DeleteEntry);
			if (selectedEntry && (*selectedEntry == SequencerState.DeleteEntry || *selectedEntry >= GetEntityTrackCount()))
				*selectedEntry = -1;
		}

		if (SequencerState.DuplicateEntry != -1)
		{
			Duplicate(SequencerState.DuplicateEntry);
		}
	}

	void CSequencerWindow::Scrollbar(bool hasScrollBar, ImVec2& scrollBarSize, int firstFrameUsed, int frameCount, ImVec2& canvas_size, ImDrawList* draw_list, const float& barWidthInPixels, int* firstFrame, const int& visibleFrameCount, ImVec2& panningViewSource, int& panningViewFrame)
	{
		if (hasScrollBar)
		{
			ImGui::InvisibleButton("scrollBar", scrollBarSize);
			ImVec2 scrollBarMin = ImGui::GetItemRectMin();
			ImVec2 scrollBarMax = ImGui::GetItemRectMax();

			// ratio = number of frames visible in control / number to total frames

			float startFrameOffset = ((float)(firstFrameUsed - GetFrameMin()) / (float)frameCount) * (canvas_size.x - SequencerState.LegendWidth);
			ImVec2 scrollBarA(scrollBarMin.x + SequencerState.LegendWidth, scrollBarMin.y - 2);
			ImVec2 scrollBarB(scrollBarMin.x + canvas_size.x, scrollBarMax.y - 1);
			draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);

			ImRect scrollBarRect(scrollBarA, scrollBarB);
			bool inScrollBar = scrollBarRect.Contains(ImGui::GetIO().MousePos);

			draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF101010, 8);


			ImVec2 scrollBarC(scrollBarMin.x + SequencerState.LegendWidth + startFrameOffset, scrollBarMin.y);
			ImVec2 scrollBarD(scrollBarMin.x + SequencerState.LegendWidth + barWidthInPixels + startFrameOffset, scrollBarMax.y - 2);
			draw_list->AddRectFilled(scrollBarC, scrollBarD, (inScrollBar || SequencerState.IsMovingScrollBar) ? 0xFF606060 : 0xFF505050, 6);

			ImRect barHandleLeft(scrollBarC, ImVec2(scrollBarC.x + 14, scrollBarD.y));
			ImRect barHandleRight(ImVec2(scrollBarD.x - 14, scrollBarC.y), scrollBarD);

			const ImGuiIO& io = ImGui::GetIO();
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
					SequencerState.FramePixelWidthTarget = SequencerState.FramePixelWidth = SequencerState.FramePixelWidth / barRatio;
					int newVisibleFrameCount = int((canvas_size.x - SequencerState.LegendWidth) / SequencerState.FramePixelWidthTarget);
					int lastFrame = *firstFrame + newVisibleFrameCount;
					if (lastFrame > GetFrameMax())
					{
						SequencerState.FramePixelWidthTarget = SequencerState.FramePixelWidth = (canvas_size.x - SequencerState.LegendWidth) / float(GetFrameMax() - *firstFrame);
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
						float previousFramePixelWidthTarget = SequencerState.FramePixelWidthTarget;
						SequencerState.FramePixelWidthTarget = SequencerState.FramePixelWidth = SequencerState.FramePixelWidth / barRatio;
						int newVisibleFrameCount = int(visibleFrameCount / barRatio);
						int newFirstFrame = *firstFrame + newVisibleFrameCount - visibleFrameCount;
						newFirstFrame = ImClamp(newFirstFrame, GetFrameMin(), ImMax(GetFrameMax() - visibleFrameCount, GetFrameMin()));
						if (newFirstFrame == *firstFrame)
						{
							SequencerState.FramePixelWidth = SequencerState.FramePixelWidthTarget = previousFramePixelWidthTarget;
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
				if (SequencerState.IsMovingScrollBar)
				{
					if (!io.MouseDown[0])
					{
						SequencerState.IsMovingScrollBar = false;
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
					if (scrollBarThumb.Contains(io.MousePos) && ImGui::IsMouseClicked(0) && firstFrame && SequencerState.IsMovingCurrentFrame && SequencerState.MovingEntry == -1)
					{
						SequencerState.IsMovingScrollBar = true;
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

	void CSequencerWindow::CopyPaste(int sequenceOptions, const ImVec2& contentMin, ImVec2& canvas_pos, ImDrawList* draw_list)
	{
		// KEEP FOR NOW
		if (sequenceOptions & SEQUENCER_COPYPASTE)
		{
			const ImGuiIO& io = ImGui::GetIO();
			ImRect rectCopy(ImVec2(contentMin.x + 100, canvas_pos.y + 2), ImVec2(contentMin.x + 100 + 30, canvas_pos.y + SequencerState.ItemHeight - 2));
			bool inRectCopy = rectCopy.Contains(io.MousePos);
			unsigned int copyColor = inRectCopy ? 0xFF1080FF : 0xFF000000;
			draw_list->AddText(rectCopy.Min, copyColor, "Copy");

			ImRect rectPaste(ImVec2(contentMin.x + 140, canvas_pos.y + 2)
				, ImVec2(contentMin.x + 140 + 30, canvas_pos.y + SequencerState.ItemHeight - 2));
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

	void CSequencerWindow::Moving(int cx, int* selectedEntry)
	{
		if (SequencerState.MovingEntry >= 0)
		{
			ImGui::CaptureMouseFromApp();
			int diffFrame = int((cx - SequencerState.MovingPos) / SequencerState.FramePixelWidth);
			if (std::abs(diffFrame) > 0)
			{
				int* start, * end;
				Get(SequencerState.MovingEntry, &start, &end, NULL, NULL);
				if (selectedEntry)
					*selectedEntry = SequencerState.MovingEntry;
				int& l = *start;
				int& r = *end;
				if (SequencerState.MovingPart & 1)
					l += diffFrame;
				if (SequencerState.MovingPart & 2)
					r += diffFrame;
				if (l < 0)
				{
					if (SequencerState.MovingPart & 2)
						r -= l;
					l = 0;
				}
				if (SequencerState.MovingPart & 1 && l > r)
					l = r;
				if (SequencerState.MovingPart & 2 && r < l)
					r = l;
				SequencerState.MovingPos += int(diffFrame * SequencerState.FramePixelWidth);
			}
			if (!ImGui::GetIO().MouseDown[0])
			{
				// single select
				if (!diffFrame && SequencerState.MovingPart && selectedEntry)
				{
					*selectedEntry = SequencerState.MovingEntry;
				}

				SequencerState.MovingEntry = -1;
			}
		}
	}

	void CSequencerWindow::Selection(bool selected, size_t& customHeight, int* selectedEntry, ImDrawList* draw_list, const ImVec2& contentMin, ImVec2& canvas_size)
	{
		if (selected)
		{
			customHeight = 0;
			for (int i = 0; i < *selectedEntry; i++)
				customHeight += GetCustomHeight(i);;
			draw_list->AddRectFilled(ImVec2(contentMin.x, contentMin.y + SequencerState.ItemHeight * *selectedEntry + customHeight), ImVec2(contentMin.x + canvas_size.x, contentMin.y + SequencerState.ItemHeight * (*selectedEntry + 1) + customHeight), 0x801080FF, 1.f);
		}
	}

	void CSequencerWindow::TrackSlotsBackground(int numberOfEntityTracks, const ImVec2& contentMin, size_t& customHeight, ImVec2& canvas_size, ImVec2& canvas_pos, int cy, int cx, ImDrawList* draw_list)
	{
		for (int i = 0; i < numberOfEntityTracks; i++)
		{
			unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;

			size_t localCustomHeight = GetCustomHeight(i);
			ImVec2 pos = ImVec2(contentMin.x + SequencerState.LegendWidth, contentMin.y + SequencerState.ItemHeight * i + 1 + customHeight);
			ImVec2 sz = ImVec2(canvas_size.x + canvas_pos.x, pos.y + SequencerState.ItemHeight - 1 + localCustomHeight);
			if (!SequencerState.IsPopupOpen && cy >= pos.y && cy < pos.y + (SequencerState.ItemHeight + localCustomHeight) && SequencerState.MovingEntry == -1 && cx>contentMin.x && cx < contentMin.x + canvas_size.x)
			{
				col += 0x80201008;
				pos.x -= SequencerState.LegendWidth;
			}
			draw_list->AddRectFilled(pos, sz, col, 0);
			customHeight += localCustomHeight;
		}
	}

	void CSequencerWindow::TrackHeader(int numberOfEntityTracks, const ImVec2& contentMin, size_t& customHeight, ImDrawList* draw_list)
	{
		//ImVec2 upperLeft(entityTrackDrawInfo.LegendRect.Min.x + componentTrackIndentation, entityTrackDrawInfo.LegendRect.Min.y + i * componentTrackHeight);

		for (int i = 0; i < numberOfEntityTracks; i++)
		{
			int type;
			Get(i, NULL, NULL, &type, NULL);
			ImVec2 tpos(contentMin.x + 3, contentMin.y + i * SequencerState.ItemHeight + 2 + customHeight);
			draw_list->AddText(tpos, 0xFFFFFFFF, GetEntityTrackLabel(i));
			customHeight += GetCustomHeight(i);
		}
	}

	void CSequencerWindow::ScrubPlayhead(int sequenceOptions, int* currentFrame, ImRect& topRect, int frameCount, int firstFrameUsed)
	{
		const ImGuiIO& io = ImGui::GetIO();
		if (!SequencerState.IsMenuOpen && !SequencerState.IsMovingCurrentFrame && !SequencerState.IsMovingScrollBar && SequencerState.MovingEntry == -1 && sequenceOptions & SEQUENCER_CHANGE_FRAME && currentFrame && *currentFrame >= 0 && topRect.Contains(io.MousePos) && io.MouseDown[0])
		{
			SequencerState.IsMovingCurrentFrame = true;
		}
		if (SequencerState.IsMovingCurrentFrame)
		{
			if (frameCount)
			{
				*currentFrame = (int)((io.MousePos.x - topRect.Min.x) / SequencerState.FramePixelWidth) + firstFrameUsed;
				if (*currentFrame < GetFrameMin())
					*currentFrame = GetFrameMin();
				if (*currentFrame >= GetFrameMax())
					*currentFrame = GetFrameMax();
			}
			if (!io.MouseDown[0])
				SequencerState.IsMovingCurrentFrame = false;
		}
	}

	void CSequencerWindow::AddEntityTrackButton(int sequenceOptions, ImDrawList* draw_list, ImVec2& canvas_pos, int* selectedEntry)
	{
		if (sequenceOptions & SEQUENCER_ADD)
		{
			// Square with one (minus) or two (plus) lines in it
			if (SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + SequencerState.LegendWidth - SequencerState.ItemHeight, canvas_pos.y + 2), true) && ImGui::GetIO().MouseReleased[0])
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
				SequencerState.IsPopupOpen = true;
			}
		}
	}

	const SEditorKeyframeColorPack CSequencerWindow::GetColorPackFromComponentType(Havtorn::EComponentType componentType) const
	{
		if (!KeyframeColorMap.contains(componentType))
			return SEditorKeyframeColorPack();

		return KeyframeColorMap.at(componentType);
	}

	void CSequencerWindow::DrawComponentTracks(ImDrawList* drawList, const SEntityTrackDrawInfo& entityTrackDrawInfo)
	{
		// ========= LEGEND ========= 
		drawList->PushClipRect(entityTrackDrawInfo.LegendClippingRect.Min, entityTrackDrawInfo.LegendClippingRect.Max, true);

		constexpr float componentTrackHeight = 20.0f;
		constexpr float componentTrackIndentation = 30.0f;

		for (int i = 0; i < entityTrackDrawInfo.ComponentTracks.size(); i++)
		{
			ImVec2 upperLeft(entityTrackDrawInfo.LegendRect.Min.x + componentTrackIndentation, entityTrackDrawInfo.LegendRect.Min.y + i * componentTrackHeight);
			ImVec2 lowerRight(entityTrackDrawInfo.LegendRect.Max.x, entityTrackDrawInfo.LegendRect.Min.y + (i + 1) * componentTrackHeight);

			drawList->AddText(upperLeft, 0xFFFFFFFF, Havtorn::GetComponentTypeString(entityTrackDrawInfo.ComponentTracks[i].ComponentType).c_str());

			// Select component track
			//if (ImRect(upperLeft, lowerRight).Contains(GImGui->IO.MousePos) && ImGui::IsMouseClicked(0))
		}

		drawList->PopClipRect();

		// ========= CONTENT ========= 
		ImGui::SetCursorScreenPos(entityTrackDrawInfo.CustomRect.Min);
		ImVec2 rectMax = entityTrackDrawInfo.CustomRect.Max;
		ImVec2 rectMin = entityTrackDrawInfo.CustomRect.Min;
		ImVec2 size = ImVec2(rectMax.x - rectMin.x, rectMax.y - rectMin.y);
		U32 id = 137 + entityTrackDrawInfo.Index; //889

		// ImGui variables / style setup
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Border, 0);
		ImGui::BeginChildFrame(id, size);
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		draw_list->PushClipRect(entityTrackDrawInfo.ClippingRect.Min, entityTrackDrawInfo.ClippingRect.Max, true);

		// Temp variables
		// NR: What is -4.5 exactly? horizontal offset from frame line or something. Look at playhead's horizontal offset
		const ImVec2 offset = ImGui::GetCursorScreenPos() + ImVec2(-4.5f, size.y);

		const ImVec2 viewSize(size.x, -size.y);
		ImVec2 min = ImVec2(float(SequencerState.FrameMin), offset.y);
		ImVec2 max = ImVec2(float(SequencerState.FrameMax), offset.y + viewSize.y - 9.0f);
		ImVec2 range = max - min + ImVec2(1.f, 0.f);  // +1 because of inclusive last frame

		//draw_list->AddRectFilled(offset, offset + viewSize, RampEdit.GetBackgroundColor());

		auto pointToRange = [&](ImVec2 pt) { return (pt - min) / range; };
		for (U32 i2 = 0; i2 < entityTrackDrawInfo.ComponentTracks.size(); i2++)
		{
			const SEditorComponentTrack& componentTrack = entityTrackDrawInfo.ComponentTracks[i2];
			for (U32 i3 = 0; i3 < componentTrack.Keyframes.size(); i3++)
			{
				U32 keyframe = componentTrack.Keyframes[i3].FrameNumber;
				ImVec2 point = ImVec2(static_cast<Havtorn::F32>(keyframe), entityTrackDrawInfo.LegendRect.Min.y + i2 * componentTrackHeight);
				const SEditorKeyframeColorPack colorPack = GetColorPackFromComponentType(componentTrack.ComponentType);

				// TODO.NR: Draw selected keyframe highlighted
				// TODO.NR: Don't base selection on DrawKeyframe function
				int returnValue = DrawKeyframe(drawList, pointToRange(point), viewSize, offset, false, colorPack.KeyframeBaseColor, colorPack.KeyframeHighlightColor);
				if (returnValue > 0)
				{
					CandidateKeyframeMetaData =
					{
						entityTrackDrawInfo.Index,
						static_cast<Havtorn::I32>(i2),
						static_cast<Havtorn::I32>(i3)
					};
					//SetSelectedKeyframe(entityTrackDrawInfo.Index, i2, i3);
				}
			}
		}

		draw_list->PopClipRect();

		ImGui::EndChildFrame();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(1);
	}

	int CSequencerWindow::DrawKeyframe(ImDrawList* draw_list, ImVec2 pos, const ImVec2 size, const ImVec2 offset, bool edited, U32 baseColor, U32 highlightColor)
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

	void CSequencerWindow::AddComponentTrack(Havtorn::EComponentType componentType)
	{
		Havtorn::SEntity* const entity = Manager->GetSelectedEntity();
		if (entity == nullptr)
			return;

		SequencerSystem->AddComponentTrackToEntityReference(entity->GUID, componentType);
	}

	void CSequencerWindow::EditSelectedKeyframe(SEditorKeyframe* selectedKeyframe)
	{
		if (!SelectedKeyframeMetaData.IsValid(this))
			return;

		Havtorn::CScene* const scene = Manager->GetCurrentScene();
		if (scene == nullptr)
			return;

		const Havtorn::SEntity* entity = Manager->GetSelectedEntity();
		if (entity == nullptr)
			return;

		Havtorn::SSequencerKeyframe* sequencerKeyframe = SequencerSystem->GetKeyframeFromEntityReference(entity->GUID, SelectedKeyframeMetaData.ComponentTrackIndex, SelectedKeyframeMetaData.KeyframeIndex);
		if (sequencerKeyframe == nullptr)
			return;

		sequencerKeyframe->FrameNumber = selectedKeyframe->FrameNumber;
		sequencerKeyframe->ShouldBlendLeft = selectedKeyframe->ShouldBlendLeft;
		sequencerKeyframe->ShouldBlendRight = selectedKeyframe->ShouldBlendRight;
			
		SequencerSystem->SortKeyframes(entity->GUID, SelectedKeyframeMetaData.ComponentTrackIndex, SelectedKeyframeMetaData.KeyframeIndex);
	}

	bool CSequencerWindow::SEditorKeyframeMetaData::IsValid(const CSequencerWindow* sequencerWindow) const
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
}
