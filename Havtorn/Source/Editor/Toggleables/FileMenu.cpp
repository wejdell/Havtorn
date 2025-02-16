// Copyright 2022 Team Havtorn. All Rights Reserved.

#include "FileMenu.h"
#include "EditorManager.h"
#include "Engine.h"

#include <Core/imgui.h>
#include <../Game/GameScene.h>

namespace ImGui
{
    CFileMenu::CFileMenu(const char* displayName, Havtorn::CEditorManager* manager)
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
        if (ImGui::Button(Name()))
            ImGui::OpenPopup(PopupName);

        if (ImGui::BeginPopup(PopupName))
        {
            if (ImGui::MenuItem("New")) 
            {
                Havtorn::GEngine::GetWorld()->RemoveScene(0);
                if (Havtorn::GEngine::GetWorld()->GetActiveScenes().size() > 0)
                {
                    Manager->SetCurrentScene(Havtorn::GEngine::GetWorld()->GetActiveScenes()[0].get());
                }
                else
                {
                    Manager->SetCurrentScene(nullptr);
                }
            }

            if (ImGui::BeginMenu("Open"))
            {
                if (ImGui::MenuItem("Demo Scene"))
                {
                    Havtorn::GEngine::GetWorld()->ChangeScene<Havtorn::CGameScene>("Assets/Scenes/DemoScene.hvs");
                    Manager->SetCurrentScene(Havtorn::GEngine::GetWorld()->GetActiveScenes()[0].get());
                }

                if (ImGui::MenuItem("Test Scene"))
                {
                    Havtorn::GEngine::GetWorld()->ChangeScene<Havtorn::CGameScene>("Assets/Scenes/TestScene.hvs");
                    Manager->SetCurrentScene(Havtorn::GEngine::GetWorld()->GetActiveScenes()[0].get());
                }

                //ImGui::MenuItem("fish_hat.c");
                //ImGui::MenuItem("fish_hat.inl");
                //ImGui::MenuItem("fish_hat.h");
                //if (ImGui::BeginMenu("More.."))
                //{
                //    ImGui::MenuItem("Hello");
                //    ImGui::MenuItem("Sailor");
                //    //if (ImGui::BeginMenu("Recurse.."))
                //    //{
                //    //}
                //    ImGui::EndMenu();
                //}
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Clear")) 
            {
                Havtorn::GEngine::GetWorld()->RemoveScene(0);
                if (Havtorn::GEngine::GetWorld()->GetActiveScenes().size() > 0)
                {
                    Manager->SetCurrentScene(Havtorn::GEngine::GetWorld()->GetActiveScenes()[0].get());
                }
                else
                {
                    Manager->SetCurrentScene(nullptr);
                }
            }

            if (ImGui::MenuItem("Generate 3D Demo Scene"))
            {
                Havtorn::GEngine::GetWorld()->OpenDemoScene<Havtorn::CGameScene>(true);
                Manager->SetCurrentScene(Havtorn::GEngine::GetWorld()->GetActiveScenes()[0].get());
            }

            if (ImGui::MenuItem("Generate 2D Demo Scene"))
            {
                Havtorn::GEngine::GetWorld()->OpenDemoScene<Havtorn::CGameScene>(false);
                Manager->SetCurrentScene(Havtorn::GEngine::GetWorld()->GetActiveScenes()[0].get());
            }

            if (ImGui::MenuItem("Save Demo Scene"))
            {
                Havtorn::GEngine::GetWorld()->SaveActiveScene("Assets/Scenes/DemoScene.hvs");
            }

            if (ImGui::MenuItem("Save Test Scene", "Ctrl+S")) 
            {
                Havtorn::GEngine::GetWorld()->SaveActiveScene("Assets/Scenes/TestScene.hvs");
            }

            //if (ImGui::MenuItem("Save As..")) {}

            ImGui::Separator();
            //if (ImGui::BeginMenu("Options"))
            //{
            //    static bool enabled = true;
            //    ImGui::MenuItem("Enabled", "", &enabled);
            //    ImGui::BeginChild("child", ImVec2(0, 60));
            //    for (int i = 0; i < 10; i++)
            //        ImGui::Text("Scrolling Text %d", i);
            //    ImGui::EndChild();
            //    static float f = 0.5f;
            //    static int n = 0;
            //    ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
            //    ImGui::InputFloat("Input", &f, 0.1f);
            //    ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
            //    ImGui::EndMenu();
            //}

            if (ImGui::BeginMenu("Colors"))
            {
                const float sz = ImGui::GetTextLineHeight();
                for (int i = 0; i < ImGuiCol_COUNT; i++)
                {
                    const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
                    ImGui::Dummy(ImVec2(sz, sz));
                    ImGui::SameLine();
                    ImGui::MenuItem(name);
                }
                ImGui::EndMenu();
            }

            // Here we demonstrate appending again to the "Options" menu (which we already created above)
            // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
            // In a real code-base using it would make senses to use this feature from very different code locations.
            //if (ImGui::BeginMenu("Options")) // <-- Append!
            //{
            //    //IMGUI_DEMO_MARKER("Examples/Menu/Append to an existing menu");
            //    static bool b = true;
            //    ImGui::Checkbox("SomeOption", &b);
            //    ImGui::EndMenu();
            //}

            if (ImGui::BeginMenu("Disabled", false)) // Disabled
            {
                IM_ASSERT(0);
            }
            if (ImGui::MenuItem("Checked", nullptr, true)) {}
            if (ImGui::MenuItem("Quit", "Alt+F4")) {}

            ImGui::EndPopup();
        }
    }

    void CFileMenu::OnDisable()
    {
    }
}