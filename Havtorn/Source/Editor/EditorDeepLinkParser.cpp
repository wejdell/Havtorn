// Copyright 2026 Team Havtorn. All Rights Reserved.

#pragma once

#include "EditorDeepLinkParser.h"
#include "EditorManager.h"
#include "Windows/AssetBrowserWindow.h"

#include <CommandLine.h>

#include <ECS/ComponentAlgo.h>

#include <../Game/GameScene.h>

namespace Havtorn
{
    CEditorDeepLinkParser::CEditorDeepLinkParser(CEditorManager* manager)
        : Manager(manager)
    {
        ParserID = UGeneralUtils::HashString("EditorDeepLink");
        UMetaCommandRouter::AddParser(this);
    }

    CEditorDeepLinkParser::~CEditorDeepLinkParser()
    {
        UMetaCommandRouter::RemoveParser(this);
    }

    void CEditorDeepLinkParser::Parse(const SMetaCommand& command)
    {
        if (command.Domain == EditorNavigationDomain)
        {
            if (command.Command == FocusEntityCommand)
                ResolveFocusEntity(command);

            if (command.Command == FocusCameraViewCommand)
                ResolveFocusCameraView(command);

            if (command.Command == FocusAssetCommand)
                ResolveFocusAsset(command);
        }
    }

    std::string CEditorDeepLinkParser::GetEntityFocusLink(const SEntity& entity) const
    {
        CScene* scene = Manager->GetContainingScene(entity);
        if (scene == nullptr)
            return {};

        const Ptr<SEditorAssetRepresentation>& assetRep = Manager->GetAssetRepFromName(scene->SceneName.AsString());
        if (assetRep.get() == nullptr)
            return {};

        // TODO.NW: May want to move these to a deep link parser in editor instead. Might want some feedback confirming the copy to clipboard also
        std::string link;
        link.append(UCommandLine::GetDeepLinkURL());
        link.append("EditorDeepLink/EditorNavigation/FocusEntity/");
        link.append("Entity=");
        link.append(std::to_string(entity.GUID));
        link.append("|Scene=" + UGeneralUtils::ConvertToPlatformAgnosticPath(assetRep->DirectoryEntry.path().string()));

        HV_LOG_TRACE("Copied DeepLink to clipboard: %s", link.c_str());

        return link;
    }

    std::string CEditorDeepLinkParser::GetCameraFocusLink() const
    {
        CScene* scene = Manager->GetCurrentWorkingScene();
        if (scene == nullptr)
            return {};

        const Ptr<SEditorAssetRepresentation>& assetRep = Manager->GetAssetRepFromName(scene->SceneName.AsString());
        if (assetRep.get() == nullptr)
            return {};

        CWorld* world = GEngine::GetWorld();
        SCameraData cameraData = UComponentAlgo::GetCameraData(world->GetMainCamera(), world->GetActiveScenes());
        if (!cameraData.IsValid())
            return {};

        // TODO.NW: May want to move these to a deep link parser in editor instead. Might want some feedback confirming the copy to clipboard also
        std::string link;
        link.append(UCommandLine::GetDeepLinkURL());
        link.append("EditorDeepLink/EditorNavigation/FocusCameraView/");
        link.append("Scene=" + UGeneralUtils::ConvertToPlatformAgnosticPath(assetRep->DirectoryEntry.path().string()));
        link.append("|Transform=");

        const SMatrix cameraTransformMatrix = cameraData.TransformComponent->Transform.GetMatrix();
        link.append(cameraTransformMatrix.ToCommaSeparatedString());
        
        HV_LOG_TRACE("Copied DeepLink to clipboard: %s", link.c_str());

        return link;
    }

