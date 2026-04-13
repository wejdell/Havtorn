// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"
#include "EditorManager.h"

#include <Assets/AssetReference.h>
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
		void UpdatePreviewMaterial(CScene* scene, const SEditorAssetRepresentation* assetRepresentation);
		void DeliverAssetDrag(CScene* toScene, const SEditorAssetRepresentation* assetRepresentation);

		SEntity GetEntityOnPixel() const;
		SVector4 GetWorldPositionOnPixel() const;
		SVector GetClosestVertexPositionOnPixel(const SEntity& forEntity) const;
		void SetContextMenuEntity(const SEntity& entity);

	private:
		// Returns U64::Max if invalid
		U64 GetEditorDataIndexOnPixel() const;
		void ClearMaterialRefs(const bool reassignLastMaterial);
		std::vector<SMaterialVertex> FindLocalVertices(CScene* scene, const SEntity& entity) const;

	private:
		F32 ViewportMenuHeight = 16.0f;
		SVector2<F32> RenderedSceneDimensions = SVector2<F32>::Zero;
		SVector2<F32> RenderedScenePosition = SVector2<F32>::Zero;

		std::vector<SSnappingOption> SnappingOptions;

		bool IsPlayButtonEngaged = false;
		bool IsPauseButtonEngaged = false;
		SEntity ContextMenuEntity = SEntity::Null;
		SAssetReference* LastMaterialReference = nullptr;
		SAssetReference LastMaterialReferenceValue;
		bool OpenedEntityContextMenu = false;
	};
}
