// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include <GeneralUtilities.h>
#include <MetaCommand/MetaCommandRouter.h>
#include <ECS/Entity.h>

namespace Havtorn
{
	class CEditorManager;
	struct SEditorAssetRepresentation;

	class CEditorDeepLinkParser : public IMetaCommandParser
	{
	public:
		CEditorDeepLinkParser() = delete;
		CEditorDeepLinkParser(CEditorManager* manager);
		~CEditorDeepLinkParser();

		virtual void Parse(const SMetaCommand& command) override;

		std::string GetEntityFocusLink(const SEntity& entity) const;
		std::string GetCameraFocusLink() const;
		std::string GetAssetFocusLink(const SEditorAssetRepresentation* assetRep) const;

	private:
		void ResolveFocusEntity(const SMetaCommand& command);
		void ResolveFocusCameraView(const SMetaCommand& command);
		void ResolveFocusAsset(const SMetaCommand& command);

		CEditorManager* Manager = nullptr;

		U32 EditorNavigationDomain = UGeneralUtils::HashString("EditorNavigation");
		U32 FocusEntityCommand = UGeneralUtils::HashString("FocusEntity");
		U32 FocusCameraViewCommand = UGeneralUtils::HashString("FocusCameraView");
		U32 FocusAssetCommand = UGeneralUtils::HashString("FocusAsset");
	};
}
