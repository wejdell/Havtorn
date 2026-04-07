// Copyright 2026 Team Havtorn. All Rights Reserved.

#include "MoveTransformEditAction.h"

#include "EditorManager.h"
#include "Windows/AssetBrowserWindow.h"

#include <ECS/Components/TransformComponent.h>
#include <ECS/ComponentAlgo.h>

namespace Havtorn
{
    SMetaCommand SMoveTransformEditAction::MakeEditActionCommand(CEditorManager* manager, STransformComponent* transformComp, SMatrix fullDeltaMatrix)
    {
        std::string commandString = "EditHistory/EntityManipulation/ChangeComponent/";

        const Ptr<SEditorAssetRepresentation>& assetRep = manager->GetAssetRepFromName(UComponentAlgo::GetContainingScene(transformComp->Owner, GEngine::GetWorld()->GetActiveScenes())->SceneName.AsString());
        commandString.append("Scene=" + UGeneralUtils::ConvertToPlatformAgnosticPath(assetRep->DirectoryEntry.path().string()));

        commandString.append("|Entity=");
        commandString.append(std::to_string(transformComp->Owner.GUID));
        commandString.append("|TransformStart=");

        const SMatrix newMatrix = transformComp->Transform.GetMatrix();
        const SMatrix oldMatrix = newMatrix * fullDeltaMatrix.FastInverse();
        commandString.append(oldMatrix.ToCommaSeparatedString());
        commandString.append("|TransformEnd=");
        commandString.append(newMatrix.ToCommaSeparatedString());

        return SMetaCommand(commandString);
    }

    SMoveTransformEditAction::SMoveTransformEditAction(const SMetaCommand& command)
        : SEditAction(command)
    {
    }

    void SMoveTransformEditAction::ResolveAction(CEditorManager* /*manager*/, const bool inverted)
    {
        SMetaCommand copy = Command;
        if (inverted)
            std::swap(copy.Parameters.at("TransformStart"), copy.Parameters.at("TransformEnd"));

        const SEntity entity = SEntity{ std::stoull(copy.Parameters.at("Entity")) };
        if (!entity.IsValid())
            return;

        // TODO.NW: We should not fail to access the scene, as if this action is resolved, 
        // any other action to unload or clear the scene should have been acted upon before we got to this point
        // Should be able to remove scene param from action in that case. These other actions are not implemented yet though.
        const CScene* scene = UComponentAlgo::GetContainingScene(entity, GEngine::GetWorld()->GetActiveScenes());
        if (scene == nullptr)
            return;

        STransformComponent* component = scene->GetComponent<STransformComponent>(entity);
        component->Transform.SetMatrix(SMatrix(copy.Parameters.at("TransformEnd")));
    }

    std::string SMoveTransformEditAction::GetCompactName()
    {
        return SEditAction::GetCompactName();
    }
}