    std::string CEditorDeepLinkParser::GetAssetFocusLink(const SEditorAssetRepresentation* assetRep) const
    {
        if (assetRep == nullptr)
            return {};

        // TODO.NW: May want to move these to a deep link parser in editor instead. Might want some feedback confirming the copy to clipboard also
        std::string link;
        link.append(UCommandLine::GetDeepLinkURL());
        link.append("EditorDeepLink/EditorNavigation/FocusAsset/");
        link.append("Asset=" + UGeneralUtils::ConvertToPlatformAgnosticPath(assetRep->DirectoryEntry.path().string()));
        
        HV_LOG_TRACE("Copied DeepLink to clipboard: %s", link.c_str());

        return link;
    }

    void CEditorDeepLinkParser::ResolveFocusEntity(const SMetaCommand& command)
    {
        if (!command.Parameters.contains("Entity"))
            return;

        const SEntity entity = SEntity{ std::stoull(command.Parameters.at("Entity")) };
        if (!entity.IsValid())
        {
            HV_LOG_WARN("Failed to resolve DeepLink: %s\nEntity field was invalid.", command.Name.c_str());
            return;
        }

        CWorld* world = GEngine::GetWorld();
        CScene* scene = UComponentAlgo::GetContainingScene(entity, world->GetActiveScenes());
        if (scene == nullptr)
        {
            // NW: Load scene
            if (!command.Parameters.contains("Scene"))
                return;

            world->ChangeScene<CGameScene>(UFileSystem::GetWorkingPath() + command.Parameters.at("Scene"));

            const std::vector<Ptr<CScene>>& scenes = world->GetActiveScenes();
            if (scenes.empty())
            {
                HV_LOG_WARN("Failed to resolve DeepLink: %s\nScene field was invalid.", command.Name.c_str());
                return;
            }

            scene = world->GetActiveScenes()[0].get();
        }

        if (!scene->HasEntity(entity.GUID))
        {
            HV_LOG_WARN("Failed to resolve DeepLink: %s\nEntity is not contained within linked scene.", command.Name.c_str());
            return;
        }

        Manager->SetCurrentWorkingScene(0);
        Manager->SetSelectedEntity(entity);
        Manager->FocusEntity(entity);
    }

    void CEditorDeepLinkParser::ResolveFocusCameraView(const SMetaCommand& command)
    {
        if (!command.Parameters.contains("Transform") || !command.Parameters.contains("Scene"))
            return;

        CWorld* world = GEngine::GetWorld();
        world->ChangeScene<CGameScene>(UFileSystem::GetWorkingPath() + command.Parameters.at("Scene"));

        const std::vector<Ptr<CScene>>& scenes = world->GetActiveScenes();
        if (scenes.empty())
        {
            HV_LOG_WARN("Failed to resolve DeepLink: %s\nScene field was invalid.", command.Name.c_str());
            return;
        }

        Manager->SetCurrentWorkingScene(0);
        
        SCameraData cameraData = UComponentAlgo::GetCameraData(world->GetMainCamera(), world->GetActiveScenes());
        if (!cameraData.IsValid())
        {
            HV_LOG_WARN("Failed to resolve DeepLink: %s\nCould not resolve main camera entity.", command.Name.c_str());
            return;
        }

        const SMatrix transformMatrix = SMatrix(command.Parameters.at("Transform"));
        cameraData.TransformComponent->Transform.SetMatrix(transformMatrix);
    }

    void CEditorDeepLinkParser::ResolveFocusAsset(const SMetaCommand& command)
    {
        if (!command.Parameters.contains("Asset"))
            return;

        const Ptr<SEditorAssetRepresentation>& assetRep = Manager->GetAssetRepFromDirEntry(std::filesystem::directory_entry(command.Parameters.at("Asset")));
        if (assetRep.get() == nullptr)
        {
            HV_LOG_WARN("Failed to resolve DeepLink: %s\nAsset field was invalid.", command.Name.c_str());
            return;
        }

        Manager->GetEditorWindow<CAssetBrowserWindow>()->BrowseTo(assetRep.get());
    }
}
