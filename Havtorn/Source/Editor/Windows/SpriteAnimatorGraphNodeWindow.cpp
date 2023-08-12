// Copyright 2022 Team Havtorn. All Rights Reserved.

// Copyright 2023 Team Havtorn. All Rights Reserved.

#include "SpriteAnimatorGraphNodeWindow.h"
#include "EditorManager.h"
#include "EditorResourceManager.h"

#include "ECS/Components/SpriteAnimatorGraphComponent.h"
#include "ECS/Components/SpriteAnimatorGraphNode.h"
#include "Core/MathTypes/EngineMath.h"

#include <imgui.h>

namespace ImGui
{
	CSpriteAnimatorGraphNodeWindow::CSpriteAnimatorGraphNodeWindow(const char* displayName, Havtorn::CEditorManager* manager)
		: CWindow(displayName, manager, false)
		, Component(nullptr)
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
				for (Havtorn::U32 i = 0; i < animationClip.KeyFrameCount(); i++)
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

					Havtorn::U64 durationIndex = Havtorn::UMath::Min<Havtorn::U64>(i, Component->AnimationClips[Component->CurrentAnimationClipKey].Durations.size() - 1);
					Havtorn::F32 duration = Component->AnimationClips[animationClipKey].Durations[durationIndex];
					if (ImGui::DragFloat("Duration", &duration, 0.01))
					{
						animationClip.Durations[durationIndex] = duration;
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
		if (ImGui::Begin(Name(), &IsEnabled))
		{
			RecursiveTree(&Component->Graph);
		}

		// AS: Want to experiment with creating/saving SpriteAnimationClips
		if (ImGui::Button("Save Asset"))
		{
			Havtorn::CScene* scene = Manager->GetCurrentScene();
			scene;
			//scene->GetEntities()[]
			//Manager->GetCurrentScene()->GetSpriteAnimatorGraphComponents()
			//Manager->GetResourceManager()->CreateAsset(Component)
		}

		ImGui::End();
	}

	void CSpriteAnimatorGraphNodeWindow::Inspect(Havtorn::SSpriteAnimatorGraphComponent& component)
	{
		Component = &component;
		SetEnabled(true);
	}
}