// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"
#include "EditorManager.h"

#include <GUI.h>

namespace Havtorn
{
	class CRenderTexture;
	class CScene;
	struct SEditorAssetRepresentation;

	class CViewportWindow : public CWindow
	{
	public:
		CViewportWindow(const char* displayName, CEditorManager* manager);
		~CViewportWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		const SVector2<F32> GetRenderedSceneDimensions() const;
		const SVector2<F32> GetRenderedScenePosition() const;

		// TODO.NW: Potentially provide render location as well?
		// Returns whether the image was hovered
		bool Render(CScene* assetDragScene, const U64 renderTargetGUID);

		void UpdatePreviewEntity(CScene* scene, const SEditorAssetRepresentation* assetRepresentation);
		void DeliverAssetDrag(CScene* toScene, const SEditorAssetRepresentation* assetRepresentation);

		void OnMouseMove(const SInputAxisPayload payload);

		void SetContextMenuEntity(const SEntity& entity);

	private:
		SVector4 GetWorldPositionOnPixel() const;

	private:
		F32 ViewportMenuHeight = 16.0f;
		SVector2<F32> RenderedSceneDimensions = SVector2<F32>::Zero;
		SVector2<F32> RenderedScenePosition = SVector2<F32>::Zero;
		SVector2<F32> MousePosition = SVector2<F32>::Zero;

		std::vector<SSnappingOption> SnappingOptions;

		bool IsPlayButtonEngaged = false;
		bool IsPauseButtonEngaged = false;
		bool WasPreviewPositionValid = true;
		SVector4 PreviousPreviewPosition = SVector4::Zero;
		SEntity ContextMenuEntity = SEntity::Null;
		bool OpenedEntityContextMenu = false;
	};
}
