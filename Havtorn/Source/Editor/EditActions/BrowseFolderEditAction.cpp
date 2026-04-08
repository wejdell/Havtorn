// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "BrowseFolderEditAction.h"
#include "EditorManager.h"
#include "Windows/AssetBrowserWindow.h"

#include <GeneralUtilities.h>

namespace Havtorn
{
    SMetaCommand SBrowseFolderEditAction::MakeEditActionCommand(const std::filesystem::path& from, const std::filesystem::path& to)
    {
        std::string commandString = "EditHistory/EditorNavigation/BrowseFolder/";
        commandString.append("From=");
        commandString.append(UGeneralUtils::ConvertToPlatformAgnosticPath(from.string()));
        commandString.append("|To=");
        commandString.append(UGeneralUtils::ConvertToPlatformAgnosticPath(to.string()));

        return SMetaCommand(commandString);
    }

    SBrowseFolderEditAction::SBrowseFolderEditAction(const SMetaCommand& command)
        : SEditAction(command)
    {
    }

    void SBrowseFolderEditAction::ResolveAction(CEditorManager* manager, const bool inverted)
    {
        SMetaCommand copy = Command;
        if (inverted)
            std::swap(copy.Parameters.at("From"), copy.Parameters.at("To"));

        manager->GetEditorWindow<CAssetBrowserWindow>()->SetCurrentPath(std::filesystem::path(copy.Parameters.at("To")), false);
    }

    std::string SBrowseFolderEditAction::GetCompactName()
    {
        std::string compactName = "Navigated from '";
        compactName.append(Command.Parameters.at("From"));
        compactName.append("' to '");
        compactName.append(Command.Parameters.at("To"));
        compactName.append("'");
        return compactName;
    }
}
