// Copyright 2022 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <MathTypes/Vector.h>

namespace Havtorn
{
	struct SComponentViewResult;
	struct SEditorAssetRepresentation;
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

	private:
		void UpdateTransformGizmo(const SComponentViewResult& result);
		void ViewManipulation(SMatrix& outCameraView, const SVector2<F32>& windowPosition, const SVector2<F32>& windowSize);
		void InspectAssetComponent(const SComponentViewResult& result);
		
		void IterateAssetRepresentations(const SComponentViewResult& result, const std::vector<std::string>& assetNames, const std::vector<std::string>& assetLabels, const std::string& modalNameToOpen, const std::string& defaultSearchDirectory);
		
		void HandleMeshAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep);
		void HandleSkeletonAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep);
		void HandleSkeletalAnimationAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep);

		void HandleMaterialAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep);
		void HandleTextureAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep);
		void HandleScriptAssetPicked(const SComponentViewResult& result, const SEditorAssetRepresentation* assetRep);
		
		void OpenAssetTool(const SComponentViewResult& result);
		
		void OpenAddComponentModal(const SEntity& entity);

	private:
		const std::string SelectMeshAssetModalName = "Select Mesh Asset";
		const std::string SelectMaterialAssetModalName = "Select Material Asset";
		const std::string SelectTextureAssetModalName = "Select Texture Asset";
		const std::string SelectScriptAssetModalName = "Select Script Asset";
		const std::string SelectSkeletonAssetModalName = "Select Skeleton";
		const std::string SelectSkeletonAnimationAssetModalName = "Select Animation";



		const std::string DefaultMeshAssetDirectory = "Assets/Tests";
		const std::string DefaultMaterialAssetDirectory = "Assets/Materials";
		const std::string DefaultTextureAssetDirectory = "Assets/Textures";
		const std::string DefaultScriptAssetDirectory = "Assets/Scripts";

		CScene* Scene = nullptr;
		U8 AssetPickedIndex = 0;
		SMatrix DeltaMatrix = SMatrix::Identity;
	};
}
