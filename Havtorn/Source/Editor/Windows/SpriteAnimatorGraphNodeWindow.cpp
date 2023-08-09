// Copyright 2022 Team Havtorn. All Rights Reserved.

#include <imgui.h>
#include "SpriteAnimatorGraphNodeWindow.h"
#include "EditorManager.h"

#include "ECS/Components/SpriteAnimatorGraphComponent.h"
#include "ECS/Components/SpriteAnimatorGraphNode.h"

namespace ImGui
{
	CSpriteAnimatorGraphNodeWindow::CSpriteAnimatorGraphNodeWindow(const char* displayName, Havtorn::CEditorManager* manager)
		: CWindow(displayName, manager, false)
		, Component(nullptr)
	{ 
	}

	CSpriteAnimatorGraphNodeWindow::~CSpriteAnimatorGraphNodeWindow()
	{ 
	}

	void CSpriteAnimatorGraphNodeWindow::OnEnable()
	{ 
	}

	void CSpriteAnimatorGraphNodeWindow::OnDisable()
	{ 
	}

	void CSpriteAnimatorGraphNodeWindow::RecursiveTree(Havtorn::SSpriteAnimatorGraphNode* node)
	{
		if (ImGui::TreeNode(node->Name.Data()))
		{
			if (node->AnimationClipKey == -1)
			{
				for (auto& child : node->Nodes)
					RecursiveTree(&child);
			}

			if (node->AnimationClipKey >= 0 && node->AnimationClipKey < Component->AnimationClips.size())
			{
				Havtorn::I16 animationClipKey = node->AnimationClipKey;
				Havtorn::SSpriteAnimationClip& animationClip = Component->AnimationClips[animationClipKey];

				ImGui::Text("Animation Clip Settings");
				ImGui::PushID(node->Name.Data());
				for (Havtorn::U32 i = 0; i < animationClip.UVRects.size(); i++)
				{
					ImGui::PushID(i);

					Havtorn::SVector4& rect = Component->AnimationClips[animationClipKey].UVRects[i];
					Havtorn::F32 uvRect[4] = { rect.X, rect.Y, rect.Z, rect.W };
					if (ImGui::DragFloat4("UVRect", uvRect, 0.01))
					{
						animationClip.UVRects[i].X = uvRect[0];
						animationClip.UVRects[i].Y = uvRect[1];
						animationClip.UVRects[i].Z = uvRect[2];
						animationClip.UVRects[i].W = uvRect[3];
					}

					Havtorn::F32 duration = Component->AnimationClips[animationClipKey].Durations[i];
					if (ImGui::DragFloat("Duration", &duration, 0.01))
					{
						animationClip.Durations[i] = duration;
					}

					ImGui::PopID();
				}
				ImGui::PopID();
			}

			ImGui::TreePop();
		}
	}

	void CSpriteAnimatorGraphNodeWindow::OnInspectorGUI()
	{
		if (ImGui::Begin(Name(), Open()))
		{
			RecursiveTree(&Component->Graph);
		}
		ImGui::End();
	}

	void CSpriteAnimatorGraphNodeWindow::Inspect(Havtorn::SSpriteAnimatorGraphComponent& component)
	{
		Component = &component;
		Enable(true);
	}
}