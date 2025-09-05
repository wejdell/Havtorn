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
            if (GUI::MenuItem("New")) 
            {
                GEngine::GetWorld()->RemoveScene(0);
                GEngine::GetWorld()->CreateScene<CGameScene>();
                if (GEngine::GetWorld()->GetActiveScenes().size() > 0)
                {
                    CScene* activeScene = GEngine::GetWorld()->GetActiveScenes()[0].get();
                    activeScene->Init("New Scene");
                    activeScene->Init3DDefaults();
                    Manager->SetCurrentScene(activeScene);
                }
                else
                {
                    Manager->SetCurrentScene(nullptr);
                }
            }

            if (GEngine::GetWorld()->GetActiveScenes().size() > 0)
            {
                if (GUI::MenuItem("Save Scene"))
                {
                    GEngine::GetWorld()->SaveActiveScene("Assets/Scenes/" + GEngine::GetWorld()->GetActiveScenes()[0]->SceneName.AsString() + ".hva");
                }
            }

            if (GUI::BeginMenu("Open"))
            {
                if (GUI::MenuItem("Demo Scene"))
                {
                    GEngine::GetWorld()->ChangeScene<CGameScene>("Assets/Scenes/DemoScene.hvs");
                    Manager->SetCurrentScene(GEngine::GetWorld()->GetActiveScenes()[0].get());
                }

                if (GUI::MenuItem("Test Scene"))
                {
                    GEngine::GetWorld()->ChangeScene<CGameScene>("Assets/Scenes/TestScene.hvs");
                    Manager->SetCurrentScene(GEngine::GetWorld()->GetActiveScenes()[0].get());
                }

                //GUI::MenuItem("fish_hat.c");
                //GUI::MenuItem("fish_hat.inl");
                //GUI::MenuItem("fish_hat.h");
                //if (GUI::BeginMenu("More.."))
                //{
                //    GUI::MenuItem("Hello");
                //    GUI::MenuItem("Sailor");
                //    //if (GUI::BeginMenu("Recurse.."))
                //    //{
                //    //}
                //    GUI::EndMenu();
                //}
                GUI::EndMenu();
            }

            if (GUI::MenuItem("Clear")) 
            {
                GEngine::GetWorld()->RemoveScene(0);
                if (GEngine::GetWorld()->GetActiveScenes().size() > 0)
                {
                    Manager->SetCurrentScene(GEngine::GetWorld()->GetActiveScenes()[0].get());
                }
                else
                {
                    Manager->SetCurrentScene(nullptr);
                }
            }

            if (GUI::MenuItem("Generate 3D Demo Scene"))
            {
                GEngine::GetWorld()->OpenDemoScene<CGameScene>(true);
                Manager->SetCurrentScene(GEngine::GetWorld()->GetActiveScenes()[0].get());
            }

            if (GUI::MenuItem("Generate 2D Demo Scene"))
            {
                GEngine::GetWorld()->OpenDemoScene<CGameScene>(false);
                Manager->SetCurrentScene(GEngine::GetWorld()->GetActiveScenes()[0].get());
            }

            if (GUI::MenuItem("Save Demo Scene"))
            {
                GEngine::GetWorld()->SaveActiveScene("Assets/Scenes/DemoScene.hvs");
            }

            if (GUI::MenuItem("Save Test Scene", "Ctrl+S")) 
            {
                GEngine::GetWorld()->SaveActiveScene("Assets/Scenes/TestScene.hvs");
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