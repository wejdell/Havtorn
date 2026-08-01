// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <MathTypes/Vector.h>
#include <GUI.h>

namespace Havtorn
{
	struct SEditorAssetRepresentation;
	struct STransformComponent;
	struct SComponent;
	struct SAssetReference;
	class CScene;

	template<typename T = F32>
	struct SVector2;

	class CInspectorWindow : public CWindow
	{
	public:
		CInspectorWindow(const char* displayName, CEditorManager* manager);
		~CInspectorWindow() override;
		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		// TODO.NW: See if we can remove the owning scene concept somehow
		void InspectEntity(const SEntity& entity, CScene* owningScene);

		// Viewing functions used in SComponentView::View implementations
		void UpdateTransformGizmo(STransformComponent* viewedTransformComp);
		void InspectAssetComponent(SComponent* viewedComponent, const EAssetType assetType, std::vector<SAssetReference*> assetReferences);
		void OpenAssetTool(SComponent* viewedComponent);
		void RenderPreview(const SComponent* viewedComponent);

	private:
		SVector VertexSnap(const SVector& pivotPosition, const SEntity& viewedEntity);
		SVector GridSnap(const SVector& pivotPosition);
		void ViewManipulation(SMatrix& outCameraView, const SVector2<F32>& windowPosition, const SVector2<F32>& windowSize);

		void AddComponentCombo(const SEntity& entity, CScene* owningScene);

		void UpdateAssetContextMenu();

		void ReassignAssetRef(const U64 assetRequester, const std::vector<SAssetReference*>& references, const U8 index, const std::string& newPath);

	private:
		U8 AssetPickedIndex = 0;
		SMatrix DeltaMatrix = SMatrix::Identity;

		SMatrix FullDeltaMatrix = SMatrix::Identity;
		bool WasUsingGizmo = false;
		bool IsUsingGizmo = false;

		// TODO.NW: When this common one starts to get annoying, we could explore retaining each offset for a list of entities.
		SVector PivotOffset = SVector::Zero; 
		SVector PivotWorldSpace = SVector::Zero;
		SVector InitialTranslation = SVector::Zero;
		SQuaternion InitialRotation = SQuaternion::Identity;
		SVector InitialOffset = SVector::Zero;

		SAssetReference* ContextMenuAssetRef = nullptr;
		U64 ContextMenuAssetRequester = 0;
		bool IsContextMenuRefHovered = false;

		SGuiTextFilter ComponentFilter = SGuiTextFilter();
	};
}
