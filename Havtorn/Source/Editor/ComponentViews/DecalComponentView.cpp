// Copyright 2024 Team Havtorn. All Rights Reserved.

#include "hvpch.h"
#include "DecalComponentView.h"

#include "EditorManager.h"
#include "Windows/InspectorWindow.h"

#include <ECS/Components/DecalComponent.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/ComponentAlgo.h>
#include <Scene/Scene.h>
#include <Engine.h>
#include <Assets/AssetReference.h>
#include <Graphics/Debug/DebugDrawUtility.h>

#include <GUI.h>

namespace Havtorn
{
	void SDecalComponentView::View(const SEntity& entityOwner, CScene* scene) const
	{
		SDecalComponent* decalComp = scene->GetComponent<SDecalComponent>(entityOwner);

		GUI::Checkbox("Render Albedo", decalComp->ShouldRenderAlbedo);
		GUI::Checkbox("Render Material", decalComp->ShouldRenderMaterial);
		GUI::Checkbox("Render Normal", decalComp->ShouldRenderNormal);

		CInspectorWindow* inspector = Manager->GetEditorWindow<CInspectorWindow>();

		STransformComponent* transform = scene->GetComponent<STransformComponent>(entityOwner);
		if (!SComponent::IsValid(transform))
		{
			inspector->InspectAssetComponent(decalComp, EAssetType::Texture, SAssetReference::ConvertToPointers(decalComp->AssetReferences));
			return;
		}

		constexpr F32 decalProjectorScale = 0.5f;
		SVector a = SVector(-decalProjectorScale, -decalProjectorScale, -decalProjectorScale);
		SVector b = SVector(-decalProjectorScale, -decalProjectorScale, decalProjectorScale);
		SVector c = SVector(decalProjectorScale, -decalProjectorScale, decalProjectorScale);
		SVector d = SVector(decalProjectorScale, -decalProjectorScale, -decalProjectorScale);
		SVector e = SVector(-decalProjectorScale, decalProjectorScale, -decalProjectorScale);
		SVector f = SVector(decalProjectorScale, decalProjectorScale, -decalProjectorScale);
		SVector g = SVector(decalProjectorScale, decalProjectorScale, decalProjectorScale);
		SVector h = SVector(-decalProjectorScale, decalProjectorScale, decalProjectorScale);

		SMatrix transformMatrix = transform->Transform.GetMatrix();

		a = (SVector4(a, 1.0f) * transformMatrix).ToVector3();
		b = (SVector4(b, 1.0f) * transformMatrix).ToVector3();
		c = (SVector4(c, 1.0f) * transformMatrix).ToVector3();
		d = (SVector4(d, 1.0f) * transformMatrix).ToVector3();
		e = (SVector4(e, 1.0f) * transformMatrix).ToVector3();
		f = (SVector4(f, 1.0f) * transformMatrix).ToVector3();
		g = (SVector4(g, 1.0f) * transformMatrix).ToVector3();
		h = (SVector4(h, 1.0f) * transformMatrix).ToVector3();

		U64 renderViewID = 0;
		if (CScene* worldScene = UComponentAlgo::GetContainingScene(entityOwner, GEngine::GetWorld()->GetActiveScenes()); worldScene != scene)
		{
			// TODO.NW: Figure out a solution to this hard coded Prefab scene override
			renderViewID = 90100;
		}

		GDebugDraw::AddLine(a, b, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(b, c, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(c, d, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(d, a, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(a, e, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(b, h, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(d, f, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(c, g, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(e, f, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(f, g, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(g, h, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);
		GDebugDraw::AddLine(h, e, SColor::Magenta, -1.0f, false, GDebugDraw::ThicknessMinimum, false, renderViewID);

		inspector->InspectAssetComponent(decalComp, EAssetType::Texture, SAssetReference::ConvertToPointers(decalComp->AssetReferences));
	}
}
