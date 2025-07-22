// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "FileMenu.h"
#include "EditorManager.h"

#include <Engine.h>
#include <../Game/GameScene.h>
#include <GUI.h>
#include <Log.h>

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
                    activeScene->Init(Manager->GetRenderManager(), "New Scene");
                    activeScene->Init3DDefaults(Manager->GetRenderManager());
                    Manager->SetCurrentScene(activeScene);
                }
                else
                {
                    Manager->SetCurrentScene(nullptr);
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

            //if (GUI::MenuItem("Save As..")) {}

            GUI::Separator();
            //if (GUI::BeginMenu("Options"))
            //{
            //    static bool enabled = true;
            //    GUI::MenuItem("Enabled", "", &enabled);
            //    GUI::BeginChild("child", SVector2<F32>(0, 60));
            //    for (int i = 0; i < 10; i++)
            //        GUI::Text("Scrolling Text %d", i);
            //    GUI::EndChild();
            //    static float f = 0.5f;
            //    static int n = 0;
            //    GUI::SliderFloat("Value", &f, 0.0f, 1.0f);
            //    GUI::InputFloat("Input", &f, 0.1f);
            //    GUI::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
            //    GUI::EndMenu();
            //}

            // Here we demonstrate appending again to the "Options" menu (which we already created above)
            // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
            // In a real code-base using it would make senses to use this feature from very different code locations.
            //if (GUI::BeginMenu("Options")) // <-- Append!
            //{
            //    //IMGUI_DEMO_MARKER("Examples/Menu/Append to an existing menu");
            //    static bool b = true;
            //    GUI::Checkbox("SomeOption", &b);
            //    GUI::EndMenu();
            //}

            if (GUI::BeginMenu("Disabled", false)) // Disabled
            {
                HV_ASSERT(false, "Menu disabled");
            }
            if (GUI::MenuItem("Checked", nullptr, true)) {}
            if (GUI::MenuItem("Quit", "Alt+F4")) {}

            GUI::EndPopup();
        }
    }

    void CFileMenu::OnDisable()
    {
    }
}