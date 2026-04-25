// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "RemoveEntityEditAction.h"

#include "EditorManager.h"
#include "Windows/AssetBrowserWindow.h"

#include <ECS/Entity.h>
#include <ECS/ComponentAlgo.h>

namespace Havtorn
{
    SMetaCommand SRemoveEntityEditAction::MakeEditActionCommand(CEditorManager* manager, const SEntity& entity, const bool removed)
    {
        std::string commandString = "EditHistory/EntityManipulation/ChangeEntity/";

        CScene* owningScene = UComponentAlgo::GetContainingScene(entity, GEngine::GetWorld()->GetActiveScenes());
        const Ptr<SEditorAssetRepresentation>& assetRep = manager->GetAssetRepFromName(owningScene->SceneName.AsString());
        commandString.append("Scene=" + UGeneralUtils::ConvertToPlatformAgnosticPath(assetRep->DirectoryEntry.path().string()));

        commandString.append("|Entity=");
        commandString.append(std::to_string(entity.GUID));
        
        commandString.append("|StateStart=");
        commandString.append(removed ? "Alive" : "Dead");

        commandString.append("|StateEnd=");
        commandString.append(removed ? "Dead" : "Alive");

        commandString.append("|Buffer=");
        commandString.append(owningScene->GetEntityStringBuffer(entity));

        return SMetaCommand(commandString);
    }

    SRemoveEntityEditAction::SRemoveEntityEditAction(const SMetaCommand& command)
        : SEditAction(command, ResolveCompactName(command))
    {
    }

    void SRemoveEntityEditAction::ResolveAction(CEditorManager* /*manager*/, const bool inverted)
    {
        SMetaCommand copy = Command;
        if (inverted)
            std::swap(copy.Parameters.at("StateStart"), copy.Parameters.at("StateEnd"));

        const SEntity entity = SEntity{ std::stoull(copy.Parameters.at("Entity")) };
        if (!entity.IsValid())
            return;

        CScene* scene = UComponentAlgo::GetSceneByPath(copy.Parameters.at("Scene"), GEngine::GetWorld()->GetActiveScenes());
        if (scene == nullptr)
            return;

        const bool shouldRemove = copy.Parameters.at("StateEnd") == "Dead";
        if (shouldRemove)
            scene->RemoveEntity(entity);
        else
            scene->AddEntityFromStringBuffer(copy.Parameters.at("Buffer"));
    }

    std::string SRemoveEntityEditAction::ResolveCompactName(const SMetaCommand& command)
    {
        const SEntity entity = SEntity{ std::stoull(command.Parameters.at("Entity")) };
        if (!entity.IsValid())
            return SEditAction::ResolveCompactName(command);

        const CScene* scene = UComponentAlgo::GetContainingScene(entity, GEngine::GetWorld()->GetActiveScenes());
        if (scene == nullptr)
            return SEditAction::ResolveCompactName(command);

        SMetaDataComponent* component = scene->GetComponent<SMetaDataComponent>(entity);

        std::string compactName = command.Parameters.at("StateEnd") == "Dead" ? "Removed " : "Added ";
        compactName.append("Entity '");
        compactName.append(component->Name.AsString());
        compactName.append("'");
        return compactName;
    }
}
