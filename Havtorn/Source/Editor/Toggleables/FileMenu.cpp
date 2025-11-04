// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "FileMenu.h"
#include "EditorManager.h"

#include <Engine.h>
#include <Assets/AssetRegistry.h>
#include <../Game/GameScene.h>
#include <GUI.h>
#include <Log.h>

#include <FileSystem.h>

namespace Havtorn
{
    CFileMenu::CFileMenu(const char* displayName, CEditorManager* manager)
        : CToggleable(displayName, manager)
    {
    }

    CFileMenu::~CFileMenu()
    {
    }

    void CFileMenu::OnEnable()
    {
    }

    void CFileMenu::OnInspectorGUI()
    {
        if (GUI::Button(Name()))
            GUI::OpenPopup(PopupName);

        if (GUI::BeginPopup(PopupName))
        {
            if (GUI::MenuItem("New Scene")) 
            {
                GEngine::GetWorld()->ClearScenes();
                GEngine::GetWorld()->CreateScene<CGameScene>();
                CScene* newScene = Manager->GetScenes().back().get();
                newScene->Init("New Scene");
                newScene->Init3DDefaults();
                Manager->SetCurrentWorkingScene(0);
            }

            if (GUI::MenuItem("Save Current Scene", 0, false, !GEngine::GetWorld()->GetActiveScenes().empty() && Manager->GetCurrentWorkingScene() != nullptr))
            {
                CScene* currentScene = Manager->GetCurrentWorkingScene();
                if (currentScene != nullptr)
                    GEngine::GetWorld()->SaveActiveScene("Assets/Scenes/" + currentScene->SceneName.AsString() + ".hva", currentScene);     
            }

            if (GUI::MenuItem("Clear Scenes")) 
            {
                GEngine::GetWorld()->ClearScenes();
                Manager->SetCurrentWorkingScene(-1);
            }

            if (GUI::MenuItem("Generate 3D Demo Scene"))
            {
                GEngine::GetWorld()->OpenDemoScene<CGameScene>(true);
                Manager->SetCurrentWorkingScene(0);
            }

            if (GUI::MenuItem("Generate 2D Demo Scene"))
            {
                GEngine::GetWorld()->OpenDemoScene<CGameScene>(false);
                Manager->SetCurrentWorkingScene(0);
            }

            GUI::Separator();
            
            if (GUI::MenuItem("Fix Up Asset Redirectors"))
            {
                GEngine::GetAssetRegistry()->FixUpAssetRedirectors();
            }

            GUI::EndPopup();
        }
    }

    void CFileMenu::OnDisable()
    {
    }
}
