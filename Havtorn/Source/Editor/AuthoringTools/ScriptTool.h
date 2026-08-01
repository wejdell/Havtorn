// Copyright 2025 Team Havtorn. All Rights Reserved.

#pragma once
#include "EditorWindow.h"

#include <Assets/AssetReference.h>
#include <Assets/RuntimeAssets/ScriptAsset.h>

#include <HexRune/Pin.h>

namespace Havtorn
{
	struct SNodeView;

	namespace HexRune
	{
		struct SScript;
		struct SNode;
	}

	struct SDataBindingInitData
	{
		CHavtornStaticString<255> Name;
		HexRune::EPinType Type = HexRune::EPinType::Bool;
		HexRune::EObjectDataType ObjectType = HexRune::EObjectDataType::None;
		EAssetType AssetType = EAssetType::None;
		U8 CurrentPinTypeIndex = 0;
		U8 CurrentObjectTypeIndex = 0;
	};

	struct SNodeOperation
	{
		std::optional<SDataBindingInitData> NewBinding = {};
		U64 RemovedBindingID = 0;
		HexRune::SPin* ModifiedLiteralValuePin = nullptr;
		SNodeView* NewNodeView = nullptr;
		SVector2<F32> NewNodePosition = SVector2<F32>::Zero;
		HexRune::SLink NewLink = {};
		std::vector<U64> RemovedNodes = {};
		std::vector<HexRune::SLink> RemovedLinks = {};
	};

	using namespace HexRune;

	class CScriptTool : public CWindow
	{
	public:
		CScriptTool(const char* displayName, CEditorManager* manager);
		~CScriptTool() override = default;

		void OnEnable() override;
		void OnInspectorGUI() override;
		void OnDisable() override;

		void OpenScript(SEditorAssetRepresentation* asset);
		void SaveScript();
		void CloseScript();

	private:
		void CommitEdit();

		void RenderScript();
		void RenderNodes();
		void HandleCreateAction();
		void HandleDeleteAction();
		void ContextMenu();
		void CommandQueue();

		SVector2<F32> GetNodeSize(const SNode* node, const SNodeView* view);
		bool IsPinLinked(U64 id, const std::vector<SLink>& links);
		bool IsPinTypeLiteral(SPin& pin);
		bool DrawLiteralTypePin(SPin& pin);
		void DrawPinIcon(const SPin& pin, bool connected, U8 alpha, bool highlighted);
		SColor GetPinTypeColor(EPinType type);
		SNode* GetNodeFromPinID(U64 id, std::vector<SNode*>& nodes);
		SPin* GetPinFromID(U64 id, SNode* node);
		SPin* GetPinFromID(U64 id, std::vector<SNode*>& nodes);
		SPin* GetOutputPinFromID(U64 id, std::vector<SNode*>& nodes);

		SAssetReference CurrentScriptAssetRef = SAssetReference();
		SScriptAsset* CurrentScriptAsset = nullptr;
		HexRune::SScript* CurrentScript = nullptr;

		HexRune::EPinType CurrentDragPinType = HexRune::EPinType::Unknown;
		SGuiTextFilter Filter;
		SNodeOperation Edit;
		SDataBindingInitData DataBindingCandidate;

		const F32 HeaderHeight = 12.0f;
		const F32 PinNameOffset = 4.0f;

		bool IsHoveringWindow = false;
	};
}
